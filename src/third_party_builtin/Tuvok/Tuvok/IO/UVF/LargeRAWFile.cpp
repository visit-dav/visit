#include "LargeRAWFile.h"

using namespace std;

#define BLOCK_COPY_SIZE (UINT64(128*1024*1024))

LargeRAWFile::LargeRAWFile(const std::string& strFilename, UINT64 iHeaderSize) :
  m_strFilename(strFilename),
  m_bIsOpen(false),
  m_bWritable(false),
  m_iHeaderSize(iHeaderSize)
{
}

LargeRAWFile::LargeRAWFile(const std::wstring& wstrFilename, UINT64 iHeaderSize) :
  m_bIsOpen(false),
  m_bWritable(false),
  m_iHeaderSize(iHeaderSize)
{
  string strFilename(wstrFilename.begin(), wstrFilename.end());
  m_strFilename = strFilename;
}

LargeRAWFile::LargeRAWFile(LargeRAWFile &other) :
  m_strFilename(other.m_strFilename+"~"),
  m_bIsOpen(other.m_bIsOpen),
  m_iHeaderSize(other.m_iHeaderSize)
{
  if (m_bIsOpen) {
    UINT64 iDataSize = other.GetCurrentSize();
    Create(iDataSize);

    other.SeekStart();

    unsigned char* pData = new unsigned char[size_t(min(iDataSize, BLOCK_COPY_SIZE))];
    for (UINT64 i = 0;i<iDataSize;i+=BLOCK_COPY_SIZE) {
		  UINT64 iCopySize = min(BLOCK_COPY_SIZE, iDataSize-i);

      other.ReadRAW(pData, iCopySize);
      WriteRAW(pData, iCopySize);
    }
    delete [] pData;    
  }
}

bool LargeRAWFile::Open(bool bReadWrite) {
  #ifdef _WIN32
    m_StreamFile = CreateFileA(m_strFilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    m_bIsOpen = m_StreamFile != INVALID_HANDLE_VALUE;
  #else
    m_StreamFile = fopen(m_strFilename.c_str(), (bReadWrite) ? "w+b" : "rb");
    m_bIsOpen = m_StreamFile != NULL;
  #endif

  m_bWritable = (m_bIsOpen) ? bReadWrite : false;

  return m_bIsOpen;
}

bool LargeRAWFile::Create(UINT64 iInitialSize) {
#ifdef _WIN32
  m_StreamFile = CreateFileA(m_strFilename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
  m_bIsOpen = m_StreamFile != INVALID_HANDLE_VALUE;
#else
  m_StreamFile = fopen(m_strFilename.c_str(), "w+b");
  m_bIsOpen = m_StreamFile != NULL;
#endif

  if (m_bIsOpen) {
    SeekPos(iInitialSize);
    SeekStart();
  }

  return m_bIsOpen;
}


void LargeRAWFile::Close() {
  if (m_bIsOpen) {
    #ifdef _WIN32
        CloseHandle(m_StreamFile);
    #else
      fclose(m_StreamFile);
    #endif
    m_bIsOpen =false;
  }
}

UINT64 LargeRAWFile::GetCurrentSize() {
  UINT64 iPrevPos = GetPos();

  SeekStart();
	UINT64 ulStart = GetPos();
  UINT64 ulEnd   = SeekEnd();
  UINT64 ulFileLength = ulEnd - ulStart;
  
  SeekPos(iPrevPos);

  return ulFileLength;
}

void LargeRAWFile::SeekStart(){
  SeekPos(0);
}

UINT64 LargeRAWFile::SeekEnd(){
  #ifdef _WIN32
    LARGE_INTEGER liTarget, liRealTarget; liTarget.QuadPart = 0;
    SetFilePointerEx(m_StreamFile, liTarget, &liRealTarget, FILE_END);
    return UINT64(liRealTarget.QuadPart)-m_iHeaderSize; 
  #else
    if(fseeko(m_StreamFile, 0, SEEK_END)==0)
      return ftello(m_StreamFile)-m_iHeaderSize;//get current position=file size!
    else
      return 0;
  #endif
}

UINT64 LargeRAWFile::GetPos(){
  #ifdef _WIN32
    LARGE_INTEGER liTarget, liRealTarget; liTarget.QuadPart = 0;
    SetFilePointerEx(m_StreamFile, liTarget, &liRealTarget, FILE_CURRENT);
    return UINT64(liRealTarget.QuadPart)-m_iHeaderSize;
  #else
      return ftello(m_StreamFile)-m_iHeaderSize;//get current position=file size!
  #endif
}

void LargeRAWFile::SeekPos(UINT64 iPos){
  #ifdef _WIN32
    LARGE_INTEGER liTarget; liTarget.QuadPart = LONGLONG(iPos+m_iHeaderSize);
    SetFilePointerEx(m_StreamFile, liTarget, NULL, FILE_BEGIN);
  #else
    fseeko(m_StreamFile, off_t(iPos+m_iHeaderSize), SEEK_SET);
  #endif
}

size_t LargeRAWFile::ReadRAW(unsigned char* pData, UINT64 iCount){
  #ifdef _WIN32
    DWORD dwReadBytes;
    ReadFile(m_StreamFile, pData, (unsigned int)(iCount), &dwReadBytes, NULL);
    return dwReadBytes;
  #else
    return fread(pData,1,iCount,m_StreamFile);
  #endif
}

size_t LargeRAWFile::WriteRAW(const unsigned char* pData, UINT64 iCount){
  #ifdef _WIN32
    DWORD dwWrittenBytes;
    WriteFile(m_StreamFile, pData, (unsigned int)(iCount), &dwWrittenBytes, NULL);
    return dwWrittenBytes;
  #else
    return fwrite(pData,1,iCount,m_StreamFile);
  #endif
}

void LargeRAWFile::Delete() {
  if (m_bIsOpen) Close();
  remove(m_strFilename.c_str());
}
