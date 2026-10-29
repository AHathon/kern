#pragma once

#define SVC_OK                  0
#define SVC_ERR_INVALID_ADDR    -1
#define SVC_ERR_INVALID_SIZE    -2
#define SVC_ERR_ALREADY_MAPPED  -3
#define SVC_ERR_OUT_OF_MEMORY   -4
#define SVC_ERR_PROC_LIMIT      -5

#ifdef __cplusplus
extern "C" {
#endif

long svcMapMem(void *addr, long size);
inline void svcFreeMem(void *addr);
inline void svcExitProcess();
long svcCreateProcess(const char *name, const void *code, long size);
inline void svcDebug(void);

#ifdef __cplusplus
}
#endif