//x86///////////////

#define PAGE_SHIFT 12
#define PAGE_SIZE (1<<(PAGE_SHIFT))

#define PAGE_LOWER_ALIGN(x) ((x)&(~(PAGE_SIZE-1)))
#define PAGE_UPPER_ALIGN(x) (PAGE_LOWER_ALIGN((x+(PAGE_SIZE-1))))


#define SINGLETON_H(classname) \
	public:\
		static classname* Instance();\
	private:\
		static classname* instance;\
		classname();
#define SINGLETON_CPP(classname) \
		classname* classname::instance = nullptr;\
		classname* classname::Instance()\
		{\
			if(classname::instance == nullptr)\
			{\
				classname::instance = new classname();\
			}\
			return classname::instance;\
		}\
		classname::classname()
	
#define USED __attribute__((used))
	
#define Outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))


#define Inb(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
_v; \
})

#define Outb_p(value,port) \
__asm__ ("outb %%al,%%dx\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:"::"a" (value),"d" (port))

#define Inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al\n" \
	"\tjmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:":"=a" (_v):"d" (port)); \
_v; \
})

#define ALIGN_UP(addr,align) (align == 0?addr:(addr+align) - ((addr+align)%align))
//////////////////////