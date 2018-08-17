
#ifndef	_INCLUDED_SDLAPP_H
#define	_INCLUDED_SDLAPP_H


// The great SDL!
#ifndef	_SDL_H
	#include <SDL.h>
#endif


// Make the exception struct part of the API
#ifndef	_INCLUDED_EXCEPTION_H
	#include "Exception.h"
#endif


// Blegh... stupid Windows (... for OpenGL ...)
#ifdef	WIN32
	#define WINGDIAPI
	#define	APIENTRY	__stdcall
	#define	CALLBACK
#endif


// OpenGL stuff
#include <gl/gl.h>
#include <gl/glu.h>


class cSDLAFont;


class cSDLApp
{
public:
	// Constructor
	cSDLApp(const int width, const int height, const bool is_2d);
	~cSDLApp(void);

	// Run the application
	void	Run(void);

protected:
	// Called every frame
	virtual bool	ProcessFrame(void) = 0;

	// Called whenever a switch between full-screen and windowed is made.
	// AfterSwitch() is also called when the app boots.
	virtual void	BeforeSwitch(void) = 0;
	virtual void	AfterSwitch(void) = 0;

	// Create a font using the required database
	cSDLAFont*	CreateFont(const char* font_db) const;

	// Window dimensions
	int		m_Width;
	int		m_Height;

	// Array of key-press states
	bool	m_Keys[SDLK_LAST];
	bool	m_KeysReleased[SDLK_LAST];

	// Position of mouse cursor relative to window client origin
	int		m_MouseX;
	int		m_MouseY;

	// Mouse button press state
	Uint8	m_MouseButtons;

	// Current video config info
	const SDL_VideoInfo*	m_VideoInfo;

	// Is the application currently fullscreen?
	bool	m_IsFullscreen;

	bool	m_Is2D;

private:
	void	SetupOpenGL(const bool is_2d);

	// Array of key-presses for the last frame
	bool	m_KeysLastFrame[SDLK_LAST];
};


#endif	/* _INCLUDED_SDLAPP_H */