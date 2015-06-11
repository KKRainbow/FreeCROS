#pragma once
#include"Global.h"
#include"stl/sstring.h"
#include"stl/slinkedlist.h"
#include"stdio.h"
#include"stdarg.h"
#include"cpu/SpinLock.h"

class Log;
class Printer
{
	private:
		friend class Log;
		Printer():xpos(0),ypos(0),video((unsigned char*)VIDEO){}
		static const int COLUMNS = 80;
		static const int LINES = 24;
		static const int ATTRIBUTE = 0b10001100;
		static const addr_t VIDEO = 0xB8000;
		int xpos = 0;
		int ypos = 0;
		volatile unsigned char *video = (unsigned char*)VIDEO;
		void GotoXY(uint16_t x,uint16_t y);
		void Putchar(uint16_t x, uint16_t y, char ch);
	public:
		void Scrollup(uint16_t n);
		void Print(const char* str);
		void Clear();
};

class Log
{
	SINGLETON_H(Log)
	private:
		lr::sstl::List<lr::sstl::AString> charList;
		Printer printer;
	public:
		int LogStr(const char* str,...); 
};

#define LOG(...) Log::Instance()->LogStr(__VA_ARGS__)
