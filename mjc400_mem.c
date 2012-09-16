#include <inttypes.h>
#include "mjc400_mem.h"

uint16_t mjc400_os_mem[OS_MEM_BANK_SIZE] = { 0 };
uint16_t mjc400_user_mem[USER_MEM_BANK_COUNT][USER_MEM_BANK_SIZE] = { { 0 } };
