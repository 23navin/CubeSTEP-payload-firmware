# CubeSTEP

CubeSat Technology Exploration Project

California State Polytechnic University, Pomona

# Payload Processor Firmware

The firmware for the Payload Processor was developed using the ESP-IDF extension for VS Code to build and upload the code to the ESP32 microprocessor. The ESP32-Pico-D4 development kit was used to test and debug the firmware while the Payload component is being developed.

To Do

* Rework logging to be consistent
* Clean up I2C calls
* Write I2C linux driver
* Add system checks
* Add api error checks
* Calibrate ADC

## [C++ Driver](https://github.com/23navin/CubeSTEP-Payload-Driver/tree/main)

A C++ driver is provided to allow the OBC to effectively communicate with the payload processor over I2C. The repository also includes a Raspbery Pi port for testing purposes.

## [OneNote Link](https://livecsupomona-my.sharepoint.com/:o:/g/personal/bnjames_cpp_edu/Ev9EsCTfl25Cn0ulAFYuXMIBKXObV0LOeldq7t2hZRmNNg)

If you are CPP-affiliated and are working on CubeSTEP, you have access to these notes.

If you have any questions, contact Benjamin James
