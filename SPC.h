

#ifdef __cplusplus
extern "C"
{
#endif

    extern int __stdcall substrcpy(unsigned char *out, unsigned char *in, unsigned char c);
    extern int __stdcall SPC_init();
    extern int __stdcall SPC_findloopbackdevice_byname(char* name);
    extern int __stdcall SPC_finddefaultdevice(char* name);
    extern int __stdcall SPC_isdefaultdevice(int devindex);
    extern int __stdcall SPC_close();
    extern int __stdcall SPC_get(int *specarray);
    extern DWORD __stdcall SPC_get_level(int chan, int linear, int smooth);
    extern void __stdcall SPC_log(char *module, char *msg);
    extern void __stdcall SPC_log_open();
    extern void __stdcall SPC_log_close();
    extern void __stdcall SPC_read_cfg(int callnum);
    extern int __stdcall SPC_check_os(void);

#ifdef __cplusplus
}
#endif

#define MAX_WIDTH 256
#define MAX_HEIGHT 32
#define SPC_UNUSED_TIME 2 // nr. seconds if display not called then stops recording

extern int spectrum_width, spectrum_height;

extern int SPC_Global_Debug;
extern int SPC_Global_AudioDevice, SPC_Global_AudioInput;
extern float SPC_Global_scale;
extern int SPC_Displaying;
extern int SPC_UseWasapi;
extern int SPC_Global_BlackChar;
extern int SPC_Bottom_Line;
extern int AutoScale;
extern float MaxScale;
extern float MinScale;
extern float UpFactor;
extern float DownFactor;
extern int ScaleBar;
extern int initialised;
extern int SPC_imin, SPC_imax, SPC_fmin, SPC_fmax, SPC_icount, SPC_fcount;
