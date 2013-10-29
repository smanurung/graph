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

#define LINE_COLOR			2
#define SCAN_COLOR			75

#define sgn(x) ((x<0)?-1:((x>0)?1:0))
#define length(x) (sizeof(x)/sizeof(x[0]))

typedef unsigned char byte;
typedef unsigned short word;

typedef struct
{
	int up;
	int bottom;
	int right;
	int left;
	int depth; 			//urutan bidang tumpuk
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
 * 1 jika (x,y) berada pada garis batas r
 */
int isBatas(Rectangle r, int x, int y)
{
//	batas kiri
	if((x == r.left) || ((x == r.right)))
	{
		return ((y>=r.up) && (y<=r.bottom));
	}
	else if((y == r.up) || (y == r.bottom))
	{
		return ((x>=r.left) && (x<=r.right));
	}
	else return 0;
}

/*
 * prekondisi: r tidak kosong
 * mencari Rectangle dengan prioritas tertinggi (nilai depth paling rendah)
 */
Rectangle getHighestPriority(Rectangle r[2])
{
	int i;
	Rectangle maxR;
	
	maxR = r[0];
	for(i=1;i<2;i++)
	{
		if(r[i].depth < maxR.depth)
		{
			maxR = r[i];
		}
	}
	return maxR;
}

int isInside(Rectangle r, int x, int y)
{
	return (((x>=r.left)&&(x<=r.right)) && ((y>=r.up)&&(y<=r.bottom)));
}

/*
 * prekondisi: x,y adalah titik batas
 * 3 jika (x,y) merupakan garis lurus horizontal
 * 2 jika (x,y) blm jelas titik potong atau bukan
 * 1 jika (x,y) titik potong
 * 0 jika (x,y) bukan titik potong
 */
int isPotong(int x,int y,byte color){
	int a = isTitik(x-1, y-1, color);
    int b = isTitik(x, y-1, color);
    int c = isTitik(x+1, y-1, color);
    int d = isTitik(x-1, y, color);
    int e = isTitik(x+1, y, color);
    int f = isTitik(x-1, y+1, color);
    int g = isTitik(x, y+1, color);
    int h = isTitik(x+1, y+1, color);
	
	if(d && e) return 3;
	else if(d || e) return 2;
	else if(a) {
		if(f || g || h) return 1;
		else return 0;

/*	
		if(b || c) return 0;
		else return 1;
*/

	} else if(b){
		if(f || g || h) return 1;
		else return 0;

/*	
		if(c) return 0;
		else return 1;
*/		
	} else if(c) return 1;
	else return 0;
}

/*
 * menentukan apakah (x,y) termasuk titik batas bidang
 */
int isTitik(int x, int y,byte col){
	return (VGA[(y<<8)+(y<<6)+x]==col);
}

void drawRectangle(Rectangle r)
{
	line_bresenham(r.left,r.up,r.right,r.up,LINE_COLOR);
	line_bresenham(r.right,r.up,r.right,r.bottom,LINE_COLOR);
	line_bresenham(r.right,r.bottom,r.left,r.bottom,LINE_COLOR);
	line_bresenham(r.left,r.bottom,r.left,r.up,LINE_COLOR);

//	scanline(r.left,r.up,r.right,r.bottom,LINE_COLOR);
}

/*
 * melakukan pengisian titik pada area
 * minx		: nilai sumbu-x terkecil pada bidang
 * miny		: nilai sumbu-y terkecil pada bidang
 * maxx		: nilai sumbu-x terbesar pada bidang
 * maxy		: nilai sumbu-y terbesar pada bidang
 * color	: warna garis pembatas bidang
 */
void scanline(int minx, int miny, int maxx, int maxy, byte color, Rectangle arrR[2]) {
	int x, y, idx, counter, xtemp;
	int arr[500]; //maks 500 titik potong
	int awalV = 1, firstV, lastV;
	Rectangle highR;
	
//	pengaturan warna
	int fillcol = SCAN_COLOR;
	color = LINE_COLOR;
	
	for(y=miny;y<=maxy;y++){
//		printf("scan baris %d\n",y);
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
					
//					SEMANTIC! masukan saja byk titik
					arr[idx++] = x;
					
					if(awalV) {
						if(a || b || c) firstV=1;
						else firstV=0;
						
//						simpan titik awal dari garis horizontal
//						arr[idx++] = x;
						
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

//				uji  kepemilikan bangun
				if((isInside(arrR[0],xtemp,y))&&(isInside(arrR[1],xtemp,y)))
				{
//					mencari prioritas tertinggi dari seluruh bangun yang ada (bertindihan)
					highR = getHighestPriority(arrR);
					if(isBatas(highR,xtemp,y))
					{
						//do nothing
					}
					else
					{
						plot_pixel(xtemp,y,fillcol);
					}
				}
				else if((isInside(arrR[0],xtemp,y)) || (isInside(arrR[1],xtemp,y)))
				{
//					if(isBatas(arrR[0],xtemp,y) || (isBatas(arrR[1],xtemp,y)))
					if(isTitik(xtemp,y,color))
					{
						//do nothing
					}
					else
					{
						plot_pixel(xtemp,y,fillcol);
					}
				}
				xtemp++;
				
			}
			counter += 1;
		}
	}
}

int main(void){
	Rectangle r1,r2;
	Rectangle arr[2];
	int color;
	int colorDiff;
	
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
		drawRectangle(r1);
		drawRectangle(r2);
	}
	else
	{
		drawRectangle(r2);
		drawRectangle(r1);
	}
	
	arr[0] = r1;
	arr[1] = r2;
//	resolusi algoritma scan line
	scanline(r1.left, r1.up, r2.right, r2.bottom, LINE_COLOR, arr);
		
	sleep(5);
	set_mode(TEXT_MODE);
	return 0;
}
