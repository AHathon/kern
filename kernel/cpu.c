#include "libraries/cpu.h"

void cpuSleep() {
	asm volatile ("wfi");
}