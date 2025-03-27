/*
   DeviceUpdater.h - Header file of the DeviceUpdater class
   Date: 27.03.2025
   Created by: Paul Mulligan
*/

#ifndef DEVICEUPDATER_H
#define DEVICEUPDATER_H

#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

using std::string;
using std::thread;
using std::mutex;
using std::vector;
using std::condition_variable;

class DeviceUpdater
{
    public:
        DeviceUpdater(string file_name);
        virtual ~DeviceUpdater();

    private:
        void StateMachine();
        void CommandInterface();
        bool DownloadArtifact();
        bool VerifyDownload();
        bool ApplyUpdate();
        bool VerifyChecksum(uint8_t checksum, vector<uint8_t>& fileBytes);

    enum States
    {
        IDLE,
        DOWNLOAD,
        VERIFY,
        APPLY,
        REBOOT
    };

    States state;

    // Threads
    thread t_state_machine;
    thread t_console;

    // Inter-thread synchronization variables
    mutex mtx;
    condition_variable cond_var;
    bool download_available;
    bool to_verify;
    bool to_apply;
    bool to_reboot;

    string artifact_file_name;
};

#endif // DEVICEUPDATER_H
