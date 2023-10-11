/**
 * @file File.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of FileCore class
**/

#include "fileControl.h"
static const char* TAG = "FileCore";


FileCore::FileCore(){
    mount();
    checkSPIFFS();

    line_number = 0;
}

FileCore::~FileCore(){
    esp_vfs_spiffs_unregister(conf.partition_label);
}

void FileCore::mount(){
    ESP_LOGI(TAG, "Initializing SPIFFS");

    conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
}

void FileCore::checkSPIFFS(){
    size_t total = 0, used = 0;
    esp_err_t ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Check consistency of reported partiton size info.
    if (used > total) {
        ESP_LOGW(TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(conf.partition_label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return;
        } else {
            ESP_LOGI(TAG, "SPIFFS_check() successful");
        }
    }
}

void FileCore::clearLog(const char *path){
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    ESP_LOGI(TAG, "%s - File cleared", path);
}

void FileCore::addLine(const char *path, const char *message){
    //open file
    FILE *file = fopen(path, "a");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    //add line
    fprintf(file, message);

    //close line
    fclose(file);
    ESP_LOGI(TAG, "Line written");
}

int FileCore::readLine(const char *path, std::string *string_out){
    //open file if first request
    if(line_number == 0){
        open_file = fopen(path, "r");
        if (open_file == NULL) {
            ESP_LOGE(TAG, "Failed to open file for writing");
            return -1;
        }
    }

    char line_buffer[BUFFER_SIZE];

    // read line
    if(fgets(line_buffer, BUFFER_SIZE, open_file) != NULL){
        // strip newline
        char *pos = strchr(line_buffer, '\n');
        if (pos) {
            *pos = '\0';
        }
        ESP_LOGI(TAG, "Read from file: '%s'", line_buffer);

        //copy line to string_out
        *string_out = std::string(line_buffer);

        //return fn and increment line count
        ESP_LOGI(TAG, "Line read %i", line_number);
        return line_number++;
    }
    else{
        //close file
        fclose(open_file);

        //reset line count
        line_number = 0;

        ESP_LOGW(TAG, "End of File");
        return -1;
    }
}