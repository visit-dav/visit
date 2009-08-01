#pragma once

#ifndef UVF_RASTERDATABLOCK_H
#define UVF_RASTERDATABLOCK_H

#include <string>
#include "DataBlock.h"
#include "../../Basics/Vectors.h"

class AbstrDebugOut;

template<class T, size_t iVecLength>
void SimpleMaxMin(const void* pIn, size_t iStart, size_t iCount,
                  std::vector<DOUBLEVECTOR4>& fMinMax) {
  const T *pDataIn = (T*)pIn;

  fMinMax.resize(iVecLength);

  for (size_t i = 0;i<iVecLength;i++) {
    fMinMax[i].x = pDataIn[iStart+i];
    fMinMax[i].y = pDataIn[iStart+i];

    /// \todo remove this if the gradient computations is implemented below
    fMinMax[i].z = -std::numeric_limits<double>::max();
    fMinMax[i].w = std::numeric_limits<double>::max();
  }

  for (size_t i = iStart+iVecLength;i<iStart+iCount;i++) {
    for (size_t iComponent = 0;iComponent<iVecLength;iComponent++) {
      if (fMinMax[iComponent].x > pDataIn[i*iVecLength+iComponent]) fMinMax[iComponent].x = pDataIn[i*iVecLength+iComponent];
      if (fMinMax[iComponent].y < pDataIn[i*iVecLength+iComponent]) fMinMax[iComponent].y = pDataIn[i*iVecLength+iComponent];
      /// \todo compute gradients
    }
  }
}

template<class T> void CombineAverage(const std::vector<UINT64>& vSource, UINT64 iTarget, const void* pIn, const void* pOut) {
  const T *pDataIn = (T*)pIn;
  T *pDataOut = (T*)pOut;

  double temp = 0;
  for (size_t i = 0;i<vSource.size();i++) {
    temp += double(pDataIn[size_t(vSource[i])]);
  }
  // make sure not to touch pDataOut before we are finished with reading pDataIn, this allows for inplace combine calls
  pDataOut[iTarget] = T(temp / double(vSource.size()));
}

template<class T, size_t iVecLength> void CombineAverage(const std::vector<UINT64>& vSource, UINT64 iTarget, const void* pIn, const void* pOut) {
  const T *pDataIn = (T*)pIn;
  T *pDataOut = (T*)pOut;

  double temp[iVecLength];  
  for (size_t i = 0;i<size_t(iVecLength);i++) temp[i] = 0.0;

  for (size_t i = 0;i<vSource.size();i++) {
    for (size_t v = 0;v<size_t(iVecLength);v++)
      temp[v] += double(pDataIn[size_t(vSource[i])*iVecLength+v]) / double(vSource.size());
  }
  // make sure not to touch pDataOut before we are finished with reading pDataIn, this allows for inplace combine calls
  for (UINT64 v = 0;v<iVecLength;v++)
    pDataOut[v+iTarget*iVecLength] = T(temp[v]);
}

class Histogram1DDataBlock;
class Histogram2DDataBlock;
class MaxMinDataBlock;

class RasterDataBlock : public DataBlock {
public:
  RasterDataBlock();
  RasterDataBlock(const RasterDataBlock &other);
  RasterDataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian);
  virtual RasterDataBlock& operator=(const RasterDataBlock& other);
  virtual ~RasterDataBlock();

  virtual bool Verify(UINT64 iSizeofData, std::string* pstrProblem = NULL) const;
  virtual bool Verify(std::string* pstrProblem = NULL) const;

  std::vector<UVFTables::DomainSemanticTable> ulDomainSemantics;
  std::vector<double> dDomainTransformation;
  std::vector<UINT64> ulDomainSize;
  std::vector<UINT64> ulBrickSize;
  std::vector<UINT64> ulBrickOverlap;
  std::vector<UINT64> ulLODDecFactor;
  std::vector<UINT64> ulLODGroups;
  std::vector<UINT64> ulLODLevelCount;
  UINT64 ulElementDimension;
  std::vector<UINT64> ulElementDimensionSize;
  std::vector<std::vector<UVFTables::ElementSemanticTable> > ulElementSemantic;
  std::vector<std::vector<UINT64> > ulElementBitSize;
  std::vector<std::vector<UINT64> > ulElementMantissa;
  std::vector<std::vector<bool> > bSignedElement;
  UINT64 ulOffsetToDataBlock;

  virtual UINT64 ComputeDataSize(std::string* pstrProblem = NULL) const ;
  virtual UINT64 ComputeHeaderSize() const;

  virtual bool SetBlockSemantic(UVFTables::BlockSemanticTable bs);

  // CONVENIANCE FUNCTIONS
  void SetScaleOnlyTransformation(const std::vector<double>& vScale);
  void SetIdentityTransformation();
  void SetTypeToScalar(UINT64 iBitWith, UINT64 iMantissa, bool bSigned, UVFTables::ElementSemanticTable semantic);
  void SetTypeToVector(UINT64 iBitWith, UINT64 iMantissa, bool bSigned, std::vector<UVFTables::ElementSemanticTable> semantic);
  void SetTypeToUByte(UVFTables::ElementSemanticTable semantic);
  void SetTypeToUShort(UVFTables::ElementSemanticTable semantic);
  void SetTypeToFloat(UVFTables::ElementSemanticTable semantic);
  void SetTypeToDouble(UVFTables::ElementSemanticTable semantic);
  void SetTypeToInt32(UVFTables::ElementSemanticTable semantic);
  void SetTypeToInt64(UVFTables::ElementSemanticTable semantic);
  void SetTypeToUInt32(UVFTables::ElementSemanticTable semantic);
  void SetTypeToUInt64(UVFTables::ElementSemanticTable semantic);

  bool GetData(std::vector<unsigned char>& vData,
               const std::vector<UINT64>& vLOD,
               const std::vector<UINT64>& vBrick) const;
  bool SetData(unsigned char* pData, const std::vector<UINT64>& vLOD, const std::vector<UINT64>& vBrick);

  const std::vector<UINT64>& GetBrickCount(const std::vector<UINT64>& vLOD) const;
  const std::vector<UINT64>& GetBrickSize(const std::vector<UINT64>& vLOD, const std::vector<UINT64>& vBrick) const;
  std::vector<UINT64> GetLODDomainSize(const std::vector<UINT64>& vLOD) const;

  bool BrickedLODToFlatData(const std::vector<UINT64>& vLOD, const std::string& strTargetFile,
                            bool bAppend = false, AbstrDebugOut* pDebugOut=NULL,
                            bool (*brickFunc)(LargeRAWFile* pSourceFile,
                                const std::vector<UINT64> vBrickSize,
                                const std::vector<UINT64> vBrickOffset,
                                void* pUserContext ) = NULL,
                            void* pUserContext = NULL,
                            UINT64 iOverlap=0) const;

  const std::vector<UINT64> GetSmallestBrickIndex() const;
  const std::vector<UINT64>& GetSmallestBrickSize() const;

  void FlatDataToBrickedLOD(const void* pSourceData, const std::string& strTempFile,
                            void (*combineFunc)(const std::vector<UINT64> &vSource, UINT64 iTarget, const void* pIn, const void* pOut),
                            void (*maxminFunc)(const void* pIn, size_t iStart,
                                               size_t iCount,
                                               std::vector<DOUBLEVECTOR4>& fMinMax),
                            MaxMinDataBlock* pMaxMinDatBlock = NULL, AbstrDebugOut* pDebugOut=NULL);
  void FlatDataToBrickedLOD(LargeRAWFile* pSourceData, const std::string& strTempFile,
                            void (*combineFunc)(const std::vector<UINT64> &vSource, UINT64 iTarget, const void* pIn, const void* pOut),
                            void (*maxminFunc)(const void* pIn, size_t iStart,
                                               size_t iCount,
                                               std::vector<DOUBLEVECTOR4>& fMinMax),
                            MaxMinDataBlock* pMaxMinDatBlock = NULL, AbstrDebugOut* pDebugOut=NULL);
  void AllocateTemp(const std::string& strTempFile, bool bBuildOffsetTables=false);


protected:
  LargeRAWFile* m_pTempFile;

  virtual UINT64 GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian);
  virtual UINT64 CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock);
  virtual DataBlock* Clone();
  virtual UINT64 GetOffsetToNextBlock() const;

  std::vector<std::vector<UINT64> > CountToVectors(std::vector<UINT64> vCountVector) const ;
  UINT64 ComputeElementSize() const;
  virtual UINT64 GetLODSize(std::vector<UINT64>& vLODIndices) const;
  virtual UINT64 ComputeLODLevelSize(const std::vector<UINT64>& vReducedDomainSize) const;
  virtual std::vector<std::vector<UINT64> > ComputeBricks(const std::vector<UINT64>& vDomainSize) const;
  virtual std::vector<std::vector<UINT64> > GenerateCartesianProduct(const std::vector<std::vector<UINT64> >& vElements, UINT64 iIndex=0) const;
  UINT64 RecompLODIndexCount() const;
  void CleanupTemp();


  friend class UVF;
  friend class Histogram1DDataBlock;
  friend class Histogram2DDataBlock;

  // CONVENIANCE FUNCTION HELPERS
  std::vector<UINT64> m_vLODOffsets;
  std::vector<std::vector<UINT64> > m_vBrickCount;
  std::vector<std::vector<UINT64> > m_vBrickOffsets;
  std::vector<std::vector<std::vector<UINT64> > > m_vBrickSizes;

  UINT64 Serialize(const std::vector<UINT64>& vec, const std::vector<UINT64>& vSizes) const;
  UINT64 GetLocalDataPointerOffset(const std::vector<UINT64>& vLOD, const std::vector<UINT64>& vBrick) const;
  UINT64 GetLocalDataPointerOffset(const UINT64 iLODIndex, const UINT64 iBrickIndex) const {return m_vLODOffsets[size_t(iLODIndex)] + m_vBrickOffsets[size_t(iLODIndex)][size_t(iBrickIndex)];}
  void SubSample(LargeRAWFile* pSourceFile, LargeRAWFile* pTargetFile, std::vector<UINT64> sourceSize, std::vector<UINT64> targetSize, void (*combineFunc)(const std::vector<UINT64> &vSource, UINT64 iTarget, const void* pIn, const void* pOut), AbstrDebugOut* pDebugOut=NULL, UINT64 iLODLevel=0, UINT64 iMaxLODLevel=0);
  UINT64 ComputeDataSizeAndOffsetTables();
  UINT64 GetLODSizeAndOffsetTables(std::vector<UINT64>& vLODIndices, UINT64 iLOD);
  UINT64 ComputeLODLevelSizeAndOffsetTables(const std::vector<UINT64>& vReducedDomainSize, UINT64 iLOD);

  bool TraverseBricksToWriteBrickToFile(
      UINT64& iBrickCounter, UINT64 iBrickCount,
      const std::vector<UINT64>& vLOD,
      const std::vector<UINT64>& vBrickCount,
      std::vector<UINT64> vCoords,
      size_t iCurrentDim, UINT64 iTargetOffset,
      std::vector<unsigned char> &vData,
      LargeRAWFile* pTargetFile, UINT64 iElementSize,
      const std::vector<UINT64>& vPrefixProd,
      AbstrDebugOut* pDebugOut,
      bool (*brickFunc)(LargeRAWFile* pSourceFile,
                        const std::vector<UINT64> vBrickSize,
                        const std::vector<UINT64> vBrickOffset,
                        void* pUserContext),
      void* pUserContext,
      UINT64 iOverlap
  ) const;

  void WriteBrickToFile(size_t iCurrentDim,
                        UINT64& iSourceOffset, UINT64& iTargetOffset,
                        const std::vector<UINT64>& vBrickSize,
                        const std::vector<UINT64>& vEffectiveBrickSize,
                        std::vector<unsigned char>& vData,
                        LargeRAWFile* pTargetFile, UINT64 iElementSize,
                        const std::vector<UINT64>& vPrefixProd,
                        const std::vector<UINT64>& vPrefixProdBrick,
                        bool bDoSeek) const;
};
#endif // UVF_RASTERDATABLOCK_H
