typedef struct cf
{
	unsigned char ch;
	unsigned char col;
}__attribute__((packed)) cf;

static unsigned short int horpos;
static unsigned short int verpos;

const short int VGA_HOR = 80;
const short int VGA_VER = 25;

void putch(char ch)
{
	cf* fb = (cf*)0xB8000;

	cf data;
	data.col = 0xF0;

	if (verpos == VGA_VER - 1)
		verpos = 0;

	if (horpos == VGA_HOR - 1)
	{
		horpos = 0;
		verpos++;
	}
	data.ch = ch;

	if (ch == '\n')
	{
		horpos = 0;
		verpos++;
		return;
	}

	fb[verpos * VGA_HOR + horpos++] = data;
}

void vmain()
{
	char data[] = "Hello humans, I am AROS";
	for (unsigned short int i = 0; i < sizeof(data) / sizeof(char); i++)
		putch(data[i]);
	for (;;);
}