#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char SPC_DATE[] = "07";
	static const char SPC_MONTH[] = "01";
	static const char SPC_YEAR[] = "2022";
	static const char SPC_UBUNTU_VERSION_STYLE[] =  "22.01";
	
	//Software Status
	static const char SPC_STATUS[] =  "Alpha";
	static const char SPC_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long SPC_MAJOR  = 1;
	static const long SPC_MINOR  = 2;
	static const long SPC_BUILD  = 231;
	static const long SPC_REVISION  = 1268;
	
	//Miscellaneous Version Types
	static const long SPC_BUILDS_COUNT  = 270;
	#define SPC_RC_FILEVERSION 1,2,231,1268
	#define SPC_RC_FILEVERSION_STRING "1, 2, 231, 1268\0"
	static const char SPC_FULLVERSION_STRING [] = "1.2.231.1268";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long SPC_BUILD_HISTORY  = 31;
	

#endif //VERSION_H
