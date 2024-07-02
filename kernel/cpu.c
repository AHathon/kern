#include "kernel/cpu.h"

void cpuSleep() {
	asm volatile ("wfi");
}