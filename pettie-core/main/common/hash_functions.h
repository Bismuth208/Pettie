#ifndef _HASH_FUNCTIONS_H
#define _HASH_FUNCTIONS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>

    // ----------------------------------------------------------------------
    // Accessors functions

    uint32_t ul_get_hash(const char* str, size_t len);

    // ----------------------------------------------------------------------
    // Core functions


#ifdef __cplusplus
}
#endif

#endif // _HASH_FUNCTIONS_H