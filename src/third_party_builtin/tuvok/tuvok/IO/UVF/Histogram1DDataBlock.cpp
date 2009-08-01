#include "Histogram1DDataBlock.h"

#include "RasterDataBlock.h"
using namespace std;
using namespace UVFTables;

Histogram1DDataBlock::Histogram1DDataBlock() : DataBlock() {
  ulBlockSemantics = BS_1D_Histogram;
  strBlockID       = "1D Histrogram";
}

Histogram1DDataBlock::Histogram1DDataBlock(const Histogram1DDataBlock &other) :
  DataBlock(other),
  m_vHistData(other.m_vHistData)
{
}

Histogram1DDataBlock& Histogram1DDataBlock::operator=(const Histogram1DDataBlock& other) {
  strBlockID = other.strBlockID;
  ulBlockSemantics = other.ulBlockSemantics;
  ulCompressionScheme = other.ulCompressionScheme;
  ulOffsetToNextDataBlock = other.ulOffsetToNextDataBlock;

  m_vHistData = other.m_vHistData;

  return *this;
}


Histogram1DDataBlock::Histogram1DDataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
  GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
}

Histogram1DDataBlock::~Histogram1DDataBlock() 
{
}

DataBlock* Histogram1DDataBlock::Clone() {
  return new Histogram1DDataBlock(*this);
}

UINT64 Histogram1DDataBlock::GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
  UINT64 iStart = iOffset + DataBlock::GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
  pStreamFile->SeekPos(iStart);

  UINT64 ulElementCount;
  pStreamFile->ReadData(ulElementCount, bIsBigEndian);

  m_vHistData.resize(size_t(ulElementCount));
  pStreamFile->ReadRAW((unsigned char*)&m_vHistData[0], ulElementCount*sizeof(UINT64));
  return pStreamFile->GetPos() - iOffset;
}

bool Histogram1DDataBlock::Compute(RasterDataBlock* source) {

  // TODO: right now we can only compute Histograms of scalar data this should be changed to a more general approach
  if (source->ulElementDimension != 1 || source->ulElementDimensionSize.size() != 1) return false;

  // TODO: right now compute Histogram assumes that the lowest LOD level consists only of a single brick, this brick is used for the hist. computation
  //       this should be changed to a more general approach
  vector<UINT64> vSmallestLOD = source->GetSmallestBrickIndex();
  const vector<UINT64>& vBricks = source->GetBrickCount(vSmallestLOD);
  for (size_t i = 0;i<vBricks.size();i++) if (vBricks[i] != 1) return false;
  
  // create temp histogram 
  size_t iValueRange = size_t(1<<(source->ulElementBitSize[0][0]));
  vector<UINT64> vTmpHist(iValueRange);
  if (vTmpHist.size() != iValueRange) return false;
  for (size_t i = 0;i<iValueRange;i++) vTmpHist[i] = 0;

  // find smallest brick in hierarchy
  std::vector<unsigned char> vcSourceData;
  vector<UINT64> vLOD = source->GetSmallestBrickIndex();
  vector<UINT64> vOneAndOnly;
  for (size_t i = 0;i<vBricks.size();i++) vOneAndOnly.push_back(0);
  if (!source->GetData(vcSourceData, vLOD, vOneAndOnly)) return false;

  vector<UINT64> vSize  = source->GetSmallestBrickSize();

  UINT64 iDataSize = 1;
  for (size_t i = 0;i<vSize.size();i++) iDataSize*=vSize[i];

  /// @todo only 8 and 16 bit integer data are supported.  this should be
  ///       changed to use a more general approach.
  if (source->ulElementBitSize[0][0] == 8) {
    for (UINT64 i = 0;i<iDataSize;i++) {
       vTmpHist[vcSourceData[size_t(i)]]++;
    }
  } else {
    if (source->ulElementBitSize[0][0] == 16) {
      unsigned short *psSourceData = (unsigned short*)(&(vcSourceData.at(0)));
      for (UINT64 i = 0;i<iDataSize;i++) {
        vTmpHist[psSourceData[size_t(i)]]++;
      }
    } else {
      return false;
    }
  }

  // find maximum-index non zero entry
  size_t iSize = 0;
  for (size_t i = 0;i<iValueRange;i++) if (vTmpHist[i] != 0) iSize = i+1;
  iValueRange = iSize;
  
  // copy non zero elements in temp histogram to histogram
  m_vHistData.resize(iValueRange);
  if (m_vHistData.size() != iValueRange) return false;
  for (size_t i = 0;i<iValueRange;i++) m_vHistData[i] = vTmpHist[i];

  // set data block information
  strBlockID = "1D Histogram for datablock " + source->strBlockID;

  return true;
}

UINT64 Histogram1DDataBlock::CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock) {
  UINT64 iStart = iOffset + DataBlock::CopyToFile(pStreamFile, iOffset, bIsBigEndian, bIsLastBlock);
  pStreamFile->SeekPos(iStart);

  UINT64 ulElementCount = UINT64(m_vHistData.size());
  pStreamFile->WriteData(ulElementCount, bIsBigEndian);
  pStreamFile->WriteRAW((unsigned char*)&m_vHistData[0], ulElementCount*sizeof(UINT64));

  return pStreamFile->GetPos() - iOffset;
}


UINT64 Histogram1DDataBlock::GetOffsetToNextBlock() const {
  return DataBlock::GetOffsetToNextBlock() + ComputeDataSize();
}

UINT64 Histogram1DDataBlock::ComputeDataSize() const {
  return sizeof(UINT64) +                  // length of the vector
       m_vHistData.size()*sizeof(UINT64);  // the vector itself
}
