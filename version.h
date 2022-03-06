#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char SPC_DATE[] = "06";
	static const char SPC_MONTH[] = "03";
	static const char SPC_YEAR[] = "2022";
	static const char SPC_UBUNTU_VERSION_STYLE[] =  "22.03";
	
	//Software Status
	static const char SPC_STATUS[] =  "Alpha";
	static const char SPC_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long SPC_MAJOR  = 1;
	static const long SPC_MINOR  = 2;
	static const long SPC_BUILD  = 237;
	static const long SPC_REVISION  = 1303;
	
	//Miscellaneous Version Types
	static const long SPC_BUILDS_COUNT  = 314;
	#define SPC_RC_FILEVERSION 1,2,237,1303
	#define SPC_RC_FILEVERSION_STRING "1, 2, 237, 1303\0"
	static const char SPC_FULLVERSION_STRING [] = "1.2.237.1303";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long SPC_BUILD_HISTORY  = 37;
	

#endif //VERSION_H
