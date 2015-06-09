#include"KeyHandler.h"
#define KEYBOARD_QUEUE_SIZE 50

keyboard_key this_key; //当前按键状态（是否有控制键按下）以及当前应该压入栈的按键，因为他要表示按键状态，所以每次压栈后不可清空
static int32_t key_map[256];
static int32_t shift_map[256];

static keyboard_key keys_queue[KEYBOARD_QUEUE_SIZE];
static int32_t queue_length = 0;
static int32_t queue_start = 0;
static int32_t queue_end = 0;

#define ASCII(eax) ((int8_t)(((eax) & 0x0000ff00) >> 8))
#define SCAN_CODE(eax) ((int8_t) ((eax) & 0x000000ff))

static void key_map_init();
extern void keyboard_interrupt(void);
static int32_t key_dequeue(); //返回 key队列中队头的索引号，并弹出该值

keyboard_key read_key() //返回一个字符
{
	keyboard_key empty_key;
	int32_t index = 0;
	int8_t c;
	// for(i=0;i<100000;i++);
	while ((index = key_dequeue()) != -1)
	{
		c = keys_queue[index].ascii;
		if (c != 0 && keys_queue[index].is_release == 0)
		{
			return keys_queue[index];
		}
	}
	empty_key.ascii = 0;
	return empty_key;
}

static int32_t key_dequeue() //返回 key队列中队头的索引号，并弹出该值
{
	if (queue_length <= 0)
		return -1;
	queue_start++;
	queue_length--;
	if (queue_start >= KEYBOARD_QUEUE_SIZE)
	{
		queue_start = 0;
		return KEYBOARD_QUEUE_SIZE;
	}
	else
	{
		return queue_start - 1;
	}

}
static void key_enqueue() //把this_key入栈
{
	if (queue_length >= KEYBOARD_QUEUE_SIZE) //队列满了
	{
		queue_length = 0;
		queue_start = 0;
		queue_end = 0;
	}
	keys_queue[queue_end] = this_key;
	queue_end = (queue_end + 1) >= KEYBOARD_QUEUE_SIZE ? 0 : queue_end + 1;
	queue_length++;
	return;
}

void do_intr(uint32_t scan_code)
{
	key_map_init();
	scan_code &= 0xff;
	if (scan_code == 0xe0) //是e0,只有这一种情况我们不把他放进队列
	{
		this_key.has_e0 = 1;
		return;
	}
	this_key.scan_code = scan_code;
	if ((scan_code >> 7) == 1) //松开了按键
	{
		scan_code &= 0x7f;
		this_key.is_release = 1;
		if (scan_code == KEY_ALT)
			this_key.alt = 0;
		if (scan_code == KEY_CTRL)
			this_key.ctrl = 0;
		if (scan_code == KEY_LSHIFT || scan_code == KEY_RSHIFT)
			this_key.shift = 0;

	}
	else
	{
		this_key.is_release = 0;
		if (scan_code == KEY_ALT)
			this_key.alt = 1;
		if (scan_code == KEY_CTRL)
			this_key.ctrl = 1;
		if (scan_code == KEY_LSHIFT || scan_code == KEY_RSHIFT)
			this_key.shift = 1;

	}
	if (this_key.shift == 0)
		this_key.ascii = key_map[scan_code];
	else
		this_key.ascii = shift_map[scan_code];


	key_enqueue();
	///
	this_key.has_e0 = 0;
	this_key.ascii = 0;
	return;
}


static void key_map_init()
{
	if(key_map[0x4F] == '1')
	{
		return;
	}
	key_map[0x4F] = '1';
	key_map[0x50] = '2';
	key_map[0x51] = '3';
	key_map[0x4B] = '4';
	key_map[0x4c] = '5';
	key_map[0x4D] = '6';
	key_map[0x47] = '7';
	key_map[0x48] = '8';
	key_map[0x49] = '9';
	key_map[0x52] = '0';
	key_map[0x53] = '.';
	key_map[12] = '-';
	key_map[13] = '=';
	key_map[26] = '[';
	key_map[27] = ']';
	key_map[39] = ';';
	key_map[40] = '\'';
	key_map[41] = '`';
	key_map[52] = '.';
	key_map[51] = ',';
	key_map[53] = '/';
	key_map[43] = '\\';
	key_map[2] = '\13';
	key_map[14] = '\b';
	key_map[5] = ' ';
	key_map[15] = '\t';
	key_map[28] = '\n';
	key_map[57] = ' ';
	key_map[16] = 'q';
	key_map[17] = 'w';
	key_map[18] = 'e';
	key_map[19] = 'r';
	key_map[20] = 't';
	key_map[21] = 'y';
	key_map[22] = 'u';
	key_map[23] = 'i';
	key_map[24] = 'o';
	key_map[25] = 'p';
	key_map[30] = 'a';
	key_map[31] = 's';
	key_map[32] = 'd';
	key_map[33] = 'f';
	key_map[34] = 'g';
	key_map[35] = 'h';
	key_map[36] = 'j';
	key_map[37] = 'k';
	key_map[38] = 'l';
	key_map[44] = 'z';
	key_map[45] = 'x';
	key_map[46] = 'c';
	key_map[47] = 'v';
	key_map[48] = 'b';
	key_map[49] = 'n';
	key_map[50] = 'm';
	key_map[2] = '1';
	key_map[3] = '2';
	key_map[4] = '3';
	key_map[5] = '4';
	key_map[6] = '5';
	key_map[7] = '6';
	key_map[8] = '7';
	key_map[9] = '8';
	key_map[10] = '9';
	key_map[11] = '0';
	shift_map[0x4F] = '1';
	shift_map[0x50] = '2';
	shift_map[0x51] = '3';
	shift_map[0x4B] = '4';
	shift_map[0x4c] = '5';
	shift_map[0x4D] = '6';
	shift_map[0x47] = '7';
	shift_map[0x48] = '8';
	shift_map[0x49] = '9';
	shift_map[0x52] = '0';
	shift_map[0x53] = '.';
	shift_map[12] = '_';
	shift_map[13] = '+';
	shift_map[26] = '(';
	shift_map[27] = ')';
	shift_map[39] = ':';
	shift_map[40] = '\"';
	shift_map[41] = '~';
	shift_map[52] = '>';
	shift_map[51] = '<';
	shift_map[53] = '?';
	shift_map[43] = '|';
	shift_map[2] = '\13';
	shift_map[14] = '\b';
	shift_map[5] = ' ';
	shift_map[15] = '\t';
	shift_map[28] = '\n';
	shift_map[57] = ' ';
	shift_map[16] = 'Q';
	shift_map[17] = 'W';
	shift_map[18] = 'E';
	shift_map[19] = 'R';
	shift_map[20] = 'T';
	shift_map[21] = 'Y';
	shift_map[22] = 'U';
	shift_map[23] = 'I';
	shift_map[24] = 'O';
	shift_map[25] = 'P';
	shift_map[30] = 'A';
	shift_map[31] = 'S';
	shift_map[32] = 'D';
	shift_map[33] = 'F';
	shift_map[34] = 'G';
	shift_map[35] = 'H';
	shift_map[36] = 'J';
	shift_map[37] = 'K';
	shift_map[38] = 'L';
	shift_map[44] = 'Z';
	shift_map[45] = 'X';
	shift_map[46] = 'C';
	shift_map[47] = 'V';
	shift_map[48] = 'B';
	shift_map[49] = 'N';
	shift_map[50] = 'M';
	shift_map[2] = '!';
	shift_map[3] = '@';
	shift_map[4] = '#';
	shift_map[5] = '$';
	shift_map[6] = '%';
	shift_map[7] = '^';
	shift_map[8] = '&';
	shift_map[9] = '*';
	shift_map[10] = '(';
	shift_map[11] = ')';
}