#include "error_flags.h"

static uint8_t error_flags = ERR_NONE;

void error_flags_set(uint8_t err_mask)
{
    error_flags |= err_mask;
}

void error_flags_clear(uint8_t err_mask)
{
    error_flags &= ~err_mask;
}

bool error_flags_is_set(uint8_t err_mask)
{
    return (error_flags & err_mask) != 0;
}

uint8_t error_flags_get(void)
{
    return error_flags;
}

void error_flags_reset(void)
{
    error_flags = ERR_NONE;
}
