/**
 * @copyright 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#pragma once


#ifdef _WIN32
    #if defined STRATA_EXPORT
        #define STRATA_API __declspec(dllexport)
    #elif defined STRATA_IMPORT
        #define STRATA_API __declspec(dllimport)
    #else
        #define STRATA_API
    #endif
#else
    #define STRATA_API
#endif


// todo: make this dependent on release switch
#define STRATA_TEST_API STRATA_API
