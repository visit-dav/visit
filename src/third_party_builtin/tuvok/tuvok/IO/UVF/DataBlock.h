#pragma once

#ifndef DATABLOCK_H
#define DATABLOCK_H

#include <string>
#include <vector>
#include "UVFTables.h"

class DataBlock {
public:
  DataBlock();
  DataBlock(const DataBlock &other);
  DataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian);
  virtual ~DataBlock();
  virtual DataBlock& operator=(const DataBlock& other);
  virtual bool Verify(UINT64 iSizeofData, std::string* pstrProblem = NULL) const;

  UVFTables::BlockSemanticTable GetBlockSemantic() const {return ulBlockSemantics;}

  std::string strBlockID;
  UVFTables::CompressionSemanticTable ulCompressionScheme;

protected:
  LargeRAWFile*  m_pStreamFile;
  UINT64    m_iOffset;

  UVFTables::BlockSemanticTable ulBlockSemantics;
  UINT64 ulOffsetToNextDataBlock;

  virtual UINT64 GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian);
  virtual UINT64 CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock);
  virtual UINT64 GetOffsetToNextBlock() const;

  virtual DataBlock* Clone();

  virtual UINT64 ComputeDataSize() const {return 0;}

  friend class UVF;
};

#endif // DATABLOCK_H
