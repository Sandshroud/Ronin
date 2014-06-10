/**
  @file Log.cpp

  @maintainer Morgan McGuire, http://graphics.cs.williams.edu
  @created 2001-08-04
  @edited  2010-01-15
 */

#include "platform.h"
#include "Log.h"
#include "format.h"
#include "Array.h"
#include "fileutils.h"
#include "FileSystem.h"
#include <time.h>

#ifdef G3D_WIN32
    #include <imagehlp.h>
#else
    #include <stdarg.h>
#endif

namespace G3D {

void logPrintf(const char* fmt, ...) {
    va_list arg_list;
    va_start(arg_list, fmt);
    G3D_Log::common()->vprintf(fmt, arg_list);
    va_end(arg_list);
}


void logLazyPrintf(const char* fmt, ...) {
    va_list arg_list;
    va_start(arg_list, fmt);
    G3D_Log::common()->lazyvprintf(fmt, arg_list);
    va_end(arg_list);
}

G3D_Log* G3D_Log::commonLog = NULL;

G3D_Log::G3D_Log(const std::string& filename, int stripFromStackBottom) :
    stripFromStackBottom(stripFromStackBottom) {

    this->filename = filename;

    logFile = FileSystem::fopen(filename.c_str(), "w");

    if (logFile == NULL) {
        std::string drive, base, ext;
        Array<std::string> path;
        parseFilename(filename, drive, path, base, ext);
        std::string logName = base + ((ext != "") ? ("." + ext) : "");

        // Write time is greater than 1ms.  This may be a network drive.... try another file.
        #ifdef G3D_WIN32
            logName = std::string(std::getenv("TEMP")) + logName;
        #else
            logName = std::string("/tmp/") + logName;
        #endif

        logFile = FileSystem::fopen(logName.c_str(), "w");
    }

    // Use a large buffer (although we flush in logPrintf)
    setvbuf(logFile, NULL, _IOFBF, 2048);

    fprintf(logFile, "Application Log\n");
    time_t t;
    time(&t);
    fprintf(logFile, "Start: %s\n", ctime(&t));
    fflush(logFile);

    if (commonLog == NULL) {
        commonLog = this;
    }
}


G3D_Log::~G3D_Log() {
    section("Shutdown");
    println("Closing log file");

    // Make sure we don't leave a dangling pointer
    if (G3D_Log::commonLog == this) {
        G3D_Log::commonLog = NULL;
    }

    fclose(logFile);
}


FILE* G3D_Log::getFile() const {
    return logFile;
}


G3D_Log* G3D_Log::common() {
    if (commonLog == NULL) {
        commonLog = new G3D_Log();
    }
    return commonLog;
}


std::string G3D_Log::getCommonLogFilename() {
    return common()->filename;
}


void G3D_Log::section(const std::string& s) {
    fprintf(logFile, "_____________________________________________________\n");
    fprintf(logFile, "\n    ###    %s    ###\n\n", s.c_str());
}


void __cdecl G3D_Log::printf(const char* fmt, ...) {
    va_list arg_list;
    va_start(arg_list, fmt);
    print(G3D_vformat(fmt, arg_list));
    va_end(arg_list);
}


void __cdecl G3D_Log::vprintf(const char* fmt, va_list argPtr) {
    vfprintf(logFile, fmt, argPtr);
    fflush(logFile);
}


void __cdecl G3D_Log::lazyvprintf(const char* fmt, va_list argPtr) {
    vfprintf(logFile, fmt, argPtr);
}


void G3D_Log::print(const std::string& s) {
    fprintf(logFile, "%s", s.c_str());
    fflush(logFile);
}


void G3D_Log::println(const std::string& s) {
    fprintf(logFile, "%s\n", s.c_str());
    fflush(logFile);
}

}
