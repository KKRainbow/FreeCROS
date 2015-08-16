/*
 * Copyright 2015 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#pragma once

/** The magic number for the Multiboot header. */
#define MULTIBOOT_HEADER_MAGIC          0x1BADB002

/** The flags for the Multiboot header. */
#define MULTIBOOT_HEADER_FLAGS          0x00000003
//Bit0 : Module align to 4kb		1
//Bit1 mem_map						1
//Bit2 video mode					0

/** Size of the multiboot header structure. */
#define MULTIBOOT_HEADER_SIZE           52

/** The magic number passed by a Multiboot-compliant boot loader.  */
#define MULTIBOOT_BOOTLOADER_MAGIC      0x2BADB002

/* Do not include in assembler source. */
#ifndef __ASSEMBLER__

#include"stl/sbitmap.h"
#include"Type.h"
/**
 *  * The symbol table for a.out.
 *   */
typedef struct AoutSymbolTable
{
	int32_t tabSize;
	uint32_t strSize;
	uint32_t address;
	uint32_t reserved;
}
		AoutSymbolTable;

/**
 *  * The section header table for ELF.
 *   */
typedef struct ElfSectionHeaderTable
{
	uint32_t num;
	uint32_t size;
	uint32_t address;
	uint32_t shndx;
}
		ElfSectionHeaderTable;

/**
 *  * The Multiboot information.
 *   */
typedef struct MultibootInfo
{
	uint32_t flags;
	uint32_t memLower;
	uint32_t memUpper;
	uint32_t bootDevice;
	uint32_t cmdline;
	uint32_t modsCount;
	uint32_t modsAddress;

	union
	{
		AoutSymbolTable aout;
		ElfSectionHeaderTable elf;
	};
	uint32_t mmapLength;
	uint32_t mmapAddress;
	uint32_t drivesLength;
	uint32_t drivesAddr;
	uint32_t configTable;
	uint32_t bootLoaderName;
	uint32_t apmTable;

	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;
}
		MultibootInfo;

/**
 *  * The module class.
 *   */
typedef struct MultibootModule
{
	uint32_t modStart;
	uint32_t modEnd;
	uint32_t string;
	uint32_t reserved;
}
		MultibootModule;

/**
 *  * The memory map. Be careful that the offset 0 is base_addr_low
 *   * but no size.
 *    */
typedef struct MultibootMemoryMap
{
	uint32_t size;
	uint32_t baseAddressLow;
	uint32_t baseAddressHigh;
	uint32_t lengthLow;
	uint32_t lengthHigh;
	uint32_t type;
}__attribute__((packed))
		MultibootMemoryMap;

typedef struct MultibootVbe
{
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint32_t vbe_interface_seg;
	uint32_t vbe_interface_off;
	uint32_t vbe_interface_len;
}MultibootVbe;

/** Fill in by the early boot process. */
extern MultibootInfo multibootInfo;

/**
 *  * @}
 *   */


class Multiboot
{
private:
	lr::sstl::Bitmap<unsigned char,1> bitmap;
	MultibootInfo InfoTable;
	char intTable[1024];
public:
	Multiboot();
	Multiboot(const Multiboot& _x);
	bool TestFlagVBE() ;
	bool TestFlagAPM() ;
	bool TestFlagName() ;
	bool TestFlagConfig() ;
	bool TestFlagDrives() ;
	bool TestFlagMmap() ;
	bool TestFlagSyms() ;
	bool TestFlagMods() ;
	bool TestFlagCmd() ;
	bool TestFlagBootDev() ;
	bool TestFlagMem() ;
	const MultibootInfo& GetMultibootInfo();
	void SetInfoAddr(MultibootInfo* _Addr);
	char* GetIntTable();
};

extern class Multiboot globalMultiboot;
#endif /* !__ASSEMBLER__ */

