#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>

#define	VIDEO_INT			0x10
#define SET_MODE			0x00
#define VGA_256_COLOR_MODE	0x13
#define TEXT_MODE			0x03

#define	SCREEN_WIDTH		320
#define SCREEN_HEIGHT		200
#define NUM_COLORS			256

#define sgn(x) ((x<0)?-1:((x>0)?1:0))

typedef unsigned char byte;
typedef unsigned short word;

//global variables
int ATAS = 50;
int BAWAH = 120;
int KIRI = 100;
int KANAN = 170;

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
	
	if(y<ATAS) code |= 1;
	else if(y>BAWAH) code |= 2;
	
	if(x>KANAN) code |= 4;
	else if(x<KIRI) code |= 8;
	
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
				x = x1 + (x2-x1) * (ATAS-y1)/(y2-y1);
				y = ATAS;
			} else if(codeout & 2) { //bottom
				x = x1 + (x2-x1) * (BAWAH-y1)/(y2-y1);
				y = BAWAH;
			} else if(codeout & 4) { //right
				y = y1 + (y2-y1) * (KANAN-x1)/(x2-x1);
				x = KANAN;
			} else { //left
				y = y1 + (y2-y1) * (KIRI-x1)/(x2-x1);
				x = KIRI;
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
	int clip1X, clip1Y, clip2X, clip2Y;
	int end1X, end1Y, end2X, end2Y, retval;
	
	srand(*my_clock);
	set_mode(VGA_256_COLOR_MODE);
	
//	color=rand()%NUM_COLORS;
	color=175;
	
//	membuat view (kotak)
	line_bresenham(KIRI,ATAS,KANAN,ATAS,color);
	line_bresenham(KANAN,ATAS,KANAN,BAWAH,color);
	line_bresenham(KANAN,BAWAH,KIRI,BAWAH,color);
	line_bresenham(KIRI,BAWAH,KIRI,ATAS,color);
	
//	garis pemotong 1
	clip1X = clip1Y = clip2X = clip2Y = 0;
	
	end1X = 130; end1Y = 30; end2X = 150; end2Y = 150;
	line_bresenham(end1X,end1Y,end2X,end2Y,color);
	retval = clipLine(end1X,end1Y,end2X,end2Y,&clip1X,&clip1Y,&clip2X,&clip2Y);
	if(retval) {
		line_bresenham(clip1X,clip1Y,clip2X,clip2Y,color-99);
	}
	
//	garis pemotong 2
	end1X = 50; end1Y = 90; end2X = 215; end2Y = 90;
	line_bresenham(end1X,end1Y,end2X,end2Y,color);
	retval = clipLine(end1X,end1Y,end2X,end2Y,&clip1X,&clip1Y,&clip2X,&clip2Y);
	if(retval) {
		line_bresenham(clip1X,clip1Y,clip2X,clip2Y,color-99);
	}
	
//	garis tak memotong
	end1X = 200; end1Y = 15; end2X = 200; end2Y = 170;
	line_bresenham(end1X,end1Y,end2X,end2Y,color);
	retval = clipLine(end1X,end1Y,end2X,end2Y,&clip1X,&clip1Y,&clip2X,&clip2Y);
	if(retval) {
		line_bresenham(clip1X,clip1Y,clip2X,clip2Y,color-99);
	}
	
//	garis memotong di dalam (trivial accept)
	end1X = KANAN; end1Y = ATAS; end2X = KIRI; end2Y = BAWAH;
	line_bresenham(end1X,end1Y,end2X,end2Y,color);
	retval = clipLine(end1X,end1Y,end2X,end2Y,&clip1X,&clip1Y,&clip2X,&clip2Y);
	if(retval) {
		line_bresenham(clip1X,clip1Y,clip2X,clip2Y,color-99);
	}
	
	sleep(2);	
	set_mode(TEXT_MODE);
	return;
}
