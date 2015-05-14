#include <stdio.h>

int main()
{
	FILE* file;
	char str[128]; // 128 characters
	int i; // loop variable
	char c;

	// Opening file
	file = fopen("0307.TXT","r");
	
	i = 0;

	while((c = getc(file)) != EOF){
		str[i] = c; 
		if(c == 10 || c == 13){
			str[i] = '\0';
			if(str[0] == '!' && str[1] == 'A' && str[2] == 'I')
				printf("%s\n", str);

			i = 0;
			continue;
		}
		i++;
	}

	// Closing file
	fclose(file);
	return 0;
}