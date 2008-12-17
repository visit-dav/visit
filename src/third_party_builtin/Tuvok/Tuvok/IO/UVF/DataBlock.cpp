#include "DataBlock.h"

#include "UVF.h"
#include <sstream>

using namespace std;
using namespace UVFTables;

DataBlock::DataBlock() : 
	strBlockID(""),
	ulCompressionScheme(COS_NONE),
	m_pStreamFile(NULL),
	m_iOffset(0),
	ulBlockSemantics(BS_EMPTY),
	ulOffsetToNextDataBlock(0)
{}

DataBlock::DataBlock(const DataBlock &other) : 
	strBlockID(other.strBlockID),
	ulCompressionScheme(other.ulCompressionScheme),
	m_pStreamFile(other.m_pStreamFile),
	m_iOffset(other.m_iOffset),
	ulBlockSemantics(other.ulBlockSemantics),
	ulOffsetToNextDataBlock(other.ulOffsetToNextDataBlock)
{}

DataBlock::DataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian)  {
	GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
}

DataBlock::~DataBlock() {
	// nothing to do here yet
}


DataBlock& DataBlock::operator=(const DataBlock& other)  { 
	strBlockID = other.strBlockID;
	ulCompressionScheme = other.ulCompressionScheme;
	m_pStreamFile = other.m_pStreamFile;
	m_iOffset = other.m_iOffset;
	ulBlockSemantics = other.ulBlockSemantics;
	ulOffsetToNextDataBlock = other.ulOffsetToNextDataBlock;

	return *this;
}

DataBlock* DataBlock::Clone() {
	return new DataBlock(*this);
}

UINT64 DataBlock::GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
	m_pStreamFile = pStreamFile;
	m_iOffset = iOffset;

	m_pStreamFile->SeekPos(iOffset);

	UINT64 ulStringLengthBlockID;
	m_pStreamFile->ReadData(ulStringLengthBlockID, bIsBigEndian);
  m_pStreamFile->ReadData(strBlockID, ulStringLengthBlockID);

	UINT64 uintSem;
	m_pStreamFile->ReadData(uintSem, bIsBigEndian);
	ulBlockSemantics = (BlockSemanticTable)uintSem;
	m_pStreamFile->ReadData(uintSem, bIsBigEndian);
	ulCompressionScheme = (CompressionSemanticTable)uintSem;

	m_pStreamFile->ReadData(ulOffsetToNextDataBlock, bIsBigEndian);

	return m_pStreamFile->GetPos() - iOffset;
}

UINT64 DataBlock::CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock) {
	pStreamFile->SeekPos(iOffset);

	pStreamFile->WriteData(UINT64(strBlockID.size()), bIsBigEndian);
  pStreamFile->WriteData(strBlockID);
	pStreamFile->WriteData(UINT64(ulBlockSemantics), bIsBigEndian);
	pStreamFile->WriteData(UINT64(ulCompressionScheme), bIsBigEndian);

	if (bIsLastBlock)
		pStreamFile->WriteData(UINT64(0), bIsBigEndian);
	else 
    pStreamFile->WriteData(ulOffsetToNextDataBlock, bIsBigEndian);

	return pStreamFile->GetPos() - iOffset;
}

UINT64 DataBlock::GetOffsetToNextBlock() const {
	return strBlockID.size() + 4 * sizeof(UINT64);
}

bool DataBlock::Verify(UINT64 iSizeofData, std::string* pstrProblem) const {
	UINT64 iCorrectSize = ComputeDataSize();
	bool bResult = iCorrectSize == iSizeofData;

	if (pstrProblem != NULL)  {
		stringstream s;
		s << "DataBlock::Verify: size mismatch. Should be " << iCorrectSize << " but parameter was " << iSizeofData << ".";
		*pstrProblem = s.str();
	}

	return bResult;
}
