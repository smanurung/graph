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
	int bottom;
	int right;
	int left;
	int depth; //urutan bidang tumpuk
} Rectangle;

//global variables
/*
int ATAS_1 = 50;
int BAWAH_1 = 120;
int KIRI_1 = 100;
int KANAN_1 = 170;
*/

/*
int ATAS_2 = 80;
int BAWAH_2 = 150;
int KIRI_2 = 130;
int KANAN_2 = 200;
*/

byte *VGA=(byte *)0xA0000000L;

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

int findRegion(Rectangle R,int x, int y){
	int code = 0;
	
	if(y<R.up) code |= 1;
	else if(y>R.bottom) code |= 2;
	
	if(x>R.right) code |= 4;
	else if(x<R.left) code |= 8;
	
	return code;
}

/*
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
				x = x1 + (x2-x1) * (ATAS_1-y1)/(y2-y1);
				y = ATAS_1;
			} else if(codeout & 2) { //bottom
				x = x1 + (x2-x1) * (BAWAH_1-y1)/(y2-y1);
				y = BAWAH_1;
			} else if(codeout & 4) { //right
				y = y1 + (y2-y1) * (KANAN_1-x1)/(x2-x1);
				x = KANAN_1;
			} else { //left
				y = y1 + (y2-y1) * (KIRI_1-x1)/(x2-x1);
				x = KIRI_1;
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
*/

/*
 * melakukan pengisian titik pada area
 * minx		: nilai sumbu-x terkecil pada bidang
 * miny		: nilai sumbu-y terkecil pada bidang
 * maxx		: nilai sumbu-x terbesar pada bidang
 * maxy		: nilai sumbu-y terbesar pada bidang
 * color	: warna garis pembatas bidang
 */
void scanline(int minx, int miny, int maxx, int maxy, byte color) {
	int x, y, idx, counter, xtemp;
	int arr[500]; //maks 500 titik potong
	int awalV = 1, firstV, lastV;
	int fillcol = (color+200)%NUM_COLORS;
	
	for(y=miny;y<=maxy;y++){
//		kosongkan array
		idx=0;
		for(x=minx;x<=maxx;x++){
//			temukan seluruh titik potong
			if(isTitik(x,y,color)){
				int pot = isPotong(x,y,color);
				
				if(pot==0) {
//					do nothing
				} else if(pot==1){
//					masukan ke dlm array
					arr[idx++]=x;
				} else if(pot==2){
					int a = isTitik(x-1, y-1, color);
				    int b = isTitik(x, y-1, color);
				    int c = isTitik(x+1, y-1, color);
					
					if(awalV) {
						if(a || b || c) firstV=1;
						else firstV=0;
						awalV=0;
					} else {
						if(a || b || c) lastV=1;
						else lastV=0;
						awalV=1;
						
						if (firstV == lastV) {
//							do nothing
						} else {
//							masukan dalam array
							arr[idx++]=x;
						}
					}
				} else if(pot==3){
//					do nothing
				}
			}
		}
//		warnai titik antarbatas
		counter = 0;
		while(counter<idx){
			xtemp = arr[counter];
			while(xtemp<arr[counter+1]){
//				uji titik batas
				if (isTitik(xtemp,y,color)) {
//					do nothing
				} else {
					plot_pixel(xtemp,y,fillcol);
				}
				xtemp++;
			}
			counter += 2;
		}
	}
}

/*
 * prekondisi: x,y adalah titik batas
 */
int isPotong(int x,int y,byte color){
	int a = isTitik(x-1, y-1, color);
    int b = isTitik(x, y-1, color);
    int c = isTitik(x+1, y-1, color);
    int d = isTitik(x-1, y, color);
    int e = isTitik(x+1, y, color);
	
	if(d && e) return 3;
	else if(d || e) return 2;
	else if(a) {
		if(b || c) return 0;
		else return 1;
	} else if(b){
		if(c) return 0;
		else return 1;
	} else if(c) return 1;
	else return 0;
}

int isTitik(int x, int y,byte col){
	return (VGA[(y<<8)+(y<<6)+x]==col);
}

void drawRectangle(Rectangle r, int color)
{
	line_bresenham(r.left,r.up,r.right,r.up,color);
	line_bresenham(r.right,r.up,r.right,r.bottom,color);
	line_bresenham(r.right,r.bottom,r.left,r.bottom,color);
	line_bresenham(r.left,r.bottom,r.left,r.up,color);

	scanline(r.left,r.up,r.right,r.bottom,color);

}

int main(void){
	Rectangle r1,r2;
	int color;
	int colorDiff;	
	
	color=DEFAULT_COLOR;	
	set_mode(VGA_256_COLOR_MODE);
	
//	representasi kotak
	r1.up = 50;
	r1.bottom = 120;
	r1.right = 170;
	r1.left = 100;
	r1.depth = 1;
	
	r2.up = 80;
	r2.bottom = 150;
	r2.right = 200;
	r2.left = 120;
	r2.depth = 2;
	
	
//	membuat view (kotak)
	if(r1.depth > r2.depth)
	{
		drawRectangle(r1,color);
		drawRectangle(r2,color+10);
	}
	else
	{
		drawRectangle(r2,color);
		drawRectangle(r1,color+10);
	}
	
//	resolusi algoritma scan line
	
		
	sleep(2);	
	set_mode(TEXT_MODE);
	return 0;
}
