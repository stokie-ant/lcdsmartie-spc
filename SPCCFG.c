//==============================================================================
// Routines to do initialisation stuff
//==============================================================================
//==============================================================================
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "SPC.h"

int SPC_Global_Debug = 0;
int SPC_Global_AudioDevice = 0;
int SPC_Global_AudioInput = 0;
float SPC_Global_scale = 1.0;
int SPC_UseWasapi = 1;
int SPC_Global_BlackChar = 0;
int SPC_Bottom_Line = 0;
int AutoScale = 0;
float MinScale = 0.1;
float MaxScale = 3.0;
float UpFactor = 0.01;
float DownFactor = 0.01;
int ScaleBar = 0;

//==============================================================================
// function to copy a string and set the first newline character to 0
//==============================================================================
extern void __stdcall strcpytonl(char *so, char *si)
{
    int i;
    for (i = 0; i < strlen(si); i++)
    {
        if (si[i] == '\n')
        {
            so[i] = 0;
            break;
        }
        else
            so[i] = si[i];
    }
}

//==============================================================================
// function to read from the configuration file
//==============================================================================
extern void __stdcall SPC_read_cfg(int callnum)
{
    FILE *fp;
    char line[128], str[80], /*str2[80],*/ mod[80];
    int linelen/*, i, j, k, l*/;

    //
    // Set globals to default values
    //
    strcpy(mod, "SPC_read_cfg_file:");
    if (callnum == 1) // Do this bit on first call only
    {
        SPC_Global_Debug = 0;
        SPC_Global_AudioDevice = 0;
        SPC_Global_scale = 1.0;
        SPC_UseWasapi = SPC_check_os();
        SPC_Global_BlackChar = 0;
        SPC_Bottom_Line = 0;
        AutoScale = 0;
        MinScale = 0.1;
        MaxScale = 3.0;
        UpFactor = 0.01;
        DownFactor = 0.01;
        ScaleBar = 0;
    }

    SPC_log(mod, "Started");
    //
    // open the configuration file then read each line looking for specific names and store the values
    //
    fp = fopen("plugins\\SPC.cfg", "r");
    if (!fp)
        SPC_log(mod, "Failed to open cfg file");
    SPC_log(mod, "File Opened");
    if (fp != NULL)
    {
        while (fgets(line, 128, fp))
        {
            strcpytonl(line, line); // remove \n character
            linelen = strlen(line);
            if (linelen > 0)
            {
                if ((line[0] != '/') && (line[0] != '\r') && (line[0] != '\n') && (line[0] != ' '))
                {
                    if (!strncmp(line, "Debug=", 6))
                        sscanf(&line[6], "%d", &SPC_Global_Debug);
                    else if (!strncmp(line, "AudioDevice=", 12))
                        sscanf(&line[12], "%d", &SPC_Global_AudioDevice);
                    else if (!strncmp(line, "AudioInput=", 11))
                        sscanf(&line[11], "%d", &SPC_Global_AudioInput);
                    else if (!strncmp(line, "Scaling=", 8))
                        sscanf(&line[8], "%f", &SPC_Global_scale);
                    else if (!strncmp(line, "UseWasapi=", 10))
                        sscanf(&line[10], "%d", &SPC_UseWasapi);
                    else if (!strncmp(line, "BlackChar=", 10))
                        sscanf(&line[10], "%d", &SPC_Global_BlackChar);
                    else if (!strncmp(line, "BottomLine=", 11))
                        sscanf(&line[11], "%d", &SPC_Bottom_Line);

                    else if (!strncmp(line, "AutoScale=", 10))
                        sscanf(&line[10], "%d", &AutoScale);

                    else if (!strncmp(line, "MaxScale=", 9))
                        sscanf(&line[9], "%f", &MaxScale);

                    else if (!strncmp(line, "MinScale=", 9))
                        sscanf(&line[9], "%f", &MinScale);

                    else if (!strncmp(line, "UpFactor=", 9))
                        sscanf(&line[9], "%f", &UpFactor);

                    else if (!strncmp(line, "DownFactor=", 11))
                        sscanf(&line[11], "%f", &DownFactor);

                    else if (!strncmp(line, "ScaleBar=", 9))
                        sscanf(&line[9], "%d", &ScaleBar);
                    else
                        SPC_log("SCP_Init Error - Line not processed:", line);
                }
            }
        }
        fclose(fp);
    }
    // log the cfg file settings
    sprintf(str, "AudioDevice=%d", SPC_Global_AudioDevice);
    SPC_log(mod, str);
    sprintf(str, "AudioInput=%d", SPC_Global_AudioInput);
    SPC_log(mod, str);
    sprintf(str, "Scaling=%f", SPC_Global_scale);
    SPC_log(mod, str);
    sprintf(str, "UseWasapi=%d", SPC_UseWasapi);
    SPC_log(mod, str);
    sprintf(str, "BlackChar=%d", SPC_Global_BlackChar);
    SPC_log(mod, str);
    sprintf(str, "BottomLine=%d", SPC_Bottom_Line);
    SPC_log(mod, str);
    sprintf(str, "AutoScale=%d", AutoScale);
    SPC_log(mod, str);
    sprintf(str, "MaxScale=%f", MaxScale);
    SPC_log(mod, str);
    sprintf(str, "MinScale=%f", MinScale);
    SPC_log(mod, str);
    sprintf(str, "UpFactor=%f", UpFactor);
    SPC_log(mod, str);
    sprintf(str, "DownFactor=%f", DownFactor);
    SPC_log(mod, str);
    sprintf(str, "ScaleBar=%d", ScaleBar);
    SPC_log(mod, str);
    SPC_log(mod, "Finished");
}

//==============================================================================
// function to Check if the OS is Vista or later
// Version Number    Description
// 6.1               Windows 7     / Windows 2008 R2
// 6.0               Windows Vista / Windows 2008
// 5.2               Windows 2003
// 5.1               Windows XP
// 5.0               Windows 2000
//==============================================================================
extern int __stdcall SPC_check_os(void)
{
    char str[80];
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    sprintf(str, "%lu.%lu", osvi.dwMajorVersion, osvi.dwMinorVersion);
    SPC_log("OS =", str);

    if (osvi.dwMajorVersion > 5)
        return 1;
    else
        return 0; // It's Vista or later
}
