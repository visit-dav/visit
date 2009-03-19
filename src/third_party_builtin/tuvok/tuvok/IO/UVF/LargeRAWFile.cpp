#include "LargeRAWFile.h"
#include <sstream>

using namespace std;


LargeRAWFile::LargeRAWFile(const std::string& strFilename, UINT64 iHeaderSize):
  m_strFilename(strFilename),
  m_bIsOpen(false),
  m_bWritable(false),
  m_iHeaderSize(iHeaderSize)
{
}

LargeRAWFile::LargeRAWFile(const std::wstring& wstrFilename, UINT64 iHeaderSize):
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
  m_bWritable(other.m_bWritable),
  m_iHeaderSize(other.m_iHeaderSize)
{
  if (m_bIsOpen) {
    UINT64 iDataSize = other.GetCurrentSize();
    Create(iDataSize);

    other.SeekStart();

    unsigned char* pData = new unsigned char[size_t(min(iDataSize,
                                                        BLOCK_COPY_SIZE))];
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
  m_StreamFile = CreateFileA(m_strFilename.c_str(),
                             (bReadWrite) ? GENERIC_READ | GENERIC_WRITE
                                          : GENERIC_READ,
                             FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    m_bIsOpen = m_StreamFile != INVALID_HANDLE_VALUE;
  #else
    m_StreamFile = fopen(m_strFilename.c_str(), (bReadWrite) ? "w+b" : "rb");
    if(m_StreamFile == NULL) {
      perror("fopen");
    }
    m_bIsOpen = m_StreamFile != NULL;
  #endif

  if (m_bIsOpen && m_iHeaderSize != 0) SeekStart();

  m_bWritable = (m_bIsOpen) ? bReadWrite : false;

  return m_bIsOpen;
}

bool LargeRAWFile::Create(UINT64 iInitialSize) {
#ifdef _WIN32
  m_StreamFile = CreateFileA(m_strFilename.c_str(),
                             GENERIC_READ | GENERIC_WRITE, 0, NULL,
                             CREATE_ALWAYS, 0, NULL);
  m_bIsOpen = m_StreamFile != INVALID_HANDLE_VALUE;
#else
  m_StreamFile = fopen(m_strFilename.c_str(), "w+b");
  m_bIsOpen = m_StreamFile != NULL;
#endif

  if (m_bIsOpen && iInitialSize>0) {
    SeekPos(iInitialSize-1);
    WriteData<unsigned char>(0,false);
    SeekStart();
  }

  return m_bIsOpen;
}

bool LargeRAWFile::Append() {
#ifdef _WIN32
  m_StreamFile = CreateFileA(m_strFilename.c_str(),
                             GENERIC_READ | GENERIC_WRITE, 0, NULL,
                             OPEN_ALWAYS, 0, NULL);
  m_bIsOpen = m_StreamFile != INVALID_HANDLE_VALUE;
#else
  m_StreamFile = fopen(m_strFilename.c_str(), "a+b");
  m_bIsOpen = m_StreamFile != NULL;
#endif

  if (m_bIsOpen) SeekEnd();

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

void LargeRAWFile::SeekStart() {
  SeekPos(0);
}

UINT64 LargeRAWFile::SeekEnd() {
  #ifdef _WIN32
    LARGE_INTEGER liTarget, liRealTarget; liTarget.QuadPart = 0;
    SetFilePointerEx(m_StreamFile, liTarget, &liRealTarget, FILE_END);
    return UINT64(liRealTarget.QuadPart)-m_iHeaderSize;
  #else
    // get current position=file size!
    if(fseeko(m_StreamFile, 0, SEEK_END)==0)
      return ftello(m_StreamFile)-m_iHeaderSize;
    else
      return 0;
  #endif
}

UINT64 LargeRAWFile::GetPos() {
  #ifdef _WIN32
    LARGE_INTEGER liTarget, liRealTarget; liTarget.QuadPart = 0;
    SetFilePointerEx(m_StreamFile, liTarget, &liRealTarget, FILE_CURRENT);
    return UINT64(liRealTarget.QuadPart)-m_iHeaderSize;
  #else
    //get current position=file size!
    return ftello(m_StreamFile)-m_iHeaderSize;
  #endif
}

void LargeRAWFile::SeekPos(UINT64 iPos) {
  #ifdef _WIN32
    LARGE_INTEGER liTarget; liTarget.QuadPart = LONGLONG(iPos+m_iHeaderSize);
    SetFilePointerEx(m_StreamFile, liTarget, NULL, FILE_BEGIN);
  #else
    fseeko(m_StreamFile, off_t(iPos+m_iHeaderSize), SEEK_SET);
  #endif
}

size_t LargeRAWFile::ReadRAW(unsigned char* pData, UINT64 iCount) {
  #ifdef _WIN32
    DWORD dwReadBytes;
    ReadFile(m_StreamFile, pData, DWORD(iCount), &dwReadBytes, NULL);
    return dwReadBytes;
  #else
    return fread(pData,1,iCount,m_StreamFile);
  #endif
}

size_t LargeRAWFile::WriteRAW(const unsigned char* pData, UINT64 iCount) {
  #ifdef _WIN32
    DWORD dwWrittenBytes;
    WriteFile(m_StreamFile, pData, DWORD(iCount), &dwWrittenBytes, NULL);
    return dwWrittenBytes;
  #else
    return fwrite(pData,1,iCount,m_StreamFile);
  #endif
}

void LargeRAWFile::Delete() {
  if (m_bIsOpen) Close();
  remove(m_strFilename.c_str());
}



bool LargeRAWFile::Copy(const std::string& strSource,
                        const std::string& strTarget, UINT64 iSourceHeaderSkip,
                        std::string* strMessage) {
  std::wstring wstrSource(strSource.begin(), strSource.end());
  std::wstring wstrTarget(strTarget.begin(), strTarget.end());
  if (!strMessage)
    return Copy(wstrSource, wstrTarget, iSourceHeaderSkip, NULL);
  else {
    std::wstring wstrMessage;
    bool bResult = Copy(wstrSource, wstrTarget, iSourceHeaderSkip,
                        &wstrMessage);
    (*strMessage) = string(wstrMessage.begin(), wstrMessage.end());
    return bResult;
  }
}


bool LargeRAWFile::Copy(const std::wstring& wstrSource,
                        const std::wstring& wstrTarget,
                        UINT64 iSourceHeaderSkip, std::wstring* wstrMessage) {
  LargeRAWFile source(wstrSource, iSourceHeaderSkip);
  LargeRAWFile target(wstrTarget);

  source.Open(false);
  if (!source.IsOpen()) {
    if (wstrMessage) (*wstrMessage) = L"Unable to open source file " +
                                      wstrSource;
    return false;
  }

  target.Create();
  if (!target.IsOpen()) {
    if (wstrMessage) (*wstrMessage) = L"Unable to open target file " +
                                      wstrTarget;
    source.Close();
    return false;
  }

  UINT64 iFileSize = source.GetCurrentSize();
  UINT64 iCopySize = min(iFileSize,BLOCK_COPY_SIZE);
  unsigned char* pBuffer = new unsigned char[size_t(iCopySize)];

  do {
    iCopySize = source.ReadRAW(pBuffer, iCopySize);
    target.WriteRAW(pBuffer, iCopySize);
  } while (iCopySize>0);

  target.Close();
  source.Close();
  return true;
}


bool LargeRAWFile::Compare(const std::string& strFirstFile,
                           const std::string& strSecondFile,
                           std::string* strMessage) {
  std::wstring wstrFirstFile(strFirstFile.begin(), strFirstFile.end());
  std::wstring wstrSecondFile(strSecondFile.begin(), strSecondFile.end());
  if (!strMessage)
    return Compare(wstrFirstFile, wstrSecondFile, NULL);
  else {
    std::wstring wstrMessage;
    bool bResult = Compare(wstrFirstFile, wstrSecondFile, &wstrMessage);
    (*strMessage) = string(wstrMessage.begin(), wstrMessage.end());
    return bResult;
  }
}

bool LargeRAWFile::Compare(const std::wstring& wstrFirstFile,
                           const std::wstring& wstrSecondFile,
                           std::wstring* wstrMessage) {
  LargeRAWFile first(wstrFirstFile);
  LargeRAWFile second(wstrSecondFile);

  first.Open(false);
  if (!first.IsOpen()) {
    if (wstrMessage) (*wstrMessage) = L"Unable to open input file " +
                                      wstrFirstFile;
    return false;
  }
  second.Open(false);
  if (!second.IsOpen()) {
    if (wstrMessage) (*wstrMessage) = L"Unable to open input file " +
                                      wstrSecondFile;
    first.Close();
    return false;
  }

  if (first.GetCurrentSize() != second.GetCurrentSize()) {
    first.Close();
    second.Close();
    if (wstrMessage) (*wstrMessage) = L"Files differ in size";
    return false;
  }

  UINT64 iFileSize = first.GetCurrentSize();
  UINT64 iCopySize = min(iFileSize,BLOCK_COPY_SIZE/2);
  unsigned char* pBuffer1 = new unsigned char[size_t(iCopySize)];
  unsigned char* pBuffer2 = new unsigned char[size_t(iCopySize)];
  UINT64 iCopiedSize = 0;
  UINT64 iDiffCount = 0;

  if (wstrMessage) (*wstrMessage) = L"";

  do {
    iCopySize = first.ReadRAW(pBuffer1, iCopySize);
    second.ReadRAW(pBuffer2, iCopySize);

    for (UINT64 i = 0;i<iCopySize;i++) {
      if (pBuffer1[i] != pBuffer2[i]) {
        if (wstrMessage) {
          wstringstream ss;
          if (iDiffCount == 0) {
            ss << L"Files differ at address " << i+iCopiedSize;
            iDiffCount = 1;
          } else {
            // don't report more than 10 differences.
            if (++iDiffCount == 10) {
              (*wstrMessage) += L" and more";
              delete [] pBuffer1;
              delete [] pBuffer2;
              first.Close();
              second.Close();
              return false;
            } else {
              ss << L", " << i+iCopiedSize;
            }
          }
          (*wstrMessage) +=ss.str();
        }
      }

    }
    iCopiedSize += iCopySize;
  } while (iCopySize > 0);

  first.Close();
  second.Close();
  delete [] pBuffer1;
  delete [] pBuffer2;
  return iDiffCount == 0;
}
