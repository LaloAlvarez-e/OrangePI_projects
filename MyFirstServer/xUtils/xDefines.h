/*
 * Defines.h
 *
 *  Created on: 8 sept 2024
 *      Author: InDeviceMex
 */

#ifndef XUTILS_XDEFINES_H_
#define XUTILS_XDEFINES_H_

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#if !defined(FALSE) && !defined(TRUE) && !defined(boolean)
typedef enum
{
    FALSE = false,
    TRUE = true
}boolean;

#elif !defined(FALSE) && !defined(TRUE)
#define false FALSE
#define true TRUE

#elif !defined(boolean)
typedef bool boolean;
#endif



typedef enum
{
    APP_enERROR_OK,

    APP_enERROR_ARGUMENT_POINTER,
    APP_enERROR_ARGUMENT_VALUE,
    APP_enERROR_ARGUMENT_RANGE,
    APP_enERROR_ARGUMENT_NOT_FOUND,

    APP_enERROR_INTERNAL_POINTER,

    APP_enERROR_FUNCTION_RETURN,
    APP_enERROR_FUNCTION_VALUE,

    APP_enERROR_INIT,

    APP_enERROR_INVALID_POINTER,
    APP_enERROR_INVALID_COMMAND,
    APP_enERROR_INVALID_METHOD,
    APP_enERROR_INVALID_PATH,
    APP_enERROR_INVALID_NAME,
    APP_enERROR_INVALID_FORMAT,
    APP_enERROR_INVALID_VALUE,
    APP_enERROR_INVALID_STATE,
    APP_enERROR_INVALID_REQUEST,
    APP_enERROR_INVALID_FILE_DESCRIPTOR,

    APP_enERROR_EXTERNAL_SIGNAL,
    APP_enERROR_SOCKET_ERROR,
    APP_enERROR_SOCKET_READ_ERROR,

    APP_enERROR_SIZE_MISMATCH,
    APP_enERROR_LENGTH_READ,
    APP_enERROR_CLOSE_FD,
    APP_enERROR_ADDRESS_SPACE,
    APP_enERROR_TIMEOUT,
    APP_enERROR_OUT_OF_MEMORY,
    APP_enERROR_NO_IPS,
    APP_enERROR_VALIDATING_IPS,
    APP_enERROR_INVALID_CLOUDNAME,
    APP_enERROR_SHUTDOWN,
    APP_enERROR_RANGE,
    APP_enERROR_PARSE,
    APP_enERROR_FULL,
    APP_enERROR_EMPTY,
}APP_nERROR;


#endif /* XUTILS_XDEFINES_H_ */
