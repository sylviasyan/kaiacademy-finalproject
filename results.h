#pragma once

#define _SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING

#include <string>
#include <boost/uuid/uuid.hpp>

struct Result {
	Result(const boost::uuids::uuid& id, //to keep track of each result
		std::string contents, 
		bool success); //bool to signal if tasks was successful or not
	const boost::uuids::uuid id;
	const std::string contents;
	const bool success;
};