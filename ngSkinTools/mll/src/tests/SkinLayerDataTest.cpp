#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
using namespace std;

#include "../SkinLayerData.h"
#include "mayaSupport.h"

TEST(skinLayerDataTest,encodeInChunks){
	stringstream source;
	stringstream out;

	string data("01234567890qwertyuiopasdfghjklzxcvbnm,./;'p[]01234567890qwertyuiopasdfghjklzxcvbnm,./;'");
	source<<data;
	
	SkinLayerData::encodeInChunks(source,out,10);

	ASSERT_EQ(string("87 {\n\"MDEyMzQ1Njc4OQ==\",\n\"MHF3ZXJ0eXVpbw==\",\n\"cGFzZGZnaGprbA==\",\n\"enhjdmJubSwuLw==\",\n\"OydwW10wMTIzNA==\",\n\"NTY3ODkwcXdlcg==\",\n\"dHl1aW9wYXNkZg==\",\n\"Z2hqa2x6eGN2Yg==\",\n\"bm0sLi87Jw==\"}"),out.str());

	setupMayaLibrary();

	MArgList args;
	args.addArg(87);
	args.addArg(MString("MDEyMzQ1Njc4OQ=="));
	args.addArg(MString("MHF3ZXJ0eXVpbw=="));
	args.addArg(MString("cGFzZGZnaGprbA=="));
	args.addArg(MString("enhjdmJubSwuLw=="));
	args.addArg(MString("OydwW10wMTIzNA=="));
	args.addArg(MString("NTY3ODkwcXdlcg=="));
	args.addArg(MString("dHl1aW9wYXNkZg=="));
	args.addArg(MString("Z2hqa2x6eGN2Yg=="));
	args.addArg(MString("bm0sLi87Jw=="));

	unsigned int lastElement = 0;
	stringstream result;
	SkinLayerData::decodeChunks(args,lastElement,result);
	ASSERT_EQ(10,lastElement);

	ASSERT_EQ(data,result.str());
}
