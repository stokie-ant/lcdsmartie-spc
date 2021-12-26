System audio spectrum analyzer plugin for LCDSmartie

By Dave Perrow

I had difficulty finding the source for this one. It was originally hosted on codeplex.
Luckily a user has a copy on github https://github.com/eeyrw/LCDSPC-MOD

I have modified it further
 - Looks for the loopback device associated with the output device marked default
   - I have hdmi and spdif outputs that got detected first
   
 - Will now switch over if the device marked default changes
   - Some devices like mine offer two sound output devices one speaker and another headphones
     when plugging in headphones the default device changes from speakers to the headphones
	 
 - Graph width changed
   - I don't know the resoning but widths were locked at 4, 8, 16 and 32

 - Changed the way custom characters are called
   - Using 1, 2, 3, 4, 5, 6, 7 and 8 didnt work well with smarties display or the desktop display plugin
     Using 176, 158, 131, 132, 133, 134, 135 and 136 (the actual locations) allows the display to work properly

 - Removed a bunch of unused variables and used proper types
   - this just removes all the compiler warnings

Buit with codeblocks 32bit and mingw
Dev-c++ can compile but causes issues with locking code and breaks when optimized
visual c++ just doesn't work at all. would need quite a time investment to fix

If there is any issues please report them here