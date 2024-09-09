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

#include <libxml/xmlreader.h>
/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
static void
processNode(xmlTextReaderPtr reader) {
    const xmlChar *name, *value;

    name = xmlTextReaderConstName(reader);
    if (name == NULL)
    name = BAD_CAST "--";

    value = xmlTextReaderConstValue(reader);

    printf("%d %d %s %d %d",
        xmlTextReaderDepth(reader),
        xmlTextReaderNodeType(reader),
        name,
        xmlTextReaderIsEmptyElement(reader),
        xmlTextReaderHasValue(reader));
    if (value == NULL)
    printf("\n");
    else {
        if (xmlStrlen(value) > 40)
            printf(" %.40s...\n", value);
        else
        printf(" %s\n", value);
    }
}

/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse and print information about an XML file.
 */
static void
streamFile(const char *filename) {
    xmlTextReaderPtr reader;
    int ret;

    reader = xmlReaderForFile(filename, NULL, 0);
    if (reader != NULL) {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) {
            processNode(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        if (ret != 0) {
            fprintf(stderr, "%s : failed to parse\n", filename);
        }
    } else {
        fprintf(stderr, "Unable to open %s\n", filename);
    }
}

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
    else
    {
        for(int argumentIndex = 0; argumentIndex < argc; argumentIndex++)
        {
            printf("Argument %d = \"%s\"\n", argumentIndex, argv[argumentIndex]);
        }
    }

    /*
     * this initialize the library and check potential ABI mismatches
     * between the version it was compiled for and the actual shared
     * library used.
     */
    LIBXML_TEST_VERSION

    streamFile(argv[2]);

    /*
     * Cleanup function for the XML library.
     */
    xmlCleanupParser();
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();

	printf("Hello World, This is the orange PI 5\n");
	return (enErrorCode);
}
