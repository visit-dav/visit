#pragma once

#ifndef HISTOGRAM1DDATABLOCK_H
#define HISTOGRAM1DDATABLOCK_H

#include "RasterDataBlock.h"

class Histogram1DDataBlock : public DataBlock
{
public:
  Histogram1DDataBlock();
  ~Histogram1DDataBlock();
  Histogram1DDataBlock(const Histogram1DDataBlock &other);
  Histogram1DDataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian);
  virtual Histogram1DDataBlock& operator=(const Histogram1DDataBlock& other);
  virtual UINT64 ComputeDataSize() const;

  bool Compute(RasterDataBlock* source);
  const std::vector<UINT64>& GetHistogram() {return m_vHistData;}
  void SetHistogram(std::vector<UINT64>& vHistData) {m_vHistData = vHistData;}

protected:
  std::vector<UINT64> m_vHistData;

  virtual UINT64 GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian);
  virtual UINT64 CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock);
  virtual UINT64 GetOffsetToNextBlock() const;

  virtual DataBlock* Clone();
};

#endif // HISTOGRAM1DDATABLOCK_H
