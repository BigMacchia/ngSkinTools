#pragma once
#include <string>
using namespace std;
#include <time.h>

/**
 * Progress window wrapper. will only display progress if
 * operation is running for considerably long time
 *
 * progress window will store progress data internally and will create
 * window in "add()" once enough time of progressing has passed
 */
class ProgressWindow
{
private:
	// amount of clock ticks to delay display of progress window
	static const time_t DELAY_DISPLAY = CLOCKS_PER_SEC*1;

	time_t started;
	bool windowInitialized;
	string title;
	int maxRange;
	int currProgress;

public:
	ProgressWindow(void);
	ProgressWindow(const string title,const int maxRange);
	~ProgressWindow(void);

	void start(const string title,const int maxRange);
	void add(const int increment=1);
	void stop();

	const bool isCanceled()const;
};
