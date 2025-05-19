#pragma once

#include "kCodeSection.h"

typedef struct {
    kCodeSection text;
    kCodeSection data;
    kCodeSection rodata;
} kCodeSet;