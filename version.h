#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char SPC_DATE[] = "26";
	static const char SPC_MONTH[] = "12";
	static const char SPC_YEAR[] = "2021";
	static const char SPC_UBUNTU_VERSION_STYLE[] =  "21.12";
	
	//Software Status
	static const char SPC_STATUS[] =  "Alpha";
	static const char SPC_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long SPC_MAJOR  = 1;
	static const long SPC_MINOR  = 1;
	static const long SPC_BUILD  = 107;
	static const long SPC_REVISION  = 602;
	
	//Miscellaneous Version Types
	static const long SPC_BUILDS_COUNT  = 140;
	#define SPC_RC_FILEVERSION 1,1,107,602
	#define SPC_RC_FILEVERSION_STRING "1, 1, 107, 602\0"
	static const char SPC_FULLVERSION_STRING [] = "1.1.107.602";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long SPC_BUILD_HISTORY  = 7;
	

#endif //VERSION_H
