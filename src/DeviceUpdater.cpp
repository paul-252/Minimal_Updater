/*
 DeviceUpdater.cpp - Implementation of the DeviceUpdater class
 Date: 27.03.2025
 Created by: Paul Mulligan
*/

#include <iostream>
#include <fstream>
#include <iterator>
#include <mutex>
#include <thread>

#include "DeviceUpdater.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;
using std::thread;
using std::unique_lock;
using std::condition_variable;
using std::ifstream;

/**
 * DeviceUpdater::DeviceUpdater(string file_name)
 *
 * DeviceUpdater constructor with artifact file name as parameter
 *
 */
DeviceUpdater::DeviceUpdater(string file_name) : state(IDLE),
                                                 download_available(false),
                                                 to_verify(false),
                                                 to_apply(false),
                                                 to_reboot(false),
                                                 artifact_file_name(file_name)

{
    if (artifact_file_name == "")
    {
        artifact_file_name = "update_artifact_good.bin";
        cout<<"artifact_file_name was not assigned. Using update_artifact_good.bin ."<<endl;;
    }

    t_console = thread(&DeviceUpdater::CommandInterface, this); //create separate thread and start function CommandInterface()
    t_state_machine = thread(&DeviceUpdater::StateMachine, this); //create separate thread and start function StateMachine()
}

/**
 * DeviceUpdater::~DeviceUpdater()
 *
 * DeviceUpdater destructor.
 * It waits for the threads to complete.
 *
 */
DeviceUpdater::~DeviceUpdater()
{
    //Join threads
    if (t_state_machine.joinable())
    {
        t_state_machine.join();
    }

    if (t_console.joinable())
    {
        t_console.join();
    }
}
/**
 * DeviceUpdater::DownloadArtifact()
 *
 * This downloads an update artifact file
 * from the path specified in artifact_file_name
 * and checks if the file has contents.
 *
 * @return true if download was successful, false if not.
 *
 */
bool DeviceUpdater::DownloadArtifact()
{
    bool download_ok = false;
    ifstream inputFile;

    inputFile.open(artifact_file_name.c_str(), std::ifstream::binary);
    cout<<"Fetching "<<artifact_file_name<<endl;
    if(inputFile.is_open())
    {
        cout<<artifact_file_name<<" opened"<<endl;
        // get length of file:
        inputFile.seekg (0, inputFile.end);
        int file_size = inputFile.tellg();
        inputFile.seekg (0, inputFile.beg);
        inputFile.close();
        // Simply check that the file is not empty.
        // In reality there would be a minimum file size to check for.
        if (file_size > 0)
        {
            cout<<"Update artifact downloaded. File size is "<<file_size<<" bytes."<<endl;
            download_ok = true;
        }
        else
        {
            cout<<"Error! Update artifact is empty.";
        }
    }

    return download_ok;
}

/**
 * DeviceUpdater::VerifyChecksum()
 *
 * This is just a very simple implementation used for demo purposes which simulates a checksum
 * verification. It works on the assumption that the last byte written to the file is the sum of
 * all the other bytes before it. Simply add all the payload bytes together and check if the sum
 * is equal to the last byte (checksum). If not, the file must be corrupted in some way.
 *
 * In reality, the checksum verification could involve passing the complete contents of the file
 * through a hashing function to generate the checksum. This checksum would be then compared to the
 * checksum provided separately by the update vendor
 * (from reading a different file or from a HTTP header etc.)
 *
 * Furthermore, the digital signature of the file would need to be verified around here to verify the
 * authenticity of the file. (Did this image actually come from Mender etc.?) Out of scope for here
 *
 * @return true if checksum verification is successful, false if not.
 *
 */

bool DeviceUpdater::VerifyChecksum(uint8_t checksum, vector<uint8_t>& fileBytes)
{
    bool checkSumVerified = false;

    if (!fileBytes.empty())
    {
        uint8_t calulated_sum = 0;
        for (unsigned int i = 0; i < fileBytes.size(); ++i)
        {
            calulated_sum += (uint8_t)fileBytes.at(i);
        }

        if (calulated_sum ==  checksum)
        {
            checkSumVerified = true;
        }
    }

    return checkSumVerified;
}

/**
 * DeviceUpdater::VerifyDownload()
 *
 * Open the file, copy its contents to a vector and
 * pass to other functions to validate its integrity
 *
 * @return true if verification is successful, false if not.
 *
 */
bool DeviceUpdater::VerifyDownload()
{
    bool verification_ok = false;
    ifstream input(artifact_file_name.c_str(), std::ios::binary);

    vector<uint8_t> fileBytes
    (
         (std::istreambuf_iterator<char>(input)),
         (std::istreambuf_iterator<char>())
    );

    input.close();

    if (!fileBytes.empty())
    {
        uint8_t checksum = (uint8_t)fileBytes.back();
        fileBytes.pop_back();
        verification_ok = VerifyChecksum(checksum, fileBytes);
    }

    return verification_ok;
}

/**
 * DeviceUpdater::ApplyUpdate()
 *
 * The function which should write the update into a Flash partition.
 * In this case, it's a dummy function which simply waits for one second
 * and returns true;
 *
 * @return true if update was written successfully, false if not.
 */
bool DeviceUpdater::ApplyUpdate()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return true;
}

/**
 * DeviceUpdater::CommandInterface()
 *
 * This function executes in a separate thread
 * and reads inputs from the user via the command line
 */
void DeviceUpdater::CommandInterface()
{
    std::string input;
    unique_lock<mutex> lock(mtx, std::defer_lock);

    while(true)
    {
        // Wait for input
        cin>>input;

        if (input == "--start-update")
        {
            lock.lock();
            download_available = true;
            cond_var.notify_one();
            lock.unlock();
            cout<<"Download should start soon.."<<endl;
        }
        else if (input == "--verify")
        {

            lock.lock();
            to_verify = true;
            cond_var.notify_one();
            cout<<"Verification should start.."<<endl;
            lock.unlock();
        }
        else if (input == "--apply")
        {
            lock.lock();
            to_apply = true;
            cond_var.notify_one();
            cout<<"Update will be applied.."<<endl;
            lock.unlock();
        }
        else if (input == "--reboot")
        {
            lock.lock();
            to_reboot = true;
            cond_var.notify_one();
            cout<<"Request to reboot.."<<endl;
            lock.unlock();
        }

        cin.clear();
        fflush(stdin);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

/**
 * StateMachine
 *
 * This function executes in a separate thread
 * and controls the switching between the upgrade states
 */
void DeviceUpdater::StateMachine()
{
    unique_lock<mutex> lock(mtx, std::defer_lock ); //Construct a lock object, but don't lock just yet.
    auto timeout_sec = std::chrono::seconds(1); // One second timeout
    const int num_seconds = 60;

    while(true)
    {
        bool downloaded = false;
        bool verified = false;
        bool applied = false;

        switch (state)
        {
            case IDLE:
                cout<<"IDLE.."<<endl;
                // Let's wait and block the thread when in this state until there is a update download available
                lock.lock();
                cond_var.wait(lock, [&]{return this->download_available;});
                download_available = false;
                state = DOWNLOAD;
                lock.unlock();
                break;

            case DOWNLOAD:
                cout<<"DOWNLOAD.."<<endl;

                // Download the update artifact. true if download was successful, false if something went wrong
                downloaded = DownloadArtifact();

                if (downloaded == true)
                {
                    cout<<"Ready to verify."<<endl;
                    // It would probably make the most sense to automatically go to the VERIFY state from here if the download appears successful.
                    // In this case however, we'll block and wait for the command-line input to go there.
                    lock.lock();

                    // Wait for to_verify to be set to true, or else the timeout

                    cond_var.wait_for(lock, timeout_sec * num_seconds, [&]{return this->to_verify;});

                    if (to_verify == true)
                    {
                        to_verify = false;
                        state = VERIFY;
                    }
                    else
                    {
                        cout<<"Time elapsed waiting for command line input. Restarting update procedure."<<endl;
                        state = IDLE;
                    }

                    lock.unlock();
                    break;

                }
                else
                {
                     // A problem downloading the artifact. It would make sense to propagate the exact error back to the caller and / or
                     // perhaps try to download it again a number of times before giving up.
                     // In this case for simplicity we'll go straight back to the IDLE state.
                     cout<<"Error downloading update artifact."<<endl;
                     state = IDLE;
                }

                break;

            case VERIFY:
                cout<<"VERIFY.."<<endl;

                verified = VerifyDownload();

                if (verified == true)
                {
                    cout<<"Update artifact was successfully verified. Ready to apply."<<endl;

                    lock.lock();
                    // Wait for to_apply to be set to true, or else the timeout
                    cond_var.wait_for(lock, timeout_sec * num_seconds, [&]{return this->to_apply;});

                    if (to_apply == true)
                    {
                        to_apply = false;
                        state = APPLY;
                    }
                    else
                    {
                        cout<<"Time elapsed waiting for command line input. Restarting update procedure."<<endl;
                        state = IDLE;
                    }

                    lock.unlock();
                    break;

                }
                else
                {
                     cout<<"Error during verification phase of update artifact."<<endl;
                     state = IDLE;
                }

                break;

            case APPLY:
                cout<<"Applying update.."<<endl;

                applied = ApplyUpdate();

                if (applied == true)
                {
                    cout<<"Update applied. Reboot the device for it to take effect."<<endl;

                    lock.lock();

                    // Wait for to_reboot to be set to true, or else the timeout
                    cond_var.wait_for(lock, timeout_sec * num_seconds, [&]{return this->to_reboot;});

                    if (to_reboot == true)
                    {
                        to_reboot = false;
                        state = REBOOT;
                    }
                    else
                    {
                        cout<<"Time elapsed waiting for command line input. Restarting update procedure."<<endl;
                        state = IDLE;
                    }

                    lock.unlock();
                }
                else
                {
                     cout<<"Error! Update failed"<<endl;
                     state = IDLE;
                }

                break;

             case REBOOT:
                cout<<"About to reboot device..."<<endl;
                state = IDLE;
                break;

             default:
                break;
        }

        // Sleep for 2 seconds
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}
