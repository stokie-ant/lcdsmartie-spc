//==============================================================================
// Code to log data to a file in the plugins directory
//==============================================================================
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include "version.h"
#include "SPC.h"

static FILE *fp_log; // log file handle

//==============================================================================
// Function to open the log file
//==============================================================================
extern void __stdcall SPC_log_open()
{
    char str[120];
    time_t time_since;
    struct tm *cur_time;

    time(&time_since);                 // get the time
    cur_time = localtime(&time_since); // convert to local time
    strftime(str, sizeof str, "%d/%m/%y %H:%M:%S", cur_time);

    if ((fp_log = fopen("plugins\\SPC.log", "w")) == NULL)
    {
        printf("Failed to open log file");
        return;
    }
    fprintf(fp_log, "*********************************************************************\n");
    fprintf(fp_log, "Spectrum LCDSmartie Plugin by Dave Perrow 8th March 2012\n");
    fprintf(fp_log, "SPC plugin version %s\n", SPC_FULLVERSION_STRING);
    fprintf(fp_log, "Log file Opened at %s\n", str);
    fprintf(fp_log, "*********************************************************************\n");
    fflush(fp_log);
    SPC_log("Log file Opened at", str);
}

//==============================================================================
// Function to close the log file
//==============================================================================
extern void __stdcall SPC_log_close()
{
    char str[120];
    time_t time_since;
    struct tm *cur_time;

    time(&time_since);                 // get the time
    cur_time = localtime(&time_since); // convert to local time
    strftime(str, sizeof str, "%d/%m/%y %H:%M:%S", cur_time);

    fprintf(fp_log, "*********************************************************************\n");
    fprintf(fp_log, "Spectrum LCDSmartie Plugin by Dave Perrow 8th March 2012\n");
    fprintf(fp_log, "SPC plugin version %s\n", SPC_FULLVERSION_STRING);
    fprintf(fp_log, "Log file Closed at %s\n", str);
    fprintf(fp_log, "*********************************************************************\n");
    fflush(fp_log);
    fclose(fp_log);
}

//==============================================================================
// Function to Log some text to the log file
// in the form: hh:mm:ss 23/07/11 <Module_Name> <Text>
//==============================================================================
extern void __stdcall SPC_log(char *module, char *msg)
{
    time_t time_since;
    struct tm *cur_time;
    struct _timeb timebuffer;
    char log_time[120];
    char timebit[256];

    _ftime(&timebuffer);
    time(&time_since);                 // get the time
    cur_time = localtime(&time_since); // convert to local time
    strftime(log_time, sizeof log_time, "%H:%M:%S", cur_time);
    sprintf(timebit, "%s.%03hu %s ", log_time, timebuffer.millitm, module);

    fprintf(fp_log, "%s%s\n", timebit, msg);
    fflush(fp_log);
}
