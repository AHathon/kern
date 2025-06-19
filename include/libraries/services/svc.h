#pragma once

#ifdef __cplusplus
extern "C" {
#endif

inline void svcMapMem(void *addr, long size);
inline void svcFreeMem(void *addr);
inline void svcExitProcess();
inline void svcDebug(void);

#ifdef __cplusplus
}
#endif