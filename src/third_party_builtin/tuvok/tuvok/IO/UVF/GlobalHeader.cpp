#include "GlobalHeader.h"
#include "UVF.h"

using namespace std;
using namespace UVFTables;

GlobalHeader::GlobalHeader() : 
  bIsBigEndian(false),
  ulFileVersion(0),
  ulChecksumSemanticsEntry(CS_NONE),
  ulAdditionalHeaderSize(0),
  ulOffsetToFirstDataBlock(0)
{}

GlobalHeader::GlobalHeader(const GlobalHeader &other) : 
  bIsBigEndian(other.bIsBigEndian),
  ulFileVersion(other.ulFileVersion),
  ulChecksumSemanticsEntry(other.ulChecksumSemanticsEntry),
  vcChecksum(other.vcChecksum),
  ulAdditionalHeaderSize(other.ulAdditionalHeaderSize),
  ulOffsetToFirstDataBlock(other.ulOffsetToFirstDataBlock)
{
}

GlobalHeader& GlobalHeader::operator=(const GlobalHeader& other)  { 
  bIsBigEndian = other.bIsBigEndian;
  ulFileVersion = other.ulFileVersion;
  ulChecksumSemanticsEntry = other.ulChecksumSemanticsEntry;
  ulAdditionalHeaderSize = other.ulAdditionalHeaderSize;
  ulOffsetToFirstDataBlock = other.ulOffsetToFirstDataBlock;
  vcChecksum = other.vcChecksum;
  return *this;
}

UINT64 GlobalHeader::GetDataPos() {
  return 8 + GetSize();
}

void GlobalHeader::GetHeaderFromFile(LargeRAWFile* pStreamFile) {
  pStreamFile->ReadData(bIsBigEndian, false);
  pStreamFile->ReadData(ulFileVersion, bIsBigEndian);
  UINT64 uintSem;
  pStreamFile->ReadData(uintSem, bIsBigEndian);
  ulChecksumSemanticsEntry = (ChecksumSemanticTable)uintSem;
  UINT64 ulChecksumLength;
  pStreamFile->ReadData(ulChecksumLength, bIsBigEndian);
   pStreamFile->ReadData(vcChecksum, ulChecksumLength, bIsBigEndian);
  pStreamFile->ReadData(ulOffsetToFirstDataBlock, bIsBigEndian);
}

void GlobalHeader::CopyHeaderToFile(LargeRAWFile* pStreamFile) {
  pStreamFile->WriteData(bIsBigEndian, false);
  pStreamFile->WriteData(ulFileVersion, bIsBigEndian);
  pStreamFile->WriteData(UINT64(ulChecksumSemanticsEntry), bIsBigEndian);
  pStreamFile->WriteData(UINT64(vcChecksum.size()), bIsBigEndian);
  pStreamFile->WriteData(vcChecksum, bIsBigEndian);
  pStreamFile->WriteData(ulOffsetToFirstDataBlock, bIsBigEndian);
}

UINT64 GlobalHeader::GetSize() {
  return GetMinSize() + vcChecksum.size() + ulOffsetToFirstDataBlock;
}

UINT64 GlobalHeader::GetMinSize() {
  return sizeof(bool) + 4 * sizeof(UINT64);
}

void GlobalHeader::UpdateChecksum(vector<unsigned char> checksum, LargeRAWFile* pStreamFile) {
  vcChecksum = checksum;
  UINT64 ulLastPos = pStreamFile->GetPos();
  pStreamFile->SeekPos(33);
  pStreamFile->WriteData(vcChecksum, bIsBigEndian);
  pStreamFile->SeekPos(ulLastPos);
}
