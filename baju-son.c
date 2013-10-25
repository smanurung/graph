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

/*
 * variabel global
 */
typedef unsigned char byte;
typedef unsigned short word;

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
	int counter;
	
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

int main(){
	int x1,y1,x2,y2;
	byte color;
	float t1,t2;
	word i,start;
	
	set_mode(VGA_256_COLOR_MODE);
	
//	warna bingkai
	color=rand()%NUM_COLORS;
	
	line_bresenham(20,40,60,20,color);
	line_bresenham(60,20,100,40,color);
	line_bresenham(100,40,140,20,color);

//	line_bresenham(140,20,180,40,color);
//	line_bresenham(180,40,160,60,color);
	line_bresenham(140,20,300,50,color);
	line_bresenham(300,50,160,60,color);

	line_bresenham(160,60,160,160,color);
	line_bresenham(160,160,40,160,color);
	line_bresenham(40,160,40,60,color);
	line_bresenham(40,60,20,40,color);
	
//	melakukan scanline fill
	scanline(20,20,300,160,color);
	
	sleep(40);
	
	set_mode(TEXT_MODE);
	
	return 0;
}
