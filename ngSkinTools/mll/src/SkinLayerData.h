#pragma once
#include "maya.h"
#include <maya/MPxData.h>
#include <sstream>
#include <iostream>
#include <boost/cstdint.hpp>

class SkinLayerManager;
class SkinLayer;

namespace SKIN_LAYER_DATA_VERSIONS {
	const boost::uint32_t V1 = 1;
	const boost::uint32_t V2 = 2;
	const boost::uint32_t V3 = 3;
	const boost::uint32_t V4 = 4; // introducing line breaks to ascii encoded format
	const boost::uint32_t CURR = V4;
}

/**
 * The sole purpose of skin layer data type is to read/write associated manager to/from file.
 * skin layer data node takes care that this data always points to the right layer manager.
 */
class SkinLayerData:public MPxData
{
private:
	SkinLayerManager * manager;
	std::stringstream rawLoadedData;
	boost::uint32_t loadedVersion;
	bool needsLoading;

	virtual MStatus readBinaryDataSegment(boost::uint32_t version, istream& in, unsigned length);
	virtual MStatus writeBinaryDataSegment(ostream& out);

public:
	static const MString typeName;
	static const MTypeId id;
	SkinLayerData(void);
	virtual ~SkinLayerData(void);

	inline SkinLayerManager * getValue() const {
		return manager;
	}
	void setValue(SkinLayerManager *value);


	virtual MStatus readASCII( const MArgList&, unsigned& lastElement );
	virtual MStatus readBinary( istream& in, unsigned length );
	virtual MStatus writeASCII( ostream& out );
	virtual MStatus writeBinary( ostream& out );

	virtual void copy( const MPxData& );
	virtual MTypeId typeId() const { return SkinLayerData::id; }
	virtual MString name() const { return SkinLayerData::typeName; }

	static void* creator() {
		return new SkinLayerData();
	}

	static void encodeInChunks(std::istream &source, std::ostream &out, const std::streamsize maxBytes);
	static void decodeChunks(const MArgList &args, unsigned int &lastElement, std::ostream &result);

	static void loadManager(SkinLayerManager &manager,std::istream &input,boost::uint32_t version);
	static void saveManager(SkinLayerManager &manager,std::ostream &output);
};


