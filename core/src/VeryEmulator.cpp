#include "SDL.h"
#include "SDL_main.h"
#include <glad/glad.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "Renderer.h"
#include "Nes.h"
#include "NesDebugInfo.h"

bool quitting = false;
void PollEvents()
{
	SDL_Event event;
	while ( SDL_PollEvent( &event ) )
	{
        ImGui_ImplSDL2_ProcessEvent(&event);
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

	const int winWidth = 1024;
	const int winHeight = 768;

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

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync

	if ( !gladLoadGLLoader( SDL_GL_GetProcAddress ) )
	{
		// LogError( "Failed to initialize OpenGL context" );
		return false;
	}

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 130");

	// Log( "GL_VENDOR:   %s", glGetString( GL_VENDOR ) );
	// Log( "GL_RENDERER: %s", glGetString( GL_RENDERER ) );
	// Log( "GL_VERSION:  %s", glGetString( GL_VERSION ) );

    Renderer *renderer = new Renderer();
    renderer->Initialize();

    Nes* nes = new Nes();
    nes->Initialize();
    nes->LoadGame("Roms/nestest.nes");

	bool show_demo_window;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while(!quitting) {
        PollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGuiNesDebug(nes);

        // // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        // {
        //     static float f = 0.0f;
        //     static int counter = 0;

        //     ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        //     ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //     ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

        //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //     ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        //     if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //         counter++;
        //     ImGui::SameLine();
        //     ImGui::Text("counter = %d", counter);

        //     ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        //     ImGui::End();
        // }

		// clear default framebuffer
        ImGui::Render();

        // run emu
        nes->Tick();
        
        // render
       	glClearColor( clear_color.x, clear_color.y, clear_color.z, clear_color.w );
        glClear( GL_COLOR_BUFFER_BIT );

        // renderer->DisplayFrame();
        nes->Present(renderer);

        glViewport( 0, 0, winWidth, winHeight );
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      	SDL_GL_SwapWindow( m_window );
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_window);
    SDL_Quit();

    return 0;
}
