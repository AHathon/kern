#include "libraries/services/svc.h"

int main() 
{
    uint64_t i = 0xFFFFFFF;
    while(i--) asm volatile("nop");
    return 0;
}
