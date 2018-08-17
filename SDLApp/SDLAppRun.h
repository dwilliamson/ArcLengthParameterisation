
#ifndef	_INCLUDED_SDLAPPRUN_H
#define	_INCLUDED_SDLAPPRUN_H


#ifndef	SDL_APP_TYPE
	#error	"The macro SDL_APP_TYPE needs to be defined before included SDLAppRun.h !"
#endif


int main(int argc, char* argv[])
{
	// Execute me
	try
	{
		SDL_APP_TYPE app(640, 480);
		app.Run();
	}

	
	// Exit with any errors
	catch (const cException& exception)
	{
		printf("The program exited abormally with the following ERROR:\n %s\n", exception.GetErrorMessage());
		return (1);
	}

	return (0);
}


#endif	/* _INCLUDED_SDLAPPRUN_H */