#include"../../Multiboot.h"


.global boot_start,stack
.extern bspmain

.code32
.section ".boot"

.long (MULTIBOOT_HEADER_MAGIC)			//魔术头
.long (MULTIBOOT_HEADER_FLAGS)			
.long ~(MULTIBOOT_HEADER_FLAGS+MULTIBOOT_HEADER_MAGIC)+1	//checksum
//后面没有内容说明是采用elf的头而非自定义

.section ".text"

#define STACK_SIZE 16*1024
boot_start:
	cli
	pushl $boot_start
	movl $(stack+STACK_SIZE-4),%esp
	movl %esp,%ebp
	call next //保证栈中有东西,这句话可有课没有
next:
	pushl %ebp  /*栈的起始地址*/
	pushl %eax
	pushl %ebx

	call bspmain
die:
	hlt
	jmp die

.section ".bss"
	.comm stack,STACK_SIZE
