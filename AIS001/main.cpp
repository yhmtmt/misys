#include <stdio.h>

int main()
{
	FILE* file;
	char c;

	// Opening file
	file = fopen("0307.TXT","r");

	while((c = getc(file)) != EOF){
		printf("%c", c);
	}

	// Closing file
	fclose(file);
	return 0;
}