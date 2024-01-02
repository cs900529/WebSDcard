#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);
void testFileIO(fs::FS &fs, const char * path);
String readHtmlFromSD(const char *filePath);
String readLatestLogs(const char *filename, size_t numLines);
void removeFirstEntry(const char *filePath);
void logFile(fs::FS &fs, const char * path, const uint16_t pv_power, const uint16_t load_power, const uint16_t flatten_power, const char * formattedTime);
void logHistory(fs::FS &fs, const char * path, const uint16_t pv_power, const uint16_t load_power, const uint16_t flatten_power, const char * formattedTime);

#endif
