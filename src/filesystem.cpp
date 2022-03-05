/**
 * Filesystem manager
 */

#include "filesystem.h"

#include <SD.h>

#include "SPIFFS.h"

/**
 * Initalizes the SPIFFS filesystem.
 */
bool FilesystemClass::init() {
    return SPIFFS.begin(true);
}

/**
 * Read a file from SPIFFS.
 */
String FilesystemClass::readFile(const char *path) {
    File file = SPIFFS.open(path);
    if (!file || file.isDirectory()) {
        return String();
    }

    String fileContent;
    while (file.available()) {
        fileContent = file.readStringUntil('\n');
        break;
    }
    return fileContent;
}

/**
 * Removes a file from SPIFFS.
 */
bool FilesystemClass::removeFile(const char *path) {
    return SPIFFS.remove(path);
}

/**
 * Write a file to SPIFFS.
 */
bool FilesystemClass::writeFile(const char *path, const char *content) {
    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
        return false;
    }
    
    return file.print(content);
}

FilesystemClass Filesystem;