
#include "SDLApp.h"
#include "SDLAFont.h"
#include <cstring>


cSDLApp::cSDLApp(const int width, const int height, const bool is_2d) :

	m_Width(width),
	m_Height(height),
	m_MouseX(0),
	m_MouseY(0),
	m_MouseButtons(0),
	m_IsFullscreen(false),
	m_Is2D(is_2d)

{
	// Clear key-state array
	memset(m_Keys, 0, sizeof(m_Keys));

	// Initialise SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		cException("Error initialising SDL - %d", SDL_GetError());

	// Setup OpenGL
	SetupOpenGL(is_2d);
}


cSDLApp::~cSDLApp(void)
{
	// Shutdown SDL
	SDL_Quit();
}


void cSDLApp::SetupOpenGL(const bool is_2d)
{
	// Retrieve current video info
	if ((m_VideoInfo = SDL_GetVideoInfo()) == 0)
		throw cException("Video query failed - %d", SDL_GetError());

	// Set OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	Uint32 full = m_IsFullscreen ? SDL_FULLSCREEN : 0;

	// Set the video mode
	if (!SDL_SetVideoMode(m_Width, m_Height, m_VideoInfo->vfmt->BitsPerPixel, SDL_OPENGL | full))
		throw cException("Video mode set failed - %d", SDL_GetError());

	// Gouraud shading
	glShadeModel(GL_SMOOTH);

	if (is_2d == false)
	{
		// Back-face culling
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);

		// Enable the z-buffer
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
	}

	// Enable pixel blend arithmetic
	glEnable(GL_BLEND);

	// Enable 2D texture mapping
	glEnable(GL_TEXTURE_2D);

	// Set the clear colour
	glClearColor(0, 0, 0, 0);

	// Setup the viewport
	glViewport(0, 0, m_Width, m_Height);

	// Set the projection matrix
	if (is_2d)
	{
		// Write all points flat
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glScalef((float)m_Height / (float)m_Width, 1, 1);
	}

	else
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0f, (float)m_Width / (float)m_Height, 1, 8192);
	}

	// Get all the possible GL strings
	const unsigned char* text_ptr[4];
	text_ptr[0] = glGetString(GL_VENDOR);
	text_ptr[1] = glGetString(GL_RENDERER);
	text_ptr[2] = glGetString(GL_VERSION);
	text_ptr[3] = glGetString(GL_EXTENSIONS);

	// Inform the user through stdout.txt
	printf("%s\n", text_ptr[0]);
	printf("%s\n", text_ptr[1]);
	printf("%s\n", text_ptr[2]);
	printf("%s\n", text_ptr[3]);

	// Set a nice little title
	SDL_WM_SetCaption("SDL+OpenGL Application", 0);
}


void cSDLApp::Run(void)
{
	// Can't set in constructor - vtable doesn't exist
	AfterSwitch();

	while (1)
	{
		SDL_Event	event;

		// Backup current frame keys
		memcpy(m_KeysLastFrame, m_Keys, sizeof(m_Keys));

		// Retrieve all events from the queue
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				// Handle key presses
				case (SDL_KEYDOWN):	m_Keys[event.key.keysym.sym] = true;	break;
				case (SDL_KEYUP):	m_Keys[event.key.keysym.sym] = false;	break;


				// Handle program exit
				case (SDL_QUIT):
					return;
			}
		}

		// Set the key released states
		for (int i = 0; i < SDLK_LAST; i++)
			m_KeysReleased[i] = (m_KeysLastFrame[i] ^ m_Keys[i]) & m_KeysLastFrame[i];

		// Get the current mouse state
		m_MouseButtons = SDL_GetMouseState(&m_MouseX, &m_MouseY);

		// Exit if the user presses escape
		if (m_Keys[SDLK_ESCAPE])
			return;

		// User want to toggle full-screen?
		if (m_Keys[SDLK_RETURN] && (m_Keys[SDLK_LALT] || m_Keys[SDLK_RALT]))
		{
			// Toggle fullscreen flag
			m_IsFullscreen ^= 1;

			// Set the required video mode
			BeforeSwitch();
			SetupOpenGL(m_Is2D);
			AfterSwitch();
		}


		// --- CALL USER CODE ---
		if (ProcessFrame() == false)
			return;
		// ----------------------


		// Make frame changes visible
		SDL_GL_SwapBuffers();
		glFlush();
	}
}


cSDLAFont* cSDLApp::CreateFont(const char* font_db) const
{
	return (new cSDLAFont(font_db, m_Width, m_Height));
}