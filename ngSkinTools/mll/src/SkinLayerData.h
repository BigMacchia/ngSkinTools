#pragma once
#include "maya.h"
#include <maya/MPxData.h>
#include <sstream>
#include <iostream>
#include <boost/cstdint.hpp>

class SkinLayerManager;
class SkinLayer;

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
	virtual MStatus writeBinaryDataSegment(boost::uint32_t version, ostream& out);
	virtual boost::uint32_t getCurrentOutputVersion();

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
};
