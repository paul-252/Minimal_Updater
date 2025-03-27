# Minimal Updater by Paul Mulligan


## Description

A C++ application implementing a state machine which simulates a basic device updater workflow.

## Details

The state machine switches between five different states : IDLE,  DOWNLOAD,  VERIFY,  APPLY and REBOOT : 

1. IDLE : No update in progress. It simply waits for the "--start-update" command from the user via the command line.
2. DOWNLOAD : It tries to open a download "artifact" file and check its size. If the download was successful, it then waits for the command "--verify" to go to the VERIFY state, or else times out and goes back to IDLE. If the download was unsuccessful, it goes directly back to the IDLE state. 
3. VERIFY : It checks the integrity of the downloded file by performing a checksum on it. (A very simple calculation in this case for demo purposes). If verification was successful, it then waits for the command "--apply" to go to the APPLY state, or else times out and goes back to IDLE. If the verification was unsuccessful, it goes directly back to the IDLE state. 
4. APPLY : Apply the update. This simply pretends to apply the update by sleeping for one second. It then waits for the command "--reboot" to go to the REBOOT state, or else times out and goes back to the IDLE state. 
5. REBOOT : It prints of a "rebooting" message and returns to the IDLE state. 

The application has two separate threads running. One which reads the user commands via the command line, and another which controls the state machine. 

There is a state timeout of 60 seconds hard-coded. If in states DOWNLOAD, VERIFY or APPLY and an expected command has not been received within this time frame, it goes back to IDLE state.

### Dependencies

This application builds and runs on a Linux system. 

This has so far been tested on Ubuntu 22.04. 

To build and run the application, it's necessary to have _build essential_ installed which contains libraries and tools for building C++ applications. 

Install on Debian or Ubuntu as follows : 

```
sudo apt-get install build-essential

```

### Installing

Clone the repository anywhere on your system using SSH or HTTPs. Go to the "<>Code" dropdown menu at the top right of the project for the options. Or else download the project zip file and extract it.


### Building the application

Build the application by simply going into the project folder and running make :

```
cd Minimal_Updater

make

```
Clean the application by running :

```

make clean

```

### Executing program

If the project built successfuly, there will be an executable file called _main.exe_ generated in the root of the project folder. 

Execute this to run the application. Pass an optional file name to the application which will be used as the update artifact file.

There are two update artifact binary files provided for testing. 

_update_artifact_good.bin_ is a non-corrupted file and should pass the verification phase. 

_update_artifact_bad.bin_ is a corrupted file and should not pass the verification phase. 

(Non-corrupted means that simply that the last byte in the filem, which is the checksum, should be a sum of the previous bytes).

Command examples : 

```
./main.exe update_artifact_good.bin

./main.exe update_artifact_bad.bin

./main.exe # No arguments provided - update_artifact_good.bin will be used in this case by default
```

## Author

Paul Mulligan

mulligan252@yahoo.ie
