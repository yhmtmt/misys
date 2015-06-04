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

void decodeAndPrintMessage1(char * str, int len)
{
  char repeate;      // repeate indicator
  unsigned int mmsi; // mmsi number
  char status;     // navigation status
  float turn;      // Rate of Turn degrees/min
  float speed;     // knot
  char accuracy;   // DGPS = 1, GPS = 0
  float lon;       // degree
  float lat;       // degree
  float course;    // xxx.x degree
  unsigned short heading; // degree
  unsigned char second;   // UTC second
  char maneuver;          // 0:na 1: no special 2:special
  char raim;              // RAIM flag
  unsigned int radio;     // radio status

  // temporal vairables used during decoding
  unsigned int tmpu;
  char tmpc;
  int tmpi;

  if(str[0] != 1)
    return;

  repeate = (str[1] & 0x30) >> 4;
  
  mmsi = ((str[1] & 0x0F) << 26) |
    ((str[2] & 0x3F) << 20) | 
    ((str[3] & 0x3F) << 14) |
    ((str[4] & 0x3F) << 8) |
    ((str[5] & 0x3F) << 2) |
    ((str[6] & 0x30) >> 4);
  
  status = (str[6] & 0x0F);
  
  tmpc = ((str[7] & 0x3F) << 2) | 
    ((str[8] & 0x30) >> 4);
  turn = (float) (tmpc * (1.0 / 4.733));
  turn *= (tmpc < 0 ? -turn : turn);
  
  tmpu = ((str[8] & 0x0F) << 6) |
    (str[9] & 0x3F);
  speed = (float)((float) tmpu * (1.0/10.0));
  
  accuracy = ((str[10] & 0x20) >> 7);
  
  tmpi = ((str[10] & 0x1F) << 27) |
    ((str[11] & 0x3F) << 21) |
    ((str[12] & 0x3F) << 15) |
    ((str[13] & 0x3F) << 9) |
    ((str[14] & 0x3E) << 3);
  tmpi >>= 4;
  tmpi = (tmpi & 0x08000000 ? tmpi | 0xF0000000 : tmpi);
  lon = (float) (tmpi * (1.0/600000.0));
  
  tmpi = ((str[14] & 0x01) << 31) |
    ((str[15] & 0x3F) << 25) | 
    ((str[16] & 0x3F) << 19) |
    ((str[17] & 0x3F) << 13) |
    ((str[18] & 0x3F) << 7) |
    ((str[19] & 0x30) << 1);
  tmpi >>= 5;
  tmpi = (tmpi & 0x04000000 ? tmpi | 0xF8000000 : tmpi);
  lat = (float) (tmpi * (1.0/600000.0));
  
  tmpu = ((str[19] & 0x0F) << 8) |
    ((str[20] & 0x3F) << 2) |
    ((str[21] & 0x30) >> 4);
  
  course = (float) (tmpu * (1.0/10.0));
  heading = ((str[21] & 0x0F) << 5) |
    ((str[22] & 0x3E) >> 1);
  
  second =  ((str[22] & 0x01) << 1) |
    ((str[23] & 0x3E) >> 1);
  
  maneuver = ((str[23] & 0x01) << 1) |
    ((str[24] & 0x20) >> 5);
  
  // str[24] & 0x0C is not used
  
  raim = (str[24] & 0x02) >> 1;
  
  radio = ((str[24] & 0x01) << 18) |
    ((str[25] & 0x3F) << 12) |
    ((str[26] & 0x3F) << 6) |
    (str[27] & 0x3F);  

  // Printing the decoded Message 1
  printf("MMSI: %u", mmsi);
  printf(" STAT: ");
  switch(status){
  case 0:
    printf("Under way using engine");
    break;
  case 1:
    printf("At anchor");
    break;
  case 2:
    printf("Not under command");
    break;
  case 3:
    printf("Restricted manoeuverability");
    break;
  case 4:
    printf("Constrained by her draught");
    break;
  case 5:
    printf("Moored");
    break;
  case 6:
    printf("Aground");
    break;
  case 7:
    printf("Engaged in Fishing");
    break;
  case 8:
    printf("Under way sailing");
    break;
  default:
    printf("Unknown state");
  }
  
  printf(" TRN: %f (deg/min)", turn);
  printf(" SPD: %f (knot)", speed);
  printf(" DGPS: %s ", accuracy ? "yes":"no");
  printf(" LON: %f (deg)", lon);
  printf(" LAT: %f (deg)", lat);
  printf(" CRS: %f (deg)", course);
  printf(" HDG: %u (deg)", heading);
  printf(" UTC: %u (sec)", second);
  printf(" MAN: %s", (maneuver ? (maneuver == 1 ? "no special":"special"):"na"));
  printf(" RAIM: %s",raim ? "yes":"no");
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

	// Process only message 1
	if(field[5][0] == 1){ // message 1 has 1 in the first 6bit of field 6.
	  for(i = 0; i < 8; i++){
	    // For field 6, we print the 6bit ASCIIs as binary digits
	    if(i == 5){
	      printf("Field[5]: ");
	      decodeAndPrintMessage1(field[5], len);
	      printf("\n");
	    }else{
	      printf("Field[%d]:%s\n", i, field[i]);
	    }  
	  }
	  //getc(stdin);
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
