#include "TRWindowsApp.h"

#include <iostream>
#include <iomanip>

namespace TinyRenderer
{
	TRWindowsApp::ptr TRWindowsApp::m_instance = nullptr;

	bool TRWindowsApp::setup(int width, int height, std::string title)
	{
		m_screen_width = width;
		m_screen_height = height;
		m_window_title = title;

		m_last_mouse_x = 0;
		m_last_mouse_y = 0;
		m_mouse_delta_x = 0;
		m_mouse_delta_y = 0;

		//Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		//Create window
		m_window_handle = SDL_CreateWindow(
			m_window_title.c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			m_screen_width,
			m_screen_height,
			SDL_WINDOW_SHOWN);

		if (m_window_handle == nullptr)
		{
			std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return false;
		}

		//Get window surface
		m_screen_surface = SDL_GetWindowSurface(m_window_handle);

		return true;
	}

	TRWindowsApp::~TRWindowsApp()
	{
		//Destroy window
		SDL_DestroyWindow(m_window_handle);
		m_window_handle = nullptr;

		//Quit SDL subsystems
		SDL_Quit();
	}

	void TRWindowsApp::readyToStart()
	{
		m_timer.start();
		m_last_time_point = m_timer.getTicks();

		m_fps = 0;
		m_fps_counter = 0.0f;
		m_fps_time_recorder = 0.0f;
	}

	void TRWindowsApp::processEvent()
	{
		m_wheel_delta = 0;
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
					m_last_mouse_x = m_events.motion.x;
					m_last_mouse_y = m_events.motion.y;
					m_mouse_delta_x = 0;
					m_mouse_delta_y = 0;
				}
				else
				{
					m_mouse_delta_x = m_events.motion.x - m_last_mouse_x;
					m_mouse_delta_y = m_events.motion.y - m_last_mouse_y;
					m_last_mouse_x = m_events.motion.x;
					m_last_mouse_y = m_events.motion.y;
				}
			}
			if (m_events.type == SDL_MOUSEBUTTONDOWN && m_events.button.button == SDL_BUTTON_LEFT)
			{
				m_mouse_left_button_pressed = true;
				m_last_mouse_x = m_events.motion.x;
				m_last_mouse_y = m_events.motion.y;
				m_mouse_delta_x = 0;
				m_mouse_delta_y = 0;
			}
			if (m_events.type == SDL_MOUSEBUTTONUP && m_events.button.button == SDL_BUTTON_LEFT)
			{
				m_mouse_left_button_pressed = false;
			}
			if (m_events.type == SDL_MOUSEWHEEL)
			{
				m_wheel_delta = m_events.wheel.y;
			}
		}
	}

	double TRWindowsApp::updateScreenSurface(
		unsigned char *pixels,
		int width,
		int height,
		int channel,
		unsigned int num_cliped_faces,
		unsigned int num_culled_faces)
	{
		//Update pixels
		SDL_LockSurface(m_screen_surface);
		{
			Uint32* destPixels = (Uint32*)m_screen_surface->pixels;
			for (int i = 0; i < width * height; ++i)
			{
				Uint32 color = SDL_MapRGB(
					m_screen_surface->format, 
					static_cast<uint8_t>(pixels[i * channel + 0]),
					static_cast<uint8_t>(pixels[i * channel + 1]),
					static_cast<uint8_t>(pixels[i * channel + 2]));
				destPixels[i] = color;
			}
		}
		SDL_UnlockSurface(m_screen_surface);
		SDL_UpdateWindowSurface(m_window_handle);

		m_delta_time = m_timer.getTicks() - m_last_time_point;
		m_last_time_point = m_timer.getTicks();

		//FPS counting
		{
			m_fps_time_recorder += m_delta_time;
			++m_fps_counter;
			if (m_fps_time_recorder > 1000.0)
			{
				m_fps = static_cast<unsigned int>(m_fps_counter);
				m_fps_counter = 0.0f;
				m_fps_time_recorder = 0.0f;

				std::stringstream ss;
				ss << " FPS:" << std::setiosflags(std::ios::left) << std::setw(3) << m_fps;
				ss << "#ClipedFaces:" << std::setiosflags(std::ios::left) << std::setw(5) << num_cliped_faces;
				ss << "#CulledFaces:" << std::setiosflags(std::ios::left) << std::setw(5) << num_culled_faces;
				SDL_SetWindowTitle(m_window_handle, (m_window_title + ss.str()).c_str());
			}
		}

		return m_delta_time;
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
		mStartTicks = 0;
		mPausedTicks = 0;

		mPaused = false;
		mStarted = false;
	}

	void TRWindowsApp::LTimer::start()
	{
		//Start the timer
		mStarted = true;

		//Unpause the timer
		mPaused = false;

		//Get the current clock time
		mStartTicks = SDL_GetTicks();
		mPausedTicks = 0;
	}

	void TRWindowsApp::LTimer::stop()
	{
		//Stop the timer
		mStarted = false;

		//Unpause the timer
		mPaused = false;

		//Clear tick variables
		mStartTicks = 0;
		mPausedTicks = 0;
	}

	void TRWindowsApp::LTimer::pause()
	{
		//If the timer is running and isn't already paused
		if (mStarted && !mPaused)
		{
			//Pause the timer
			mPaused = true;

			//Calculate the paused ticks
			mPausedTicks = SDL_GetTicks() - mStartTicks;
			mStartTicks = 0;
		}
	}

	void TRWindowsApp::LTimer::unpause()
	{
		//If the timer is running and paused
		if (mStarted && mPaused)
		{
			//Unpause the timer
			mPaused = false;

			//Reset the starting ticks
			mStartTicks = SDL_GetTicks() - mPausedTicks;

			//Reset the paused ticks
			mPausedTicks = 0;
		}
	}

	Uint32 TRWindowsApp::LTimer::getTicks()
	{
		//The actual timer time
		Uint32 time = 0;

		//If the timer is running
		if (mStarted)
		{
			//If the timer is paused
			if (mPaused)
			{
				//Return the number of ticks when the timer was paused
				time = mPausedTicks;
			}
			else
			{
				//Return the current time minus the start time
				time = SDL_GetTicks() - mStartTicks;
			}
		}

		return time;
	}
	
}