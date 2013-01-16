#pragma once

#include "defines.h"
#include <time.h>

#ifdef _SHOW_TIMERS
namespace debug  {
	class Timer {
	private:
		LARGE_INTEGER started;
		LARGE_INTEGER frequency;
		bool running;
		LONGLONG totalTime;
		LONGLONG maxSegment;
		unsigned int segments;
	public:
		Timer(){
			reset();
			QueryPerformanceFrequency(&frequency);
		}
		inline void reset(){
			started.QuadPart = 0;
			segments = 0;
			maxSegment = 0;
			running=false;
			totalTime = 0;
		}

		inline void start(){
			QueryPerformanceCounter(&started);
			running = true;
		}

		inline void stop(){
			if (running){
				LARGE_INTEGER finished;
				QueryPerformanceCounter(&finished);
				LONGLONG segmentTime = finished.QuadPart-started.QuadPart;
				totalTime += segmentTime;
				if (maxSegment<segmentTime)
					maxSegment = segmentTime;
				segments++;
			}
			running = false;
		}
		inline void print(const char * const name){
			if (segments)
				cout <<"timer "<<name<< ":"
					<<totalTime<<" (~"<<(static_cast<double>(totalTime)/frequency.QuadPart)<<"s), "
					<< segments << " segments ("<<(static_cast<double>(totalTime)/segments)<<"c/s, "
					<< " max:"<<maxSegment<<")"
					<<endl;
		}
	};

}

#define _TIMING(expression) expression;
#else
#define _TIMING(expression);
#endif



#ifdef _DEBUG
	#define TIMER_RESET(variable) variable.reset();
	#define TIMER_RESUME(variable) variable.start();
	#define TIMER_PAUSE(variable) variable.stop();
	#define TIMER_PRINT(variable,name) variable.print(name);
	#define TIMER_START(variable) debug::Timer variable;variable.start();
	#define TIMER_STOP(variable,name) variable.stop();variable.print(name)
#else
	#define TIMER_RESET(variable)
	#define TIMER_RESUME(variable)
	#define TIMER_PAUSE(variable)
	#define TIMER_PRINT(variable,name)
	#define TIMER_START(variable)
	#define TIMER_STOP(variable,name)
#endif

