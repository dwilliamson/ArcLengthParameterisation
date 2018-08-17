
SDLApp Framework
================

This is a very simple little library that handles initialisation and shutdown of both SDL
and OpenGL in a C++ type framework with a little bit of nice exception handling. I use it
whenever I need to write quick test applications.

This is something that's always confused me about Windows programming. Back in the DOS days
I could whip up an ASM set video mode and plot pixel in 30 seconds and a graphical text
print routine within say another minute. Ready to test!

SDL's a great step in the right direction (and it's cross-platform as a bonus, too) but
it's still too much effort for even the simplest of tests. Plus it has no font support
(not that I would want SDL to support such a feature). This framework can get me up and
running in under a minute which is just as good as it was back with DOS.

(In fairness, I am getting a lot more now for my initialising troubles than I was back in
the DOS days).

To use it simply point your Visual Studio directories to the include and lib path and
link with SDLAppD.lib (debug) or SDLApp.lib (release). Main header include is <SDLApp.h>.
You can create and store fonts with that header file but if you want to use the fonts you'll
have to include <SDLAFont.h> along with it.

There is no main() function, just an application object which you will use to initialise in
the constructor and shutdown in the destructor. SDLApp handles all the message processing,
key-press and mouse movement recording, OpenGL buffer swapping, and other things. So,
just create an application class that inherits from cSDLApp, like this:


	class cMyApp : public cSDLApp
	{
	public:
		// Constructor/destructor
		cMyApp(const int width, const int height);
		~cMyApp(void);

	private:
		// You need to provide implementations for these methods
		bool ProcessFrame(void);
		void BeforeSwitch(void);
		void AfterSwitch(void);
	};


Constructor
-----------
The constructor to cSDLApp is of type (const int, const int, const bool). The first and
second parameters are the window width/height, the third parameter is if you want your
OpenGL session to be 2D or not. You will need to pass width/height and your choice of
2D/3D onto cSDLApp in your cMyApp constructor initialiser list, over-riding width and
height if you so want (default is 640x480).

ProcessFrame
------------
This gets called every frame after all message handling has been performed. At this point
in time you can inspect the m_Keys[] array to see which keys are currently held down, and
the m_KeysReleased[] array to see which keys were released on the last frame. The array
is indexed with the SDLK_* constants. You can inspect m_MouseX and m_MouseY for the current
mouse position relative to the top-left of the screen. m_MouseButtons contains the SDL
mouse button press state. m_IsFullScreen tells you whether the app is currently full-screen
or not (should really be hidden behind a const accessor so play nice and don't modify it!).

BeforeSwitch
AfterSwitch
------------
You can press ALT+ENTER in any SDLApp to switch from windowed to full-screen and vice
versa. These functions get called before the switch and after the switch so that you can
release and re-initialise any volatile video data you have lying around.


After you have done all that create a Main.cpp file and put only these three lines in it:

	#include "MyApp.h"
	#define SDL_APP_TYPE cMyApp
	#include <SDLAppRun.h>

And your program should now run! There are so many different ways the above functionality
could have been achieved which are a lot more elegant than this but none are faster to
use than this!


Exceptions
----------
Anytime during the program you can throw exceptions of type cException and they will be
caught and output to a file called stderr.txt (easy cross-platform, messy cross-platform
is displaying a dialogue box). Example:

	if (some_error_occurred)
		throw cException("Exception of type %d", error_number);

Arguments to the exception constructor are the same format as the printf() command.


Fonts
-----
There is a MakeFont utility which will generate 2 texture pages worth of any font you
specify and output a "font database". You can create fonts by calling the CreateFont()
method in cSDLApp with the only parameter being the filename of the font database.
Writing text with the font is simply:

	font_ptr->WriteText("My Text", xpos, ypos, scale);

Positions are from around -1.5 to 1.5 on the x, and -1 to 1 on the y. Scale 1 is normal
but 1.5 looks better. To facilitate writing text on multiple lines there's also a
GetLineHeight() method for you to use.


- Don Williamson (don@donw.co.uk)
  4 August, 2002