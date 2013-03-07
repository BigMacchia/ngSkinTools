#pragma once
#include <gtest/gtest.h>
#include "../maya.h"

bool setupMayaLibrary();


/**
 * compare two arrays, raise error if not equal
 */
inline void assertEquals(const MDoubleArray & array1,const MDoubleArray & array2){
	ASSERT_EQ(array1.length(),array2.length());
	for (unsigned int i=0;i<array1.length();i++){
		ASSERT_NEAR(array1[i],array2[i],0.000001);
	}
}