/**
 * @file File.h
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Configure and control Flash memory over SPI.
**/

#ifndef _file_H_INCLUDED
#define _file_H_INCLUDED

#include "esp_log.h"
#include "esp_err.h"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_spiffs.h"

#include <string>
#include <vector>

namespace spiffsControl{
    constexpr int buffer_size = 256;

    /**
     * @brief Flash storage core driver:
     * @note - mount FAT flash storage
     * @note - browse file directory
     * @note - create, modify, and remove files
     */
    class spiffs {
    public:
        /**
         * @brief Construct a new File Core object
         * @note constructor calls mount() to access file storage
         */
        spiffs();
        ~spiffs();

        /**
         * @brief mounts the SPIFFS flash storage device over SPI
         * 
         */
        void mount();

        /**
         * @brief Checks the SPIFFS partition
         * 
         */
        void checkSPIFFS();

        /**
         * @brief Overwrites with a blank file
         * 
         * @param path file location
         */
        void clearLog(const char *path);

        /**
         * @brief Adds a line to the end of a file
         * 
         * @param path file location
         * @param message data to append
         */
        void addLine(const char *path, const char *message);

        /**
         * @brief Reads a line from the file in consecutive order.
         * Starting from the first line. Call again to read the next line
         * 
         * @param path file location
         * @param string_out string to write line data to
         * @return int - if -1, there are no more lines to read, else returns
         * the number of the line returned.
         */
        int readLine(const char *path, std::string *string_out);

    private:
        esp_vfs_spiffs_conf_t conf; //config data for SPIFFS
        int line_number; //number of line last read by readLine()
        FILE *open_file; //File object being read by readLine()
    };
}

#endif // _file_H_INCLUDED