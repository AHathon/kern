#pragma once

#define NR_SYSCALLS 3

#define EC_UNKNOWN              0b000000
#define EC_TRAPPED_WF           0b000001
#define EC_ILLEGAL_EXECUTION    0b001110
#define EC_SVC_32               0b010001
#define EC_SVC_64               0b010101
#define EC_DATA_ABORT_NC_EL     0b100101