#include "Histogram2DDataBlock.h"
#include <memory.h>
#include <Basics/Vectors.h>

using namespace std;
using namespace UVFTables;


Histogram2DDataBlock::Histogram2DDataBlock() : 
  DataBlock(),
  m_fMaxGradMagnitude(0)
{
  ulBlockSemantics = BS_2D_Histogram;
  strBlockID       = "2D Histrogram";
}

Histogram2DDataBlock::Histogram2DDataBlock(const Histogram2DDataBlock &other) :
  DataBlock(other),
  m_vHistData(other.m_vHistData),
  m_fMaxGradMagnitude(other.m_fMaxGradMagnitude)
{
}

Histogram2DDataBlock& Histogram2DDataBlock::operator=(const Histogram2DDataBlock& other) {
	strBlockID = other.strBlockID;
	ulBlockSemantics = other.ulBlockSemantics;
	ulCompressionScheme = other.ulCompressionScheme;
	ulOffsetToNextDataBlock = other.ulOffsetToNextDataBlock;

	m_vHistData = other.m_vHistData;
  m_fMaxGradMagnitude = other.m_fMaxGradMagnitude;

	return *this;
}


Histogram2DDataBlock::Histogram2DDataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
	GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);	
}

Histogram2DDataBlock::~Histogram2DDataBlock() 
{
}

DataBlock* Histogram2DDataBlock::Clone() {
	return new Histogram2DDataBlock(*this);
}

UINT64 Histogram2DDataBlock::GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
  UINT64 iStart = iOffset + DataBlock::GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
	pStreamFile->SeekPos(iStart);

	UINT64 ulElementCountX, ulElementCountY;
  pStreamFile->ReadData(m_fMaxGradMagnitude, bIsBigEndian);
	pStreamFile->ReadData(ulElementCountX, bIsBigEndian);
	pStreamFile->ReadData(ulElementCountY, bIsBigEndian);

  m_vHistData.resize(size_t(ulElementCountX));
  vector<UINT64> tmp((size_t)ulElementCountY);
  for (size_t i = 0;i<size_t(ulElementCountX);i++) {
    pStreamFile->ReadRAW((unsigned char*)&tmp[0], ulElementCountY*sizeof(UINT64));
    m_vHistData[i] = tmp;
  }

  return pStreamFile->GetPos() - iOffset;
}

/// \todo right now compute Histogram assumes that the lowest LOD level consists only of a single brick, this brick is used for the hist. computation
//       this should be changed to a more general approach
bool Histogram2DDataBlock::Compute(RasterDataBlock* source) {
  /// \todo right now we can only compute Histograms of scalar data this should be changed to a more general approach
  if (source->ulElementDimension != 1 || source->ulElementDimensionSize.size() != 1) return false;

  /// \todo right now compute Histogram assumes that the lowest LOD level consists only of a single brick, this brick is used for the hist. computation
  //       this should be changed to a more general approach
  vector<UINT64> vSmallestLOD = source->GetSmallestBrickIndex();
  const vector<UINT64>& vBricks = source->GetBrickCount(vSmallestLOD);
  for (unsigned int i = 0;i<vBricks.size();i++) if (vBricks[i] != 1) return false;
  
  /// \todo right now we can only compute 2D Histograms of at least 3D data this should be changed to a more general approach
  //       also we require that the first three entries as X,Y,Z
  if (source->ulDomainSize.size() < 3 || source->ulDomainSemantics[0] != UVFTables::DS_X ||
      source->ulDomainSemantics[1] != UVFTables::DS_Y || source->ulDomainSemantics[2] != UVFTables::DS_Z) return false;

  size_t iValueRange = size_t(1<<(source->ulElementBitSize[0][0]));

  std::vector< std::vector<UINT64> > vTmpHist(iValueRange);
  for (size_t i = 0;i<iValueRange;i++) {
    vTmpHist[i].resize(256);
    for (size_t j = 0;j<256;j++) {
      vTmpHist[i][j] = 0;
    }
  }

  unsigned char *pcSourceData = NULL;

  vector<UINT64> vLOD = source->GetSmallestBrickIndex();
  vector<UINT64> vOneAndOnly;
  for (size_t i = 0;i<vBricks.size();i++) vOneAndOnly.push_back(0);
  if (!source->GetData(&pcSourceData, vLOD, vOneAndOnly)) return false;

  vector<UINT64> vSize = source->GetSmallestBrickSize();

  UINT64 iDataSize = 1;
  for (size_t i = 0;i<vSize.size();i++) iDataSize*=vSize[i];

  /// \todo right now only 8 and 16 bit integer data is supported this should be changed to a more general approach
  m_fMaxGradMagnitude = 0.0f;
  if (source->ulElementBitSize[0][0] == 8) {
    for (size_t z = 0;z<size_t(vSize[2]);z++) {
      for (size_t y = 0;y<size_t(vSize[1]);y++) {
        for (size_t x = 0;x<size_t(vSize[0]);x++) {

          size_t iCenter = x+size_t(vSize[0])*y+size_t(vSize[0])*size_t(vSize[1])*z;
          size_t iLeft   = iCenter;
          size_t iRight  = iCenter;
          size_t iTop    = iCenter;
          size_t iBottom = iCenter;
          size_t iFront  = iCenter;
          size_t iBack   = iCenter;

          FLOATVECTOR3 vScale(0,0,0);

          if (x > 0)          {iLeft   = iCenter-1; vScale.x++;}
          if (x < vSize[0]-1) {iRight  = iCenter+1; vScale.x++;}
          if (y > 0)          {iTop    = iCenter-size_t(vSize[0]);vScale.y++;}
          if (y < vSize[1]-1) {iBottom = iCenter+size_t(vSize[0]);vScale.y++;}
          if (z > 0)          {iFront  = iCenter-size_t(vSize[0])*size_t(vSize[1]);vScale.z++;}
          if (z < vSize[2]-1) {iBack   = iCenter+size_t(vSize[0])*size_t(vSize[1]);vScale.z++;}

          FLOATVECTOR3   vGradient(float(pcSourceData[iLeft]-pcSourceData[iRight])/(255.0f*vScale.x),
                                   float(pcSourceData[iTop]-pcSourceData[iBottom])/(255.0f*vScale.y),
                                   float(pcSourceData[iFront]-pcSourceData[iBack])/(255.0f*vScale.y));

          if (vGradient.length() > m_fMaxGradMagnitude) m_fMaxGradMagnitude = vGradient.length();
        }
      }
    }
    for (size_t z = 0;z<size_t(vSize[2]);z++) {
      for (size_t y = 0;y<size_t(vSize[1]);y++) {
        for (size_t x = 0;x<size_t(vSize[0]);x++) {

          size_t iCenter = x+size_t(vSize[0])*y+size_t(vSize[0])*size_t(vSize[1])*z;
          size_t iLeft   = iCenter;
          size_t iRight  = iCenter;
          size_t iTop    = iCenter;
          size_t iBottom = iCenter;
          size_t iFront  = iCenter;
          size_t iBack   = iCenter;

          FLOATVECTOR3 vScale(0,0,0);

          if (x > 0)          {iLeft   = iCenter-1; vScale.x++;}
          if (x < vSize[0]-1) {iRight  = iCenter+1; vScale.x++;}
          if (y > 0)          {iTop    = iCenter-size_t(vSize[0]);vScale.y++;}
          if (y < vSize[1]-1) {iBottom = iCenter+size_t(vSize[0]);vScale.y++;}
          if (z > 0)          {iFront  = iCenter-size_t(vSize[0])*size_t(vSize[1]);vScale.z++;}
          if (z < vSize[2]-1) {iBack   = iCenter+size_t(vSize[0])*size_t(vSize[1]);vScale.z++;}

          FLOATVECTOR3   vGradient(float(pcSourceData[iLeft]-pcSourceData[iRight])/(255.0f*vScale.x),
                                   float(pcSourceData[iTop]-pcSourceData[iBottom])/(255.0f*vScale.y),
                                   float(pcSourceData[iFront]-pcSourceData[iBack])/(255.0f*vScale.y));

          unsigned char iGardientMagnitudeIndex = (unsigned char)(vGradient.length()/m_fMaxGradMagnitude*255);
          vTmpHist[pcSourceData[iCenter]][iGardientMagnitudeIndex]++;
        }
      }
    }
  } else {
    if (source->ulElementBitSize[0][0] == 16) {
      unsigned short *psSourceData = (unsigned short*)pcSourceData;
      for (size_t z = 0;z<size_t(vSize[2]);z++) {
        for (size_t y = 0;y<size_t(vSize[1]);y++) {
          for (size_t x = 0;x<size_t(vSize[0]);x++) {

            size_t iCenter = x+size_t(vSize[0])*y+size_t(vSize[0])*size_t(vSize[1])*z;
            size_t iLeft   = iCenter;
            size_t iRight  = iCenter;
            size_t iTop    = iCenter;
            size_t iBottom = iCenter;
            size_t iFront  = iCenter;
            size_t iBack   = iCenter;

            FLOATVECTOR3 vScale(0,0,0);

            if (x > 0)          {iLeft   = iCenter-1; vScale.x++;}
            if (x < vSize[0]-1) {iRight  = iCenter+1; vScale.x++;}
            if (y > 0)          {iTop    = iCenter-size_t(vSize[0]);vScale.y++;}
            if (y < vSize[1]-1) {iBottom = iCenter+size_t(vSize[0]);vScale.y++;}
            if (z > 0)          {iFront  = iCenter-size_t(vSize[0])*size_t(vSize[1]);vScale.z++;}
            if (z < vSize[2]-1) {iBack   = iCenter+size_t(vSize[0])*size_t(vSize[1]);vScale.z++;}

            FLOATVECTOR3   vGradient(float(pcSourceData[iLeft]-pcSourceData[iRight])/(65536.0f*vScale.x),
                                     float(pcSourceData[iTop]-pcSourceData[iBottom])/(65536.0f*vScale.y),
                                     float(pcSourceData[iFront]-pcSourceData[iBack])/(65536.0f*vScale.y));

            if (vGradient.length() > m_fMaxGradMagnitude) m_fMaxGradMagnitude = vGradient.length();
          }
        }
      }
      for (size_t z = 0;z<size_t(vSize[2]);z++) {
        for (size_t y = 0;y<size_t(vSize[1]);y++) {
          for (size_t x = 0;x<size_t(vSize[0]);x++) {

            size_t iCenter = x+size_t(vSize[0])*y+size_t(vSize[0])*size_t(vSize[1])*z;
            size_t iLeft   = iCenter;
            size_t iRight  = iCenter;
            size_t iTop    = iCenter;
            size_t iBottom = iCenter;
            size_t iFront  = iCenter;
            size_t iBack   = iCenter;

            FLOATVECTOR3 vScale(0,0,0);

            if (x > 0)          {iLeft   = iCenter-1; vScale.x++;}
            if (x < vSize[0]-1) {iRight  = iCenter+1; vScale.x++;}
            if (y > 0)          {iTop    = iCenter-size_t(vSize[0]);vScale.y++;}
            if (y < vSize[1]-1) {iBottom = iCenter+size_t(vSize[0]);vScale.y++;}
            if (z > 0)          {iFront  = iCenter-size_t(vSize[0])*size_t(vSize[1]);vScale.z++;}
            if (z < vSize[2]-1) {iBack   = iCenter+size_t(vSize[0])*size_t(vSize[1]);vScale.z++;}

            FLOATVECTOR3   vGradient(float(pcSourceData[iLeft]-pcSourceData[iRight])/(65536.0f*vScale.x),
                                     float(pcSourceData[iTop]-pcSourceData[iBottom])/(65536.0f*vScale.y),
                                     float(pcSourceData[iFront]-pcSourceData[iBack])/(65536.0f*vScale.y));

            unsigned char iGardientMagnitudeIndex = (unsigned char)(vGradient.length()/m_fMaxGradMagnitude*255);
            vTmpHist[psSourceData[iCenter]][iGardientMagnitudeIndex]++;
          }
        }
      }
    } else {
      delete [] pcSourceData;
      return false;
    }
  }

  delete [] pcSourceData;

  size_t iSize = 0;
  for (size_t x = iSize;x<iValueRange;x++) {
    for (size_t y = 0;y<256;y++) {
      if (vTmpHist[x][y] != 0) iSize = x+1;
    }
  }
  iValueRange = iSize;


  m_vHistData.resize(iValueRange);
  for (size_t i = 0;i<iValueRange;i++) {
    m_vHistData[i].resize(256);
    for (size_t j = 0;j<256;j++) {
      m_vHistData[i][j] = vTmpHist[i][j];
    }
  }

  // set data block information
	strBlockID = "2D Histogram for datablock " + source->strBlockID;

  return true;
}

UINT64 Histogram2DDataBlock::CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock) {
  UINT64 iStart = iOffset + DataBlock::CopyToFile(pStreamFile, iOffset, bIsBigEndian, bIsLastBlock);
	pStreamFile->SeekPos(iStart);

	UINT64 ulElementCountX = UINT64(m_vHistData.size());
	UINT64 ulElementCountY = UINT64(m_vHistData[0].size());

  pStreamFile->WriteData(m_fMaxGradMagnitude, bIsBigEndian);
  pStreamFile->WriteData(ulElementCountX, bIsBigEndian);
  pStreamFile->WriteData(ulElementCountY, bIsBigEndian);

  vector<UINT64> tmp;
  for (size_t i = 0;i<ulElementCountX;i++) {
    tmp = m_vHistData[i];
    pStreamFile->WriteRAW((unsigned char*)&tmp[0], ulElementCountY*sizeof(UINT64));
  }


	return pStreamFile->GetPos() - iOffset;  
}


UINT64 Histogram2DDataBlock::GetOffsetToNextBlock() const {
	return DataBlock::GetOffsetToNextBlock() + ComputeDataSize();
}

UINT64 Histogram2DDataBlock::ComputeDataSize() const {

	UINT64 ulElementCountX = UINT64(m_vHistData.size());
  UINT64 ulElementCountY = UINT64((ulElementCountX == 0) ? 0 : m_vHistData[0].size());

	return 1*sizeof(float) +                                // the m_fMaxGradMagnitude value
         2*sizeof(UINT64) +						            		    // length of the vectors
		     ulElementCountX*ulElementCountY*sizeof(UINT64);  // the vectors themselves
}
