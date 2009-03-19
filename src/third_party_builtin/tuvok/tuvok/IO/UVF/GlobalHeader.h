#pragma once
#ifndef GLOBALHEADER_H
#define GLOBALHEADER_H

#include <vector>
#include "UVFTables.h"

class GlobalHeader {
public:
  GlobalHeader();
  GlobalHeader(const GlobalHeader &other);
  GlobalHeader& operator=(const GlobalHeader& other);

  bool                              bIsBigEndian;
  UINT64                            ulFileVersion;
  UVFTables::ChecksumSemanticTable  ulChecksumSemanticsEntry;
  std::vector<unsigned char>        vcChecksum;
  UINT64                            ulAdditionalHeaderSize;

protected:
  UINT64 GetDataPos();
  void GetHeaderFromFile(LargeRAWFile* pStreamFile);
  void CopyHeaderToFile(LargeRAWFile* pStreamFile);
  UINT64 GetSize();
  static UINT64 GetMinSize();
  UINT64 ulOffsetToFirstDataBlock;
  void UpdateChecksum(std::vector<unsigned char> checksum, LargeRAWFile* pStreamFile);

  friend class UVF;
};

#endif // GLOBALHEADER_H
