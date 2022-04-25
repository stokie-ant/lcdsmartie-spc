//
// Spc.cpp : Defines the entry point for the DLL application.
//
#define CPP
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include "SPC.h"
#include "version.h"

#define DLLEXPORT __declspec(dllexport)

// Custom character definitions for the bars - note the first entry is not used (Yes it is!)
// $Chr(SPC_Global_BlackChar) is used for a full bar (normally 255)
char const *custdef[9] = {"0,0,0,0,0,0,0,0", "0,0,0,0,0,0,0,31", "0,0,0,0,0,0,31,31", "0,0,0,0,0,31,31,31",
                    "0,0,0,0,31,31,31,31", "0,0,0,31,31,31,31,31", "0,0,31,31,31,31,31,31", "0,31,31,31,31,31,31,31",
                    "31,31,31,31,31,31,31,31"};
// Custom character Chr codes - 176 is not used
// That's a lie - it is used
// change to using actual custom char positions 1,2,3,4,5,6,7,8 don't work as well with smartie's display and desktop display
int custchar[9] = {0, 176, 158, 131, 132, 133, 134, 135, 136};

int initialised = 0; // Flag to avoid duplicate calls to the init routine
static int spectrum_smooth = 1;
#define MAX_SMOOTH 10
#define MAX_SPEC 256
static int specarray[MAX_SPEC], prev_specarray[MAX_SMOOTH][MAX_SPEC];
static int display[4][MAX_SPEC];
int defaultdevindex;

/*********************************************************
 *         SmartieInit                                   *
 * This function will be called when the plugin is 1st   *
 * loaded. This function is OPTIONAL.                    *
 *********************************************************/
extern "C" DLLEXPORT void __stdcall SmartieInit()
{
    int i, j;
    char name[256];
    if (initialised == 0)
    {
        SPC_log_open();
        SPC_read_cfg(1);
        initialised = SPC_init();
        if (initialised < 0)
        {
          SPC_log("No device to init", 0);
          return;
        }

        // have to do this again here so we can switch devices should the default change
        defaultdevindex = SPC_finddefaultdevice(name);

        for (i = 0; i < MAX_SPEC; i++)
        {
            specarray[i] = 0;
            display[0][i] = 0;
            for (j = 0; j < MAX_SMOOTH; j++)
                prev_specarray[j][i] = 0;
        }
    }
}

/*********************************************************
 *         SmartieFini                                   *
 *  This function will be called just before the plugin  *
 * is unloaded. This function is OPTIONAL.               *
 *********************************************************/
extern "C" DLLEXPORT void __stdcall SmartieFini()
{
    SPC_close();
    SPC_log_close();
}

/*********************************************************
 *         GetMinRefreshInterval                         *
 * Define the minimum interval that a screen should get  *
 * fresh data from our plugin.                           *
 * The actual value used by Smartie will be the higher   *
 * of this value and of the "dll check interval" setting *
 * on the Misc tab.  [This function is optional, Smartie *
 * will assume 300ms if it is not provided.]             *
 *********************************************************/
extern "C" DLLEXPORT int __stdcall GetMinRefreshInterval()
{
    return 5;
}

/*********************************************************
 *         Function 1                                    *
 *  Returns the given line of spectrum display           *
 * p1=l#n#s where l=line number (1 at bottom, max 4)     *
 *                n=Total number of lines (max 4)        *
 *                s=Smoothness level (1-10) [Optional]   *
 * p2=width (maximum 32) this will be rounded down to    *
 *    the nearest of 4,8,16,32                           *
 *********************************************************/
extern "C" DLLEXPORT char const *__stdcall function1(char *param1, char *param2)
{
    static char str[2048], str1[2048]; // note output can be up to 1600 chars long
    int i, j, /*k,*/ line, numlines, width, smooth;
    int custused[9];
    char name[256];

    // check to see if we still have the default device
	if (!SPC_isdefaultdevice(defaultdevindex) == 0)
	{
		defaultdevindex = SPC_finddefaultdevice(name);
		SPC_close();
		if (defaultdevindex == -1)
            return "";

		if (SPC_init() < 0)
        {
            SPC_log("Cant init audio device: ", name);
            return "";
		}
	}

    // Parse the parameters
    i = substrcpy((unsigned char *)str, (unsigned char *)param1, '#');
    if (i == 0)
        return "Param1 line number incorrect ?#n";
    line = atoi(param1);
    if (line < 0)
        line = 1;
    if (line > 4)
        line = 4;

    j = substrcpy((unsigned char *)str, (unsigned char *)&param1[2], '#');
    if (j == 0)
        return "Param1 total lines incorrect n#?";
    numlines = atoi(&param1[2]);
    if (numlines < 0)
        numlines = 1;
    if (numlines > 4)
        numlines = 4;
    spectrum_height = numlines * 8;

    str[0]=0; // Erase the line char from "i = substrcpy((unsigned char *)str, (unsigned char *)param1, '#');"

    smooth = 1;
    if (param1[3] == '#')
    {
        smooth = atoi(&param1[4]);
        if (smooth < 1)
            smooth = 1;
        if (smooth > MAX_SMOOTH)
            smooth = MAX_SMOOTH;
    }
    spectrum_smooth = smooth;

    // Brute force work out the correct width // why?
    width = atoi(param2);
//    if (width >= 32)
//        width = 40;
//    else if (width >= 16)
//        width = 16;
//    else if (width >= 8)
//        width = 8;
//    else
//        width = 4;
    spectrum_width = width;

    if (line == numlines)
    {
        // Get the latest data
        SPC_get(specarray);
        // Correct for smoothness
        for (i = 0; i < width; i++)
        {
            for (j = smooth; j > 0; j--)
            {
                prev_specarray[j][i] = prev_specarray[j - 1][i];
            }
        }
        for (i = 0; i < width; i++)
        {
            prev_specarray[0][i] = specarray[i];
        }
        for (i = 0; i < width; i++)
        {
            for (j = 1; j < smooth; j++)
            {
                specarray[i] = specarray[i] + prev_specarray[j][i];
            }
        }
        for (i = 0; i < width; i++)
        {
            if (specarray[i] > 0)
                specarray[i] = specarray[i] / smooth;
        }
    }

    // Work out which custom characters are required
    for (i = 0; i < 9; i++)
        custused[i] = 0;
    for (i = 0; i < width; i++)
    {
        j = (specarray[i] - ((line - 1) * 8));
        if (j > 8)
            j = 8;
        if (j < 0)
            j = 0;
        if ((j == 0) && (line == 1))
            j = 1;
        if ((j > 0) && (j < 9))
            custused[j] = 1;
    }

    // Why isn't this for i=1 to 8 and the if (i!=9) is totally redundant???
    // this code defines custom char codes 1 to 8
    //    sprintf(str,"$CustomChar(1,%s)",custdef[1]);
    for (i = 1; i < 9; i++)
        if (custused[i] == 1)
        {
            if ((i != 8) || (SPC_Global_BlackChar == 0))
            {
                sprintf(str1, "$CustomChar(%d,%s)", i, custdef[i]);
                strcat(str, str1);
            }
        }

    // Work out the characters to send to the display
    for (i = 0; i < width; i++)
    {
        j = (specarray[i] - ((line - 1) * 8));
        if (j > 8)
            j = 8;
        if (j < 0)
            j = 0;
        if ((j == 0) && (line == 1) && (SPC_Bottom_Line != 0))
            j = 1;
        if (j == 0)
            strcpy(str1, " ");
        else if ((j > 7) && (SPC_Global_BlackChar != 0))
            sprintf(str1, "$Chr(%d)", SPC_Global_BlackChar);
        else
            sprintf(str1, "$Chr(%d)", custchar[j]);

        strcat(str, str1);
    }

    return str;
}

/*********************************************************
 *         Function 2                                    *
 *  Returns min/max/count/fmin/fmax/fcount               *
 * main use is for debugging but max can be used for vol *
 *********************************************************/
extern "C" DLLEXPORT char *__stdcall function2(char *param1, char *param2)
{
    static char str[256];
    if (!strncmp(param1, "MIN", 3))
        sprintf(str, "%d", SPC_imin);
    else if (!strncmp(param1, "MIN", 3))
        sprintf(str, "%d", SPC_imin);
    else if (!strncmp(param1, "MAX", 3))
        sprintf(str, "%d", SPC_imax);
    else if (!strncmp(param1, "COUNT", 5))
        sprintf(str, "%d", SPC_icount);
    else if (!strncmp(param1, "FMIN", 4))
        sprintf(str, "%d", SPC_fmin);
    else if (!strncmp(param1, "FMAX", 4))
        sprintf(str, "%d", SPC_fmax);
    else if (!strncmp(param1, "FCOUNT", 6))
        sprintf(str, "%d", SPC_fcount);
    else
        sprintf(str, "%+5d,%+5d,%+5d %+5d,%+5d,%+5d", SPC_imin, SPC_imax, SPC_icount, SPC_fmin, SPC_fmax, SPC_fcount);
    return str;
}

/*********************************************************
 *         Function 3                                    *
 *  Returns the version number of this code              *
 *********************************************************/
extern "C" DLLEXPORT char *__stdcall function3(char *param1, char *param2)
{
    static char ver[256];
    strcpy(ver, "SPC plugin v");
    strcat(ver, SPC_FULLVERSION_STRING);
    return ver;
}

/*********************************************************
 *         Function 4                                    *
 *  Returns the level param1 chan=M=both,L=left,R=right  *
 *   param2 0=linear, 1=exp, 2=log                       *
 *   followed by #n where n=buffer to average over       *
 *********************************************************/
extern "C" DLLEXPORT char *__stdcall function4(char *param1, char *param2)
{
    int sm = 0;
    static char str[20];

    if (param2[1]  == '#')
        sm = atoi(&param2[2]);
    if (!strncmp(param1, "L", 1))
        sprintf(str, "%lu", SPC_get_level(1, param2[0] - '0', sm));
    else if (!strncmp(param1, "R", 1))
        sprintf(str, "%lu", SPC_get_level(2, param2[0] - '0', sm));
    else
        sprintf(str, "%lu", SPC_get_level(0, param2[0] - '0', sm));
    return str;
}

/*********************************************************
 *         Function 5                                    *
 *  Returns the level param1 chan=M=both,L=left,R=right  *
 *   param2 0=linear, 1=exp, 2=log                       *
 *   followed by #n where n=buffer to average over       *
 *********************************************************/
extern "C" DLLEXPORT char *__stdcall function5(char *param1, char *param2)
{
    int sm = 0;
    static char str[20];

    if (param2[1] == '#')
        sm = atoi(&param2[2]);
    if (!strncmp(param1, "L", 1))
        sprintf(str, "%lu", SPC_get_level(1, param2[0] - '0', sm) / 328);
    else if (!strncmp(param1, "R", 1))
        sprintf(str, "%lu", SPC_get_level(2, param2[0] - '0', sm) / 328);
    else
        sprintf(str, "%lu", SPC_get_level(0, param2[0] - '0', sm) / 328);
    return str;
}
