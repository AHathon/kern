#pragma once

#include "libraries/types.h"

#define EMMC_BLOCK_SIZE 512

#ifdef __cplusplus
extern "C" {
#endif

int EMMC_Init();
int EMMC_ReadBlocks(uint32_t lba, uint32_t count, uint8_t *buf);

#ifdef __cplusplus
}
#endif