#include <iostream>
#include <limits>
#include <iomanip>



// winGDI name clash with logging.h
#undef ERROR
#include "glog/logging.h"
#include <gtest/gtest.h>


using namespace std;


#undef max
void waitForAnyKey(){
	std::cin.ignore( std::numeric_limits <std::streamsize>::max(), '\n' );
}

void inline setupLogging(const char executableName[]){
	google::InitGoogleLogging(executableName);
	google::LogToStderr();
}



int main(int argc, char * argv[])
{
	setupLogging(argv[0]);
	LOG(INFO) << "Initializing...";

	::testing::InitGoogleTest(&argc,argv); 

	LOG(INFO) << "Starting tests";

	int result = RUN_ALL_TESTS();
		
	LOG(INFO) << "Finished";
	//waitForAnyKey();
	return result;
}

