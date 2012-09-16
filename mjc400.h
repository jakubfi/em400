#include <inttypes.h>
#include <stdio.h>

void mjc400_init();
void mjc400_reset();
void mjc400_clear_mem();
void mjc400_fetch_instr();
int16_t mjc400_fetch_data();
int mjc400_step();
int mjc400_execute();
int __mjc400_load_image(const char* fname, uint16_t *ptr, uint16_t len);
int mjc400_load_os_image(const char* fname, uint16_t addr);
int mjc400_load_user_image(const char* fname, unsigned short bank, uint16_t addr);
void mjc400_dump_os_mem();
int16_t mjc400_get_eff_arg();
