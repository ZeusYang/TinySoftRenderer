#ifndef TRWINDOWSAPP_H
#define TRWINDOWSAPP_H

#include "SDL2/SDL.h"

#include <string>
#include <sstream>
#include <memory>

namespace TinyRenderer
{
	class TRWindowsApp final
	{
	private:
		TRWindowsApp() = default;

		TRWindowsApp(TRWindowsApp&) = delete;
		TRWindowsApp& operator=(const TRWindowsApp&) = delete;

		bool setup(int width, int height, std::string title);

	public:

		typedef std::shared_ptr<TRWindowsApp> ptr;

		//Timer
		class LTimer
		{
		public:
			//Initializes variables
			LTimer();

			//The various clock actions
			void start();
			void stop();
			void pause();
			void unpause();

			//Gets the timer's time
			Uint32 getTicks();

			//Checks the status of the timer
			bool isStarted() { return m_started; }
			bool isPaused() { return m_paused && m_started; }

		private:
			//The clock time when the timer started
			Uint32 m_startTicks;

			//The ticks stored when the timer was paused
			Uint32 m_pausedTicks;

			//The timer status
			bool m_paused;
			bool m_started;
		};

		~TRWindowsApp();

		void readyToStart();

		//Event
		void processEvent();
		bool shouldWindowClose() const { return m_quit; }
		double getTimeFromStart() { return m_timer.getTicks(); }
		int getMouseMotionDeltaX() const { return m_mouseDeltaX; }
		int getMouseMotionDeltaY() const { return m_mouseDeltaY; }
		int getMouseWheelDelta() const { return m_wheelDelta; }
		bool getIsMouseLeftButtonPressed() const { return m_mouseLeftButtonPressed; }

		//Copy the rendered image to screen for displaying
		double updateScreenSurface(
			unsigned char *pixels,
			int width, 
			int height, 
			int channel,
			unsigned int num_triangles);

		static TRWindowsApp::ptr getInstance();
		static TRWindowsApp::ptr getInstance(int width, int height, const std::string title = "winApp");

	private:

		//Mouse tracking
		int m_lastMouseX, m_lastMouseY;
		int m_mouseDeltaX, m_mouseDeltaY;
		bool m_mouseLeftButtonPressed = false;
		int m_lastWheelPos;
		int m_wheelDelta;

		//Timer
		LTimer m_timer;
		double m_lastTimePoint;
		double m_deltaTime;
		double m_fpsCounter;
		double m_fpsTimeRecorder;
		unsigned int m_fps;

		//Screen size
		int m_screenWidth;
		int m_screenHeight;

		bool m_quit = false;
		
		//Window title
		std::string m_windowTitle;

		//Event handler
		SDL_Event m_events;

		//Window handler
		SDL_Window* m_windowHandle = nullptr;
		SDL_Surface* m_screenSurface = nullptr;

		//Singleton pattern
		static TRWindowsApp::ptr m_instance;
	};
}

#endif
