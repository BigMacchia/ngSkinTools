#pragma once

#include "maya.h"


// define output only if debug is enabled
#ifdef _DEBUG
#define DEBUG_COUT(T) cout << T
#else
#define DEBUG_COUT(T)
#endif

// define output with endline
#define DEBUG_COUT_ENDL(T) DEBUG_COUT( T << endl )


// shortcut to stream an xyz value, like vector or point
#define DEBUG_COUT_VECTOR(V) "["<<V.x<<","<<V.y<<","<<V.z<<"]"

// outputs 4x4 matrix
#define DEBUG_DUMP_MATRIX(name,M) \
	DEBUG_COUT_ENDL(name<<"1:"<<M[0][0]<<" "<<M[0][1]<<" "<<M[0][2]<<" "<<M[0][3]);\
	DEBUG_COUT_ENDL(name<<"2:"<<M[1][0]<<" "<<M[1][1]<<" "<<M[1][2]<<" "<<M[1][3]);\
	DEBUG_COUT_ENDL(name<<"3:"<<M[2][0]<<" "<<M[2][1]<<" "<<M[2][2]<<" "<<M[2][3]);\
	DEBUG_COUT_ENDL(name<<"4:"<<M[3][0]<<" "<<M[3][1]<<" "<<M[3][2]<<" "<<M[3][3]);



/**
 * simple iteration over all items in a loop
 */
#define VECTOR_FOREACH(T,vectorData,variable) for (std::vector<T>::iterator variable=vectorData.begin();variable!=vectorData.end();variable++)
#define VECTOR_FOREACH_CONST(T,vectorData,variable) for (std::vector<T>::const_iterator variable=vectorData.begin();variable!=vectorData.end();variable++)
#define SET_FOREACH(T,setData,variable) for (std::set<T>::iterator variable=setData.begin();variable!=setData.end();variable++)
#define SET_FOREACH_CONST(T,setData,variable) for (std::set<T>::const_iterator variable=setData.begin();variable!=setData.end();variable++)


#ifdef _DEBUG
#define DEBUG_EXECUTE(statement) statement
#else
#define DEBUG_EXECUTE(statement) 
#endif

#define DEBUG_REPORT_ERROR_STATUS(T) DEBUG_EXECUTE(if(!status) cout <<T<<endl)

