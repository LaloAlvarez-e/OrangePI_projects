/*
 * main.c
 *
 *  Created on: 8 sept 2024
 *      Author: InDeviceMex
 */
#include "main.h"
#include "xErrorHandling.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char** argv)
{
    /*This variable is used for a general error code
     * all custom function must return this type of error
     * and this error must be validated*/
    APP_nERROR  enErrorCode = APP_enERROR_OK;


    /*Variable to track specific errors, at the end of the execution this will be formated
    * But more important, is possible to know if there was an error on the execution
    */
    errorSpecific_t* errorReturn = (errorSpecific_t*) malloc(sizeof(errorSpecific_t));

    /*Number of specific error code */
    errorSpecificValue_t* errorSpecific = (errorSpecificValue_t*) malloc(10 * sizeof(errorSpecificValue_t));
    uint8_t errorSpecificCount = 10;
    size_t errorFunctionLine = 0;

    /*Name of this current thread*/
    const char* context = "Thread Main";

    ErrorSpecific_Init(errorReturn);

    /**
     * @brief checking if at least the execution has the minimum number of parameters on the call
     *
     */
    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./<application> --configFile <path/to/configFile/name.conf>\n");
        fflush(stderr);
        errorFunctionLine = __LINE__;
        enErrorCode = APP_enERROR_ARGUMENT_VALUE;
    }


	printf("Hello World, This is the orange PI 5\n");
	return (enErrorCode);
}
