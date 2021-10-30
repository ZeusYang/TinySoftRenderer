#include "TRWindowsApp.h"

#include <iostream>
#include <iomanip>

#include "TRParallelWrapper.h"

namespace TinyRenderer
{
	TRWindowsApp::ptr TRWindowsApp::m_instance = nullptr;

	bool TRWindowsApp::setup(int width, int height, std::string title)
	{
		m_screenWidth = width;
		m_screenHeight = height;
		m_windowTitle = title;

		m_lastMouseX = 0;
		m_lastMouseY = 0;
		m_mouseDeltaX = 0;
		m_mouseDeltaY = 0;

		//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		//Create window
		m_windowHandle = SDL_CreateWindow(
			m_windowTitle.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			m_screenWidth,
			m_screenHeight,
			SDL_WINDOW_SHOWN);

		if (m_windowHandle == nullptr)
		{
			std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		//Get window surface
		m_screenSurface = SDL_GetWindowSurface(m_windowHandle);

		return true;
	}

	TRWindowsApp::~TRWindowsApp()
	{
		//Destroy window
		SDL_DestroyWindow(m_windowHandle);
		m_windowHandle = nullptr;

		//Quit SDL subsystems
		SDL_Quit();
	}

	void TRWindowsApp::readyToStart()
	{
		m_timer.start();
		m_lastTimePoint = m_timer.getTicks();

		m_fps = 0;
		m_fpsCounter = 0.0f;
		m_fpsTimeRecorder = 0.0f;
	}

	void TRWindowsApp::processEvent()
	{
		m_wheelDelta = 0;
		//Handle events queue
		while (SDL_PollEvent(&m_events) != 0)
		{
			//Quit the program
			if (m_events.type == SDL_QUIT || (
				m_events.type == SDL_KEYDOWN && m_events.key.keysym.sym == SDLK_ESCAPE))
			{
				m_quit = true;
			}
			if (m_events.type == SDL_MOUSEMOTION)
			{
				static bool firstEvent = true;
				if (firstEvent)
				{
					firstEvent = false;
					m_lastMouseX = m_events.motion.x;
					m_lastMouseY = m_events.motion.y;
					m_mouseDeltaX = 0;
					m_mouseDeltaY = 0;
				}
				else
				{
					m_mouseDeltaX = m_events.motion.x - m_lastMouseX;
					m_mouseDeltaY = m_events.motion.y - m_lastMouseY;
					m_lastMouseX = m_events.motion.x;
					m_lastMouseY = m_events.motion.y;
				}
			}
			if (m_events.type == SDL_MOUSEBUTTONDOWN && m_events.button.button == SDL_BUTTON_LEFT)
			{
				m_mouseLeftButtonPressed = true;
				m_lastMouseX = m_events.motion.x;
				m_lastMouseY = m_events.motion.y;
				m_mouseDeltaX = 0;
				m_mouseDeltaY = 0;
			}
			if (m_events.type == SDL_MOUSEBUTTONUP && m_events.button.button == SDL_BUTTON_LEFT)
			{
				m_mouseLeftButtonPressed = false;
			}
			if (m_events.type == SDL_MOUSEWHEEL)
			{
				m_wheelDelta = m_events.wheel.y;
			}
		}
	}

	double TRWindowsApp::updateScreenSurface(
		unsigned char *pixels,
		int width,
		int height,
		int channel,
		unsigned int num_triangles)
	{
		//Update pixels
		SDL_LockSurface(m_screenSurface);
		{
			Uint32* destPixels = (Uint32*)m_screenSurface->pixels;
			parallelFor((size_t)0, (size_t)width *height, [&](const size_t &index)
			{
				Uint32 color = SDL_MapRGB(
					m_screenSurface->format,
					static_cast<uint8_t>(pixels[index * channel + 0]),
					static_cast<uint8_t>(pixels[index * channel + 1]),
					static_cast<uint8_t>(pixels[index * channel + 2]));
				destPixels[index] = color;
			});
		}
		SDL_UnlockSurface(m_screenSurface);
		SDL_UpdateWindowSurface(m_windowHandle);

		m_deltaTime = m_timer.getTicks() - m_lastTimePoint;
		m_lastTimePoint = m_timer.getTicks();

		//FPS counting
		{
			m_fpsTimeRecorder += m_deltaTime;
			++m_fpsCounter;
			if (m_fpsTimeRecorder > 1000.0)
			{
				m_fps = static_cast<unsigned int>(m_fpsCounter);
				m_fpsCounter = 0.0f;
				m_fpsTimeRecorder = 0.0f;

				std::stringstream ss;
				ss << " FPS:" << std::setiosflags(std::ios::left) << std::setw(3) << m_fps;
				ss << " #Triangles:" << std::setiosflags(std::ios::left) << std::setw(5) << num_triangles;
				SDL_SetWindowTitle(m_windowHandle, (m_windowTitle + ss.str()).c_str());
			}
		}

		return m_deltaTime;
	}

	TRWindowsApp::ptr TRWindowsApp::getInstance()
	{
		if (m_instance == nullptr)
		{
			return getInstance(800, 600, "WinApp");
		}
		return m_instance;
	}

	TRWindowsApp::ptr TRWindowsApp::getInstance(int width, int height, const std::string title)
	{
		if (m_instance == nullptr)
		{
			m_instance = std::shared_ptr<TRWindowsApp>(new TRWindowsApp());
			if (!m_instance->setup(width, height, title))
			{
				return nullptr;
			}
		}
		return m_instance;
	}

	//-------------------------------------------Timer---------------------------------------------------

	TRWindowsApp::LTimer::LTimer()
	{
		//Initialize the variables
		m_startTicks = 0;
		m_pausedTicks = 0;

		m_paused = false;
		m_started = false;
	}

	void TRWindowsApp::LTimer::start()
	{
		//Start the timer
		m_started = true;

		//Unpause the timer
		m_paused = false;

		//Get the current clock time
		m_startTicks = SDL_GetTicks();
		m_pausedTicks = 0;
	}

	void TRWindowsApp::LTimer::stop()
	{
		//Stop the timer
		m_started = false;

		//Unpause the timer
		m_paused = false;

		//Clear tick variables
		m_startTicks = 0;
		m_pausedTicks = 0;
	}

	void TRWindowsApp::LTimer::pause()
	{
		//If the timer is running and isn't already paused
		if (m_started && !m_paused)
		{
			//Pause the timer
			m_paused = true;

			//Calculate the paused ticks
			m_pausedTicks = SDL_GetTicks() - m_startTicks;
			m_startTicks = 0;
		}
	}

	void TRWindowsApp::LTimer::unpause()
	{
		//If the timer is running and paused
		if (m_started && m_paused)
		{
			//Unpause the timer
			m_paused = false;

			//Reset the starting ticks
			m_startTicks = SDL_GetTicks() - m_pausedTicks;

			//Reset the paused ticks
			m_pausedTicks = 0;
		}
	}

	Uint32 TRWindowsApp::LTimer::getTicks()
	{
		//The actual timer time
		Uint32 time = 0;

		//If the timer is running
		if (m_started)
		{
			//If the timer is paused
			if (m_paused)
			{
				//Return the number of ticks when the timer was paused
				time = m_pausedTicks;
			}
			else
			{
				//Return the current time minus the start time
				time = SDL_GetTicks() - m_startTicks;
			}
		}

		return time;
	}
	
}