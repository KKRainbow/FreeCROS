#include"Log.h"
SINGLETON_CPP(Log)
{
	this->instance->printer.Clear();
}

int Log::LogStr(const char* str,...)
{
	char res[500];
	int count;
	va_list vlist;

	va_start(vlist,str);
	count = vsprintf(res,str,vlist);
	va_end(vlist);

	printer.Print(res);
	//charList.PushBack(res);
	return count;

}

void Printer::GotoXY(uint16_t x,uint16_t y)
{
	xpos = x;
	ypos = y;
	int cons_pos = x+y*COLUMNS;
	// ?????????ĵ?8λ
	Outb(0x0f,0x3d4);
	Outb((uint8_t)(cons_pos&0xff),0x3d5);
	// ?????????ĸ?8λ
	Outb(0x0e,0x3d4);
	Outb((uint8_t)((cons_pos>>8)&0xff),0x3d5);
}

void Printer::Print(const char* str)
{
	int i;
	for (;*str;str++)
	{
		switch (*str) {
			case '\n':
				for (i=xpos;i<COLUMNS;i++)
				{
					Putchar(i,ypos,' ');
				}
				xpos = 0;
				ypos++;
				break;
			case '\b':
				if (xpos==0&&ypos==0) ;
				else if (xpos==0) {
					xpos = COLUMNS-1;
					ypos--;
				} else {
					xpos--;
				}
				Putchar(xpos,ypos,' ');
				break;
			default:
				Putchar(xpos,ypos,*str);
				if (xpos==COLUMNS-1) {
					xpos = 0;
					ypos++;
				} else {
					xpos++;
				}
				break;
		}
	}
	if (ypos<=(LINES-1)) {
		GotoXY(xpos,ypos);
	} else {
		Scrollup(ypos-(LINES-1));
		GotoXY(xpos,(LINES-1));
	}
}
void Printer::Scrollup(uint16_t n)
{
	memmove((void*)VIDEO,((char*)VIDEO+COLUMNS*2*n),COLUMNS*(LINES-n)*2);
	memset(((char*)VIDEO+COLUMNS*2*(LINES-n)),0,COLUMNS*n*2);
	xpos = 0;
	ypos = LINES;
}
void Printer::Clear()
{
	int i;

	video = (unsigned char *) VIDEO;

	for (i = 0; i < COLUMNS * LINES * 2; i+=2)
	{
		*(video + i) = 0;
		*(video + i+1) = ATTRIBUTE;
	}

	xpos = 0;
	ypos = 0;
}

void Printer::Putchar(uint16_t x, uint16_t y, char ch)
{
	*(char*)(VIDEO+(x+(y*80))*2) = ch;
	*(uint8_t*)(VIDEO+(x+(y*80))*2+1) = ATTRIBUTE;
}
