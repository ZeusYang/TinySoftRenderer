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
			bool isStarted() { return mStarted; }
			bool isPaused() { return mPaused && mStarted; }

		private:
			//The clock time when the timer started
			Uint32 mStartTicks;

			//The ticks stored when the timer was paused
			Uint32 mPausedTicks;

			//The timer status
			bool mPaused;
			bool mStarted;
		};

		~TRWindowsApp();

		void readyToStart();

		//Event
		void processEvent();
		bool shouldWindowClose() const { return m_quit; }
		double getTimeFromStart() { return m_timer.getTicks(); }
		int getMouseMotionDeltaX() const { return m_mouse_delta_x; }
		int getMouseMotionDeltaY() const { return m_mouse_delta_y; }
		int getMouseWheelDelta() const { return m_wheel_delta; }
		bool getIsMouseLeftButtonPressed() const { return m_mouse_left_button_pressed; }

		//Copy the rendered image to screen for displaying
		double updateScreenSurface(
			unsigned char *pixels,
			int width, 
			int height, 
			int channel,
			unsigned int num_cliped_faces,
			unsigned int num_culled_faces);

		static TRWindowsApp::ptr getInstance();
		static TRWindowsApp::ptr getInstance(int width, int height, const std::string title = "winApp");

	private:

		//Mouse tracking
		int m_last_mouse_x, m_last_mouse_y;
		int m_mouse_delta_x, m_mouse_delta_y;
		bool m_mouse_left_button_pressed = false;
		int m_last_wheel_pos;
		int m_wheel_delta;

		//Timer
		LTimer m_timer;
		double m_last_time_point;
		double m_delta_time;
		double m_fps_counter;
		double m_fps_time_recorder;
		unsigned int m_fps;

		//Screen size
		int m_screen_width;
		int m_screen_height;

		bool m_quit = false;
		
		//Window title
		std::string m_window_title;

		//Event handler
		SDL_Event m_events;

		//Window handler
		SDL_Window* m_window_handle = nullptr;
		SDL_Surface* m_screen_surface = nullptr;

		//Singleton pattern
		static TRWindowsApp::ptr m_instance;
	};
}

#endif
