#include "Fs.h"
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true

class FileCore {
public:
    FileCore();
    ~FileCore();

    void mount();

    void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

    void readFile(fs::FS &fs, const char * path);

    void writeFile(fs::FS &fs, const char * path, const char * message);

    void appendFile(fs::FS &fs, const char * path, const char * message);

    void renameFile(fs::FS &fs, const char * path1, const char * path2);

    void deleteFile(fs::FS &fs, const char * path);

    void testFileIO(fs::FS &fs, const char * path);

};


/*
    demo from https://www.tutorialspoint.com/esp32_for_iot/esp32_for_iot_spiffs_storage.htm


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