#ifndef Filesystem_h
#define Filesystem_h

#include "FS.h"
#include <WString.h>

class FilesystemClass
{
public:
    static void init();
    static String readFile(fs::FS &fs, const char *path);
    static void writeFile(fs::FS &fs, const char *path, const char *message);
};

extern FilesystemClass Filesystem;

#endif