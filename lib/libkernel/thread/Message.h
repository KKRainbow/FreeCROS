#pragma once


struct Message
{
	uint64_t timeStamp;
	pid_t source;
	pid_t destination;
	int content[8];
};
