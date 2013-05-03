#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
using namespace std;

#include "../SkinLayerData.h"
#include "mayaSupport.h"

TEST(skinLayerDataTest,encodeSmallData){
	setupMayaLibrary();
	stringstream source;
	stringstream out;

	string data("1234");
	source<<data;
	
	SkinLayerData::encodeInChunks(source,out,10);

	ASSERT_EQ(string("4 {\n\"MTIzNA==\"}"),out.str());
}






TEST(skinLayerDataTest,encodeInChunks){
	setupMayaLibrary();

	stringstream source;
	stringstream out;

	string data("01234567890qwertyuiopasdfghjklzxcvbnm,./;'p[]01234567890qwertyuiopasdfghjklzxcvbnm,./;'");
	source<<data;
	
	SkinLayerData::encodeInChunks(source,out,10);

	ASSERT_EQ(string("87 {\n\"MDEyMzQ1Njc4OQ==\",\n\"MHF3ZXJ0eXVpbw==\",\n\"cGFzZGZnaGprbA==\",\n\"enhjdmJubSwuLw==\",\n\"OydwW10wMTIzNA==\",\n\"NTY3ODkwcXdlcg==\",\n\"dHl1aW9wYXNkZg==\",\n\"Z2hqa2x6eGN2Yg==\",\n\"bm0sLi87Jw==\"}"),out.str());


	MStringArray items;
	items.append(MString("MDEyMzQ1Njc4OQ=="));
	items.append(MString("MHF3ZXJ0eXVpbw=="));
	items.append(MString("cGFzZGZnaGprbA=="));
	items.append(MString("enhjdmJubSwuLw=="));
	items.append(MString("OydwW10wMTIzNA=="));
	items.append(MString("NTY3ODkwcXdlcg=="));
	items.append(MString("dHl1aW9wYXNkZg=="));
	items.append(MString("Z2hqa2x6eGN2Yg=="));
	items.append(MString("bm0sLi87Jw=="));

	unsigned int lastElement = 0;
	std::streamsize bytesDecoded;
	stringstream result;
	SkinLayerData::decodeChunks(items,result,bytesDecoded);
	ASSERT_EQ(87,bytesDecoded);

	ASSERT_EQ(data,result.str());
}
