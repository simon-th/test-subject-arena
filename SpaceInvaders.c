// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 11/21/2017 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017

 Copyright 2017 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "ADC.h"
#include "gfx.h"
#include "Hardware.h"
#include "sound.h"
#include "Objects.h"
#include "UART.h"

extern uint32_t ADCvalue[4];

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

uint8_t GameOver = 0;
uint8_t Level = 0;
uint8_t hardfault;

//----------------Menus---------------------



void GPIOPortF_Handler(void){
	//pause menu
  GPIO_PORTF_ICR_R = 0x10;      // acknowledge flag4
	ST7735_FillScreen(0x0000);    // black screen
	ST7735_SetCursor(7, 5);
	ST7735_OutString("Paused");
	ST7735_SetCursor(6, 7);
	ST7735_OutString("Press SW2");
	ST7735_SetCursor(6, 8);
	ST7735_OutString("to resume");
	//DisableInterrupts();
	while((GPIO_PORTF_DATA_R & 0x01) == 0x00){
		}
	uint32_t delay = 0;
	while(delay < 80000){
			delay ++;
	  }
	while((GPIO_PORTF_DATA_R & 0x01) == 0x01 ){
	  }
	while(delay < 800000){
			delay ++;
	 }
	//EnableInterrupts();
}

void Select_Level(void) {
 // --UUU-- wait for release; delay for 20ms; and then wait for press
	while(GPIO_PORTF_DATA_R == 0){
		}
	uint32_t delay = 0;
	while(delay < 80000){
			delay ++;
	  }
	while (((GPIO_PORTF_DATA_R & 0x01) == 0x01 ) && ((GPIO_PORTF_DATA_R & 0x10) == 0x10 )){
	  }
	if ((GPIO_PORTF_DATA_R & 0x01) == 0x01 )	{
		Level = 0;
	}
	else if ((GPIO_PORTF_DATA_R & 0x10) == 0x10 )	{
		Level = 1;
	}
} 

void MenuScreen(void){
	ST7735_FillScreen(0x0000);
	ST7735_DrawBitmap(2, 77, splash, 124, 39);
	ST7735_SetCursor(5, 9);
	ST7735_OutString("Normal (SW1)");
	ST7735_SetCursor(6, 12);
	ST7735_OutString("Lite (SW2)");
	Select_Level();
}




// ---------------Game Objects-------------

// struct containing all the platforms

struct plt platform[6] = {
	{21, 31, 30, 6, 0},
	{77, 31, 30, 6, 0},
	{49, 61, 30, 6, 1},
	{21, 91, 30, 6, -1},
	{77, 91, 30, 6, 1},
	{49, 121, 30, 6, 0}
};


// struct containing both players
// player[0] = player 1
// player[1] = player 2

struct bullet p1ammo[5] = {
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0}
};

struct bullet p2ammo[5] = {
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0},
	{0, 0, 0, 1, 0, 0, 0}
};

struct plr player[2] = {
	{30,			// xpos
	14,				// ypos
	30,				// xorig
	14,				// yorig
	12,				// lives
	1,				// fDir
	0,				// mDir
	},
	{86,			// xpos
	131,			// ypos
	86,				// xorig
	131,			// yorig
	12,				// lives
	-1,				// fDir
	0,				// mDir
	}	
};

struct prt portal[1] = {
	{25,66}
//	{14, 135}
};

struct bom bomb[2] = {
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0}
};

// ---------------Game Engine--------------

uint8_t moveSpeed[2] = {2, 2};

void drawPlatform(int16_t x, int16_t y, int16_t w, int16_t h) {
	rectBuffer (x, y, w, h, 0x24);
//	rectBuffer (x+1, y, w-2, 1, 0x2965);
	rectBuffer (x+5, y+h-2, w-10, 1, 0xF6);
}

void drawBackground(void)	{
	rectBuffer (0, 0, 128, 160, 0xb6);
	rectBuffer (11, 64, 2, 21, 0x49);
	rectBuffer (115, 64, 2, 21, 0x49);
	rectBuffer (13, 25, 2, 102, 0x6d);
	rectBuffer (63, 25, 2, 102, 0x6d);
	rectBuffer (113, 25, 2, 102, 0x6d);
	rectBuffer (13, 25, 102, 1, 0x6d);
	rectBuffer (13, 126, 102, 1, 0x6d);
	rectBuffer (15, 83, 48, 2, 0x53);
	rectBuffer (65, 83, 48, 2, 0x53);
	rectBuffer (15, 69, 30, 11, 0x73);
	bitmapBuffer (45, 69, bgMid1, 18, 11);
	bitmapBuffer (65, 69, bgMid2, 48, 11);
	bitmapBuffer (0, 148, bgBottom, 128, 12);
	rectBuffer (0, 0, 128, 10, 0x00);
	rectBuffer (0, 11, 128, 1, 0x24);
	rectBuffer (0, 12, 128, 1, 0x00);
	rectBuffer (0, 13, 1, 135, 0x00);
	rectBuffer (127, 13, 1, 135, 0x00);
}

void drawPortal(int16_t y, int16_t length)	{
	rectBuffer(0, y, 2, length, 0xF6);
	rectBuffer(2, y, 1, length, 0xFF);
	rectBuffer(126, y, 2, length, 0xF6);
	rectBuffer(125, y, 1, length, 0xFF);

}

void setDirection (void) {
	for (uint8_t i = 0; i < 2; i ++) {
		ADC_In(ADCvalue);
		if (ADCvalue[i] < 1365) {
			player[i].mDir = -1;
			player[i].fDir = -1;
		}
		else if (ADCvalue[i] > 2732) {
			player[i].mDir = 1;
			player[i].fDir = 1;
		}
		else {
			player[i].mDir = 0;
		}
	}
}

void playerCollision (void)	{
	if ((((player[0].ypos + 18) >= player[1].ypos) && (player[0].ypos <= player[1].ypos)) || (((player[1].ypos + 18) >= player[0].ypos) && (player[1].ypos <= player[0].ypos)))	{
		if (((player[0].xpos <= player[1].xpos) && ((player[0].xpos + 12) >= player[1].xpos)) || ((player[1].xpos <= player[0].xpos) && ((player[1].xpos + 12) >= player[0].xpos)))	{
			if (player[1].xpos == 116) {
				player[0].xpos = player[1].xpos - 12;
			}
			else if ((player[0].mDir != 0) && (player[1].mDir == 0)){
				if (player[0].xpos < player[1].xpos)	{
					player[0].xpos = player[1].xpos - 12;
				}
				else {
					player[0].xpos = player[1].xpos + 12;
				}
			}
			else if ((player[1].mDir != 0) && (player[0].mDir == 0)){
				if (player[0].xpos < player[1].xpos)	{
					player[1].xpos = player[0].xpos + 12;
				}
				else {
					player[1].xpos = player[0].xpos - 12;
				}
			}
			else {
				if (player[0].xpos < player[1].xpos) {
					player[0].xpos -= 3;
					player[1].xpos += 3;
				}
				else {
					player[1].xpos -= 3;
					player[0].xpos += 3;
				}
			}
		}
	}
}

void movePlayers(void)	{
	for (uint8_t i = 0; i < 2; i ++)	{	
		player[i].xpos = player[i].xpos + moveSpeed[i]*player[i].mDir;
		if (player[i].xpos > 116) {
			if (Level == 0)	{
				if ((player[i].ypos >= portal[0].y) && ((player[i].ypos + 18) <= (portal[0].y + portal[0].length)))	{
					player[i].xpos = 0;
					Sound_Landing();
				}
				else	{
					player[i].xpos = 116;
				}
			}
			else	{
				player[i].xpos = 116;
			}
		}
		else if (player[i].xpos < 0) {
			if (Level == 0)	{
				if ((player[i].ypos >= portal[0].y) && ((player[i].ypos + 18) <= (portal[0].y + portal[0].length)))	{
					player[i].xpos = 116;
					Sound_Landing();
				}
				else	{
					player[i].xpos = 0;
				}
			}
			else	{
				player[i].xpos = 0;
			}
		}
		playerCollision();
	}	
}

void movePlatforms(void)	{
	platform[2].x += 2*platform[2].mDir;
	if ((platform[2].x + platform[2].w) > 113)	{
		platform[2].mDir = -platform[2].mDir;
	}
	else if (platform[2].x < 15)	{
		platform[2].mDir = -platform[2].mDir;
	}
	
	platform[3].x += platform[3].mDir;
	if ((platform[3].x + platform[3].w) > 63)	{
		platform[3].mDir = -platform[3].mDir;
	}
	else if (platform[3].x < 15)	{
		platform[3].mDir = -platform[3].mDir;
	}
	
	platform[4].x += platform[4].mDir;
	if ((platform[4].x + platform[4].w) > 113)	{
		platform[4].mDir = -platform[4].mDir;
	}
	else if (platform[4].x < 65)	{
		platform[4].mDir = -platform[4].mDir;
	}
	
}

void Shoot1(void)	{	
	uint8_t i = 0;
	GPIO_PORTF_DATA_R |= 0xA;
	while (i < 3)	{
		if (p1ammo[i].display == 0)	{
			p1ammo[i].display = 1;
			p1ammo[i].y = player[0].ypos + 8;
			if (player[0].fDir > 0) {
				p1ammo[i].x = player[0].xpos + 12;
				p1ammo[i].dir = 1;
			}
			else {
				p1ammo[i].x = player[0].xpos - 1;
				p1ammo[i].dir = -1;
			}
			p1ammo[i].xorig = p1ammo[i].x;
			p1ammo[i].yorig = p1ammo[i].y;
			Sound_Zap();
			i = 5;
		}
		else {
			i ++;
		}
	}
}

void Shoot2(void)	{	
	uint8_t i = 0;
	GPIO_PORTF_DATA_R |= 0xA;
	while (i < 3)	{
		if (p2ammo[i].display == 0)	{
			p2ammo[i].display = 1;
			p2ammo[i].y = player[1].ypos + 8;
			if (player[1].fDir > 0) {
				p2ammo[i].x = player[1].xpos + 12;
				p2ammo[i].dir = 1;
			}
			else {
				p2ammo[i].x = player[1].xpos - 1;
				p2ammo[i].dir = -1;
			}
			p2ammo[i].xorig = p2ammo[i].x;
			p2ammo[i].yorig = p2ammo[i].y;
			Sound_Zap();
			i = 5;
		}
		else {
			i ++;
		}
	}
}



void drawBullet (void)	{
	for (uint8_t j = 0; j < 3; j ++)	{
		if (p1ammo[j].display == 1)	{
			rectBuffer(p1ammo[j].x, p1ammo[j].y, 2, 2, 0x00);
			if ((p1ammo[j].y == p1ammo[j].yorig) && (p1ammo[j].x == p1ammo[j].xorig) && (p1ammo[j].count == 0))	{
				if (p1ammo[j].dir == 1)	{
					rectBuffer(player[0].xpos + 11, p1ammo[j].y - 1, 1, 4, 0x1F);
					rectBuffer(player[0].xpos + 12, p1ammo[j].y, 1, 2, 0x1F);
					rectBuffer(player[0].xpos + 10, p1ammo[j].y, 1, 2, 0x1F);
				}
				else if (p1ammo[j].dir == -1)	{
					rectBuffer(player[0].xpos, p1ammo[j].y - 1, 1, 4, 0x1F);
					rectBuffer(player[0].xpos - 1, p1ammo[j].y, 1, 2, 0x1F);
					rectBuffer(player[0].xpos + 1, p1ammo[j].y, 1, 2, 0x1F);
				}
			}
			p1ammo[j].x = p1ammo[j].x + (p1ammo[j].dir * 6);
			if (p1ammo[j].x > 128) {
				if (Level == 0)	{
					if ((p1ammo[j].y >= portal[0].y) && (p1ammo[j].y < (portal[0].y + portal[0].length)) && ((p1ammo[j].count) < 2))	{
						p1ammo[j].x = 0;
						p1ammo[j].count ++;
					}
					else	{
						p1ammo[j].display = 0;
						p1ammo[j].count = 0;
					}
				}
				else {
					p1ammo[j].display = 0;
				}
			}
			else if (p1ammo[j].x < 0) {
				if (Level == 0)	{
					if ((p1ammo[j].y >= portal[0].y) && (p1ammo[j].y < (portal[0].y + portal[0].length)) && ((p1ammo[j].count) < 2))	{
						p1ammo[j].x = 127;
						p1ammo[j].count ++;
					}
					else	{
						p1ammo[j].display = 0;
						p1ammo[j].count = 0;
					}
				}
				else {
					p1ammo[j].display = 0;
				}
			}
			for (uint8_t k = 0; k < 6; k++)	{
				if ((p1ammo[j].y >= platform[k].y) && (p1ammo[j].y < (platform[k].y + platform[k].h)) && (p1ammo[j].x >= platform[k].x) && (p1ammo[j].x < (platform[k].x + platform[k].w)))	{
					p1ammo[j].display = 0;
					p1ammo[j].count = 0;
				}
			}
			for (uint8_t k = 0; k < 2; k ++) {
				if ((p1ammo[j].x >= player[k].xpos) && (p1ammo[j].x <= (player[k].xpos + 12)) && (p1ammo[j].y >= player[k].ypos) && (p1ammo[j].y <= (player[k].ypos + 18))) {
					if (player[k].lives > 0) {
						player[k].lives --;
						p1ammo[j].display = 0;
						p1ammo[j].count = 0;
					}
					Sound_Explosion();
				}
			}
		}
	}
	for (uint8_t j = 0; j < 3; j ++)	{
		if (p2ammo[j].display == 1)	{
			rectBuffer(p2ammo[j].x, p2ammo[j].y, 2, 2, 0x00);
			if ((p2ammo[j].y == p2ammo[j].yorig) && (p2ammo[j].x == p2ammo[j].xorig) && (p2ammo[j].count == 0))	{
				if (p2ammo[j].dir == 1)	{
					rectBuffer(player[1].xpos + 11, p2ammo[j].y - 1, 1, 4, 0x1F);
					rectBuffer(player[1].xpos + 12, p2ammo[j].y, 1, 2, 0x1F);
					rectBuffer(player[1].xpos + 10, p2ammo[j].y, 1, 2, 0x1F);
				}
				else if (p2ammo[j].dir == -1)	{
					rectBuffer(player[1].xpos, p2ammo[j].y - 1, 1, 4, 0x1F);
					rectBuffer(player[1].xpos - 1, p2ammo[j].y, 1, 2, 0x1F);
					rectBuffer(player[1].xpos + 1, p2ammo[j].y, 1, 2, 0x1F);
				}
			}
			p2ammo[j].x = p2ammo[j].x + (p2ammo[j].dir * 6);
			if (p2ammo[j].x > 128) {
				if (Level == 0)	{
					if ((p2ammo[j].y >= portal[0].y) && (p2ammo[j].y < (portal[0].y + portal[0].length)) && ((p2ammo[j].count) < 2))	{
						p2ammo[j].x = 0;
						p2ammo[j].count ++;
					}
					else	{
						p2ammo[j].display = 0;
						p2ammo[j].count = 0;
					}
				}
				else {
					p2ammo[j].display = 0;
				}
			}
			else if (p2ammo[j].x < 0) {
				if (Level == 0)	{
					if ((p2ammo[j].y >= portal[0].y) && (p2ammo[j].y < (portal[0].y + portal[0].length)) && ((p2ammo[j].count) < 2))	{
						p2ammo[j].x = 127;
						p2ammo[j].count ++;
					}
					else	{
						p2ammo[j].display = 0;
						p2ammo[j].count = 0;
					}
				}
				else {
					p2ammo[j].display = 0;
				}
			}
			for (uint8_t k = 0; k < 6; k++)	{
				if ((p2ammo[j].y >= platform[k].y) && (p2ammo[j].y < (platform[k].y + platform[k].h)) && (p2ammo[j].x >= platform[k].x) && (p2ammo[j].x < (platform[k].x + platform[k].w)))	{
					p2ammo[j].display = 0;
					p2ammo[j].count = 0;
				}
			}
			for (uint8_t k = 0; k < 2; k ++) {
				if ((p2ammo[j].x >= player[k].xpos) && (p2ammo[j].x <= (player[k].xpos + 12)) && (p2ammo[j].y >= player[k].ypos) && (p2ammo[j].y <= (player[k].ypos + 18))) {
					if (player[k].lives > 0) {
						player[k].lives --;
						p2ammo[j].display = 0;
						p2ammo[j].count = 0;
					}
					Sound_Explosion();
				}
			}
		}
	}
}

void drawLifeBars (void) 	{
	uint8_t x = 125;
	for (uint8_t i = 0; i < player[1].lives; i++)	{
		rectBuffer (x, 3, 2, 5, 0xff);
		x -= 3;
	}
	x = 2;
	for (uint8_t i = 0; i < player[0].lives; i++)	{
		rectBuffer (x, 3, 2, 5, 0xff);
		x += 3;
	}
}




uint8_t gravityArray[9] = {1, 1, 1, 2, 2, 3, 3, 4, 4};
uint8_t gravityIndex[2] = {0,0};
uint8_t gravityIndexBomb[2] = {0,0};
uint8_t jumpArray[14] = {0, 5, 5, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1};
uint8_t jumpIndex[2] = {0,0};
uint8_t jumpStatus[2] = {0,0};

void detectPlatformBomb (void) {
	for (uint8_t p = 0; p < 2; p ++)	{
		for (uint8_t n = 0; n < 6; n ++) {
			if (((bomb[p].x + 8) > platform[n].x) && ((bomb[p].x) < (platform[n].x + platform[n].w))) {
				if (((bomb[p].y + 8) > platform[n].y) && ((bomb[p].y + 8) < (platform[n].y + platform[n].h))) {
					bomb[p].y = platform[n].y - 8;
					bomb[p].hity = bomb[p].y - 9;
					bomb[p].yorig = bomb[p].y;
					gravityIndexBomb[p] = 0;
				}
//				else if ((bomb[p].y > platform[n].y) && (bomb[p].y < (platform[n].y + platform[n].h))) {
//					bomb[p].y = platform[n].y + platform[n].h;
//				}
			}
			else	{
				bomb[p].yorig = 141;
			}
		}
		if (bomb[p].y < 13)	{
			bomb[p].y = 13;
			bomb[p].hity = bomb[p].y - 9;
		}
		if (bomb[p].y >= 141)	{
			bomb[p].y = 141;
			bomb[p].hity = bomb[p].y - 9;
			bomb[p].yorig = bomb[p].y;
			gravityIndexBomb[p] = 0;
		}
	}
}


void detectPlatform (void) {
	for (uint8_t p = 0; p < 2; p ++)	{
		for (uint8_t n = 0; n < 6; n ++) {
			if (((player[p].xpos + 11) > platform[n].x) && ((player[p].xpos + 1) < (platform[n].x + platform[n].w))) {
				if (((player[p].ypos + 17) > platform[n].y) && ((player[p].ypos + 17) < (platform[n].y + platform[n].h)) && (jumpIndex[p] == 0)) {
					player[p].ypos = platform[n].y - 17;
					player[p].yorig = player[p].ypos;
					moveSpeed[p] = 2;
					gravityIndex[p] = 0;
					jumpStatus[p] = 0;
					jumpIndex[p] = 0;
				}
				else if ((player[p].ypos > platform[n].y) && (player[p].ypos < (platform[n].y + platform[n].h))) {
					player[p].ypos = platform[n].y + platform[n].h;
					jumpIndex[p] = 0;
			//		jumpStatus[p] = 0;
				}
			}
			else	{
				player[p].yorig = 131;
				//jumpIndex[p] = 0;
			}
		}
		if (player[p].ypos < 13)	{
			player[p].ypos = 13;
			jumpIndex[p] = 0;
		}
		if (player[p].ypos >= 131)	{
			player[p].ypos = 131;
			player[p].yorig = player[p].ypos;
			moveSpeed[p] = 2;
			jumpStatus[p] = 0;
			jumpIndex[p] = 0;
			gravityIndex[p] = 0;
		}
	}
}	


uint8_t drawIdx1 = 0;
uint8_t drawIdx2 = 0;

void drawPlayers (void)	{
	if (player[0].fDir < 0) {
		if (player[0].mDir == 0)	{
			bitmapBuffer(player[0].xpos, player[0].ypos, Player1left[1], 12, 18);
		}
		else {
			drawIdx1 = (drawIdx1 + 1)%2;
			bitmapBuffer(player[0].xpos, player[0].ypos, Player1left[drawIdx1], 12, 18);			
		}
	}
	else {
		if (player[0].mDir == 0)	{
			bitmapBuffer(player[0].xpos, player[0].ypos, Player1right[1], 12, 18);
		}
		else {
			drawIdx1 = (drawIdx1 + 1)%2;
			bitmapBuffer(player[0].xpos, player[0].ypos, Player1right[drawIdx1], 12, 18);			
		}
	}
	
	if (player[1].fDir < 0) {
		if (player[1].mDir == 0)	{
			bitmapBuffer(player[1].xpos, player[1].ypos, Player2left[1], 12, 18);
		}
		else {
			drawIdx2 = (drawIdx2 + 1)%2;
			bitmapBuffer(player[1].xpos, player[1].ypos, Player2left[drawIdx2], 12, 18);			
		}
	}
	else {
		if (player[1].mDir == 0)	{
			bitmapBuffer(player[1].xpos, player[1].ypos, Player2right[1], 12, 18);
		}
		else {
			drawIdx2 = (drawIdx2 + 1)%2;
			bitmapBuffer(player[1].xpos, player[1].ypos, Player2right[drawIdx2], 12, 18);			
		}
	}
	
}

void GravityBomb (void)	{
	for (uint8_t p = 0; p < 2; p ++) 	{
		if (bomb[p].y < bomb[p].yorig)	{
			if (gravityIndexBomb[p] < 9)	{
					bomb[p].y += gravityArray[gravityIndexBomb[p]];
					bomb[p].hity = bomb[p].y - 9;
					gravityIndexBomb[p] ++;
				}
			else	{
				bomb[p].y += 5;
				bomb[p].hity = bomb[p].y - 9;
			}
		}
	}
}

void Gravity (void)	{
	for (uint8_t p = 0; p < 2; p ++) 	{
		if (jumpIndex[p] == 0){
			if (player[p].ypos < player[p].yorig)	{
				//moveSpeed[p] = 1;
				jumpStatus[p] = 1;
				if (gravityIndex[p] < 9)	{
					player[p].ypos += gravityArray[gravityIndex[p]];
					gravityIndex[p] ++;
				}
				else	{
					player[p].ypos += 5;
				}
			}
		}
		else {
			//moveSpeed[p] = 1;
			player[p].ypos -= jumpArray[jumpIndex[p]];
			jumpIndex[p] ++;
			if (jumpIndex[p] >= 14)	{
				jumpIndex[p] = 0;
			}
		}
	}
}

uint8_t Countdown[2] = {0, 0};

void explodeBomb1(void)	{
	Countdown[0] ++;
	Sound_Countdown();
	if (Countdown[0] == 3)	{
		TIMER2_CTL_R = 0x00000000;
		Countdown[0] = 0;
		bomb[0].display = 0;
		bomb[0].explode = 1;
		for (uint8_t i = 0; i < 9; i ++)	{
			Sound_Explosion();
		}
		for (uint8_t i = 0; i < 2; i ++)	{
			if (((player[i].xpos + 11) > (bomb[0].hitx)) && ((player[i].xpos + 11) < (bomb[0].hitx + 26)))	{
				if ((((player[i].ypos + 17) > (bomb[0].hity)) && ((player[i].ypos + 17) < (bomb[0].hity + 26))) || (((player[i].ypos) < (bomb[0].hity + 26)) && ((player[i].ypos) > (bomb[0].hity))))	{
					if (player[i].lives < 5)	{
						player[i].lives = 0;
					}
					else {
						player[i].lives -= 5;
					}
				}
			}
			else if (((player[i].xpos) < (bomb[0].hitx + 26)) && ((player[i].xpos) > (bomb[0].hitx)))	{
				if ((((player[i].ypos + 17) > (bomb[0].hity)) && ((player[i].ypos + 17) < (bomb[0].hity + 26))) || (((player[i].ypos) < (bomb[0].hity + 26)) && ((player[i].ypos) > (bomb[0].hity))))	{
					if (player[i].lives < 5)	{
						player[i].lives = 0;
					}
					else {
						player[i].lives -= 5;
					}
				}
			}
		}
	}
};

void explodeBomb2(void)	{
	Countdown[1] ++;
	Sound_Countdown();
	if (Countdown[1] == 3)	{
		TIMER3_CTL_R = 0x00000000;
		Countdown[1] = 0;
		bomb[1].display = 0;
		bomb[1].explode = 1;
		for (uint8_t i = 0; i < 9; i ++)	{
			Sound_Explosion();
		}
		for (uint8_t i = 0; i < 2; i ++)	{
			if (((player[i].xpos + 11) > (bomb[1].hitx)) && ((player[i].xpos + 11) < (bomb[1].hitx + 26)))	{
				if ((((player[i].ypos + 17) > (bomb[1].hity)) && ((player[i].ypos + 17) < (bomb[1].hity + 26))) || (((player[i].ypos) < (bomb[1].hity + 26)) && ((player[i].ypos) > (bomb[1].hity))))	{
					if (player[i].lives < 5)	{
						player[i].lives = 0;
					}
					else {
						player[i].lives -= 5;
					}
				}
			}
			else if (((player[i].xpos) < (bomb[1].hitx + 26)) && ((player[i].xpos) > (bomb[1].hitx)))	{
				if ((((player[i].ypos + 17) > (bomb[1].hity)) && ((player[i].ypos + 17) < (bomb[1].hity + 26))) || (((player[i].ypos) < (bomb[1].hity + 26)) && ((player[i].ypos) > (bomb[1].hity))))	{
					if (player[i].lives < 5)	{
						player[i].lives = 0;
					}
					else {
						player[i].lives -= 5;
					}
				}
			}
		}
	}
};

void plantBomb(uint8_t i)	{
	if (bomb[i].display == 0)	{
		bomb[i].x = player[i].xpos;
		bomb[i].y = player[i].ypos + 4;
		bomb[i].yorig = 141;
		bomb[i].explode = 0;
		bomb[i].display = 1;
		bomb[i].hitx = bomb[i].x - 9;
		bomb[i].hity = bomb[i].y - 9;
	}
	if (i == 0)	{
		Timer2_Init(&explodeBomb1, 0x4C4B400);
	}
	else if (i == 1)	{
		Timer3_Init(&explodeBomb2, 0x4C4B400);
	}
};

void drawBomb(void)	{
	for (uint8_t i = 0; i < 2; i ++)	{
		if (bomb[i].display == 1)	{
			bitmapBuffer(bomb[i].x, bomb[i].y, bombSp, 8, 8);
		}
	}
};

	
void Jump1 (void)	{
	if (jumpStatus[0] == 0)	{
		jumpStatus[0] = 1;
		jumpIndex[0] = 1;
		Sound_Jump();
		TIMER0_CTL_R = 0x00000000;
	}
	else	{
		TIMER0_CTL_R = 0x00000000;
	}
}

void Jump2 (void)	{
	if (jumpStatus[1] == 0)	{
		jumpStatus[1] = 1;
		jumpIndex[1] = 1;
		Sound_Jump();
		TIMER1_CTL_R = 0x00000000;
	}
	else	{
		TIMER1_CTL_R = 0x00000000;
	}
}

void GPIOPortB_Handler(void){
  GPIO_PORTB_ICR_R = 0x10;      // acknowledge flag3
	plantBomb(0);
}

void GPIOPortA_Handler(void){
  GPIO_PORTA_ICR_R = 0x10;      // acknowledge flag3
  plantBomb(1);
}


void GPIOPortE_Handler(void){
  GPIO_PORTE_ICR_R = 0x8;      // acknowledge flag3
  Shoot1();
}

void GPIOPortC_Handler(void){
  GPIO_PORTC_ICR_R = 0x10;      // acknowledge flag4
  Shoot2();
}

//------------------Main Game Engine Function------------------
	
void drawFrame(void) {	
//	uint32_t player1switch = GPIO_PORTB_DATA_R;
//	uint32_t player2switch = GPIO_PORTA_DATA_R;
	drawBackground();
	setDirection();
	movePlayers();
	if (ADCvalue[2] < 910) {
		Timer0_Init(&Jump1, 3125000);
	}
	if (ADCvalue[3] < 910) {
		Timer1_Init(&Jump2, 3125000);
	}
	Gravity();
	GravityBomb();
	if (Level == 0)	{
		drawPortal(portal[0].y, portal[0].length);
	}
	if (Level == 0)	{
		movePlatforms();
	}
	detectPlatform();
	detectPlatformBomb();
	drawLifeBars();
	for (uint8_t i = 0; i < 6; i ++)	{
		drawPlatform(platform[i].x, platform[i].y, platform[i].w, platform[i].h);
	}
	drawPlayers();
	drawBomb();
	drawBullet();
	for (uint8_t i = 0; i < 2; i ++ )	{	
		if ((bomb[i].explode >= 1) && (bomb[i].explode < 3))	{
			bitmapBuffer(bomb[i].hitx + 1, bomb[i].hity + 2, boom, 22, 23); 
			bomb[i].explode ++;
			GPIO_PORTF_DATA_R |= 0xA;
		}
		else if ((bomb[i].explode <= 4) && (bomb[i].explode >= 3))	{
			rectBuffer(0, 0, 128, 160, 0xff);
			bomb[i].explode ++;
			GPIO_PORTF_DATA_R |= 0xE;
		}
		else {
			bomb[i].explode = 0;
		}
	}
	
	ST7735_DrawBitmap (0, 159, buffer, 128, 160);
	GPIO_PORTF_DATA_R &= ~0xE;
	if ((player[0].lives == 0) || (player[1].lives == 0)) {
		GameOver = 1;
		Sound_Gameover();
	}
	hardfault = 13;
}

//-----------------------------------------


int main(void){
  PLL_Init(Bus80MHz);
//	SysTick_Init();
	ADC_Init();
	Button_Init();
	Sound_Init();
	ST7735_InitR(INITR_REDTAB);
	MenuScreen();
	uint32_t delay = 0;
	while(delay < 8000000){
			delay ++;
	  }
	EdgeTriggeredPortA_Init();
	EdgeTriggeredPortB_Init();
	EdgeTriggeredPortC_Init();
	EdgeTriggeredPortE_Init();
	EdgeTriggeredPortF_Init();
	while (GameOver == 0) {
		drawFrame();	
	}
	
	ST7735_FillScreen(0x00);
	ST7735_SetCursor(6, 5);
	ST7735_OutString("Game Over");
	ST7735_SetCursor(4, 7);
	if (player[0].lives == 0)	{
		if (player[1].lives == 0)	{
			ST7735_OutString("  Game Tied");
		}
		else {
			ST7735_OutString("Player 2 Wins");
		}
	}
	else if (player[1].lives == 0)	{
		ST7735_OutString("Player 1 Wins");
	}
}

void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 363620;  // 0.025sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
