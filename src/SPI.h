/**
 * @file SPI.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Configure and control Flash memory over SPI.
 * @note Sourced from: https://www.tutorialspoint.com/esp32_for_iot/esp32_for_iot_spiffs_storage.htm
 * @note SPIFF reference: https://github.com/espressif/arduino-esp32/tree/master/libraries/SPIFFS
 * @note Fs reference: https://github.com/espressif/arduino-esp32/tree/master/libraries/FS
**/

#ifndef _SPI_H_INCLUDED //idk why vscode is having an issue with this. it recognizes the endif and compiles without any problems :|
#define _SPI_H_INCLUDED

#include "Fs.h"
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true //Mount definition
/**
 * Flash storage core driver:
 *  - mount FAT flash storage
 *  - browse file directory
 *  - create, modify, and remove files
 */

class FileCore {
public:
    /* methods*/

    /**
     * constructor
     * 
     * @note constructor calls mount() to access flash storage.
     */
    FileCore();
    ~FileCore();

    /**
     * @brief mounts the FAT flash storage device over SPI
     * 
     */
    void mount();

    /**
     * @brief displays file directory in Serial monitor
     * 
     * @param fs ESP32 File System library
     * @param dirname root folder to read
     * @param levels number of levels under root folder to read
     */
    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

    /**
     * @brief displays a file's contents in the Serial Monitor
     * 
     * @param fs ESP32 File System library
     * @param path file's location
     */
    void readFile(fs::FS &fs, const char * path);

    /**
     * @brief creates a file
     * 
     * @param fs ESP32 File System library
     * @param path file's location
     * @param message data to be written on creation
     */
    void writeFile(fs::FS &fs, const char * path, const char * message);

    /**
     * @brief appends data to the end of an existing file
     * 
     * @param fs ESP32 File System library
     * @param path file's location
     * @param message data to be appended
     */
    void appendFile(fs::FS &fs, const char * path, const char * message);

    /**
     * @brief redefine's a file's path and name
     * 
     * @param fs ESP32 File System library
     * @param path1 file's original location and name
     * @param path2 file's modified location and name
     */
    void renameFile(fs::FS &fs, const char * path1, const char * path2);

    /**
     * @brief removes a file
     * 
     * @param fs ESP32 File System library
     * @param path file to be removed
     */
    void deleteFile(fs::FS &fs, const char * path);

    /**
     * @brief benchmark to measure read and write time for a 1MB file
     * 
     * @param fs ESP32 FIle System library
     * @param path location of benchmark file
     */
    void testFileIO(fs::FS &fs, const char * path);

};
#endif // _SPI_H_INCLUDED


/*
    application demo
    https://www.tutorialspoint.com/esp32_for_iot/esp32_for_iot_spiffs_storage.htm


    Serial.begin(115200);
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    listDir(SPIFFS, "/", 0);
    writeFile(SPIFFS, "/hello.txt", "Hello ");
    appendFile(SPIFFS, "/hello.txt", "World!\r\n");
    readFile(SPIFFS, "/hello.txt");
    renameFile(SPIFFS, "/hello.txt", "/foo.txt");
    readFile(SPIFFS, "/foo.txt");
    deleteFile(SPIFFS, "/foo.txt");
    testFileIO(SPIFFS, "/test.txt");
    deleteFile(SPIFFS, "/test.txt");
    Serial.println( "Test complete" );
*/