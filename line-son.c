#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <time.h>
//#include <unistd.h>

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
	
	counter=0;
	
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
//			usleep(1000);
			sleep(1);
			
			counter++;
			
//			ganti warna
			if(counter%5==0){
				color = (color+10)%NUM_COLORS;
			}
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
//			usleep(1000);
			sleep(1);

			counter++;
			
//			ganti warna
			if(counter%5==0){
				color = (color+10)%NUM_COLORS;
			}
		}
	}
}

void main(){
	int x1,y1,x2,y2,color;
	float t1,t2;
	word i,start;
	
	srand(*my_clock);
	set_mode(VGA_256_COLOR_MODE);
	
	x1=rand()%SCREEN_WIDTH;
	y1=rand()%SCREEN_HEIGHT;
	x2=rand()%SCREEN_WIDTH;
	y2=rand()%SCREEN_HEIGHT;
	color=rand()%NUM_COLORS;
	
	for(i=0;i<5;i++){		
		line_bresenham(x1,y1,x2,y2,color);
		color=rand()%NUM_COLORS;
	}
	
	set_mode(TEXT_MODE);
	
	return;
}
