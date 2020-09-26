int api_openwin(char* buf, int xsize, int ysize, int col_inv, char *title);
void api_initmalloc(void);
char* api_malloc(int size);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
int api_getkey(int mode);
void api_end(void);

unsigned char rbg2pal(int r, int g, int b, int x, int y);

void HariMain(void)
{
	char *buf;
	int win, x, y;
	api_initmalloc();
	buf = api_malloc(144 * 164);
	win = api_openwin(buf, 144, 164, -1 , "color2");
	for(y = 0; y < 128; y++)
	{
		for(x = 0; x < 128; x++)
		{
			buf[(x + 8) + (y + 28) * 144] = rbg2pal(x * 2, y * 2, 0, x, y);
		}
	}
	api_refreshwin(win, 8 ,28, 136, 156);
	api_getkey(1);
	api_end();
}

unsigned char rbg2pal(int r, int g, int b, int x, int y)
{
	int table[4] = {3, 1, 0, 2};
	//static int table2[2] = {0,0}; /* これはギリcolorに問題が起きない */
	int i;
	
	x &= 1;
	y &= 1;
	i = table[x + y * 2];
	r = (r * 21) / 256;
	g = (g * 21) / 256;
	b = (b * 21) / 256;
	r = (r + i) / 4;
	g = (g + i) / 4;
	b = (b + i) / 4;
	return 16 + r + g * 6 + b * 36;
	/*
	x0y0 16
	x1y0 16
	x2y0 16
	x3y0 16
	x4y0 16
	x5y0 16
	x6y0 16
	x7y0 16
	
	x8 y0 17
	x9 y0 16
	x10y0 17
	x11y0 16
	x12y0 17
	x13y0 16
	x14y0 17
	x15y0 16
	x16y0 17
	x17y0 16
	
	x18y0 17
	x19y0 17
	x20y0 17
	・・・
	x127y0 21
	*/
}
