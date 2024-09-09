/**
 *
 * @file xLogging.h
 * @copyright
 * @verbatim InDeviceMex 2021 @endverbatim
 *
 * @par Responsibility
 * @verbatim InDeviceMex Developers @endverbatim
 *
 * @version
 * @verbatim 1.0 @endverbatim
 *
 * @date
 * @verbatim 8 sept 2024 @endverbatim
 *
 * @author
 * @verbatim InDeviceMex @endverbatim
 *
 * @par Change History
 * @verbatim
 * Date           Author     Version     Description
 * 8 sept 2024     InDeviceMex    1.0         initial Version@endverbatim
 */

#ifndef XUTILS_XLOGGING_XLOGGING_H_
#define XUTILS_XLOGGING_XLOGGING_H_

#include "xDefines.h"
#include "xErrorHandling.h"
#include <stdarg.h>
#include <pthread.h>
#include <stdio.h>

typedef struct
{
    char* path;
    char* name;
    char* filePath;
    uint32_t maxSize;
    uint8_t backupNumber;
    uint8_t reserved[15];
}logFileInfo_t;


typedef struct
{
    uint32_t init;
    logFileInfo_t* info;
    pthread_mutex_t printMutex;
    pthread_mutexattr_t mutexPropierties;
    FILE* fileDescriptor;
    uint8_t reserved[12];
}logger_t;

extern logger_t* mainLoggerHandler;
extern logger_t* javaLoggerHandler;


APP_nERROR Logger_Init(logger_t** logerHandler, logFileInfo_t* logFile, const char* context, errorSpecific_t* errorReturn);
APP_nERROR Logger_DeInit(logger_t* loggerHandler);
int Logger_PrintData (logger_t* loggerHandler, char* pcFormat, const char* pcIdentifier,
               const char* pcFunctionName, const unsigned int uiFunctionLine, unsigned char* data, size_t dataLength, ... );
int Logger_PrintNoFormat (logger_t* loggerHandler, char* pcFormat, const char* pcIdentifier,
               const char* pcFunctionName, const unsigned int uiFunctionLine);
int Logger_Print (logger_t* loggerHandler, char* pcFormat, const char* pcIdentifier,
               const char* pcFunctionName, const unsigned int uiFunctionLine, ... );

int Logger_PrintNoHeader (logger_t* loggerHandler, char* pcFormat, ... );

#define LOG_CLOUD(handler, Str, Ctx, ...) {Logger_Print(handler, Str, Ctx, __FUNCTION__, __LINE__, ## __VA_ARGS__);}
#define LOG_hydraCloud(handler, Str, Ctx, DATA, DATA_LEN, ...) {Logger_PrintData(handler, Str, Ctx, __FUNCTION__, __LINE__, DATA, DATA_LEN, ## __VA_ARGS__);}
#define LOG_MAIN(Str, Ctx, ...) {Logger_Print(mainLoggerHandler, Str, Ctx, __FUNCTION__, __LINE__, ## __VA_ARGS__);}
#define LOG_JAVA(Str, Ctx, ...) {Logger_Print(javaLoggerHandler, Str, Ctx, __FUNCTION__, __LINE__, ## __VA_ARGS__);}
#define LOG_MAIN_DATA(Str, Ctx, DATA, DATA_LEN, ...) {Logger_PrintData(mainLoggerHandler, Str, Ctx, __FUNCTION__, __LINE__, DATA, DATA_LEN, ## __VA_ARGS__);}
#define LOG_MAIN_NOFORMAT(Str, Ctx) {Logger_PrintNoFormat(mainLoggerHandler, Str, Ctx, __FUNCTION__, __LINE__);}

#endif /* XUTILS_XLOGGING_XLOGGING_H_ */
