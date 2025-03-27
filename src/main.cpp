/*
 main.cpp - Application for testing the DeviceUpdater class
 Date: 27.03.2025
 Created by: Paul Mulligan
*/

#include <iostream>
#include <string>

#include "DeviceUpdater.h"

using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
    cout << "Device Updater application running.." << endl<<endl;

    cout<< "Command line arguments accepted : "<< endl;
    cout<< "--start-update : transition from IDLE state to DOWNLOAD state in order to download the update."<< endl;
    cout<< "--verify       : transition from DOWNLOAD state to VERIFY state if the download was successful, to verify the update file."<< endl;
    cout<< "--apply        : transition from VERIFY state to APPLY state if the verification was successful, to apply the update."<< endl;
    cout<< "--reboot       : transition from APPLY state to REBOOT state"<< endl<<endl;

    string update_artifact_file;
    if (argc > 1)
    {
        // If there is a command line argument,
        // this will specify the download artifact file name to use
        update_artifact_file = argv[1];
    }

    DeviceUpdater updater(update_artifact_file);

    return 0;
}
