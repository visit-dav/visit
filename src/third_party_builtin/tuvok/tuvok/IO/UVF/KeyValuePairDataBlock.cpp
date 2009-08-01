#include "KeyValuePairDataBlock.h"

using namespace std;
using namespace UVFTables;

KeyValuePairDataBlock::KeyValuePairDataBlock() {
  ulBlockSemantics = BS_KEY_VALUE_PAIRS;
  strBlockID       = "KeyValue Pair Block";
}

KeyValuePairDataBlock::KeyValuePairDataBlock(const KeyValuePairDataBlock &other) : 
  DataBlock(other), m_KeyValuePairs(other.m_KeyValuePairs)
{
}

KeyValuePairDataBlock::KeyValuePairDataBlock(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
  GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
}

KeyValuePairDataBlock& KeyValuePairDataBlock::operator=(const KeyValuePairDataBlock& other) {
  strBlockID = other.strBlockID;
  ulBlockSemantics = other.ulBlockSemantics;
  ulCompressionScheme = other.ulCompressionScheme;
  ulOffsetToNextDataBlock = other.ulOffsetToNextDataBlock;

  m_KeyValuePairs = other.m_KeyValuePairs;

  return *this;
}

UINT64 KeyValuePairDataBlock::GetHeaderFromFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian) {
  UINT64 iStart = iOffset + DataBlock::GetHeaderFromFile(pStreamFile, iOffset, bIsBigEndian);
  pStreamFile->SeekPos(iStart);

  UINT64 ulElementCount;
  pStreamFile->ReadData(ulElementCount, bIsBigEndian);

  for (UINT64 i = 0;i<ulElementCount;i++) {
    string key = "", value = "";
    UINT64 iStrLength;

    pStreamFile->ReadData(iStrLength, bIsBigEndian);
    pStreamFile->ReadData(key, iStrLength);

    pStreamFile->ReadData(iStrLength, bIsBigEndian);
    pStreamFile->ReadData(value, iStrLength);

    KeyValuePair p(key, value);

    m_KeyValuePairs.push_back(p);
  }

  return pStreamFile->GetPos() - iOffset;
}

UINT64 KeyValuePairDataBlock::CopyToFile(LargeRAWFile* pStreamFile, UINT64 iOffset, bool bIsBigEndian, bool bIsLastBlock) {
  UINT64 iStart = iOffset + DataBlock::CopyToFile(pStreamFile, iOffset, bIsBigEndian, bIsLastBlock);
  pStreamFile->SeekPos(iStart);

  pStreamFile->WriteData(UINT64(m_KeyValuePairs.size()), bIsBigEndian);

  for (size_t i = 0;i<m_KeyValuePairs.size();i++) {
    string key(m_KeyValuePairs[i].strKey), value(m_KeyValuePairs[i].strValue);

    pStreamFile->WriteData(UINT64(key.length()), bIsBigEndian);
    pStreamFile->WriteData(key);

    pStreamFile->WriteData(UINT64(value.length()), bIsBigEndian);
    pStreamFile->WriteData(value);
  }

  return pStreamFile->GetPos() - iOffset;
}

DataBlock* KeyValuePairDataBlock::Clone() {
  return new KeyValuePairDataBlock(*this);
}

UINT64 KeyValuePairDataBlock::GetOffsetToNextBlock() const {
  return DataBlock::GetOffsetToNextBlock() + ComputeDataSize();
}


UINT64 KeyValuePairDataBlock::ComputeDataSize() const {
  UINT64 iCharCount = 0;

  for (size_t i = 0;i<m_KeyValuePairs.size();i++)
    iCharCount += m_KeyValuePairs[i].strKey.length() + m_KeyValuePairs[i].strValue.length();

  return sizeof(UINT64) +                 // size of the vector
       iCharCount +                   // the strings themselves
       m_KeyValuePairs.size() * 2 * sizeof(UINT64);  // UINT64s indicating the stringlength
}

bool KeyValuePairDataBlock::AddPair(string key, string value) {
  for (size_t i = 0;i<m_KeyValuePairs.size();i++) {
    if (key == m_KeyValuePairs[i].strKey) return false;
  }

  KeyValuePair p(key,value);
  m_KeyValuePairs.push_back(p);

  return true;
}
