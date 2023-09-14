# CubeSTEP

CubeSat Technology Exploration Project

California State Polytechnic University, Pomona

# Payload Processor Firmware

The firmware for the Payload Processor was developed using the PlatformIO extension for VS Code to build and upload the code to the ESP32 microprocessor. The ESP32-Pico-D4 development kit was used to test and debug the firmware while the Payload component is being developed.

## [C++ Driver](https://github.com/23navin/CubeSTEP-Payload-Driver/tree/main)

A C++ driver is provided to allow the OBC to effectively communicate with the payload processor over I2C. The repository also includes a Raspbery Pi port for testing purposes.

## [OneNote Link](https://livecsupomona-my.sharepoint.com/:o:/g/personal/bnjames_cpp_edu/Ev9EsCTfl25Cn0ulAFYuXMIBKXObV0LOeldq7t2hZRmNNg)

If you are CPP-affiliated and are working on CubeSTEP, you have access to these notes.

If you have any questions, contact Benjamin James



## How to use

Build with g++ with the `-lwiringPi` parameter. For example:

```
g++ -o i2c_operation i2c_operation.cpp -lwiringPi
```

Run `./i2c_operation` your console with the operation you would like performed with any necessary parameters. All arguments must be in hexadecimal. The first argument should be an opcode (1 byte). If the operation requires a parameter, include it as the second argument. The size of the parameter should not exceed the size listed in the documentation. For example, the below command can be used to set the device's time (opcode 0x93) to epoch 1694565311 (which is equal to 0x650103BF).

```
./i2c_operation 93 650103BF
```

After executing, the command will first perform the following actions in the specified order:

1. Parses opcode from command line argument

2. Sends opcode to the device

3. Wait for confirmation from the device

If there is a parameter, the following will happen:

4. Parses pararmeter from command line argument

5. Sends parameter to the device

If the device is expected to return data, the following will happen

6. Waits for device to send data start byte

7. Receives data from device

The commands are documented in OneNote. Contact Benjamin James.
