#pragma once

void svcMapMem(void *addr, long size);
void svcFreeMem(void *addr);
void svcExitProcess();
void svcDebug(void);