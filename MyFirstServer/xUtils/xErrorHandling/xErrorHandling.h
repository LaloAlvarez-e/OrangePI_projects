/**
 *
 * @file xErrorHandling.h
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

#ifndef XUTILS_XERRORHANDLING_XERRORHANDLING_H_
#define XUTILS_XERRORHANDLING_XERRORHANDLING_H_

#include "xDefines.h"

#define ERROR_SPECIFIC_DEPTH_MAX (10)
#define ERROR_DEPTH_MAX (20)
#define FUNCTION_NAME_MAX (64)
#define ERROR_NAME_MAX (256)

typedef struct
{
    char valueName[FUNCTION_NAME_MAX];
    char errorValueString[ERROR_NAME_MAX];
    size_t errorValue;
    char reserved[12];
}errorSpecificValue_t;

typedef struct
{
    errorSpecificValue_t errorValue[ERROR_SPECIFIC_DEPTH_MAX];
    char errorFunctionName[FUNCTION_NAME_MAX];
    size_t errorValid;
    size_t errorCounter;
    size_t errorFunctionLine;
    APP_nERROR errorGlobal;
}errorStructure_t;

typedef struct
{
    errorStructure_t error[ERROR_DEPTH_MAX];
    size_t errorDepth;
    char reserved[12];
}errorSpecific_t;


void ErrorSpecific_Init(errorSpecific_t* errorReturn);
void ErrorSpecific_Restart(errorSpecific_t* errorReturn);
void ErrorSpecific_SetFirst(APP_nERROR enErrorCode, errorSpecific_t* errorReturn,
                            errorSpecificValue_t* errorSpecific, uint8_t errorSpecificCount,
                            uint32_t functionLine, const char* functionName);
void ErrorSpecific_SetNext(APP_nERROR enErrorCode, errorSpecific_t* errorReturn,
                        errorSpecificValue_t* errorSpecific, uint8_t errorSpecificCount,
                        uint32_t functionLine, const char* functionName);
void ErrorSpecific_GetErrorByfunction(errorSpecific_t* errorReturn, const char* functionName, errorStructure_t* errorSpecific);
void ErrorSpecific_Get(errorSpecific_t* errorReturn, uint8_t index, errorStructure_t* errorSpecific);
void ErrorSpecific_GetTail(errorSpecific_t* errorReturn, errorStructure_t* errorSpecific);
void ErrorSpecific_GetHead(errorSpecific_t* errorReturn, errorStructure_t* errorSpecific);

void ErrorSpecific_Format(errorSpecific_t* errorReturn);


#endif /* XUTILS_XERRORHANDLING_XERRORHANDLING_H_ */
