#pragma once

#ifndef KEYVALUEPAIRDATABLOCK_H
#define KEYVALUEPAIRDATABLOCK_H

#include "DataBlock.h"

class KeyValuePair {
public:
  KeyValuePair() : strKey(""), strValue("")  {}

  KeyValuePair(std::string _strKey, std::string _strValue) :
    strKey(_strKey), strValue(_strValue)  {}

  std::string strKey;
  std::string strValue;
};

#ifdef max
  #undef max
#endif

class KeyValuePairDataBlock : public DataBlock
{
public:
  KeyValuePairDataBlock();
  KeyValuePairDataBlock(const KeyValuePairDataBlock &other);
  KeyValuePairDataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian);
  virtual KeyValuePairDataBlock& operator=(const KeyValuePairDataBlock& other);

  UINT64 GetKeyCount() const {return m_KeyValuePairs.size();}
  std::string GetKeyByIndex(UINT64 iIndex) const {return m_KeyValuePairs[size_t(iIndex)].strKey;}
  std::string GetValueByIndex(UINT64 iIndex) const {return m_KeyValuePairs[size_t(iIndex)].strValue;}
  UINT64 GetIndexByKey(std::string strKey) const {for (UINT64 i = 0;i<GetKeyCount();i++) if (GetKeyByIndex(i) == strKey) return i; return UVF_INVALID;}

  bool AddPair(std::string key, std::string value);
  virtual UINT64 ComputeDataSize() const;

protected:
  std::vector<KeyValuePair> m_KeyValuePairs;

  virtual UINT64 GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian);
  virtual UINT64 CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock);
  virtual UINT64 GetOffsetToNextBlock() const;

  virtual DataBlock* Clone();

  friend class UVF;
};


#endif // KEYVALUEPAIRDATABLOCK_H
