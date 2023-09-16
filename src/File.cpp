/**
 * @file File.cpp
 * @author Benjamin Navin (bnjames@cpp.edu)
 * 
 * @brief Implementation of FileCore class
**/

#include "File.h"

FileCore::FileCore(){
    mount();
}

FileCore::~FileCore(){
} //should be unused

void FileCore::mount(){
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        log_e("SPIFFS Mount Failed");
        return;
    }

    log_i("SPIFFS Mount Successfull");
    return;
}

void FileCore::listDir(const char * dirname, uint8_t levels){
    log_d("Listing directory: %s\r", dirname);

    File root = SPIFFS.open(dirname);
    if(!root){
        log_w("%s - failed to open directory", dirname);
        return;
    }
    if(!root.isDirectory()){
        log_w("%s - not a directory", dirname);
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            log_d("  DIR : %s", file.name());
            if(levels){
                listDir(file.path(), levels -1);
            }
        }else{
            log_d("  FILE : %s\t SIZE: %i", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

void FileCore::readFile(const char * path){
    log_d("Reading file: %s\r", path);

    File file = SPIFFS.open(path);
    if(!file || file.isDirectory()){
        log_w("%s - failed to open file for reading", path);
        return;
    }

    log_d("- read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

std::vector<String> FileCore::loadFile(const char *path){
    log_d("Reading file: %s\r", path);

    File file = SPIFFS.open(path);
    if(!file || file.isDirectory()){
        log_w("%s - failed to open file for reading", path);
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

void FileCore::writeFile(const char * path, const char * message){
    log_d("Writing file: %s\r", path);

    File file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        log_d("- file written");
    } else {
        log_w("- write failed");
    }
    file.close();
}

void FileCore::appendFile(const char * path, const char * message){
    log_d("Appending to file: %s\r", path);

    File file = SPIFFS.open(path, FILE_APPEND);
    if(!file){
        log_w("%s - failed to open file for appending", path);
        return;
    }
    if(file.print(message)){
        log_d("- message appended");
    } else {
        log_w("- append failed");
    }
    file.close();
}

void FileCore::renameFile(const char * path1, const char * path2){
    log_d("Renaming file %s to %s\r", path1, path2);
    if (SPIFFS.rename(path1, path2)){
        log_d("- file %s renamed to %s", path1, path2);
    } else {
        log_w("- %s rename failed", path1);
    }
}

void FileCore::deleteFile(const char * path){
    log_d("Deleting file: %s\r", path);
    if(SPIFFS.remove(path)){
        log_d("- file %s deleted", path);
    } else {
        log_w("- %s delete failed", path);
    }
}

uint32_t FileCore::testFileIO(const char * path){
    log_d("Starting File test");
    static uint8_t buf[512];
    size_t len = 0;
    File file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        return -1;
    }

    size_t i;
    log_d("- writing" );
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
        log_d("- reading" );
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