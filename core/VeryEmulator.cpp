#include "SDL.h"
#include "SDL_main.h"
#include <glad/glad.h>

bool quitting = false;
void PollEvents()
{
	SDL_Event event;
	while ( SDL_PollEvent( &event ) )
	{
		switch ( event.type )
		{
			case SDL_QUIT:
			{
				quitting = true;
				// Log( "Quitting App..." );
				break;
			}
        }
    }
}

SDLMAIN_DECLSPEC int SDL_main(int argc, char *argv[])
{
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER ) < 0 )
	{
		// LogError( "Failed to initialize SDL [%s]", SDL_GetError() );
		return false;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );

	// auto& cl = Util::CommandLine::Get();
	// const int winWidth = cl.GetOption( "windowwidth", 640 );
	// const int winHeight = cl.GetOption( "windowheight", 480 );

	const int winWidth = 640;
	const int winHeight = 480;

	const uint32_t winFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	auto m_window = SDL_CreateWindow( "PSX Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, winWidth, winHeight, winFlags );
	if ( m_window == nullptr )
	{
		// LogError( "Failed to create SDL window [%s]", SDL_GetError() );
		return false;
	}

	auto m_glContext = SDL_GL_CreateContext( m_window );
	if ( m_glContext == nullptr )
	{
		// LogError( "Failed to create OpenGL context [%s]", SDL_GetError() );
		return false;
	}

	if ( !gladLoadGLLoader( SDL_GL_GetProcAddress ) )
	{
		// LogError( "Failed to initialize OpenGL context" );
		return false;
	}

	// Log( "GL_VENDOR:   %s", glGetString( GL_VENDOR ) );
	// Log( "GL_RENDERER: %s", glGetString( GL_RENDERER ) );
	// Log( "GL_VERSION:  %s", glGetString( GL_VERSION ) );

	// clear default framebuffer
	glClearColor( 0, 0, 0, 1 );
	glViewport( 0, 0, winWidth, winHeight );
	glClear( GL_COLOR_BUFFER_BIT );

    while(!quitting) {
        PollEvents();

       	glClearColor( 1, 0, 0, 1 );
        glViewport( 0, 0, winWidth, winHeight );
        glClear( GL_COLOR_BUFFER_BIT );
      	SDL_GL_SwapWindow( m_window );
    }

    return 0;
}
