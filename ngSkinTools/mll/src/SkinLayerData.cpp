#define NOMINMAX
#include <boost/cstdint.hpp>
#include "SkinLayerData.h"
#include "defines.h"
#include "SkinLayerManager.h"
#include "SkinLayer.h"
#include "StatusException.h"

#include "base64.h"


const MString SkinLayerData::typeName("ngSkinLayerDataStorage");
const MTypeId SkinLayerData::id(0x00115A81);



template <class T>
inline void read(std::istream &in, T &value){
	in.read((char *)&value,sizeof(T));
}
template <class T>
inline void writeValue(std::ostream &out,const T value){
	out.write((char *)&value,sizeof(T));
}


class StreamHandler
{
private:
	boost::uint32_t version;
public:
	StreamHandler(const boost::uint32_t version){
		this->version = version;
	}
	inline boost::uint32_t getVersion(){
		return version;
	}
};

class BinaryStreamReader: public StreamHandler{
private:
	std::istream *in;
public:
	BinaryStreamReader(std::istream &in,const boost::uint32_t version):StreamHandler(version){
		this->in = &in;
	}
	template <class T>
	inline T readValue(){
		T value;
		in->read((char *)&value,sizeof(T));
		return value;
	}
};

class BinaryStreamWriter: public StreamHandler{
private:
	std::ostream * out;
public:
	BinaryStreamWriter(std::ostream &out): StreamHandler(SKIN_LAYER_DATA_VERSIONS::CURR){
		this->out = &out;
	}

	template <class T>
	inline void writeValue(const T value){
		out->write((char *)&value,sizeof(T));
	}
};



SkinLayerData::SkinLayerData(void):
	manager(NULL),
	needsLoading(false),
	MPxData()
{
}

SkinLayerData::~SkinLayerData(void)
{
}


void saveWeights(BinaryStreamWriter &out,const InfluenceWeightsMap &weights){
	// writting number of verts in this weights array
	out.writeValue<boost::uint32_t>(weights.getNumVerts());

	unsigned int usedInfluences = 0;
	VECTOR_FOREACH_CONST(unsigned int,weights.inflPhysicalToLogical,it){
		if (!(*it==InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX || !weights.isLogicalInfluenceUsed(*it))){
			usedInfluences++;
		}
	}
	out.writeValue<boost::uint32_t>(usedInfluences);
	

	// write weights for those influences that have weights
	const double *currInfluence = weights.getVertWeights();
	for (std::vector<unsigned int>::const_iterator it=weights.inflPhysicalToLogical.begin();it!=weights.inflPhysicalToLogical.end();it++,currInfluence++){
		if (*it==InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX || !weights.isLogicalInfluenceUsed(*it))
			continue;

		// write influence logical index
		out.writeValue<boost::uint32_t>(*it);
		const double *curr = currInfluence;

		// list influence weights 
		for (unsigned int i=0;i<weights.getNumVerts();i++,curr+=weights.getNumInfluences()){
			out.writeValue<double>(*curr);
		}
	}
}

void loadWeights(BinaryStreamReader &in,InfluenceWeightsMap &weights){
	
	unsigned int numVerts=in.readValue<boost::uint32_t>();
	unsigned int numInfluences=in.readValue<boost::uint32_t>();

	// empty layer?
	if (numInfluences==0)
		return;

	// allocate memory sufficient enough for data that's about to load
	weights.resize(numVerts,numInfluences,false);

	for (unsigned int i=0;i<numInfluences;i++){
		unsigned int logicalIndex=in.readValue<boost::uint32_t>();

		weights.addInfluenceMapping(logicalIndex);
		double *curr = weights.getLogicalInfluence(logicalIndex);
		for (unsigned int v=0;v<numVerts;v++,curr+=weights.getNumInfluences()){
			*curr = in.readValue<double>();
		}
	}

	weights.recalcTransparency();
}

void saveMask(BinaryStreamWriter &out,const SkinLayerWeightList &weights){
	out.writeValue<boost::uint32_t>(weights.getSize());
	if (weights.isInitialized()) {
		const double * currValue = weights.getWeights();
		for (unsigned int i=0;i<weights.getSize();i++,currValue++){
			out.writeValue<double>(*currValue);
		}
	}
}

void loadMask(BinaryStreamReader &in,SkinLayerWeightList &weights){
	unsigned int size=in.readValue<boost::uint32_t>();
	weights.resize(size,0.0);
	for (unsigned int i=0;i<size;i++){
		weights.setWeight(i,in.readValue<double>());
	}
}

void saveLayer(BinaryStreamWriter &outWriter,const SkinLayer &layer){

	if (layer.getParent()!=NULL){
		// layer properties
		MString name = layer.getName();
		
		outWriter.writeValue<boost::uint32_t>(name.length());
		if (name.length()>0){
			const char * chars = name.asChar();
			for (unsigned int i=0;i<name.length();i++,chars++){
				outWriter.writeValue<char>(*chars);
			}
		}

		outWriter.writeValue<boost::uint32_t>(layer.isEnabled()?1:0);
		outWriter.writeValue<double>(layer.getOpacity());

		saveMask(outWriter,layer.maskWeightList);

		saveWeights(outWriter,layer.influenceWeightList);
	}
	


	// casting to uint because of compatibility between 32bit and 64bit readers
	//unsigned int numChildren = static_cast<unsigned int>(layer.children.size());
	outWriter.writeValue<boost::uint32_t>(static_cast<unsigned int>(layer.children.size()));
	for (std::vector<SkinLayer *>::const_iterator i=layer.children.begin();i!=layer.children.end();i++){
		saveLayer(outWriter,**i);
	}
		



}

void loadLayer(SkinLayerManager &manager, BinaryStreamReader &inReader, SkinLayer &layer){
	
	if (layer.getParent()!=NULL){
		// layer properties
		unsigned int nameLength=inReader.readValue<boost::uint32_t>();
		if (nameLength>0) {
			char * buffer = new char[nameLength];
			for (unsigned int i=0;i<nameLength;i++){
				buffer[i] = inReader.readValue<char>();
			}
			MString name(buffer,nameLength);
			layer.setName(name);
			delete [] buffer;
		}
		else {
			layer.setName("");
		}

		if (inReader.getVersion()>=SKIN_LAYER_DATA_VERSIONS::V3){
			layer.setEnabled(inReader.readValue<boost::uint32_t>()==1);
		}

		layer.setOpacity(inReader.readValue<double>());

		loadMask(inReader,layer.maskWeightList);
		loadWeights(inReader,layer.influenceWeightList);

	}
	else {
		DEBUG_COUT_ENDL("reading root layer");
	}

	unsigned int numChildren=inReader.readValue<boost::uint32_t>();
	DEBUG_COUT_ENDL("number of children:"<<numChildren);
	for (unsigned int i=0;i<numChildren;i++){
		SkinLayer *child = manager.createLayer();
		child->setParent(&layer);
		loadLayer(manager,inReader,*child);
	}

}

void loadManualOverrides(BinaryStreamReader &inReader,std::map<unsigned int,unsigned int> &manualOverrides){
	unsigned int total=inReader.readValue<boost::uint32_t>();

	manualOverrides.clear();
	for (unsigned int i=0;i<total;i++){
		unsigned int source = inReader.readValue<boost::uint32_t>();
		unsigned int destination = inReader.readValue<boost::uint32_t>();
		manualOverrides[source] = destination;
	}
}
void saveManualOverrides(BinaryStreamWriter &outWriter,std::map<unsigned int,unsigned int> &manualOverrides){
	outWriter.writeValue<boost::uint32_t>(static_cast<boost::uint32_t>(manualOverrides.size()));
	for (std::map<unsigned int,unsigned int>::const_iterator i=manualOverrides.begin();i!=manualOverrides.end();i++){
		outWriter.writeValue<boost::uint32_t>(i->first);
		outWriter.writeValue<boost::uint32_t>(i->second);
	}
}

void SkinLayerData::loadManager(SkinLayerManager &manager,std::istream &input,boost::uint32_t version){
	{
		BinaryStreamReader inReader(input,version);
		LocalManagerSuspension managerSuspension(manager);

		loadLayer(manager,inReader,*manager.rootLayer);
		manager.findDefaultCurrentLayer();

		// save manual pairs
		if (inReader.getVersion()>=SKIN_LAYER_DATA_VERSIONS::V2){
			loadManualOverrides(inReader,manager.mirrorManualOverrides);
		}
	}
}

void SkinLayerData::saveManager(SkinLayerManager &manager, std::ostream &output){
	BinaryStreamWriter outWriter(output);

	saveLayer(outWriter,*manager.rootLayer);
	
	saveManualOverrides(outWriter,manager.mirrorManualOverrides);
}


inline void copyStream(std::istream &in, std::ostream &out, size_t length){
	char * buffer = new char[length];
	in.read(buffer,length);
	out.write(buffer,length);
	delete [] buffer;
}


MStatus SkinLayerData::readBinaryDataSegment(boost::uint32_t version, std::istream &in, unsigned int length){
	if (version>SKIN_LAYER_DATA_VERSIONS::CURR){
	// version check
		// failed to read skin layer data
		DEBUG_COUT_ENDL("loaded version:"<<version<<",expecting "<<SKIN_LAYER_DATA_VERSIONS::CURR);
		MGlobal::displayError("failed reading skin layer data: newer ngSkinTools plugin version required");
		return MS::kFailure;
	}
	this->loadedVersion = version;

	//read raw data, parse later when manager is set
	copyStream(in,rawLoadedData,length);
	needsLoading = length>0;
	
	return MS::kSuccess;
}

MStatus SkinLayerData::writeBinaryDataSegment(std::ostream &out){
	SkinLayerManager * manager = this->manager;
	SkinLayerManager tempManager;
	
	if (manager==NULL || needsLoading) {
		manager = &tempManager;
		rawLoadedData.seekg(0);
		loadManager(*manager,rawLoadedData,loadedVersion);
	}

	saveManager(*manager,out);

	return MS::kSuccess;
}

MStatus SkinLayerData::writeBinary(std::ostream &out){

	// write version
	writeValue<boost::uint32_t>(out,SKIN_LAYER_DATA_VERSIONS::CURR);
	return this->writeBinaryDataSegment(out);
}


MStatus SkinLayerData::readBinary(std::istream &in, unsigned int length){
	if (length==0)
		return MS::kSuccess;

	boost::uint32_t version;
	read<boost::uint32_t>(in,version);
	length -= sizeof(boost::uint32_t);
	
	return readBinaryDataSegment(version,in,length);
}

MStatus SkinLayerData::readASCII(const MArgList &args, unsigned int &lastElement){
	// get version
	const int version = args.asInt(lastElement++);

	if (version<SKIN_LAYER_DATA_VERSIONS::V4){
		// old, non-chunked version - left here to support old file versions
		MStatus status;
		MString inputValue = args.asString(lastElement++,&status);
		if (!status)
			return status;

		int len;
		const char * contents = inputValue.asChar(len);
		const std::string &decodedValue = base64_decode(std::string(contents,len));
		std::stringstream inputStream(decodedValue);

		return this->readBinaryDataSegment(version,inputStream,static_cast<unsigned int>(decodedValue.length()));
	}

	// newer format stores encoded data in small chunks, so that maya has easier time parsing the file
	try {
		std::stringstream result;
		decodeChunks(args,lastElement,result);
		return this->readBinaryDataSegment(version,result,static_cast<unsigned int>(result.str().length()));
	}
	catch (StatusException &e){
		return e.getStatus();
	}
}

void SkinLayerData::decodeChunks(const MStringArray &chunks,std::ostream &result,std::streamsize & bytesDecoded){
	bytesDecoded = 0;
	for (unsigned int i=0;i<chunks.length();i++){
		int len=0;
		const char * contents = chunks[i].asUTF8(len);
		const std::string &decodedValue = base64_decode(std::string(contents,len));
		result.write(decodedValue.c_str(),decodedValue.length());
		bytesDecoded += decodedValue.length();
	}
}

void SkinLayerData::decodeChunks(const MArgList &args, unsigned int &lastElement, std::ostream &result){
	MStatus status;
	std::streamsize bytesToDecode =  args.asInt(lastElement++);

	MStringArray chunks = args.asStringArray(lastElement,&status);
	CHECK_STATUS("error reading input file",status);

	std::streamsize bytesDecoded;
	decodeChunks(chunks,result,bytesDecoded);

	if (bytesDecoded!=bytesToDecode){
		throwStatusException("Error decoding binary data",MStatus::kInvalidParameter);
	}
}


void SkinLayerData::encodeInChunks(std::istream &source, std::ostream &out, const std::streamsize maxBytes){
	char *buffer = new char[maxBytes];

	// length
	source.seekg(0,ios::end);
	std::streamsize totalBytes=source.tellg();

	out << totalBytes <<" {";

	source.seekg (0, ios::beg);
	while (totalBytes>0){
		out<<"\n";
		std::streamsize bytesRead = std::min(totalBytes,maxBytes);
		source.read(buffer,bytesRead);
		totalBytes-=bytesRead;

		std::string encodedData = base64_encode(reinterpret_cast<const unsigned char *>(buffer),bytesRead);
		out<<"\"" << encodedData <<"\"";
		if (totalBytes<=0)
			out<<"}";
		else {
			out<<",";
		}
	}

	delete [] buffer;
}

MStatus SkinLayerData::writeASCII(std::ostream &out){
	out << SKIN_LAYER_DATA_VERSIONS::V4 << " ";

	std::stringstream data;
	MStatus status=this->writeBinaryDataSegment(data);
	if (status!=MS::kSuccess)
		return status;

	encodeInChunks(data,out,1024);

	return MS::kSuccess;
}

void SkinLayerData::copy(const MPxData &other){
	const SkinLayerData& otherData = static_cast<const SkinLayerData&>(other);
	if (otherData.needsLoading){
		this->rawLoadedData.str(otherData.rawLoadedData.str());
	}
	needsLoading = otherData.needsLoading;
	setValue(otherData.getValue());
}

void SkinLayerData::setValue(SkinLayerManager *value)
{
	this->manager = value;

	if (needsLoading && manager!=NULL){
		needsLoading = false;
		rawLoadedData.seekg(0,ios::beg);

		loadManager(*this->manager,rawLoadedData,this->loadedVersion);
	}
}

