/**
 * @file File.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Configure and control Flash memory over SPI.
 * @note Sourced from: https://www.tutorialspoint.com/esp32_for_iot/esp32_for_iot_spiffs_storage.htm
 * @note SPIFF reference: https://github.com/espressif/arduino-esp32/tree/master/libraries/SPIFFS
 * @note Fs reference: https://github.com/espressif/arduino-esp32/tree/master/libraries/FS
**/

#ifndef _file_H_INCLUDED
#define _file_H_INCLUDED

#include "esp_log.h"
#include "esp_err.h"

#include "Fs.h"
#include "SPIFFS.h"
#include <string>
#include <vector>

using namespace std;

#define FORMAT_SPIFFS_IF_FAILED true // Formats flash storage if it cannot be mounted

/**
 * @brief Flash storage core driver:
 * @note - mount FAT flash storage
 * @note - browse file directory
 * @note - create, modify, and remove files
 */
class FileCore {
public:
    /**
     * @brief Construct a new File Core object
     * @note constructor calls mount() to access file storage
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
     * @param dirname root folder to read
     * @param levels number of levels under root folder to read
     */
    void listDir(const char * dirname, uint8_t levels);

    /**
     * @brief displays a file's contents in the Serial Monitor
     * 
     * @param path file's location
     */
    void readFile(const char * path);

    std::vector<String> loadFile(const char *path);

    int read_file(std::string *string_out);

    void select_file(const char *path);

    void deselect_file();

    /**
     * @brief creates a file
     * 
     * @param path file's location
     * @param message data to be written on creation
     */
    esp_err_t writeFile(const char * path, const char * message);

    /**
     * @brief appends data to the end of an existing file
     * 
     * @param path file's location
     * @param message data to be appended
     */
    void appendFile(const char * path, const char * message);

    /**
     * @brief redefine's a file's path and name
     * 
     * @param path1 file's original location and name
     * @param path2 file's modified location and name
     */
    void renameFile(const char * path1, const char * path2);

    /**
     * @brief removes a file
     * 
     * @param path file to be removed
     */
    esp_err_t deleteFile(const char * path);

    /**
     * @brief benchmark to measure read and write time for a 1MB file
     * 
     * @param path location of benchmark file
     * @return time in milliseconds
     */
    uint32_t testFileIO(const char * path);

private:
    File open_file;
    uint8_t line;
};

#endif // _file_H_INCLUDED