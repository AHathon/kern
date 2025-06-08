#pragma once

#include "libraries/types.h"

extern volatile uint32_t mbox[36];

enum 
{
    MBOX_REQUEST  = 0
};

//Mailbox channels
typedef enum 
{
    CH_POWER = 0,
    CH_FB    = 1,
    CH_VUART = 2,
    CH_VCHIQ = 3,
    CH_LEDS  = 4,
    CH_BTNS  = 5,
    CH_TOUCH = 6,
    CH_COUNT = 7,
    CH_PROP  = 8
} MailboxChannel;

//Mailbox tags
typedef enum 
{
    TAG_SETPHYWH       = 0x48003,
    TAG_SETVIRTWH      = 0x48004,
    TAG_SETVIRTOFF     = 0x48009,
    TAG_SETDEPTH       = 0x48005,
    TAG_SETPXLORDR     = 0x48006,
    TAG_GETFB          = 0x40001,
    TAG_GETPITCH       = 0x40008,

    TAG_LAST           = 0
} MailboxTag;

void Mailbox_Append(MailboxTag tag, ...);
uint32_t Mailbox_Call(uint8_t c);