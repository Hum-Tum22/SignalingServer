#pragma once


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

int base64_encode(uint8_t dst[], const uint8_t* src, int srcLen);
unsigned char* base64_decode(const char* code0);