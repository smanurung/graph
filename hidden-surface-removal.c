#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>

/*
#include "rectangle.h"
*/

#define	VIDEO_INT			0x10
#define SET_MODE			0x00
#define VGA_256_COLOR_MODE	0x13
#define TEXT_MODE			0x03

#define	SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define NUM_COLORS			256
#define DEFAULT_COLOR		175

#define sgn(x) ((x<0)?-1:((x>0)?1:0))

typedef unsigned char byte;
typedef unsigned short word;

typedef struct
{
	int up;
	int down;
	int right;
	int left;
} Rectangle;

//global variables
int KOTAK_ATAS = 50;
int KOTAK_BAWAH = 120;
int KOTAK_KIRI = 100;
int KOTAK_KANAN = 170;

int DEPTH_KOTAK = 1;
int DEPTH_SEGITIGA = 2;

byte *VGA=(byte *)0xA0000000L;
word *my_clock=(word *)0x0000046C;

void set_mode(byte mode){
	union REGS regs;
	
	regs.h.ah = SET_MODE;
	regs.h.al = mode;
	int86(VIDEO_INT, &regs, &regs);
}

void plot_pixel(int x, int y, byte color){
	VGA[(y<<8)+(y<<6)+x] = color;
}

void line_bresenham(int x1, int y1, int x2, int y2, byte color){
	int i,dx,dy,sdx,sdy,dxabs,dyabs,x,y,px,py;
	
	dx=x2-x1;
	dy=y2-y1;
	dxabs=abs(dx);
	dyabs=abs(dy);
	sdx=sgn(dx);
	sdy=sgn(dy);
	x=dyabs>>1;
	y=dxabs>>1;
	px=x1;
	py=y1;
	
	VGA[(py<<8)+(py<<6)+px]=color;
	
	if(dxabs>=dyabs){
		for(i=0;i<dxabs;i++){
			y+=dyabs;
			if(y>=dxabs){
				y-=dxabs;
				py+=sdy;
			}
			px+=sdx;
			plot_pixel(px,py,color);		
		}
	} else {
		for(i=0;i<dyabs;i++){
			x+=dxabs;
			if(x>=dyabs){
				x-=dyabs;
				px+=sdx;
			}
			py+=sdy;
			plot_pixel(px,py,color);
		}
	}
}

int findRegion(int x, int y){
	int code = 0;
	
	if(y<KOTAK_ATAS) code |= 1;
	else if(y>KOTAK_BAWAH) code |= 2;
	
	if(x>KOTAK_KANAN) code |= 4;
	else if(x<KOTAK_KIRI) code |= 8;
	
	return code;
}

int clipLine(int x1, int y1, int x2, int y2, int * x3, int * y3, int * x4, int * y4) {
	int code1, code2, codeout;
	int accept=0, done=0;
		
	code1 = findRegion(x1,y1);
	code2 = findRegion(x2,y2);
	
	do {
		if(!(code1 | code2)) accept = done = 1; //trivial accept
		else if(code1 & code2) done = 1; //trivial reject
		else {
			int x,y;
						
			codeout = code1 ? code1 : code2;
			if(codeout & 1) { //top
				x = x1 + (x2-x1) * (KOTAK_ATAS-y1)/(y2-y1);
				y = KOTAK_ATAS;
			} else if(codeout & 2) { //bottom
				x = x1 + (x2-x1) * (KOTAK_BAWAH-y1)/(y2-y1);
				y = KOTAK_BAWAH;
			} else if(codeout & 4) { //right
				y = y1 + (y2-y1) * (KOTAK_KANAN-x1)/(x2-x1);
				x = KOTAK_KANAN;
			} else { //left
				y = y1 + (y2-y1) * (KOTAK_KIRI-x1)/(x2-x1);
				x = KOTAK_KIRI;
			}
			
			if(codeout == code1){
				x1 = x;
				y1 = y;
				code1 = findRegion(x1,y1);
			} else {
				x2 = x;
				y2 = y;
				code2 = findRegion(x2,y2);
			}
		}
	} while(done == 0);
	
	if(accept) {
		(*x3) = x1;
		(*x4) = x2;
		(*y3) = y1;
		(*y4) = y2;
	} else { //should never get into this
		(*x3) = (*x4) = (*y3) = (*y4) = 0;
	}
	return accept;
}

void main(){
	int color;
	Rectangle r;
	
	color=DEFAULT_COLOR;
	
	set_mode(VGA_256_COLOR_MODE);
	
//	membuat view (kotak)
	line_bresenham(KOTAK_KIRI,KOTAK_ATAS,KOTAK_KANAN,KOTAK_ATAS,color);
	line_bresenham(KOTAK_KANAN,KOTAK_ATAS,KOTAK_KANAN,KOTAK_BAWAH,color);
	line_bresenham(KOTAK_KANAN,KOTAK_BAWAH,KOTAK_KIRI,KOTAK_BAWAH,color);
	line_bresenham(KOTAK_KIRI,KOTAK_BAWAH,KOTAK_KIRI,KOTAK_ATAS,color);

//	membuat view (segitiga)
	line_bresenham(190,30,120,150,color);
	line_bresenham(120,150,220,120,color);
	line_bresenham(220,120,190,30,color);

//	resolusi algoritma scan line
	
		
	sleep(2);	
	set_mode(TEXT_MODE);
	return;
}
