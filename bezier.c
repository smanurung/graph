#include "tgraph.h"

int x[100];
int y[100];
int len;
Screen mainScreen;

double power(double a, int b)
{
	double res = 1.0;
	int i;
	for(i = 0; i < b; i++)
	{
		res *= a;
	}
	return res;
}

double combination(int a, int b)
{
	int res = 1;
	int i;
	for(i = a; i > a-b; i--)
	{
		res *= i;
	}
	
	for(i = 1; i <= b; i++)
	{
		res /= i;
	}
	return (double) res;
}

double polynom(int i, int m, double t)
{
	return combination(m, i) * power(1 - t, m - i) * power(t, i);
}

double formula(double t, int m, int st)
{
	double res = 0;
	int i;
	for(i = 0; i < m; i++)
	{
		res += polynom(i, m, t) * (st == 0 ? x[i] : y[i]);
	}
	return res;
}

void drawBezier()
{
	double deg = 0.001;
	double i = 0;
	while(i < 1.0005)
	{
		double posx = formula(i, len, 0);
		double posy = formula(i, len, 1);
		put_pixel((int) posx, (int) posy, 100, &mainScreen);
		//printf("%d %d\n", (int) posx, (int) posy);
		i += deg;
	}
}

void drawBox()
{
	int i,j,k;
	int warna = rand();
	
	for (i = 0; i <= 3; i++)
				for (j = 0; j <= 3; j++)
					put_pixel((int) i, (int) j, warna, &mainScreen);
	
	for (k = 0; k < len; k++)
	{
		warna = rand();
		for (i = x[k]-2; i <= x[k]+2; i++)
			for (j = y[k]-2; j <= y[k]+2; j++)
				put_pixel((int) i, (int) j, warna, &mainScreen);
	}
}

int main()
{
	init_screen(&mainScreen,0,0,320,200);

	len = 3;
	x[0] = 50;
	y[0] = 50;
	x[1] = 150;
	y[1] = 50;
	x[2] = 50;
	y[2] = 150;
	
	video_mode();
	drawBox();
	drawBezier();
	plot_pixel(mainScreen);
	
	while(1) {
		if (left_mouse_pressed()){
			break;
		}
	}
	
	text_mode();
	
	return 0;
}
