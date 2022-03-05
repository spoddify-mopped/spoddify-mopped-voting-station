/**
 * Filesystem manager
 */

#ifndef Filesystem_h
#define Filesystem_h

#include <WString.h>

#include "FS.h"

class FilesystemClass {
   public:
    static bool init();
    static String readFile(const char *path);
    static bool removeFile(const char *path);
    static bool writeFile(const char *path, const char *message);
};

extern FilesystemClass Filesystem;

#endif