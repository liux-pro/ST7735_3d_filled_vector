#pragma once

#include "stdint.h"

/**
 * this function will be called once at the program starting.
 */
void setup();

/**
 * this function will be called period 60 times per second.
 * @param buffer a pointer to frame buffer.
 */
void loop(uint8_t *buffer);
