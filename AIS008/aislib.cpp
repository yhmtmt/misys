#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "aislib.h"

bool isAIVDM(char * str){
	return str[0] == '!' && str[1] == 'A' && str[2] == 'I';
}

void splitAIVDMField(char * str, char field[8][128]){
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

void decodeMessage1(VDM1 * d, char * str, int len)
{
	// temporal vairables used during decoding
	unsigned int tmpu;
	char tmpc;
	int tmpi;

	if(str[0] != 1)
		return;

	d->repeate = (str[1] & 0x30) >> 4;

	d->mmsi = ((str[1] & 0x0F) << 26) |
		((str[2] & 0x3F) << 20) | 
		((str[3] & 0x3F) << 14) |
		((str[4] & 0x3F) << 8) |
		((str[5] & 0x3F) << 2) |
		((str[6] & 0x30) >> 4);

	d->status = (str[6] & 0x0F);

	tmpc = ((str[7] & 0x3F) << 2) | 
		((str[8] & 0x30) >> 4);
	d->turn = (float) (tmpc * (1.0 / 4.733));
	d->turn *= (tmpc < 0 ? -d->turn : d->turn);

	tmpu = ((str[8] & 0x0F) << 6) |
		(str[9] & 0x3F);
	d->speed = (float)((float) tmpu * (1.0/10.0));

	d->accuracy = ((str[10] & 0x20) >> 7);

	tmpi = ((str[10] & 0x1F) << 27) |
		((str[11] & 0x3F) << 21) |
		((str[12] & 0x3F) << 15) |
		((str[13] & 0x3F) << 9) |
		((str[14] & 0x3E) << 3);
	tmpi >>= 4;

	tmpi = (tmpi & 0x08000000 ? tmpi | 0xF0000000 : tmpi);
	d->lon = (float) (tmpi * (1.0/600000.0));

	tmpi = ((str[14] & 0x01) << 31) |
		((str[15] & 0x3F) << 25) | 
		((str[16] & 0x3F) << 19) |
		((str[17] & 0x3F) << 13) |
		((str[18] & 0x3F) << 7) |
		((str[19] & 0x30) << 1);
	tmpi >>= 5;
	tmpi = (tmpi & 0x04000000 ? tmpi | 0xF8000000 : tmpi);
	d->lat = (float) (tmpi * (1.0/600000.0));

	tmpu = ((str[19] & 0x0F) << 8) |
		((str[20] & 0x3F) << 2) |
		((str[21] & 0x30) >> 4);

	d->course = (float) (tmpu * (1.0/10.0));
	d->heading = ((str[21] & 0x0F) << 5) |
		((str[22] & 0x3E) >> 1);

	d->second =  ((str[22] & 0x01) << 1) |
		((str[23] & 0x3E) >> 1);

	d->maneuver = ((str[23] & 0x01) << 1) |
		((str[24] & 0x20) >> 5);

	// str[24] & 0x0C is not used

	d->raim = (str[24] & 0x02) >> 1;

	d->radio = ((str[24] & 0x01) << 18) |
		((str[25] & 0x3F) << 12) |
		((str[26] & 0x3F) << 6) |
		(str[27] & 0x3F);  
}

void printMessage1(FILE * ofile, VDM1 * d)
{
	fprintf(ofile, "%u,", d->mmsi);
	switch(d->status){
	case 0:
		fprintf(ofile, "Under way using engine");
		break;
	case 1:
		fprintf(ofile, "At anchor");
		break;
	case 2:
		fprintf(ofile, "Not under command");
		break;
	case 3:
		fprintf(ofile, "Restricted manoeuverability");
		break;
	case 4:
		fprintf(ofile, "Constrained by her draught");
		break;
	case 5:
		fprintf(ofile, "Moored");
		break;
	case 6:
		fprintf(ofile, "Aground");
		break;
	case 7:
		fprintf(ofile, "Engaged in Fishing");
		break;
	case 8:
		fprintf(ofile, "Under way sailing");
		break;
	default:
		fprintf(ofile, "Unknown state");
	}
	fprintf(ofile, ",");

	fprintf(ofile,"%f,", d->turn);
	fprintf(ofile,"%f,", d->speed);
	fprintf(ofile,"%s,", d->accuracy ? "yes":"no");
	fprintf(ofile,"%f,", d->lon);
	fprintf(ofile,"%f,", d->lat);
	fprintf(ofile,"%f,", d->course);
	fprintf(ofile,"%u,", d->heading);
	fprintf(ofile,"%u,", d->second);
	fprintf(ofile,"%s,", (d->maneuver ? (d->maneuver == 1 ? "no special":"special"):"na"));
	fprintf(ofile,"%s,", d->raim ? "yes":"no");
	fprintf(ofile,"\n");
}



void decodeAndPrintMessage1(FILE * ofile, char * str, int len)
{
	VDM1 d;

	// temporal vairables used during decoding
	unsigned int tmpu;
	char tmpc;
	int tmpi;

	if(str[0] != 1)
		return;

	d.repeate = (str[1] & 0x30) >> 4;

	d.mmsi = ((str[1] & 0x0F) << 26) |
		((str[2] & 0x3F) << 20) | 
		((str[3] & 0x3F) << 14) |
		((str[4] & 0x3F) << 8) |
		((str[5] & 0x3F) << 2) |
		((str[6] & 0x30) >> 4);

	d.status = (str[6] & 0x0F);

	tmpc = ((str[7] & 0x3F) << 2) | 
		((str[8] & 0x30) >> 4);
	d.turn = (float) (tmpc * (1.0 / 4.733));
	d.turn *= (tmpc < 0 ? -d.turn : d.turn);

	tmpu = ((str[8] & 0x0F) << 6) |
		(str[9] & 0x3F);
	d.speed = (float)((float) tmpu * (1.0/10.0));

	d.accuracy = ((str[10] & 0x20) >> 7);

	tmpi = ((str[10] & 0x1F) << 27) |
		((str[11] & 0x3F) << 21) |
		((str[12] & 0x3F) << 15) |
		((str[13] & 0x3F) << 9) |
		((str[14] & 0x3E) << 3);
	tmpi >>= 4;

	tmpi = (tmpi & 0x08000000 ? tmpi | 0xF0000000 : tmpi);
	d.lon = (float) (tmpi * (1.0/600000.0));

	tmpi = ((str[14] & 0x01) << 31) |
		((str[15] & 0x3F) << 25) | 
		((str[16] & 0x3F) << 19) |
		((str[17] & 0x3F) << 13) |
		((str[18] & 0x3F) << 7) |
		((str[19] & 0x30) << 1);
	tmpi >>= 5;
	tmpi = (tmpi & 0x04000000 ? tmpi | 0xF8000000 : tmpi);
	d.lat = (float) (tmpi * (1.0/600000.0));

	tmpu = ((str[19] & 0x0F) << 8) |
		((str[20] & 0x3F) << 2) |
		((str[21] & 0x30) >> 4);

	d.course = (float) (tmpu * (1.0/10.0));
	d.heading = ((str[21] & 0x0F) << 5) |
		((str[22] & 0x3E) >> 1);

	d.second =  ((str[22] & 0x01) << 1) |
		((str[23] & 0x3E) >> 1);

	d.maneuver = ((str[23] & 0x01) << 1) |
		((str[24] & 0x20) >> 5);

	// str[24] & 0x0C is not used

	d.raim = (str[24] & 0x02) >> 1;

	d.radio = ((str[24] & 0x01) << 18) |
		((str[25] & 0x3F) << 12) |
		((str[26] & 0x3F) << 6) |
		(str[27] & 0x3F);  

	// Printing the decoded Message 1
	fprintf(ofile, "%u,", d.mmsi);
	switch(d.status){
	case 0:
		fprintf(ofile, "Under way using engine");
		break;
	case 1:
		fprintf(ofile, "At anchor");
		break;
	case 2:
		fprintf(ofile, "Not under command");
		break;
	case 3:
		fprintf(ofile, "Restricted manoeuverability");
		break;
	case 4:
		fprintf(ofile, "Constrained by her draught");
		break;
	case 5:
		fprintf(ofile, "Moored");
		break;
	case 6:
		fprintf(ofile, "Aground");
		break;
	case 7:
		fprintf(ofile, "Engaged in Fishing");
		break;
	case 8:
		fprintf(ofile, "Under way sailing");
		break;
	default:
		fprintf(ofile, "Unknown state");
	}
	fprintf(ofile, ",");

	fprintf(ofile,"%f,", d.turn);
	fprintf(ofile,"%f,", d.speed);
	fprintf(ofile,"%s,", d.accuracy ? "yes":"no");
	fprintf(ofile,"%f,", d.lon);
	fprintf(ofile,"%f,", d.lat);
	fprintf(ofile,"%f,", d.course);
	fprintf(ofile,"%u,", d.heading);
	fprintf(ofile,"%u,", d.second);
	fprintf(ofile,"%s,", (d.maneuver ? (d.maneuver == 1 ? "no special":"special"):"na"));
	fprintf(ofile,"%s,", d.raim ? "yes":"no");
	fprintf(ofile,"\n");
}
