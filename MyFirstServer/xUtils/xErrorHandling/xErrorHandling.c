/**
 *
 * @file xErrorHandling.c
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
#include "xErrorHandling.h"
#include "xLogging.h"
#include <string.h>

void ErrorSpecific_Init(errorSpecific_t* errorReturn)
{
    if((errorSpecific_t*) 0 != errorReturn)
    {
        memset(errorReturn, 0, sizeof(errorSpecific_t));
    }
}

void ErrorSpecific_Restart(errorSpecific_t* errorReturn)
{
    if((errorSpecific_t*) 0 != errorReturn)
    {
        errorReturn->errorDepth = 0;
    }
}
void ErrorSpecific_SetFirst(APP_nERROR enErrorCode, errorSpecific_t* errorReturn,
                            errorSpecificValue_t* errorSpecific, uint8_t errorSpecificCount,
                            uint32_t functionLine, const char* functionName)
{
    if(APP_enERROR_OK != enErrorCode)
    {
        if((errorSpecific_t*) 0 != errorReturn)
        {
            errorReturn->errorDepth = 0;
            ErrorSpecific_SetNext(enErrorCode, errorReturn, errorSpecific, errorSpecificCount, functionLine, functionName);
        }
    }
}

void ErrorSpecific_SetNext(APP_nERROR enErrorCode, errorSpecific_t* errorReturn,
                        errorSpecificValue_t* errorSpecific, uint8_t errorSpecificCount,
                        uint32_t functionLine, const char* functionName)
{
    if(APP_enERROR_OK != enErrorCode)
    {
        if((errorSpecific_t*) 0 != errorReturn)
        {
            uint32_t u32Count = errorReturn->errorDepth;
            if(ERROR_DEPTH_MAX > u32Count)
            {
                errorStructure_t * errorIndex = &(errorReturn->error[u32Count]);
                errorIndex->errorFunctionLine = functionLine;
                strcpy(errorIndex->errorFunctionName,functionName);
                if((errorSpecificValue_t*) 0 != errorSpecific)
                {
                    size_t iter;
                    errorSpecificCount = (errorSpecificCount <= ERROR_SPECIFIC_DEPTH_MAX) ? errorSpecificCount : ERROR_SPECIFIC_DEPTH_MAX;
                    for(iter = 0; iter < errorSpecificCount; iter++)
                    {
                        strcpy(errorIndex->errorValue[iter].valueName, errorSpecific->valueName);
                        strcpy(errorIndex->errorValue[iter].errorValueString, errorSpecific->errorValueString);
                        errorIndex->errorValue[iter].errorValue = errorSpecific->errorValue;
                        errorSpecific++;
                    }
                    errorIndex->errorCounter = errorSpecificCount;
                }
                errorIndex->errorGlobal= enErrorCode;
                errorReturn->errorDepth++;
                errorIndex->errorValid= errorReturn->errorDepth;
            }
        }
    }
}

void ErrorSpecific_GetErrorByfunction(errorSpecific_t* errorReturn, const char* functionName, errorStructure_t* errorSpecific)
{
    if(((errorSpecific_t*) 0 != errorReturn) && ((errorStructure_t*) 0 != errorSpecific) && ((const char*) 0 != functionName))
    {
        if(0 != *functionName)
        {
            size_t deep = 0;
            uint8_t found = -1;
            errorStructure_t*  currentError = &(errorReturn->error[0]);
            while((errorReturn->errorDepth > deep) && (0 != found))
            {
                found = strcmp(currentError->errorFunctionName,functionName);
                if(0 == found)
                {
                    ErrorSpecific_Get(errorReturn, deep, errorSpecific);
                }
                currentError++;
                deep++;
            }

        }
        else
        {
            errorSpecific->errorValid = 0;
        }
    }
}


void ErrorSpecific_Get(errorSpecific_t* errorReturn, uint8_t index, errorStructure_t* errorSpecific)
{
    if(((errorSpecific_t*) 0 != errorReturn) && ((errorStructure_t*) 0 != errorSpecific))
    {
        if((0 != errorReturn->error[index].errorValid) && ((index + 1UL) <= errorReturn->errorDepth))
        {
            errorStructure_t*  currentError = &(errorReturn->error[index]);
            errorSpecific->errorCounter = currentError->errorCounter;
            errorSpecific->errorFunctionLine = currentError->errorFunctionLine;
            errorSpecific->errorGlobal = currentError->errorGlobal;
            errorSpecific->errorValid = currentError->errorValid;

            size_t iter;
            for(iter = 0; iter < currentError->errorCounter; iter++)
            {
                strncpy(errorSpecific->errorValue[iter].valueName, currentError->errorValue[iter].valueName, FUNCTION_NAME_MAX);
                errorSpecific->errorValue[iter].errorValue =currentError->errorValue[iter].errorValue;
            }
            strncpy(errorSpecific->errorFunctionName, currentError->errorFunctionName, FUNCTION_NAME_MAX);
        }
        else
        {
            errorSpecific->errorValid = 0;
        }
    }
}
void ErrorSpecific_GetTail(errorSpecific_t* errorReturn, errorStructure_t* errorSpecific)
{
    if(((errorSpecific_t*) 0 != errorReturn) && ((errorStructure_t*) 0 != errorSpecific))
    {
        if(0 < errorReturn->errorDepth)
        {
            ErrorSpecific_Get(errorReturn, (errorReturn->errorDepth - 1), errorSpecific);
        }
        else
        {
            errorSpecific->errorValid = 0;
        }
    }
}

void ErrorSpecific_GetHead(errorSpecific_t* errorReturn, errorStructure_t* errorSpecific)
{
    if(((errorSpecific_t*) 0 != errorReturn) && ((errorStructure_t*) 0 != errorSpecific))
    {
        if(0 < errorReturn->errorDepth)
        {
            ErrorSpecific_Get(errorReturn, 0, errorSpecific);
        }
        else
        {
            errorSpecific->errorValid = 0;
        }
    }
}

void ErrorSpecific_Format(errorSpecific_t* errorReturn)
{
    if((errorSpecific_t*) 0 != errorReturn)
    {
        if(0 < errorReturn->errorDepth)
        {
            LOG_MAIN("[-] Formation Error Stack", "ErrorSpecific");
            LOG_MAIN("[-] Error Depth           : [%u]", "ErrorSpecific", errorReturn->errorDepth);
            size_t iter = 0;
            errorStructure_t* currentError = &(errorReturn->error[0]);
            for(iter = 0; iter < errorReturn->errorDepth; iter++)
            {
                if(0 != errorReturn->error[iter].errorValid)
                {
                    LOG_MAIN("[-] Error stack           : [%u]", "ErrorSpecific", iter);
                    LOG_MAIN("[-]      Function         : [%s] Line [%u]", "ErrorSpecific", currentError->errorFunctionName, currentError->errorFunctionLine);
                    LOG_MAIN("[-]      App Error        : [%u]", "ErrorSpecific", currentError->errorGlobal);
                    size_t errorNumber = 0;
                    errorSpecificValue_t* specificError = &(currentError->errorValue[0]);
                    for(errorNumber = 0; errorNumber < currentError->errorCounter; errorNumber++)
                    {
                        if(0 == specificError->errorValueString[0])
                        {
                            LOG_MAIN("[-]      Specific Error   [%s]: Value [%u]", "ErrorSpecific", specificError->valueName, specificError->errorValue);
                        }
                        else
                        {
                            LOG_MAIN("[-]      Specific Error   [%s]: Value [%u] String [%s]", "ErrorSpecific", specificError->valueName, specificError->errorValue, specificError->errorValueString);
                        }
                        specificError++;
                    }
                }
                currentError++;
            }
        }
        else
        {
            LOG_MAIN("[+] Formation Error Stack, No Error", "ErrorSpecific");
        }
    }
}


