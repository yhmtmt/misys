#include <stdio.h>

bool isAIVDM(char * str){
  return str[0] == '!' && str[1] == 'A' && str[2] == 'I';
}

bool splitAIVDMField(char * str, char field[8][128]){
  int i = 0, ifield = 0, ichar = 0, len = 0;
  while(str[i] != '\0'){
    field[ifield][ichar] = str[i];
    if(str[i] == ',' || str[i] == '*'){
      field[ifield][ichar] = '\0';
      ichar = 0;
      ifield++;
      i++;
      continue;
    }
    ichar++;
    i++;
  }
  field[ifield][ichar] = '\0';
}

int freeArmoring(char * str){
  int ichar = 0;
  // dearmoring
  for(ichar = 0; str[ichar] != '\0'; ichar++){
    str[ichar] -= 48;   // Note: 6bit ASCII begins with '0'(48)
    if(str[ichar] > 40) // Note: 6bit ASCII skips 88 to 95.
      str[ichar] -= 8;  // Need to subtract 8 in case the character exceeds 88(40+48)
  }
  return ichar;
}

void print6bitASCII(char * str, int len)
{
  for(int ichar = 0; ichar < len; ichar++){
    // printing a 6bit ASCII character as binary format.
    for(char mask=0x20; mask != 0x00; mask >>= 1){
      if((mask & str[ichar]) != 0)
	printf("1");
      else
	printf("0");
    }
  }
}

int main()
{
  FILE* file;
  char str[128]; // 128 characters
  int i; // loop variable
  char c;
  char field[8][128];
  
  // Opening file
  file = fopen("../0307.TXT","r");
  
  i = 0;
  
  while((c = getc(file)) != EOF){
    str[i] = c; 
    if(c == 10 || c == 13){ // CR or LF
      str[i] = '\0'; // Termination Character
      if(isAIVDM(str)){
	// Here we split the string into 8 fileds with delimiters '*' and ','
	splitAIVDMField(str, field);

	// length of the field 6 is now stored in ichar, and moved to len
	int len = freeArmoring(field[5]);

	// printing data fields
	for(i = 0; i < 8; i++){
	  // For field 6, we print the 6bit ASCIIs as binary digits
	  if(i == 5){
	    printf("Field[5]: ");
	    print6bitASCII(field[5], len);
	    printf("\n");
	  }else{
	    printf("Field[%d]:%s\n", i, field[i]);
	  }  
	}
      }
      i = 0;
      continue;
    }
    i++;
  }
  
  // Closing file
  fclose(file);
  return 0;
}
