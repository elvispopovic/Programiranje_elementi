#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv)
{
	char broj[16];
	int i;
	char** pptArgv;
	for(i=0, pptArgv=argv; i<argc; i++, pptArgv++)
		puts(*pptArgv);
	for(i=9; i>=0; i--)
	{
		itoa(i,broj,10);
		fputs(broj, stdout);
	}
	puts("\nKraj.");
	return 0;
}