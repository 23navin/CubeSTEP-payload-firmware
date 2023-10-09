/**
 * @file File.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of FileCore class
**/

#include "File.h"
static const char* TAG = "FileCore";


FileCore::FileCore(){
    mount();
}

FileCore::~FileCore(){
} //should be unused

void FileCore::mount(){
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        ESP_LOGE(TAG, "SPIFFS Mount Failed");
        return;
    }

    ESP_LOGI(TAG, "SPIFFS Mount Successfull");
    return;
}

void FileCore::listDir(const char * dirname, uint8_t levels){
    ESP_LOGD(TAG, "Listing directory: %s\r", dirname);

    File root = SPIFFS.open(dirname);
    if(!root){
        ESP_LOGW(TAG, "%s - failed to open directory", dirname);
        return;
    }
    if(!root.isDirectory()){
        ESP_LOGW(TAG, "%s - not a directory", dirname);
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            ESP_LOGD(TAG, "  DIR : %s", file.name());
            if(levels){
                listDir(file.path(), levels -1);
            }
        }else{
            ESP_LOGD(TAG, "  FILE : %s\t SIZE: %i", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

void FileCore::readFile(const char * path){
    ESP_LOGD(TAG, "Reading file: %s\r", path);

    File file = SPIFFS.open(path);
    if(!file || file.isDirectory()){
        ESP_LOGW(TAG, "%s - failed to open file for reading", path);
        return;
    }

    ESP_LOGD(TAG, "- read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

std::vector<String> FileCore::loadFile(const char *path){
    ESP_LOGD(TAG, "Reading file: %s\r", path);

    File file = SPIFFS.open(path);
    if(!file || file.isDirectory()){
        ESP_LOGW(TAG, "%s - failed to open file for reading", path);
        //return;
    }

    vector<String> lines;
    String buffer;

    while(file.available()){
        lines.push_back(file.readStringUntil('\n'));
    }
    
    file.close();
    
    return lines;
}

void FileCore::select_file(const char *path){
    ESP_LOGI(TAG, "Opening File for reading");
    open_file = SPIFFS.open(path);
    if(!open_file || open_file.isDirectory()){
        ESP_LOGW(TAG, "%s - failed to open file for reading", path);
    }
    line = 0;
}

void FileCore::deselect_file(){
    ESP_LOGI(TAG, "Closing file");
    open_file.close();
}

int FileCore::read_file(std::string *string_out){
    if(open_file.available()) {
        ESP_LOGD(TAG, "Reading Line %i", line);
        String buffer;
        buffer = open_file.readStringUntil('\n');
        *string_out = std::string(buffer.c_str());
        line++;

        return line;
    }

    return -1;
}

esp_err_t FileCore::writeFile(const char * path, const char * message){
    ESP_LOGD(TAG, "Writing file: %s\r", path);

    File file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return ESP_FAIL;
    }
    if(file.print(message)){
        ESP_LOGD(TAG, "- file written");
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "- write failed");
        return ESP_FAIL;
    }
    file.close();
}

void FileCore::appendFile(const char * path, const char * message){
    ESP_LOGV(TAG, "Appending to file: %s\r", path);

    File file = SPIFFS.open(path, FILE_APPEND);
    if(!file){
        ESP_LOGW(TAG, "%s - failed to open file for appending", path);
        return;
    }
    if(file.print(message)){
        ESP_LOGV(TAG, "- message appended");
    } else {
        ESP_LOGW(TAG, "- append failed");
    }
    file.close();
}

void FileCore::renameFile(const char * path1, const char * path2){
    ESP_LOGD(TAG, "Renaming file %s to %s\r", path1, path2);
    if (SPIFFS.rename(path1, path2)){
        ESP_LOGD(TAG, "- file %s renamed to %s", path1, path2);
    } else {
        ESP_LOGW(TAG, "- %s rename failed", path1);
    }
}

esp_err_t FileCore::deleteFile(const char * path){
    ESP_LOGD(TAG, "Deleting file: %s\r", path);
    if(SPIFFS.remove(path)){
        ESP_LOGV(TAG, "- file %s deleted", path);
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "- %s delete failed", path);
        return ESP_FAIL;
    }
}

uint32_t FileCore::testFileIO(const char * path){
    ESP_LOGI(TAG, "Starting File test");
    static uint8_t buf[512];
    size_t len = 0;
    File file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        return -1;
    }

    size_t i;
    ESP_LOGD(TAG, "- writing" );
    uint32_t start = millis();
    uint32_t end;
    for(i=0; i<2048; i++){
        // if ((i & 0x001F) == 0x001F){
        // Serial.print(".");
        // }
        file.write(buf, 512);
    }
    // Serial.println("");
    file.close();

    file = SPIFFS.open(path);
    i = 0;
    if(file && !file.isDirectory()){
        len = file.size();
        size_t flen = len;
        ESP_LOGD(TAG, "- reading" );
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            // if ((i++ & 0x001F) == 0x001F){
            // Serial.print(".");
            // }
            len -= toRead;
        }
        // Serial.println("");
        end = millis() - start;
        file.close();

        SPIFFS.remove(path);
        return end;
    } else {
        return -1;     
    }

    return -1;
}