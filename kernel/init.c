#include "init.h"
#include "memory.h"
#include "trap.h"


void init_all(void){
    screen_init();
    sys_vector_init();
    memory_init();
}