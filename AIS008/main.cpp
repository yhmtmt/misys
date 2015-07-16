#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "aislib.h"

int main()
{
	FILE* file, * ofile; // ofile: output file
	char str[128]; // 128 characters
	int i, j, imsg; // loop variable
	char c;
	char field[8][128];
	VDM1 * msgs;
	msgs = (VDM1*) malloc(sizeof(VDM1) * 100000);

	clock_t t1, t2, t3, t4;

	// Opening file
	file = fopen("../../0307.TXT","r");
	ofile = fopen("decode.csv", "w");

	i = 0;
	imsg = 0;

	t1 = clock();

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
					decodeMessage1(&msgs[imsg], field[5], len);
					imsg++;
				}
			}
			i = 0;
			continue;
		}
		i++;
	}

	t2 = clock();

	int nmsgs = imsg; // save the number of messages

	// msgs has bunch of data

	// sorting
	for(i = 0; i < nmsgs - 1; i++){
		for(j = 1; j < nmsgs - i; j++){
			if(msgs[j].mmsi < msgs[j-1].mmsi){
				VDM1 tmp;
				tmp = msgs[j];
				msgs[j] = msgs[j-1];
				msgs[j-1] = tmp;
			}
		}
	}

	t3 = clock();

	// output msgs to our output file
	unsigned int mmsi = 0;

	for(imsg = 0; imsg < nmsgs; imsg++){
		if(mmsi != msgs[imsg].mmsi){
			char fname[128];

			mmsi = msgs[imsg].mmsi;
			
			sprintf(fname, "mmsi%u.csv", mmsi);
			printf("Writing %s\n", fname);
			fclose(ofile);
			ofile = fopen(fname, "w");
		}
		printMessage1(ofile, &msgs[imsg]);
	}

	t4 = clock();
	// Closing file
	fclose(file);
	fclose(ofile);

	free(msgs);

	// calculate processing time 
	double tload, tsort, tsave;
	tload = (double)(t2 - t1) / (double) CLOCKS_PER_SEC;
	tsort = (double)(t3 - t2) / (double) CLOCKS_PER_SEC;
	tsave = (double)(t4 - t3) / (double) CLOCKS_PER_SEC;

	printf("Load Time %f(s), Sort Time %f(s) Save Time %f(s)\n",
		tload, tsort, tsave);

	return 0;
}
