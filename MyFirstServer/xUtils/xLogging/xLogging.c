/**
 *
 * @file xLogging.c
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

#include "xLogging.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>


logger_t *mainLoggerHandler = (logger_t*) 0;
logger_t* javaLoggerHandler = (logger_t*) 0;

static int Logger_CheckFile(logger_t* loggerHandler);
static APP_nERROR Logger_GetTime(char *timeString);
static APP_nERROR Logger_Rotate(logger_t* loggerHandler);

static APP_nERROR Logger_GetTime(char *timeString)
{
#define LOG_timeStringMax (40)
    APP_nERROR enErrorCode = APP_enERROR_OK;
    struct timeval timeValue;
    time_t * timeInSeconds;
    time_t timeInMicroSeconds;
    struct tm * timeFormat;
    static bool mutexTimeInit = FALSE;
    static pthread_mutex_t timerMutex;
    char dateString[LOG_timeStringMax];

    if((char*) 0 == timeString)
    {
        enErrorCode = APP_enERROR_INVALID_POINTER;
    }
    /*Init mutex, before this point all attributes must be set up*/
    if(APP_enERROR_OK == enErrorCode)
    {
        /*according to documentation INIT always returns 0*/
        if(FALSE == mutexTimeInit)
        {
            int errorSpecific = pthread_mutex_init(&timerMutex, NULL);
            if (0 != errorSpecific)
            {
                enErrorCode = APP_enERROR_INIT;
            }
            else
            {
                mutexTimeInit = TRUE;
            }
        }
    }
    if(APP_enERROR_OK == enErrorCode)
    {
        int sErrorTime = gettimeofday(&timeValue, NULL);
        if(0 == sErrorTime)
        {
            timeInSeconds = &(timeValue.tv_sec);
            timeInMicroSeconds = timeValue.tv_usec;
            timeInMicroSeconds /= 1000;

            /*note: localtime return an static structure pointer is not need to freed it*/
            pthread_mutex_lock(&timerMutex);
            timeFormat = localtime(timeInSeconds);
            strftime(dateString, LOG_timeStringMax, "%4Y-%m-%d %X", timeFormat);
            pthread_mutex_unlock(&timerMutex);
            sprintf(timeString, "%s.%03ld ", dateString, timeInMicroSeconds);
        }
        else
        {
            sprintf(timeString, "     Overflow time     ");
        }
    }

    return (enErrorCode);
}

static int Logger_CheckFile(logger_t* loggerHandler)
{
    APP_nERROR enErrorCode = APP_enERROR_OK;
    if((logger_t*) 0 == loggerHandler)
    {
        enErrorCode = APP_enERROR_ARGUMENT_POINTER;
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        struct stat currentFileStat;
        char* filePathReg = loggerHandler->info->filePath;
        pthread_mutex_unlock(&(loggerHandler->printMutex));
        int errorSpecific = stat(filePathReg, &currentFileStat);
        /*File doesnt exist, previous file descriptor is corrupted, need to be created again*/
        if(0 != errorSpecific)
        {
            if(0 != loggerHandler->fileDescriptor)
            {
                fclose(loggerHandler->fileDescriptor);
                loggerHandler->fileDescriptor = (FILE*) 0;
            }
            loggerHandler->fileDescriptor = fopen(filePathReg, "a+");
            if((FILE*) 0 == loggerHandler->fileDescriptor)
            {
                enErrorCode = APP_enERROR_INVALID_PATH;
            }
            else
            {
                fflush(loggerHandler->fileDescriptor);
            }
        }

        pthread_mutex_unlock(&(loggerHandler->printMutex));
    }
    return (enErrorCode);
}

APP_nERROR Logger_Rotate(logger_t* loggerHandler)
{
    APP_nERROR enErrorCode = APP_enERROR_OK;
    bool reopen = FALSE;
    bool fileExist = FALSE;
    long errorCount = 0;
    char* filePathReg = (char*) 0;
    struct stat currentFileStat;
    if((logger_t*) 0 == loggerHandler)
    {
        errorCount++;
        enErrorCode = APP_enERROR_ARGUMENT_POINTER;
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        errorCount++;
        pthread_mutex_lock(&(loggerHandler->printMutex));
        filePathReg = loggerHandler->info->filePath;
        int errorSpecific = stat(filePathReg, &currentFileStat);
        /*File doestn exists*/
        if(0 != errorSpecific)
        {
            reopen = TRUE;
        }
        else
        {
            fileExist = TRUE;
        }
        pthread_mutex_unlock(&(loggerHandler->printMutex));
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        uint32_t maxSizeReg = loggerHandler->info->maxSize;
        errorCount++;
        if(TRUE == fileExist)
        {
            /*needs to be rotated*/
            if(maxSizeReg <= currentFileStat.st_size)
            {
                pthread_mutex_lock(&(loggerHandler->printMutex));
                uint32_t backupNumberReg = loggerHandler->info->backupNumber;
                if(0 != backupNumberReg)
                {
                    int pathlength = strlen(filePathReg);
                    pathlength += 1 + 1 + 3;  /*filename.xxx*/
                    char* backupFileName = (char*) calloc(pathlength, sizeof(char));
                    char* backupFileName_Next = (char*) calloc(pathlength, sizeof(char));
                    char* backupFileNamePointer = backupFileName;
                    char* backupFileNamePointer_Next = backupFileName_Next;

                    for(int i = backupNumberReg; i > 0; i--)
                    {
                        struct stat backupFileStat;
                        backupFileNamePointer = backupFileName;
                        int conversionLength = snprintf(backupFileNamePointer , (pathlength - 1), "%s.%u", filePathReg, (unsigned int) i);
                        if(pathlength > conversionLength)
                        {
                            int errorSpecific = stat(backupFileNamePointer, &backupFileStat);
                            if(0 == errorSpecific)
                            {
                                if(backupNumberReg != i)
                                {
                                    backupFileNamePointer_Next = backupFileName_Next;
                                    snprintf(backupFileNamePointer_Next , (pathlength - 1), "%s.%u", filePathReg, (unsigned int) (i + 1));
                                    rename(backupFileNamePointer, backupFileNamePointer_Next);
                                }
                                remove(backupFileNamePointer);
                            }
                        }
                    }
                    rename(filePathReg, backupFileNamePointer);
                    if((char*) 0 != backupFileName)
                    {
                        free(backupFileName);
                        backupFileName = (char*) 0;
                    }

                    if((char*) 0 != backupFileName)
                    {
                        free(backupFileName_Next);
                        backupFileName_Next = (char*) 0;
                    }
                }

                remove(filePathReg);

                reopen = TRUE;
                pthread_mutex_unlock(&(loggerHandler->printMutex));
            }
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        errorCount++;
        if(TRUE == reopen)
        {
            pthread_mutex_lock(&(loggerHandler->printMutex));
            if(0 != loggerHandler->fileDescriptor)
            {
                fclose(loggerHandler->fileDescriptor);
                loggerHandler->fileDescriptor = (FILE*) 0;
            }
            loggerHandler->fileDescriptor = fopen(filePathReg, "a+");
            if((FILE*) 0 == loggerHandler->fileDescriptor)
            {
                enErrorCode = APP_enERROR_INVALID_PATH;
            }
            else
            {
                fflush(loggerHandler->fileDescriptor);
            }
            pthread_mutex_unlock(&(loggerHandler->printMutex));
        }
    }

    return (enErrorCode);
}

int Logger_Print (logger_t* loggerHandler, char* pcFormat, const char* pcIdentifier,
               const char* pcFunctionName, const unsigned int uiFunctionLine, ... )
{
#define PARAM_LEN (80)
#define MAX_PRINT_LENGTH (3900)
#define LINE_LENGTH (120)
    APP_nERROR enErrorCode = APP_enERROR_OK;
    int uxLength = 0;
    int uxLineLength = 0;
    char* pcBufferReg = malloc(MAX_PRINT_LENGTH + 1);
    const char pcFunctionNameReg[PARAM_LEN] = "Unknown";
    char pcLocalTime[PARAM_LEN];
    const char* pcFunc;
    va_list vaList;
    va_start(vaList, uiFunctionLine);

    if((char*) 0 == pcBufferReg)
    {
        enErrorCode = APP_enERROR_INVALID_POINTER;
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        if((logger_t*) 0 == loggerHandler)
        {
            enErrorCode = APP_enERROR_ARGUMENT_POINTER;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        enErrorCode = Logger_CheckFile(loggerHandler);
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        memset(pcLocalTime, 0, PARAM_LEN);
        enErrorCode = Logger_GetTime(pcLocalTime);
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        pcBufferReg[0] = 0;
        pcBufferReg[MAX_PRINT_LENGTH] = 0;
        pcFunc = ((char*) 0 == pcFunctionName) ? pcFunctionNameReg : pcFunctionName;

        pthread_mutex_lock(&(loggerHandler->printMutex));
        int headerLength = 0;
        if((char*) 0 == pcIdentifier)
        {
            headerLength = fprintf(loggerHandler->fileDescriptor,"%s [%s:%4u]: ",
                                    pcLocalTime, pcFunc, uiFunctionLine);
        }
        else
        {
            headerLength = fprintf(loggerHandler->fileDescriptor,"%s [%s:%4u|%s]: ",
                                    pcLocalTime, pcFunc, uiFunctionLine, pcIdentifier);
        }
        uxLength = headerLength;
        uxLineLength = LINE_LENGTH;
        if(0 != pcFormat)
        {
            if(0 != *pcFormat)
            {
                int formatLength = vsnprintf(pcBufferReg, MAX_PRINT_LENGTH + 1, pcFormat, vaList);
                if(formatLength >= MAX_PRINT_LENGTH)
                {
                    formatLength = MAX_PRINT_LENGTH;
                }
                pcBufferReg[formatLength] = 0;
                int iter = 0;
                do
                {
                    if(0 != iter)
                    {
                        uxLength += fprintf(loggerHandler->fileDescriptor, "%*c", headerLength, ' ');
                        fflush(loggerHandler->fileDescriptor);
                    }

                    char* currentBuffer = &pcBufferReg[iter];
                    char* linePointer = strchr(currentBuffer, '\n');
                    int printLength = 0;
                    if((char*) 0 != linePointer)
                    {
                        int difference = (linePointer - currentBuffer)/sizeof(char);
                        if(uxLineLength > difference)
                        {
                            iter += difference;
                            printLength = difference;
                        }
                        else
                        {
                            iter += uxLineLength;
                            printLength = uxLineLength;
                        }
                    }
                    else
                    {
                        if(uxLineLength < formatLength)
                        {
                            iter += uxLineLength;
                            printLength = uxLineLength;
                        }
                        else
                        {
                            iter += formatLength;
                            printLength = formatLength;
                        }
                    }

                    char tempChar = pcBufferReg[iter];
                    pcBufferReg[iter] = 0;
                    fprintf(loggerHandler->fileDescriptor, "%s", currentBuffer);
                    fflush(stderr);
                    fflush(loggerHandler->fileDescriptor);
                    uxLength += printLength;
                    uxLength += fprintf(loggerHandler->fileDescriptor,"\n");
                    formatLength -= printLength;
                    pcBufferReg[iter] = tempChar;
                    if('\n' == tempChar)
                    {
                        iter++;
                        formatLength--;
                    }
                }while((0 < formatLength) && (0 != pcBufferReg[iter]));
            }
            else
            {
                uxLength += fprintf(loggerHandler->fileDescriptor,"Empty LOG \n");
            }
        }
        else
        {
            uxLength += fprintf(loggerHandler->fileDescriptor,"Empty LOG \n");
        }

        if(APP_enERROR_OK == enErrorCode)
        {
            enErrorCode = Logger_Rotate(loggerHandler);
        }
        fflush(loggerHandler->fileDescriptor);
        pthread_mutex_unlock(&(loggerHandler->printMutex));
    }
    if((char*) 0 != pcBufferReg)
    {
        free(pcBufferReg);
        pcBufferReg = (char*) 0;
    }
    va_end(vaList);
    return (uxLength);
}



int Logger_PrintNoHeader (logger_t* loggerHandler, char* pcFormat, ... )
{
#define PARAM_LEN (80)
#define MAX_PRINT_LENGTH (3900)
#define LINE_LENGTH (120)
    APP_nERROR enErrorCode = APP_enERROR_OK;
    int uxLength = 0;
    int uxLineLength = 0;
    char* pcBufferReg = malloc(MAX_PRINT_LENGTH + 1);
    char pcLocalTime[PARAM_LEN];
    va_list vaList;
    va_start(vaList, pcFormat);

    if((char*) 0 == pcBufferReg)
    {
        enErrorCode = APP_enERROR_INVALID_POINTER;
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        if((logger_t*) 0 == loggerHandler)
        {
            enErrorCode = APP_enERROR_ARGUMENT_POINTER;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        enErrorCode = Logger_CheckFile(loggerHandler);
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        memset(pcLocalTime, 0, PARAM_LEN);
        enErrorCode = Logger_GetTime(pcLocalTime);
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        pcBufferReg[0] = 0;
        pcBufferReg[MAX_PRINT_LENGTH] = 0;
        pthread_mutex_lock(&(loggerHandler->printMutex));
        int headerLength = fprintf(loggerHandler->fileDescriptor,"%s : ", pcLocalTime);
        uxLength = headerLength;
        uxLineLength = LINE_LENGTH;
        if(0 != pcFormat)
        {
            if(0 != *pcFormat)
            {
                int formatLength = vsnprintf(pcBufferReg, MAX_PRINT_LENGTH + 1, pcFormat, vaList);
                if(formatLength >= MAX_PRINT_LENGTH)
                {
                    formatLength = MAX_PRINT_LENGTH;
                }
                pcBufferReg[formatLength] = 0;
                int iter = 0;
                do
                {
                    if(0 != iter)
                    {
                        uxLength += fprintf(loggerHandler->fileDescriptor, "%*c", headerLength, ' ');
                        fflush(loggerHandler->fileDescriptor);
                    }

                    char* currentBuffer = &pcBufferReg[iter];
                    char* linePointer = strchr(currentBuffer, '\n');
                    int printLength = 0;
                    if((char*) 0 != linePointer)
                    {
                        int difference = (linePointer - currentBuffer)/sizeof(char);
                        if(uxLineLength > difference)
                        {
                            iter += difference;
                            printLength = difference;
                        }
                        else
                        {
                            iter += uxLineLength;
                            printLength = uxLineLength;
                        }
                    }
                    else
                    {
                        if(uxLineLength < formatLength)
                        {
                            iter += uxLineLength;
                            printLength = uxLineLength;
                        }
                        else
                        {
                            iter += formatLength;
                            printLength = formatLength;
                        }
                    }

                    char tempChar = pcBufferReg[iter];
                    pcBufferReg[iter] = 0;
                    fprintf(loggerHandler->fileDescriptor, "%s", currentBuffer);
                    fflush(loggerHandler->fileDescriptor);
                    uxLength += printLength;
                    uxLength += fprintf(loggerHandler->fileDescriptor,"\n");
                    formatLength -= printLength;
                    pcBufferReg[iter] = tempChar;
                    if('\n' == tempChar)
                    {
                        iter++;
                        formatLength--;
                    }
                }while((0 < formatLength) && (0 != pcBufferReg[iter]));
            }
            else
            {
                uxLength += fprintf(loggerHandler->fileDescriptor,"Empty LOG \n");
            }
        }
        else
        {
            uxLength += fprintf(loggerHandler->fileDescriptor,"Empty LOG \n");
        }

        if(APP_enERROR_OK == enErrorCode)
        {
            enErrorCode = Logger_Rotate(loggerHandler);
        }
        fflush(loggerHandler->fileDescriptor);
        pthread_mutex_unlock(&(loggerHandler->printMutex));
    }
    if((char*) 0 != pcBufferReg)
    {
        free(pcBufferReg);
        pcBufferReg = (char*) 0;
    }
    va_end(vaList);
    return (uxLength);
}


int Logger_PrintNoFormat (logger_t* loggerHandler, char* pcFormat, const char* pcIdentifier,
               const char* pcFunctionName, const unsigned int uiFunctionLine)
{
#define PARAM_LEN (80)
#define MAX_PRINT_LENGTH (3900)
#define LINE_LENGTH (120)
    APP_nERROR enErrorCode = APP_enERROR_OK;
    int uxLength = 0;
    int uxLineLength = 0;
    const char pcFunctionNameReg[PARAM_LEN] = "Unknown";
    char pcLocalTime[PARAM_LEN];
    const char* pcFunc;

    if((logger_t*) 0 == loggerHandler)
    {
        enErrorCode = APP_enERROR_ARGUMENT_POINTER;
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        enErrorCode = Logger_CheckFile(loggerHandler);
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        memset(pcLocalTime, 0, PARAM_LEN);
        enErrorCode = Logger_GetTime(pcLocalTime);
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        pcFunc = ((char*) 0 == pcFunctionName) ? pcFunctionNameReg : pcFunctionName;

        pthread_mutex_lock(&(loggerHandler->printMutex));
        int headerLength = 0;
        if((char*) 0 == pcIdentifier)
        {
            headerLength = fprintf(loggerHandler->fileDescriptor,"%s [%s:%4u]: ",
                                    pcLocalTime, pcFunc, uiFunctionLine);
        }
        else
        {
            headerLength = fprintf(loggerHandler->fileDescriptor,"%s [%s:%4u|%s]: ",
                                    pcLocalTime, pcFunc, uiFunctionLine, pcIdentifier);
        }
        uxLength = headerLength;
        uxLength = headerLength;
        uxLineLength = LINE_LENGTH;
        if(0 != pcFormat)
        {
            if(0 != *pcFormat)
            {
                size_t formatLength = strlen(pcFormat);
                size_t iter = 0;
                char* printBuffer = (char*) malloc(uxLineLength + 1);
                if((char*) 0 != printBuffer)
                {
                    printBuffer[0] = 0;
                    printBuffer[uxLineLength] = 0;
                    do
                    {
                        if(0 != iter)
                        {
                            uxLength += fprintf(loggerHandler->fileDescriptor, "%*c", headerLength, ' ');
                            fflush(loggerHandler->fileDescriptor);
                        }

                        char* currentBuffer = &pcFormat[iter];
                        int position = iter;
                        char* linePointer = strchr(currentBuffer, '\n');
                        if((char*) 0 != linePointer)
                        {
                            size_t difference = (linePointer - currentBuffer)/sizeof(char);
                            if(uxLineLength > difference)
                            {
                                iter += difference;
                            }
                            else
                            {
                                iter += uxLineLength;
                            }
                        }
                        else
                        {
                            if(uxLineLength < formatLength)
                            {
                                iter += uxLineLength;
                            }
                            else
                            {
                                iter += formatLength;
                            }
                        }

                        char tempChar = pcFormat[iter];
                        position = iter - position;
                        memcpy(printBuffer,currentBuffer, position);
                        printBuffer[position] = 0;
                        if(0 != *printBuffer)
                        {
                            size_t printLength = fprintf(loggerHandler->fileDescriptor, "%s", printBuffer);
                            fflush(loggerHandler->fileDescriptor);
                            uxLength += printLength;
                            uxLength += fprintf(loggerHandler->fileDescriptor,"\n");
                        }
                        formatLength -= position;
                        if('\n' == tempChar)
                        {
                            iter++;
                            formatLength--;
                        }
                    }while(0 != formatLength);
                }
                if((char*) 0 != printBuffer)
                {
                    free(printBuffer);
                    printBuffer =(char*) 0;
                }
            }
            else
            {
                uxLength += fprintf(loggerHandler->fileDescriptor,"Empty LOG \n");
            }
        }
        else
        {
            uxLength += fprintf(loggerHandler->fileDescriptor,"Empty LOG \n");
        }

        if(APP_enERROR_OK == enErrorCode)
        {
            enErrorCode = Logger_Rotate(loggerHandler);
        }
        fflush(loggerHandler->fileDescriptor);
        pthread_mutex_unlock(&(loggerHandler->printMutex));
    }
    return (uxLength);
}

int Logger_PrintData (logger_t* loggerHandler, char* pcFormat, const char* pcIdentifier,
               const char* pcFunctionName, const unsigned int uiFunctionLine, unsigned char* data, size_t dataLength, ... )
{

#define PARAM_LEN (80)
#define MAX_PRINT_LENGTH (3900)
#define LINE_LENGTH (120)
    int uxLength = 0;
    size_t dataLengthTotal = 0;
    va_list vaList;
    va_start(vaList, dataLength);
    uxLength += Logger_Print(loggerHandler, pcFormat, pcIdentifier, pcFunctionName, uiFunctionLine, vaList);
    dataLengthTotal = ((dataLength *3) > MAX_PRINT_LENGTH) ? MAX_PRINT_LENGTH/3 : dataLength;
    char* dataInHeOriginal = malloc((dataLengthTotal * 3) + 1);
    if((char*) 0 != dataInHeOriginal)
    {
        dataInHeOriginal[0] = 0;
        dataInHeOriginal[dataLengthTotal * 3] = 0;
        char* dataInHex = dataInHeOriginal;
        for(size_t i = 0; i < dataLengthTotal; i++)
        {
            size_t counter = sprintf(dataInHex, "%02X ", *data);
            data = data + 1;
            dataInHex = dataInHex + counter;
        }
        *dataInHex = 0;
        uxLength += Logger_Print(loggerHandler, "%s", pcIdentifier, pcFunctionName, uiFunctionLine, dataInHeOriginal);
    }
    if((char*) 0 != dataInHeOriginal)
    {
        free(dataInHeOriginal);
        dataInHeOriginal = (char*) 0;
    }
    va_end(vaList);
    return (uxLength);
}


#define DEFAULT_MAX_FILE_SIZE (100UL * 1024UL *1024UL)
APP_nERROR Logger_Init(logger_t **logerHandler, logFileInfo_t* logFileInfo, const char* context, errorSpecific_t* errorReturn)
{
    APP_nERROR enErrorCode = APP_enERROR_OK;
    logger_t* loggerTemp = (logger_t*) 0;
    bool mutexAttrInit = FALSE;
    bool mutexInit = FALSE;
    struct stat pathStats;

    size_t maxFileSize = 0;;
    char* fileDirectory = (char*) 0;
    char* fileName = (char*) 0;
    int pthreadError = 0;


    errorSpecificValue_t* errorSpecific = (errorSpecificValue_t*) malloc(10 * sizeof(errorSpecificValue_t));
    uint8_t errorSpecificCount = 10;
    size_t errorFunctionLine = 0;


    if(APP_enERROR_OK == enErrorCode)
    {
        if((logFileInfo_t*) 0 == logFileInfo)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_ARGUMENT_POINTER;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        if((logger_t**) 0 == logerHandler)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_ARGUMENT_POINTER;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        if((logger_t*) 0 != *logerHandler)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_ARGUMENT_VALUE;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        fileName = logFileInfo->name;
        if((char*) 0 == fileName)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_ARGUMENT_POINTER;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        if(0 == *fileName)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_ARGUMENT_VALUE;
        }
    }

    /*Check if filePath exists*/
    if(APP_enERROR_OK == enErrorCode)
    {
        fileDirectory = logFileInfo->path;
        if(((char*) 0 != fileDirectory) && ((char) 0 != *fileDirectory))
        {
            int statValue = stat(fileDirectory, &pathStats);
            if(0 != statValue)
            {
                errorFunctionLine = __LINE__;
                enErrorCode = APP_enERROR_INVALID_PATH;
            }
        }
    }

    /*Check if filePath is valid, this path must be a directory*/
    if(APP_enERROR_OK == enErrorCode)
    {
        if(((char*) 0 != fileDirectory) && ((char) 0 != *fileDirectory))
        {
            bool isDirectory = S_ISDIR(pathStats.st_mode);
            if(FALSE == isDirectory)
            {
                errorFunctionLine = __LINE__;
                enErrorCode = APP_enERROR_INVALID_PATH;
            }
        }
    }

#if 0 /*unblock if set a specific extention is needed*/

    char* dotPointer = (char*) 0;
    if(APP_enERROR_OK == enErrorCode)
    {
        dotPointer = strchr(fileName, '.');
        if((char*) 0 == dotPointer)
        {
            errorFunctionLine = __LINE__;
            errorSpecific = (size_t)  dotPointer;
            enErrorCode = APP_enERROR_INVALID_NAME;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        int errorSpecific = 0;
        errorSpecific = strncmp(dotPointer,".log", 10);
        if(0 != errorSpecific)
        {
            errorFunctionLine = __LINE__;
            errorSpecific = (size_t)  errorSpecific;
            enErrorCode = APP_enERROR_INVALID_NAME;
        }
    }

#endif
    /*Reserve memory fo the new logger_t structure, this will act as a handler*/
    if(APP_enERROR_OK == enErrorCode)
    {
        loggerTemp = (logger_t*) calloc(1, sizeof(logger_t));
        if((logger_t*) 0 == loggerTemp)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_INVALID_POINTER;
        }
    }

    int dirLength = 0;
    if(APP_enERROR_OK == enErrorCode)
    {
        int nameLength = 0;
        int fullLength = 0;
        loggerTemp->info = logFileInfo;
        if(((char*) 0 != fileDirectory) && ((char) 0 != *fileDirectory))
        {
            dirLength = strlen(fileDirectory); /*this length doesnt consider null terminating chat*/
        }

        nameLength = strlen(fileName);
        fullLength = dirLength + nameLength + 5; /*adding '/' and \0*/
        loggerTemp->info->filePath = (char*) calloc(fullLength, sizeof(char*));
        if((char*) 0 == loggerTemp->info->filePath)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_INVALID_POINTER;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        if((char*) 0 != fileDirectory)
        {
            if((char) 0 != *fileDirectory)
            {
                if('/' != fileDirectory[dirLength - 1])
                {
                    sprintf(loggerTemp->info->filePath, "%s/%s", fileDirectory, fileName);
                }
                else
                {
                    sprintf(loggerTemp->info->filePath, "%s%s", fileDirectory, fileName);
                }
            }
            else
            {
                sprintf(loggerTemp->info->filePath, "%s", fileName);
            }
        }
        else
        {
            sprintf(loggerTemp->info->filePath, "%s", fileName);
        }
        loggerTemp->fileDescriptor = fopen(loggerTemp->info->filePath, "a+");
        if((FILE*) 0 == loggerTemp->fileDescriptor)
        {
            errorFunctionLine = __LINE__;
            errorSpecificCount = 1;
            strcpy(errorSpecific[0].valueName, "errno");
            errorSpecific[0].errorValue = errno;
            enErrorCode = APP_enERROR_INVALID_PATH;
        }
        else
        {
            fflush(loggerTemp->fileDescriptor);
        }
    }

    /*Init mutex attributes*/
    if(APP_enERROR_OK == enErrorCode)
    {
        /*First init mutex attributes*/
        pthreadError = pthread_mutexattr_init(&(loggerTemp->mutexPropierties));
        if (0 != pthreadError)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_INIT;
        }
    }

    /*Init set mutex type attributes, if more atrtributes are needed you can add after this block*/
    if(APP_enERROR_OK == enErrorCode)
    {
        mutexAttrInit = TRUE;
        /*First init mutex attributes*/
        pthreadError = pthread_mutexattr_settype(&(loggerTemp->mutexPropierties), PTHREAD_MUTEX_ERRORCHECK);
        if (0 != pthreadError)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_INIT;
        }
    }

    /*Init mutex, before this point all attributes must be set up*/
    if(APP_enERROR_OK == enErrorCode)
    {
        /*according to documentation INIT always returns 0*/
        pthreadError = pthread_mutex_init(&(loggerTemp->printMutex), &(loggerTemp->mutexPropierties));
        if (0 != pthreadError)
        {
            errorFunctionLine = __LINE__;
            enErrorCode = APP_enERROR_INIT;
        }
    }

    if(APP_enERROR_OK == enErrorCode)
    {
        maxFileSize = logFileInfo->maxSize;
        mutexInit = TRUE;
        loggerTemp->init = TRUE;
        loggerTemp->info->maxSize = (0 == maxFileSize) ? DEFAULT_MAX_FILE_SIZE : maxFileSize;
        *logerHandler = loggerTemp;
    }
    else
    {
        loggerTemp->init = FALSE;
        if(TRUE == mutexAttrInit)
        {
            pthread_mutexattr_destroy(&(loggerTemp->mutexPropierties));
            memset(&(loggerTemp->mutexPropierties), 0, sizeof(pthread_mutexattr_t));
        }
        if(TRUE == mutexInit)
        {
            pthread_mutex_destroy(&(loggerTemp->printMutex));
        }
        if((FILE*) 0 != loggerTemp->fileDescriptor)
        {
            fclose(loggerTemp->fileDescriptor);
            loggerTemp->fileDescriptor = (FILE*) 0;
        }
        if((char*) 0 != loggerTemp->info->filePath)
        {
            free(loggerTemp->info->filePath);
            loggerTemp->info->filePath = (char*) 0;
        }
        if((logger_t*) 0 != loggerTemp)
        {
            free(loggerTemp);
            loggerTemp = (logger_t*) 0;
        }

    }

    if(APP_enERROR_OK != enErrorCode)
    {
        if((errorSpecificValue_t *) 0 != errorSpecific)
        {
            strcpy(errorSpecific[0].valueName, "logerHandler");
            errorSpecific[0].errorValue = (size_t) logerHandler;
            errorSpecific[0].errorValueString[0] = 0;
            strcpy(errorSpecific[1].valueName, "logFileInfo");
            errorSpecific[1].errorValue = (size_t) logFileInfo;
            errorSpecific[1].errorValueString[0] = 0;
            strcpy(errorSpecific[2].valueName, "loggerTemp");
            errorSpecific[2].errorValue = (size_t) loggerTemp;
            errorSpecific[2].errorValueString[0] = 0;

            strcpy(errorSpecific[3].valueName, "mutexAttrInit");
            errorSpecific[3].errorValue = mutexAttrInit;
            errorSpecific[3].errorValueString[0] = 0;
            strcpy(errorSpecific[4].valueName, "mutexInit");
            errorSpecific[4].errorValue = mutexInit;
            errorSpecific[4].errorValueString[0] = 0;

            strcpy(errorSpecific[5].valueName, "maxFileSize");
            errorSpecific[5].errorValue = maxFileSize;
            errorSpecific[5].errorValueString[0] = 0;
            strcpy(errorSpecific[6].valueName, "fileDirectory");
            errorSpecific[6].errorValue = (size_t) fileDirectory;
            if((char*) 0 != fileDirectory)
            {
                strcpy( errorSpecific[6].errorValueString, fileDirectory);
            }
            else
            {
                errorSpecific[6].errorValueString[0] = 0;
            }
            strcpy(errorSpecific[7].valueName, "fileName");
            errorSpecific[7].errorValue = (size_t) fileName;
            if((char*) 0 != fileName)
            {
                strcpy( errorSpecific[7].errorValueString, fileName);
            }
            else
            {
                errorSpecific[7].errorValueString[0] = 0;

            }
            strcpy(errorSpecific[8].valueName, "errno");
            errorSpecific[8].errorValue = errno;
            strcpy( errorSpecific[8].errorValueString, strerror(errno));
            strcpy(errorSpecific[9].valueName, "pthreadError");
            errorSpecific[9].errorValue = pthreadError;
            errorSpecific[9].errorValueString[0] = 0;

            ErrorSpecific_SetNext(enErrorCode, errorReturn, errorSpecific, errorSpecificCount, errorFunctionLine, __FUNCTION__);
        }
    }

    if((errorSpecificValue_t *) 0 != errorSpecific)
    {
        free(errorSpecific);
        errorSpecific = (errorSpecificValue_t *) 0;
    }

    return (enErrorCode);
}

APP_nERROR Logger_DeInit(logger_t* loggerHandler)
{
    APP_nERROR enErrorCode = APP_enERROR_OK;
    if((logger_t*) 0 == loggerHandler)
    {
        enErrorCode = APP_enERROR_ARGUMENT_POINTER;
    }
    if(APP_enERROR_OK == enErrorCode)
    {
        if(TRUE == loggerHandler->init)
        {
            pthread_mutexattr_destroy(&(loggerHandler->mutexPropierties));
            pthread_mutex_destroy(&(loggerHandler->printMutex));
        }
        if((FILE*) 0 != loggerHandler->fileDescriptor)
        {
            fclose(loggerHandler->fileDescriptor);
            loggerHandler->fileDescriptor = (FILE*) 0;
        }
        if((char*) 0 != loggerHandler->info->filePath)
        {
            free(loggerHandler->info->filePath);
            loggerHandler->info->filePath = (char*) 0;
        }
        if((logger_t *) 0 != loggerHandler)
        {
            free(loggerHandler);
            loggerHandler = (logger_t *) 0;
        }
    }
    return (enErrorCode);
}


