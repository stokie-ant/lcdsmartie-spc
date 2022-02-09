#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>
#include "SPC.h"
#include "bass.h"
#include "basswasapi.h"

int SPC_imin, SPC_imax, SPC_fmin, SPC_fmax, SPC_icount, SPC_fcount;

static double SPC_last_called; // When the display was last called for
DWORD spectrum_timer;          // Flag to indicate if timer running
int spectrum_width = MAX_WIDTH,
    spectrum_height = MAX_HEIGHT; // width and height of display chars x bits
int audio_device = (-1);          //
int spectrum_array[MAX_WIDTH];    // array to hold heights for the display
int SPC_started = 0;              // Flag indicating that BASS devices have been initialised
float scaleby = 1;
float ascale=1;
HRECORD SPC_chan;

//==============================================================================
// function to copy a string up to char c or 0
//==============================================================================
extern int __stdcall substrcpy(unsigned char *out, unsigned char *in, unsigned char c)
{
    int i, ret;

    ret = 0;
    for (i = 0; i < (strlen((char *)in) + 1); i++)
    {
        out[i] = in[i];
        if ((in[i] == c) || (in[i] == 0))
        {
            out[i] = 0;
            ret = i;
            break;
        }
    }
    return ret;
}

//==============================================================================
// Locking routines - should be used around access to the spectrum_array
//==============================================================================
static int lock_initialised = 0;
HANDLE ghSemaphore;
#define MAX_SEM_COUNT 10

//==============================================================================
// Routine to lock
//==============================================================================
extern void __stdcall SPC_lock()
{
  //  DWORD dwWaitResult;
    char str[80];

    if (lock_initialised == 0)
    {
        // Create a semaphore with initial and max counts of MAX_SEM_COUNT
        // default security attributes // initial count // maximum count // unnamed semaphore
        ghSemaphore = CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT, NULL);
        if (ghSemaphore == NULL)
        {
            sprintf(str, " %lu", GetLastError()); //
            SPC_log("SPC_Lock:", str);
            return;
        }
    }
    lock_initialised = 1;
    /*dwWaitResult =*/ WaitForSingleObject(ghSemaphore, INFINITE);
}

//==============================================================================
// Unlock routine
//==============================================================================
extern void __stdcall SPC_unlock()
{
    char str[80];
    // handle to semaphore // increase count by one // not interested in previous count
    if (!ReleaseSemaphore(ghSemaphore, 1, NULL))
    {
        sprintf(str, "ReleaseSemaphore error: %ld\n", GetLastError());
        SPC_log("SPC_unlock", str);
    }
}

//==============================================================================
// Recording callback - not doing anything with the data
// Function does nothing.
//==============================================================================
DWORD CALLBACK SPC_DummyRoutine(void *ptr, DWORD length, void *user)
{
    int i;
    float imax, imin;
    float *buffer;
    buffer = (float *)ptr;
    imax = buffer[1];
    imin = buffer[1];
    for (i = 2; i < (length / 4); i++)
    {
        if (buffer[i] > imax)
            imax = buffer[i];
        if (buffer[i] < imin)
            imin = buffer[i];
    }
    SPC_imin = (int)(imin * 100);
    SPC_imax = (int)(imax * 100);
    SPC_icount = length / 4;
    return 1;
} // continue recording
// Recording callback - not doing anything with the data
//BOOL CALLBACK SPC_DuffRecording(HRECORD handle, const void *buffer, DWORD length, void *user)
BOOL CALLBACK SPC_DuffRecording(HRECORD handle, const void *ptr, DWORD length, void *user)
{
//    char str[80];
    int i;
    int imax, imin;
    short int *buffer;
    buffer = (short int *)ptr;
    imax = buffer[1];
    imin = buffer[1];
    for (i = 2; i < (length / 2); i++)
    {
        if (buffer[i] > imax)
            imax = buffer[i];
        if (buffer[i] < imin)
            imin = buffer[i];
    }
    //  sprintf(str,"length=%d, min=%d, max=%d",length,imin,imax); SPC_log("Callback",str);
    SPC_imin = imin;
    SPC_imax = imax;
    SPC_icount = length / 2;
    return TRUE;
} // continue recording

//==============================================================================
// Log a BASS error messages
//==============================================================================
void Error(char *mod, char *es)
{
    char mes[1024];
    sprintf(mes, "%s\n(error code: %d)", es, BASS_ErrorGetCode());
    SPC_log(mod, mes);
    //   MessageBox(win,mes,0,0);
}

//==============================================================================
// update the spectrum display - the interesting bit :)
// processes the FFT data from BASS and puts it in the spectrum_array
//==============================================================================
void CALLBACK SPC_UpdateSpectrum(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    int x, y, /*y1,*/ fftlen;
//    char str[80];
    float fft[2048]; // 1024 fft values returned *2
    DWORD i, j;
    float fmin, fmax;
    double tdiff, end_time;

    // Check if it's been more than SPC_UNUSED_TIME since the display routine was called.
    // If it has then close BASS if it isn't closed already
    end_time = (double)clock() / CLOCKS_PER_SEC; // Counter time
    tdiff = end_time - SPC_last_called;          // in seconds
    if ((int)tdiff > SPC_UNUSED_TIME)
    {
        if (spectrum_timer)
        {
            timeKillEvent(spectrum_timer);
            spectrum_timer = 0;
        }
        SPC_lock();
        for (x = 0; x < spectrum_width; x++)
            spectrum_array[x] = 0;
        SPC_unlock();
        return;
    }

    if (AutoScale > 0){
      scaleby = scaleby + UpFactor;
    if ( scaleby > MaxScale )
      scaleby = MaxScale; // cap it
    }

    if (SPC_started == 1)
    {
        for (x = 0; x < 2048; x++)
            fft[x] = 0.0; // clear the buffer
       // y1 = 0;
        if (SPC_UseWasapi == 1) // on Vista or Win7
        {
            BASS_WASAPI_SetDevice(audio_device);                                      // set device context to input/loopback device
            i = BASS_WASAPI_GetData(fft, BASS_DATA_FFT2048| BASS_DATA_FFT_NOWINDOW);// | BASS_DATA_FFT_NOWINDOW); // get the FFT data
        }
        else // on XP or earlier
        {
            i = BASS_ChannelGetData(SPC_chan, fft, BASS_DATA_FFT2048);// | BASS_DATA_FFT_NOWINDOW);
            if (i < 0)
                Error("Timer", "ChannelGetData-"); // get the FFT data
        }
        fftlen = ((int)i / 4);
        SPC_fcount = fftlen; // should always be 1024

        fmin = fft[1];
        fmax = fft[1];
        for (j = 1; j < 1024; j++)
        {
            if (fft[j] > fmax)
                fmax = fft[j];
            if (fft[j] < fmin)
                fmin = fft[j];
        }
        if (SPC_fmin > (fmin * 10000.0))
            SPC_fmin = (fmin * 10000.0);
        if (SPC_fmax < (fmax * 10000.0))
            SPC_fmax = (fmax * 10000.0);

        if (fftlen < 1024)
        {
            SPC_lock();
            for (x = 0; x < spectrum_width; x++)
                spectrum_array[x] = 0;
            SPC_unlock();
        }
        else
        {
            int b0 = 0;
            int maxy = 0;
            SPC_lock();
            ascale=scaleby;
            for (x = 0; x < spectrum_width; x++)
            {
                float peak = 0;
                int b1 = pow(2, x * 10.0 / (spectrum_width - 1));
                if (b1 > 1023)
                    b1 = 1023;
                if (b1 <= b0)
                    b1 = b0 + 1; // make sure it uses at least 1 FFT bin
                for (; b0 < b1; b0++)
                    if (peak < fft[1 + b0])
                        peak = fft[1 + b0];

                y = (int)(sqrt(peak) * SPC_Global_scale * ascale * 3.5 * (float)(spectrum_height)); // scale it (sqrt to make low values more visible)
                if (y > spectrum_height){
                    y = spectrum_height; // cap it
                    if (AutoScale > 0){
                      scaleby = scaleby - DownFactor;
                      if (scaleby < MinScale)
                        scaleby = MinScale; // cap it
                    }

                    }

                if (y < 0)
                    y = 0;
                spectrum_array[x] = y;
                if (y > maxy)
                    maxy = y;
            }
            SPC_unlock();
        }
    }
    else
    {
        SPC_lock();
        for (x = 0; x < spectrum_width; x++)
            spectrum_array[x] = 0;
        SPC_unlock();
    }
}

//==============================================================================
// return loopback device index given device name
//==============================================================================
extern int __stdcall SPC_findloopbackdevice_byname(char* name)
{
	BASS_WASAPI_DEVICEINFO info;
	int i;
    for (i = 0; BASS_WASAPI_GetDeviceInfo(i, &info); i++)
	{
		if((strcmp(name, info.name)==0) && (info.flags & BASS_DEVICE_INPUT) && (info.flags & BASS_DEVICE_LOOPBACK) && (info.flags & BASS_DEVICE_ENABLED))
            return i;
	}
	return 0;
}

//==============================================================================
// return default output device name
//==============================================================================
extern int __stdcall SPC_finddefaultdevice(char* name)
{
	BASS_WASAPI_DEVICEINFO info;
	int i;
	for (i = 0; BASS_WASAPI_GetDeviceInfo(i, &info); i++)
	{
		if ((info.flags & BASS_DEVICE_DEFAULT) && !(info.flags & BASS_DEVICE_INPUT))
		{
		    strcpy(name, info.name);
		    return i;
		}
    }
    return 0;
}

//==============================================================================
// Check if given index is default device. For checking if it changed ie. plugging in headphones
//==============================================================================
extern int __stdcall SPC_isdefaultdevice(int devindex)
{
	BASS_WASAPI_DEVICEINFO info;
	BASS_WASAPI_GetDeviceInfo(devindex, &info);
	if((info.flags & BASS_DEVICE_DEFAULT) && !(info.flags & BASS_DEVICE_INPUT))
	{
		return 0;
	}

	return 1;
}

//==============================================================================
// Initialisation - find the right device and initialise the BASS functions
//==============================================================================
extern int __stdcall SPC_init()
{
    int i, j;
    char str[256];
    BASS_WASAPI_DEVICEINFO info;
    BASS_DEVICEINFO binfo;
    const char *devname;
	char name[256];

    for (i = 0; i < MAX_WIDTH; i++)
        spectrum_array[i] = 0;
    SPC_started = 0;
    audio_device = 0;
    if (SPC_UseWasapi == 1) // on Vista or Win7

    {
    	SPC_finddefaultdevice(name);
    	audio_device = SPC_findloopbackdevice_byname(name);

        for (i = 0; BASS_WASAPI_GetDeviceInfo(i, &info); i++)
        {
           /* if ((info.flags & BASS_DEVICE_INPUT)                                              // device is an loopback device (not input)
                && (info.flags & BASS_DEVICE_LOOPBACK) && (info.flags & BASS_DEVICE_ENABLED)) // and it is enabled
            {
                if (audio_device == 0)
                    audio_device = i;
            }*/

            sprintf(str, "entry=%d, type=%lu, mixfreq=%lu, mixchans=%lu, flags=%lu, name=%s",
                    i, info.type, info.mixfreq, info.mixchans, info.flags, info.name);

            if (info.flags & BASS_DEVICE_ENABLED)
                strcat(str, "[Enabled]");
            if (info.flags & BASS_DEVICE_DEFAULT)
                strcat(str, "[Default]");
            if (info.flags & BASS_DEVICE_INIT)
                strcat(str, "[Init]");
            if (info.flags & BASS_DEVICE_INPUT)
                strcat(str, "[Input]");
            if (info.flags & BASS_DEVICE_LOOPBACK)
                strcat(str, "[Loopback]");

            if (info.type == BASS_WASAPI_TYPE_NETWORKDEVICE)
                strcat(str, " [BASS_WASAPI_TYPE_NETWORKDEVICE]");
            if (info.type == BASS_WASAPI_TYPE_SPEAKERS)
                strcat(str, " [BASS_WASAPI_TYPE_SPEAKERS]");
            if (info.type == BASS_WASAPI_TYPE_LINELEVEL)
                strcat(str, " [BASS_WASAPI_TYPE_LINELEVEL]");
            if (info.type == BASS_WASAPI_TYPE_HEADPHONES)
                strcat(str, " [BASS_WASAPI_TYPE_HEADPHONES]");
            if (info.type == BASS_WASAPI_TYPE_MICROPHONE)
                strcat(str, " [BASS_WASAPI_TYPE_MICROPHONE]");
            if (info.type == BASS_WASAPI_TYPE_HEADSET)
                strcat(str, " [BASS_WASAPI_TYPE_HEADSET]");
            if (info.type == BASS_WASAPI_TYPE_HANDSET)
                strcat(str, " [BASS_WASAPI_TYPE_HANDSET]");
            if (info.type == BASS_WASAPI_TYPE_DIGITAL)
                strcat(str, " [BASS_WASAPI_TYPE_DIGITAL]");
            if (info.type == BASS_WASAPI_TYPE_SPDIF)
                strcat(str, " [BASS_WASAPI_TYPE_SPDIF]");
            if (info.type == BASS_WASAPI_TYPE_HDMI)
                strcat(str, " [BASS_WASAPI_TYPE_HDMI]");
            if (info.type == BASS_WASAPI_TYPE_UNKNOWN)
                strcat(str, " [BASS_WASAPI_TYPE_UNKNOWN]");

            SPC_log("SPC_init", str);
        }
        if (SPC_Global_AudioDevice != 0)
            audio_device = SPC_Global_AudioDevice;
        sprintf(str, "Chosen audio device is %d", audio_device);
        SPC_log("SPC_init", str);

        BASS_WASAPI_GetDeviceInfo(audio_device, &info);
        // initialize BASS 'no sound' device to allow double buffering
        if (!BASS_Init(0, 44100, 0, 0, NULL))
        {
            Error("SPC_init", "Can't initialize no sound device");
            return 0;
        }
        // initialise our recording device at 44100hz stereo
        if (!BASS_WASAPI_Init(audio_device, info.mixfreq, info.mixchans,
                              BASS_WASAPI_BUFFER,
                              0.5, 0, &SPC_DummyRoutine, NULL))
        {
            Error("SPC_init", "Can't initialize chosen audio device");
            return 0;
        }
        // Initialise the main device too since this prevents buzzing
        BASS_WASAPI_Init(audio_device - 1, 0, 0, 0, 0.5, 0, NULL, NULL); // initialize corresponding output device
        BASS_WASAPI_SetDevice(audio_device);                             // set device context back to input/loopback device
        // start recording (44100hz mono 16-bit)
        if (!(BASS_WASAPI_Start()))
        {
            Error("SPC_init", "Can't start recording");
            return 0;
        }
    }
    else // on XP or earlier
    {
        // Log the devices
        for (i = 0; BASS_GetDeviceInfo(i, &binfo); i++)
        {
            sprintf(str, "Device %d:", i);
            strcat(str, binfo.name);
            if (binfo.flags & BASS_DEVICE_ENABLED)
                strcat(str, "[Enabled]");
            if (binfo.flags & BASS_DEVICE_DEFAULT)
                strcat(str, "[Default]");
            if (binfo.flags & BASS_DEVICE_INIT)
                strcat(str, "[Init]");
            SPC_log("SPC_init", str);

            // Log the inputs
            if (BASS_RecordInit(i))
            {
                for (j = 0; (devname = BASS_RecordGetInputName(j)); j++)
                {
                    sprintf(str, "         Input %d:", j);
                    strcat(str, devname);
                    if (!(BASS_RecordGetInput(j, NULL) & BASS_INPUT_OFF))
                        strcat(str, " [On]");
                    SPC_log("SPC_init", str);
                }
                BASS_RecordFree(); // free current device (and recording channel) if there is one
            }
        }

        sprintf(str, "Chosen audio device is %d input %d", SPC_Global_AudioDevice, SPC_Global_AudioInput);
        SPC_log("SPC_init", str);
        // initialize BASS recording (default device)
        if (!BASS_RecordInit(SPC_Global_AudioDevice))
        {
            Error("SPC_init", "Can't initialize chosen audio device");
            return 0;
        }
        if (!BASS_RecordSetDevice(SPC_Global_AudioDevice))
        {
            Error("SPC_init", "Can't Set Device");
            return 0;
        }
        for (i = 0; BASS_RecordSetInput(i, BASS_INPUT_OFF, -1); i++)
            ; // 1st disable all inputs, then...
        if (!BASS_RecordSetInput(SPC_Global_AudioInput, BASS_INPUT_ON, 1))
        {
            Error("SPC_init", "Can't Set Input");
            return 0;
        }
        SPC_chan = BASS_RecordStart(44100, 2, 0, &SPC_DuffRecording, 0); // start recording (44100hz mono 16-bit) 200msecs between calls
        if (!SPC_chan)
        {
            Error("SPC_init", "Can't start recording");
            return 0;
        }
    }

    SPC_started = 1;
    SPC_last_called = (double)clock() / CLOCKS_PER_SEC; // Counter time

    spectrum_timer = timeSetEvent(25, 25, (LPTIMECALLBACK)&SPC_UpdateSpectrum, 0, TIME_PERIODIC);
    return 1;
}

//==============================================================================
// Close down the timer callbacks and free any BASS resources
//==============================================================================
extern int __stdcall SPC_close()
{
    if (spectrum_timer)
    {
        timeKillEvent(spectrum_timer);
        spectrum_timer = 0;
    }
    if (SPC_UseWasapi == 1)
    {
        if (SPC_started == 1)
        {
            BASS_WASAPI_Stop(TRUE);
        }
        SPC_started = 0;
        BASS_WASAPI_SetDevice(audio_device); // set device context back to input/loopback device
        BASS_WASAPI_Free();
        BASS_WASAPI_SetDevice(audio_device - 1); // set device context back to playback device
        BASS_Free();
        BASS_SetDevice(0); // set device context back to input/loopback device
        BASS_Free();       // free null device too
    }
    else
    {
        BASS_RecordFree();
    }

    return 1;
}

//==============================================================================
// Routine to copy the spectrum_array for the caller
//==============================================================================
extern int __stdcall SPC_get(int *array)
{
    int i;
    float sbar;
    SPC_last_called = (double)clock() / CLOCKS_PER_SEC; // Counter time
    if (SPC_UseWasapi == 1)
    {
        i = BASS_WASAPI_GetData(NULL, BASS_DATA_AVAILABLE);
    }
    else
    {
        i = BASS_ChannelGetData(SPC_chan, NULL, BASS_DATA_AVAILABLE); // get the Qty of sample data
    }
    //    sprintf(str,"%d bytes available",i);
    //    SPC_log("Update2",str);
    if ((i > 0) && (!spectrum_timer))
        spectrum_timer = timeSetEvent(25, 25, (LPTIMECALLBACK)&SPC_UpdateSpectrum, 0, TIME_PERIODIC);

    SPC_lock();
    for (i = 0; i < spectrum_width; i++){
        array[i] = spectrum_array[i];

        if (ScaleBar > 0){
          sbar = (scaleby/(MaxScale - MinScale))*MAX_HEIGHT;
          array[spectrum_width - 1] = sbar;
        }

        }
    SPC_unlock();
    return 0;
}

extern DWORD __stdcall SPC_get_level(int chan, int linear, int smooth)
{
    DWORD level;
    DWORD left, right;
    double dleft, dright;
    static int vol_smoothl[256], vol_smoothr[256];
    static int vsptrl = 0, vsptrr = 0;
    long tleft = 0, tright = 0;
    int i;

    if (SPC_UseWasapi == 1)
    {
        level = BASS_WASAPI_GetLevel();
    }
    else
    {
        level = BASS_ChannelGetLevel(SPC_chan);
    }

    left = LOWORD(level);  // the left level
    right = HIWORD(level); // the right level

    if (linear == 1) // return the logarithmic value
    {
        dleft = exp(((double)left) / 3277.0) / exp(10.0);
        dright = exp(((double)right) / 3277.0) / exp(10.0);
        left = (unsigned int)(dleft * 32768.0);
        right = (unsigned int)(dright * 32768.0);
    }
    else if (linear == 2)
    {
        if (left > 0)
            left = (int)(32768.0 * log10((double)left / 3277.0));
        if (right > 0)
            right = (int)(32768.0 * log10((double)right / 3277.0));
    }

    // If the user has asked for integration then do it.
    if (smooth > 256)
        smooth = 256;
    if (smooth > 0)
    {
        vol_smoothl[vsptrl] = left;
        vol_smoothr[vsptrr] = right;
        vsptrl = (vsptrl + 1) % smooth;
        vsptrr = (vsptrr + 1) % smooth;
        for (i = 0; i < smooth; i++)
        {
            tleft = tleft + vol_smoothl[i];
            tright = tright + vol_smoothr[i];
        }
        left = (int)(tleft / (long)smooth);
        right = (int)(tright / (long)smooth);
    }

    switch (chan)
    {
    case 1:
        return left;
    case 2:
        return right;
    default:
        return (left + right) / 2;
    }
}
