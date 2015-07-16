
struct VDM1 {
	char repeate;      // repeate indicator (8bit integer)
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
};

bool isAIVDM(char * str);
void splitAIVDMField(char * str, char field[8][128]);
int freeArmoring(char * str);
void print6bitASCII(char * str, int len);
void decodeMessage1(VDM1 * d, char * str, int len);
void printMessage1(FILE * ofile, VDM1 * d);
void decodeAndPrintMessage1(FILE * ofile, char * str, int len);