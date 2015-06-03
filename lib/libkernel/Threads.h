#pragma once


struct Message
{
	uint64_t timeStamp;
	pid_t source;
	pid_t destination;
	uint32_t content[8];
};


enum ThreadType{KERNEL,SERVER,USER};