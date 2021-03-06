int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_initmalloc(void);
void *api_malloc(int size);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
void api_refreshwin(int win, int x0, int y0, int x1,int y1);
void api_closewin(int win);
int api_getkey(int mode);
void api_end(void);

int HariMain(void)
{
	int win, i;
	char *buf;
	api_initmalloc();
	buf = api_malloc(160 * 100);
	win = api_openwin(buf, 160, 100, -1, "line");
	for(i = 0; i < 8; i++)
	{
		api_linewin(win, 8,  26, 77, i * 9 + 26, i);
		api_linewin(win, 88, 26, i * 9 + 88, 89, i);
	}
	api_refreshwin(win, 6, 26, 154, 90);
	
	while(1)
	{
		if(api_getkey(1) == 0x1c)
		{
			break;
		}
	}
	api_closewin(win);
	api_end();
}