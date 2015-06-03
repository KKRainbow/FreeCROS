#pragma once
#include"Global.h"
struct Segment
{
	uint32_t limitLow:16;
	uint32_t baseLow:16;;
	uint32_t baseMid:8;
	uint32_t type:4;
	uint32_t descpType:1;
	uint32_t dpl:2;
	uint32_t present:1;
	uint32_t limitHigh:4;
	uint32_t AVL:1;
	uint32_t L:1;
	uint32_t DB:1;
	uint32_t G:1;
	uint32_t baseHigh:8;
	Segment()
	{
		present = 1;
		AVL = 0;
		DB = 1;
		G = 1;
		descpType = 1;
		L = 0;
	}
	private:
	static Segment BuildSegment(addr_t base,int type,int dpl = 0,addr_t limit = 0xfffff)
	{
		Segment res;
		SetBase(res,base);
		SetLimit(res,limit);
		res.dpl = dpl;
		res.type = type;
		return res;
	}
	public:
	static void SetBase(Segment& seg,addr_t base)
	{
		seg.baseLow = base&0xffff;
		seg.baseMid = (base>>16)&0xff;
		seg.baseHigh = (base>>24)&0xff;
	}
	static uint32_t GetBase(Segment& seg)
	{
		uint32_t res = 0;
		res |= seg.baseHigh;
		res <<= 8;
		res |= seg.baseMid;
		res <<= 16;
		res |= seg.baseLow; 
		return res;
	}
	static void SetLimit(Segment& seg,addr_t limit)
	{
		seg.limitLow = limit&0xffff;
		seg.limitHigh = (limit>>16)&0xf;
	}
	static uint32_t GetLimit(Segment& seg)
	{
		uint32_t res = 0;
		res |= seg.limitHigh;
		res <<= 16;
		res |= seg.limitLow;
		return res;
	}
	static Segment BuildCodeSegment(addr_t base,int dpl = 0,addr_t limit = 0xfffff)
	{
		return BuildSegment(base,0b1010,dpl,limit);
	}
	static Segment BuildDataSegment(addr_t base,int dpl = 0,addr_t limit = 0xfffff)
	{
		return BuildSegment(base,0b0010,dpl,limit);
	}
	static Segment BuildTSS(addr_t base,int dpl = 0,addr_t limit = 0xfffff)
	{
		Segment res =  BuildSegment(base,0b1001,dpl,limit);
		res.DB = 0;
		res.descpType = 0;
		res.L = 0;
		res.G = 0;
		return res;
	}
	static Segment BuildLDT(addr_t base,int dpl = 0,addr_t limit = 0xfffff)
	{
		Segment res = BuildSegment(base,0b0010,dpl,limit);
		res.DB = 0;
		res.descpType = 0;
		res.L = 0;
		return res;
	}

};
