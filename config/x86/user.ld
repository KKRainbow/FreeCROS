/*
 * Copyright (C) 2009 Niek Linnenbank
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

ENTRY(main)
TARGET("elf32-i386")
OUTPUT_FORMAT("elf32-i386")

phys = 0x10000000;
virt = 0x10000000;

SECTIONS
{
    . = 0x10000000;
    kernelStart = .;

    /DISCARD/ :
    {
        *(.note.gnu.build-id)
    }

    .text virt : AT(phys)
    {
		code = .;
		*(.boot)
		*(.text)
		*(*.text)
		*(.gnu.linkonce.*)
		*(.rodata)
		*(.rodata.*)
		. = ALIGN(4096);
    }
    .data : AT (phys + (data - code))
    {
		data = .;
		*(.data)

        CTOR_LIST = .;
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
        KEEP (*(.preinit_array))
        KEEP (*(.init_array.*))
        KEEP (*(.init_array))
		LONG(0)
        CTOR_END = .;

        DTOR_LIST = .;
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
        KEEP (*(.fini_array.*))
        KEEP (*(.fini_array))
        LONG(0)
        DTOR_END = .;
		. += 4;

		initStart = .;
		KEEP (*(SORT(.init*)))
		initEnd   = .;


		. = ALIGN(4096);
    } 
	.eh_frame :
	{
		KEEP(*(.eh_frame))
		binEnd = .;
	}
    .bss : AT(phys + (bss - code) )
    {
		bss = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)

       	 . += 4;
		kernelEnd = .;
		. = 0x00400000;
    }

}
