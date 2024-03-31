#include "hash_functions.h"

//
#include "esp_attr.h"
#include "sdkconfig.h"

// ----------------------------------------------------------------------
uint32_t IRAM_ATTR __attribute__((optimize("-O2"))) ul_get_hash(const char* str, size_t len)
{
    uint32_t hash = 20883u;

    for (size_t i = 0; i < len; i++)
    {
        hash += str[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}
