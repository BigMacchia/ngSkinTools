#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
using namespace std;

#include "../base64.h"
#include "../maya.h"


TEST(base64checks,simpleEncodeDecode){
	// use memory stream, but cast to standard ostream for later code
	std::stringstream output;
	output << "whatever works for you!";

	// encode
	std::string & preEncodeContents = output.str();
	string encodedData = base64_encode(reinterpret_cast<const unsigned char *>(preEncodeContents.c_str()),preEncodeContents.size());

	// decode
	std::string decodedData = base64_decode(encodedData);

	ASSERT_TRUE(decodedData==preEncodeContents);
}

TEST(base64checks,mayaStringToOStream){
	// use memory stream, but cast to standard ostream for later code
	std::stringstream output;
	MString whatever = "whatever works for you, works for me too!";


	output << whatever;
	cout << output.str() << "\n";

}