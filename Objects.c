// This file contains the struct definitions for the players, bullets and platforms.

#include <stdint.h>

// struct definition for platform

struct plt {
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
};

// struct definiton for bullets

struct bullet {
	int16_t x;
	int16_t y;
	int16_t display;		// 0 don't display, 1 display
};

// struct definition for players

struct plr {
	int16_t xpos;
	int16_t ypos;
	int16_t xorig;
	int16_t yorig;
	uint8_t lives;
	int8_t fDir;				// face direction, positive = right, negative = left
	int8_t mDir;				// move direction, 1 = right, 0 = still, -1 = left
};
