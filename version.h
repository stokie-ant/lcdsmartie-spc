#ifndef VERSION_H
#define VERSION_H

	//Date Version Types
	static const char SPC_DATE[] = "25";
	static const char SPC_MONTH[] = "04";
	static const char SPC_YEAR[] = "2022";
	static const char SPC_UBUNTU_VERSION_STYLE[] =  "22.04";
	
	//Software Status
	static const char SPC_STATUS[] =  "Alpha";
	static const char SPC_STATUS_SHORT[] =  "a";
	
	//Standard Version Type
	static const long SPC_MAJOR  = 1;
	static const long SPC_MINOR  = 3;
	static const long SPC_BUILD  = 310;
	static const long SPC_REVISION  = 1683;
	
	//Miscellaneous Version Types
	static const long SPC_BUILDS_COUNT  = 424;
	#define SPC_RC_FILEVERSION 1,3,310,1683
	#define SPC_RC_FILEVERSION_STRING "1, 3, 310, 1683\0"
	static const char SPC_FULLVERSION_STRING [] = "1.3.310.1683";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long SPC_BUILD_HISTORY  = 10;
	

#endif //VERSION_H
