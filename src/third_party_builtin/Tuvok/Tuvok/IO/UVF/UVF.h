#pragma once
#ifndef UVF_H
#define UVF_H

#ifndef UVFVERSION
	#define UVFVERSION 1
#else
	#if UVFVERSION != 1
		>> VERSION MISMATCH <<
	#endif
#endif

#include "UVFTables.h"
#include "DataBlock.h"
#include "GlobalHeader.h"

#include "RasterDataBlock.h"
#include "KeyValuePairDataBlock.h"
#include "Histogram1DDataBlock.h"
#include "Histogram2DDataBlock.h"
#include "MaxMinDataBlock.h"


class UVF
{
public:
	static UINT64 ms_ulReaderVersion;

	UVF(std::wstring wstrFilename);
	virtual ~UVF(void);

	bool Open(bool bVerify=true, bool bReadWrite=false, std::string* pstrProblem = NULL);
	void Close();

	const GlobalHeader& GetGlobalHeader() const {return m_GlobalHeader;}
	UINT64 GetDataBlockCount() const {return UINT64(m_DataBlocks.size());}
	const DataBlock* GetDataBlock(UINT64 index) const {return m_DataBlocks[size_t(index)].first;}

	// file creation routines
	bool SetGlobalHeader(const GlobalHeader& GlobalHeader);
	bool AddDataBlock(DataBlock* dataBlock, UINT64 iSizeofData, bool bUseSourcePointer=false);
	bool Create();

  static bool IsUVFFile(const std::wstring& wstrFilename);
  static bool IsUVFFile(const std::wstring& wstrFilename, bool& bChecksumFail);

protected:
  bool			      m_bFileIsLoaded;
	bool			      m_bFileIsReadWrite;
  LargeRAWFile    m_streamFile;

	GlobalHeader m_GlobalHeader;
  std::vector< std::pair<DataBlock*,bool> > m_DataBlocks;

	bool ParseGlobalHeader(bool bVerify, std::string* pstrProblem = NULL);
	void ParseDataBlocks();
	static bool VerifyChecksum(LargeRAWFile& streamFile, GlobalHeader& globalHeader, std::string* pstrProblem = NULL);	
	static std::vector<unsigned char> ComputeChecksum(LargeRAWFile& streamFile, UVFTables::ChecksumSemanticTable eChecksumSemanticsEntry);

  static bool CheckMagic(LargeRAWFile& streamFile);

	// file creation routines
	UINT64 ComputeNewFileSize();
	void UpdateChecksum();

};

#endif // UVF_H
