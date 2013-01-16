#pragma once

#include <boost/format.hpp>
#include "maya.h"
#include "defines.h"
#include <exception>

/// used as exception for failed MStatus checks
class StatusException: public std::exception
{
private:
	const MStatus status;
public:
	StatusException(const MStatus &status): status(status) {
	}

	inline const MStatus getStatus() const {
		return this->status;
	}
};


inline void throwStatusException(MString errorMessage, const MStatus &status){
	MGlobal::displayError(errorMessage);
	throw StatusException(status);
}

inline void throwStatusException(boost::basic_format<char> message, const MStatus &status){
	throwStatusException(MString(message.str().c_str()), status);
}



inline void CHECK_STATUS(const char * const errorMessage,const MStatus &status){
	if( !status ) {									
		DEBUG_COUT("Status check failed at: " << errorMessage <<endl);
		throwStatusException(MString("Status check failed at: ")+MString(errorMessage),status);
	}
}


