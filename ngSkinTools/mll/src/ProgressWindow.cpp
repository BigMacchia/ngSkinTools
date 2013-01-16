#include <maya/MProgressWindow.h>
#include "ProgressWindow.h"
#include "maya.h"

ProgressWindow::ProgressWindow(void)
{
}

ProgressWindow::ProgressWindow(const std::string title, const int maxRange)
{
	this->start(title,maxRange);
}

ProgressWindow::~ProgressWindow(void)
{
	this->stop();
}

void ProgressWindow::start(const std::string title, const int maxRange){
	this->maxRange = maxRange;
	this->title = title;

	this->started = clock();
	this->currProgress = 0;
	this->windowInitialized = false;

}

void ProgressWindow::stop(){
	if (this->windowInitialized) {
		CHECK_MSTATUS(MProgressWindow::endProgress());
		this->windowInitialized = false;
	}
}

void ProgressWindow::add(const int increment){
	
	// check if we need to create actual progress window
	if (!this->windowInitialized && (clock()>this->started+ProgressWindow::DELAY_DISPLAY) ){
		if (MProgressWindow::reserve())
		{
			this->windowInitialized = true;
			MProgressWindow::setProgressRange(0, this->maxRange);
			MProgressWindow::setTitle(this->title.c_str());
			MProgressWindow::setInterruptable(true);
			MProgressWindow::setProgress(this->currProgress);
			MProgressWindow::startProgress();

			MProgressWindow::setProgressStatus(this->title.c_str());
		}
	}

	if (this->windowInitialized)
	    MProgressWindow::advanceProgress(increment);
	else
		this->currProgress += increment;

}

const bool ProgressWindow::isCanceled() const {
	return this->windowInitialized && MProgressWindow::isCancelled();
}
