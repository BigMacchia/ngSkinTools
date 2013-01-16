#pragma once

#include "timer.h"

#ifdef _SHOW_TIMERS

class SkinLayerPerformanceTrack
{
public:
	static SkinLayerPerformanceTrack instance;

	debug::Timer timeAttrWrites;
	debug::Timer timeSetValue;
	debug::Timer timeDisplayNode;
	debug::Timer timerSetWeight;
	debug::Timer timerUpdateDisplay;
	debug::Timer timerUpdateWeights;
	debug::Timer timerNormalize;
	debug::Timer timerOperationTotal;
	debug::Timer timerGetValue;
	debug::Timer timerParseArgs;
	debug::Timer timerMixWeights;
	debug::Timer timerAddOwnWeights;

	void resetTimers(){
		timeAttrWrites.reset();
		timeSetValue.reset();
		timeDisplayNode.reset();
		timerSetWeight.reset();
		timerUpdateDisplay.reset();
		timerUpdateWeights.reset();
		timerNormalize.reset();
		timerOperationTotal.reset();
		timerGetValue.reset();
		timerParseArgs.reset();
		timerMixWeights.reset();
		timerAddOwnWeights.reset();
	}

	void printOut(char * caption=NULL){
		cout << "==="<<(caption?caption:"Skin Layer performance timers")<<"==================" << endl;
		timeSetValue.print(				"total time                   ");
		timerParseArgs.print(			"	...parse args             ");
		timerSetWeight.print(			"   ...write weight           ");
		timerUpdateWeights.print(		"   ...weights update upstream");
		timerUpdateDisplay.print(		"   ...refresh trigger        ");
		timeAttrWrites.print(			"   ...attr writes            ");
		timerMixWeights.print(			"   ...timerMixWeights        ");
		timerAddOwnWeights.print(		"   ...add own weights        ");
		cout << "----------------------------------------------------" << endl;
		timerGetValue.print(		"get value        ");
		timerOperationTotal.print(		"operation total     ");
		timerNormalize.print(		"normalize weights");
		timeDisplayNode.print(		"display node     ");
		cout << "====================================================" << endl;
	}
};

#endif
