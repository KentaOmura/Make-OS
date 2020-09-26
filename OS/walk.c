int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_initmalloc(void);
void *api_malloc(int size);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
void api_refreshwin(int win, int x0, int y0, int x1,int y1);
void api_closewin(int win);
int api_getkey(int mode);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_end(void);

int HariMain(void)
{
	int win, i, x, y;
	char *buf;
	
	api_initmalloc();
	buf = api_malloc(160 * 100);
	win = api_openwin(buf, 160, 100, -1, "walk");
	api_boxfilwin(win, 4, 24, 155, 95, 10 /* —Î */);
	x = 76;
	y = 56;
	api_putstrwin(win, x, y, 3, 1, "*");
	while(1)
	{
		i = api_getkey(1);
		api_putstrwin(win, x, y, 3, 1, "*");
		if(i == '4' && x > 4) {x -= 8;}
		if(i == '6' && x < 148) {x += 8;}
		if(i == '8' && y > 24) {y -= 8;}
		if(i == '2' && y < 80) {y += 8;}
		if(0x1c == i) {break;}
		api_putstrwin(win, x, y, 3, 1, "*");
	}
	api_closewin(win);
	api_end();
}