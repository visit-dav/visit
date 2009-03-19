#include "MaxMinDataBlock.h"

#include <memory.h>
using namespace std;
using namespace UVFTables;

#ifdef min
  #undef min
#endif

#ifdef max
  #undef max
#endif


MaxMinDataBlock::MaxMinDataBlock() : 
  DataBlock()
{
  ulBlockSemantics = BS_MAXMIN_VALUES;
  strBlockID       = "Brick Max/Min Values";
  m_vfMaxMinData.clear();
}

MaxMinDataBlock::MaxMinDataBlock(const MaxMinDataBlock &other) :
  DataBlock(other),
  m_vfMaxMinData(other.m_vfMaxMinData)
{
}

MaxMinDataBlock& MaxMinDataBlock::operator=(const MaxMinDataBlock& other) {
  strBlockID = other.strBlockID;
  ulBlockSemantics = other.ulBlockSemantics;
  ulCompressionScheme = other.ulCompressionScheme;
  ulOffsetToNextDataBlock = other.ulOffsetToNextDataBlock;

  m_vfMaxMinData = other.m_vfMaxMinData;

  return *this;
}


MaxMinDataBlock::MaxMinDataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
  GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
}

MaxMinDataBlock::~MaxMinDataBlock() 
{
}

DataBlock* MaxMinDataBlock::Clone() {
  return new MaxMinDataBlock(*this);
}

UINT64 MaxMinDataBlock::GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
  UINT64 iStart = iOffset + DataBlock::GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
  pStreamFile->SeekPos(iStart);

  UINT64 ulElementCount;
  pStreamFile->ReadData(ulElementCount, bIsBigEndian);

  m_vfMaxMinData.resize(size_t(ulElementCount));
  pStreamFile->ReadRAW((unsigned char*)&m_vfMaxMinData[0], ulElementCount*sizeof(InternalMaxMinElemen));

  return pStreamFile->GetPos() - iOffset;
}

UINT64 MaxMinDataBlock::CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock) {
  UINT64 iStart = iOffset + DataBlock::CopyToFile(pStreamFile, iOffset, bIsBigEndian, bIsLastBlock);
  pStreamFile->SeekPos(iStart);

  UINT64 ulElementCount = UINT64(m_vfMaxMinData.size());
  pStreamFile->WriteData(ulElementCount, bIsBigEndian);
  pStreamFile->WriteRAW((unsigned char*)&m_vfMaxMinData[0], ulElementCount*sizeof(InternalMaxMinElemen));

  return pStreamFile->GetPos() - iOffset;
}


UINT64 MaxMinDataBlock::GetOffsetToNextBlock() const {
  return DataBlock::GetOffsetToNextBlock() + ComputeDataSize();
}

UINT64 MaxMinDataBlock::ComputeDataSize() const {
  return sizeof(UINT64) +                                          // length of the vector
         sizeof(InternalMaxMinElemen) * m_vfMaxMinData.size();    // the vector itself
}


const InternalMaxMinElemen& MaxMinDataBlock::GetValue(size_t iIndex) {
  return m_vfMaxMinData[iIndex];
}

void MaxMinDataBlock::StartNewValue() {
  InternalMaxMinElemen elem(std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
  m_vfMaxMinData.push_back(elem);
}

void MaxMinDataBlock::MergeData(double fMin, double fMax, double fMinGrad, double fMaxGrad) {
  MergeData(InternalMaxMinElemen(fMin, fMax, fMinGrad, fMaxGrad));
}

void MaxMinDataBlock::MergeData(const InternalMaxMinElemen& data) {
  m_vfMaxMinData[m_vfMaxMinData.size()-1].Merge(data);
}
