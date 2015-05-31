#include "ELF.h"

ELF::ELF(const char *p, int f, ELFHeader *h) : ExecutableFormat(p), fd(f)
{
	memcpy(&header, h, sizeof(header));
}

ELF::ELF(void *p, ELFHeader *h) : ExecutableFormat(p),memory(p)
{
	memcpy(&header, h, sizeof(header));
}

ELF::~ELF()
{
}

ExecutableFormat * ELF::detectPath(const char* path)
{
	return nullptr;
}
ExecutableFormat * ELF::detectMem(void* memory)
{
	ELFHeader header;

	header = *(ELFHeader*)memory;

	if (header.ident[ELF_INDEX_MAGIC0] == ELF_MAGIC0 &&
			header.ident[ELF_INDEX_MAGIC1] == ELF_MAGIC1 &&
			header.ident[ELF_INDEX_MAGIC2] == ELF_MAGIC2 &&
			header.ident[ELF_INDEX_MAGIC3] == ELF_MAGIC3)
	{
		/* Only accept current, 32-bit ELF executable programs. */
		if (header.ident[ELF_INDEX_CLASS] == ELF_CLASS_32 &&
				header.version == ELF_VERSION_CURRENT &&
				header.type    == ELF_TYPE_EXEC)
		{
			return new ELF(memory, &header);
		}
	}
	return nullptr;
}

int ELF::regions(MemoryRegion *regions, size_t max)
{
	ELFSegment* segments;
	size_t count = 0;
	char* currPos;

	/* Must be of the same sizes. */
	if (!(header.programHeaderEntrySize == sizeof(ELFSegment) &&
				header.programHeaderEntryCount < 16))
	{
		return -1;
	}
	/* Point to the program header. */
	currPos = (char*)memory + header.programHeaderOffset;
	/* Read all segments. */
	segments = (ELFSegment*)currPos;
	/* Fill in the memory regions. */
	for (size_t i = 0; i < max && i < header.programHeaderEntryCount; i++)
	{
		/* We are only interested in loadable segments. */
		if (segments[i].type != ELF_SEGMENT_LOAD)
		{
			continue;
		}
		regions[i].virtualAddress = segments[i].virtualAddress;
		regions[i].size  = segments[i].memorySize;
		regions[i].flags = 1;
		regions[i].data  = new uint8_t[segments[i].memorySize];

		/* Read segment contents from file. */
		currPos = (char*)memory+segments[i].offset;
		memcpy(regions[i].data,currPos,segments[i].fileSize);

		/* Nulify remaining space. */
		if (segments[i].memorySize > segments[i].fileSize)
		{
			memset(regions[i].data + segments[i].fileSize, 0,
					segments[i].memorySize - segments[i].fileSize);
		}
		/* Increment counter. */
		count++;
	}
	/* All done. */
	return count;
}

addr_t ELF::entry()
{
	return header.entry;
}
