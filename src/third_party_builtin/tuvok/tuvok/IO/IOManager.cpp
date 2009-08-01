/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
  \file    IOManager.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#include <fstream>
#include <sstream>
#include <map>
#include "boost/cstdint.hpp"

#include "IOManager.h"
#include <Controller/Controller.h>
#include <IO/DICOM/DICOMParser.h>
#include <IO/Images/ImageParser.h>
#include <Basics/SysTools.h>
#include <Renderer/GPUMemMan/GPUMemMan.h>
#include "Metadata.h"
#include "TuvokJPEG.h"
#include "uvfDataset.h"

#include "BOVConverter.h"
#include "NRRDConverter.h"
#include "QVISConverter.h"
#include "REKConverter.h"
#include "I3MConverter.h"
#include "StkConverter.h"
#include "TiffVolumeConverter.h"
#include "VFFConverter.h"

using namespace std;
using namespace tuvok;

IOManager::IOManager() :
  m_pFinalConverter(NULL)
{
  m_vpConverters.push_back(new QVISConverter());
  m_vpConverters.push_back(new NRRDConverter());
  m_vpConverters.push_back(new StkConverter());
  m_vpConverters.push_back(new TiffVolumeConverter());
  m_vpConverters.push_back(new VFFConverter());
  m_vpConverters.push_back(new BOVConverter());
  m_vpConverters.push_back(new REKConverter());
  m_vpConverters.push_back(new I3MConverter());
}


void IOManager::RegisterExternalConverter(AbstrConverter* pConverter) {
  m_vpConverters.push_back(pConverter);
}

void IOManager::RegisterFinalConverter(AbstrConverter* pConverter) {
  if ( m_pFinalConverter ) delete m_pFinalConverter;
  m_pFinalConverter = pConverter;
}


IOManager::~IOManager()
{
  for (size_t i = 0;i<m_vpConverters.size();i++) delete m_vpConverters[i];
  m_vpConverters.clear();

  delete m_pFinalConverter;
}

vector<FileStackInfo*> IOManager::ScanDirectory(std::string strDirectory) {

  MESSAGE("Scanning directory %s", strDirectory.c_str());

  std::vector<FileStackInfo*> fileStacks;

  DICOMParser parseDICOM;
  parseDICOM.GetDirInfo(strDirectory);

  // Sort out DICOMs with embedded images that we can't read.
  for (size_t iStackID = 0;iStackID < parseDICOM.m_FileStacks.size();iStackID++) {
    DICOMStackInfo* f = new DICOMStackInfo((DICOMStackInfo*)parseDICOM.m_FileStacks[iStackID]);

    // if trying to load JPEG files. check if we can handle the JPEG payload
    if (f->m_bIsJPEGEncoded) {
      for(size_t i=0; i < f->m_Elements.size(); ++i) {
        if(!tuvok::JPEG(f->m_Elements[i]->m_strFileName,
                        dynamic_cast<SimpleDICOMFileInfo*>
                          (f->m_Elements[i])->GetOffsetToData()).valid()) {
          WARNING("Can't load JPEG in stack %d, element %u!", iStackID, i);
          // should probably be using ptr container lib here instead of trying to
          // explicitly manage this.
          delete *(parseDICOM.m_FileStacks.begin()+iStackID);
          parseDICOM.m_FileStacks.erase(parseDICOM.m_FileStacks.begin()+iStackID);
          iStackID--;
          break;
        }
      }
    }

    delete f;
  }


  if (parseDICOM.m_FileStacks.size() == 1)
    MESSAGE("  found a single DICOM stack");
  else
    MESSAGE("  found %i DICOM stacks", int(parseDICOM.m_FileStacks.size()));

  for (size_t iStackID = 0;iStackID < parseDICOM.m_FileStacks.size();iStackID++) {
    DICOMStackInfo* f = new DICOMStackInfo((DICOMStackInfo*)parseDICOM.m_FileStacks[iStackID]);

    stringstream s;
    s << f->m_strFileType << " Stack: " << f->m_strDesc;
    f->m_strDesc = s.str();

    fileStacks.push_back(f);
  }

  ImageParser parseImages;
  parseImages.GetDirInfo(strDirectory);

  if (parseImages.m_FileStacks.size() == 1)
    MESSAGE("  found a single image stack");
  else
    MESSAGE("  found %i image stacks", int(parseImages.m_FileStacks.size()));

  for (size_t iStackID = 0;iStackID < parseImages.m_FileStacks.size();iStackID++) {
    ImageStackInfo* f = new ImageStackInfo((ImageStackInfo*)parseImages.m_FileStacks[iStackID]);

    stringstream s;
    s << f->m_strFileType << " Stack: " << f->m_strDesc;
    f->m_strDesc = s.str();

    fileStacks.push_back(f);
  }

   // add other image parsers here

  MESSAGE("  scan complete");

  return fileStacks;
}

#ifdef DETECTED_OS_WINDOWS
  #pragma warning(disable:4996)
#endif

bool IOManager::ConvertDataset(FileStackInfo* pStack, const std::string& strTargetFilename, const std::string& strTempDir) {

  MESSAGE("Request to convert stack of %s files to %s received", pStack->m_strDesc.c_str(), strTargetFilename.c_str());

  if (pStack->m_strFileType == "DICOM") {
    MESSAGE("  Detected DICOM stack, starting DICOM conversion");

    DICOMStackInfo* pDICOMStack = ((DICOMStackInfo*)pStack);

    MESSAGE("  Stack contains %i files",  int(pDICOMStack->m_Elements.size()));
    MESSAGE("    Series: %i  Bits: %i (%i)", pDICOMStack->m_iSeries, pDICOMStack->m_iAllocated, pDICOMStack->m_iStored);
    MESSAGE("    Date: %s  Time: %s", pDICOMStack->m_strAcquDate.c_str(), pDICOMStack->m_strAcquTime.c_str());
    MESSAGE("    Modality: %s  Description: %s", pDICOMStack->m_strModality.c_str(), pDICOMStack->m_strDesc.c_str());
    MESSAGE("    Aspect Ratio: %g %g %g", pDICOMStack->m_fvfAspect.x, pDICOMStack->m_fvfAspect.y, pDICOMStack->m_fvfAspect.z);

    string strTempMergeFilename = strTargetFilename + "~";

    MESSAGE("    Creating intermediate file %s", strTempMergeFilename.c_str());

    ofstream fs;
    fs.open(strTempMergeFilename.c_str(),fstream::binary);
    if (fs.fail())  {
      T_ERROR("Could not create temp file %s aborted conversion.", strTempMergeFilename.c_str());
      return false;
    }

    char *pData = NULL;
    for (size_t j = 0;j<pDICOMStack->m_Elements.size();j++) {

      UINT32 iDataSize = pDICOMStack->m_Elements[j]->GetDataSize();

      pData = new char[iDataSize];

      if (pDICOMStack->m_bIsJPEGEncoded) {
        MESSAGE("JPEG is %d bytes, offset %d", iDataSize,
                dynamic_cast<SimpleDICOMFileInfo*>(pDICOMStack->m_Elements[j])
                ->GetOffsetToData());
        tuvok::JPEG jpg(pDICOMStack->m_Elements[j]->m_strFileName,
                        dynamic_cast<SimpleDICOMFileInfo*>
                          (pDICOMStack->m_Elements[j])->GetOffsetToData());
        if(!jpg.valid()) {
          delete []pData;
          WARNING("'%s' reports an embedded JPEG, but the JPEG is invalid.",
                  pDICOMStack->m_Elements[j]->m_strFileName.c_str());
          return false;
        }
        MESSAGE("jpg is: %u bytes (%ux%u, %u components)", jpg.size(),
                jpg.width(), jpg.height(), jpg.components());

        const char *jpeg_data = jpg.data();
        std::copy(jpeg_data, jpeg_data + jpg.size(), pData);
      } else {
        // the first call does a "new" on pData
        pDICOMStack->m_Elements[j]->GetData((void**)&pData);
      }


      if (pDICOMStack->m_bIsBigEndian != EndianConvert::IsBigEndian()) {
        switch (pDICOMStack->m_iAllocated) {
          case  8 : break;
          case 16 : {
                for (UINT32 k = 0;k<iDataSize/2;k++)
                  ((short*)pData)[k] = EndianConvert::Swap<short>(((short*)pData)[k]);
                } break;
          case 32 : {
                for (UINT32 k = 0;k<iDataSize/4;k++)
                  ((float*)pData)[k] = EndianConvert::Swap<float>(((float*)pData)[k]);
                } break;
        }
      }

      // Create temporary file with the DICOM (image) data.  We pretend 3
      // component data is 4 component data to simplify processing later.
      /// @todo FIXME: this code assumes 3 component data is always 3*char
      if (pDICOMStack->m_iComponentCount == 3) {
        UINT32 iRGBADataSize = (iDataSize / 3 ) * 4;

        // Later we'll tell RAWConverter that this dataset has
        // m_iComponentCount components.  Since we're upping the number of
        // components here, we update the component count too.
        pDICOMStack->m_iComponentCount = 4;
        // Do note that the number of components in the data and the number of
        // components in our in-memory copy of the data now differ.

        unsigned char *pRGBAData = new unsigned char[ iRGBADataSize ];
        for (UINT32 k = 0;k<iDataSize/3;k++) {
          pRGBAData[k*4+0] = pData[k*3+0];
          pRGBAData[k*4+1] = pData[k*3+1];
          pRGBAData[k*4+2] = pData[k*3+2];
          pRGBAData[k*4+3] = 255;
        }

        fs.write((char*)pRGBAData, iRGBADataSize);
        delete [] pRGBAData;
      } else {
        fs.write(pData, iDataSize);
      }
    }
    delete [] pData;

    fs.close();
    MESSAGE("    done creating intermediate file %s", strTempMergeFilename.c_str());

    UINTVECTOR3 iSize = pDICOMStack->m_ivSize;
    iSize.z *= UINT32(pDICOMStack->m_Elements.size());

    /// \todo evaluate pDICOMStack->m_strModality

    bool result = RAWConverter::ConvertRAWDataset(strTempMergeFilename, strTargetFilename, strTempDir, 0,
                                    pDICOMStack->m_iAllocated, pDICOMStack->m_iComponentCount,
                                    pDICOMStack->m_bIsBigEndian != EndianConvert::IsBigEndian(),
                                    pDICOMStack->m_iAllocated >=32, /// \todo read sign property from DICOM file
                                    false, /// \todo read float property from DICOM file
                                    iSize, pDICOMStack->m_fvfAspect,
                                    "DICOM stack", SysTools::GetFilename(pDICOMStack->m_Elements[0]->m_strFileName)
                                    + " to " + SysTools::GetFilename(pDICOMStack->m_Elements[pDICOMStack->m_Elements.size()-1]->m_strFileName));

    if( remove(strTempMergeFilename.c_str()) != 0 ) {
      WARNING("Unable to remove temp file %s", strTempMergeFilename.c_str());
    }

    return result;
  } else {
     if (pStack->m_strFileType == "IMAGE") {
        MESSAGE("  Detected Image stack, starting image conversion");
        MESSAGE("  Stack contains %i files",  int(pStack->m_Elements.size()));

        string strTempMergeFilename = strTargetFilename + "~";
        MESSAGE("    Creating intermediate file %s", strTempMergeFilename.c_str());

        ofstream fs;
        fs.open(strTempMergeFilename.c_str(),fstream::binary);
        if (fs.fail())  {
          T_ERROR("Could not create temp file %s aborted conversion.", strTempMergeFilename.c_str());
          return false;
        }

        char *pData = NULL;
        for (size_t j = 0;j<pStack->m_Elements.size();j++) {
          pStack->m_Elements[j]->GetData((void**)(char**)&pData); // the first call does a "new" on pData, the strange casting here is to avoid pointer aliasing issues

          UINT32 iDataSize = pStack->m_Elements[j]->GetDataSize();
          fs.write(pData, iDataSize);
        }
        delete [] pData;


        fs.close();
        MESSAGE("    done creating intermediate file %s", strTempMergeFilename.c_str());

        UINTVECTOR3 iSize = pStack->m_ivSize;
        iSize.z *= UINT32(pStack->m_Elements.size());

        bool result = RAWConverter::ConvertRAWDataset(strTempMergeFilename, strTargetFilename, strTempDir, 0,
                                        pStack->m_iAllocated, pStack->m_iComponentCount,
                                        pStack->m_bIsBigEndian != EndianConvert::IsBigEndian(),
                                        pStack->m_iComponentCount >= 32,
                                        false,
                                        iSize, pStack->m_fvfAspect,
                                        "Image stack", SysTools::GetFilename(pStack->m_Elements[0]->m_strFileName)
                                        + " to " + SysTools::GetFilename(pStack->m_Elements[pStack->m_Elements.size()-1]->m_strFileName));

        if( remove(strTempMergeFilename.c_str()) != 0 ) {
          WARNING("Unable to remove temp file %s", strTempMergeFilename.c_str());
        }

        return result;
     }
  }


  T_ERROR("Unknown source stack type %s", pStack->m_strFileType.c_str());
  return false;
}

#ifdef DETECTED_OS_WINDOWS
  #pragma warning(default:4996)
#endif


bool IOManager::MergeDatasets(const std::vector <std::string>& strFilenames, const std::vector <double>& vScales,
                              const std::vector<double>& vBiases, const std::string& strTargetFilename,
                              const std::string& strTempDir,
                              bool bUseMaxMode, bool bNoUserInteraction) {
  MESSAGE("Request to merge multiple data sets into %s received.", strTargetFilename.c_str());

  // convert the input files to RAW
  UINT64        iComponentSizeG=0;
  UINT64        iComponentCountG=0;
  bool          bConvertEndianessG=false;
  bool          bSignedG=false;
  bool          bIsFloatG=false;
  UINTVECTOR3   vVolumeSizeG(0,0,0);
  FLOATVECTOR3  vVolumeAspectG(0,0,0);
  string strTitleG      = "Merged data from multiple files";
  stringstream  ss;
  for (size_t i = 0;i<strFilenames.size();i++) {
    ss << SysTools::GetFilename(strFilenames[i]);
    if (i<strFilenames.size()-1) ss << " ";
  }
  string        strSourceG = ss.str();

  bool bRAWCreated = false;
  vector<MergeDataset> vIntermediateFiles;
  for (size_t iInputData = 0;iInputData<strFilenames.size();iInputData++) {
    MESSAGE("Reading data sets %s...", strFilenames[iInputData].c_str());
    string strExt       = SysTools::ToUpperCase(SysTools::GetExt(strFilenames[iInputData]));

    bRAWCreated = false;
    MergeDataset IntermediateFile;
    IntermediateFile.fScale = vScales[iInputData];
    IntermediateFile.fBias = vBiases[iInputData];

    if (strExt == "UVF") {

      UVFDataset v(strFilenames[iInputData],false);
      if (!v.IsOpen()) break;

      UINT64 iLODLevel = 0; // always extract the highest quality here

      IntermediateFile.iHeaderSkip = 0;

      if (iInputData == 0)  {
        iComponentSizeG = v.GetInfo().GetBitWidth();
        iComponentCountG = v.GetInfo().GetComponentCount();
        bConvertEndianessG = !v.GetInfo().IsSameEndianness();
        bSignedG = v.GetInfo().GetIsSigned();
        bIsFloatG = v.GetInfo().GetIsFloat();
        vVolumeSizeG = UINTVECTOR3(v.GetInfo().GetDomainSize(iLODLevel));
        vVolumeAspectG = FLOATVECTOR3(v.GetInfo().GetScale());
      } else {
        if (iComponentSizeG  != v.GetInfo().GetBitWidth() ||
            iComponentCountG != v.GetInfo().GetComponentCount() ||
            bConvertEndianessG != !v.GetInfo().IsSameEndianness() ||
            bSignedG != v.GetInfo().GetIsSigned() ||
            bIsFloatG != v.GetInfo().GetIsFloat() ||
            vVolumeSizeG != UINTVECTOR3(v.GetInfo().GetDomainSize(iLODLevel))) {
          bRAWCreated = false;
          break;
        }
        if (vVolumeAspectG != FLOATVECTOR3(v.GetInfo().GetScale()))
          WARNING("Different aspect ratios found.");
      }

      IntermediateFile.strFilename = strTempDir + SysTools::GetFilename(strFilenames[iInputData]) + SysTools::ToString(rand()) +".raw";
      IntermediateFile.bDelete = true;

      if (!v.Export(iLODLevel, IntermediateFile.strFilename, false)) {
        if (SysTools::FileExists(IntermediateFile.strFilename)) remove(IntermediateFile.strFilename.c_str());
        break;
      } else bRAWCreated = true;
      vIntermediateFiles.push_back(IntermediateFile);
    } else {

      UINT64        iComponentSize=0;
      UINT64        iComponentCount=0;
      bool          bConvertEndianess=false;
      bool          bSigned=false;
      bool          bIsFloat=false;
      UINTVECTOR3   vVolumeSize(0,0,0);
      FLOATVECTOR3  vVolumeAspect(0,0,0);
      string        strTitle = "";
      string        strSource = "";
      UVFTables::ElementSemanticTable eType = UVFTables::ES_UNDEFINED;


      for (size_t i = 0;i<m_vpConverters.size();i++) {
        const std::vector<std::string>& vStrSupportedExt = m_vpConverters[i]->SupportedExt();
        for (size_t j = 0;j<vStrSupportedExt.size();j++) {
          if (vStrSupportedExt[j] == strExt) {
            bRAWCreated = m_vpConverters[i]->ConvertToRAW(strFilenames[iInputData], strTempDir, bNoUserInteraction,
                                            IntermediateFile.iHeaderSkip, iComponentSize, iComponentCount, bConvertEndianess, bSigned, bIsFloat, vVolumeSize, vVolumeAspect,
                                            strTitle, eType, IntermediateFile.strFilename, IntermediateFile.bDelete);
            strSource = SysTools::GetFilename(strFilenames[iInputData]);
            if (!bRAWCreated) continue;
          }
        }
      }

      if (!bRAWCreated && m_pFinalConverter) {
        bRAWCreated = m_pFinalConverter->ConvertToRAW(strFilenames[iInputData], strTempDir, bNoUserInteraction,
                                        IntermediateFile.iHeaderSkip, iComponentSize, iComponentCount, bConvertEndianess, bSigned, bIsFloat, vVolumeSize, vVolumeAspect,
                                        strTitle, eType, IntermediateFile.strFilename, IntermediateFile.bDelete);
        strSource = SysTools::GetFilename(strFilenames[iInputData]);
      }


      if (!bRAWCreated) break;

      vIntermediateFiles.push_back(IntermediateFile);

      if (iInputData == 0)  {
        iComponentSizeG = iComponentSize;
        iComponentCountG = iComponentCount;
        bConvertEndianessG = bConvertEndianess;
        bSignedG = bSigned;
        bIsFloatG = bIsFloat;
        vVolumeSizeG = vVolumeSize;
        vVolumeAspectG = vVolumeAspect;
      } else {
        if (iComponentSizeG  != iComponentSize ||
            iComponentCountG != iComponentCount ||
            bConvertEndianessG != bConvertEndianess ||
            bSignedG != bSigned ||
            bIsFloatG != bIsFloat ||
            vVolumeSizeG != vVolumeSize) {
          bRAWCreated = false;
          break;
        }

        if (vVolumeAspectG != vVolumeAspect)
          WARNING("Different aspect ratios found.");
      }
    }

  }

  if (!bRAWCreated) {
    for (size_t i = 0;i<vIntermediateFiles.size();i++) {
      if (vIntermediateFiles[i].bDelete && SysTools::FileExists(vIntermediateFiles[i].strFilename))
        remove(vIntermediateFiles[i].strFilename.c_str());
    }
    return false;
  }

  // merge the raw files into a single RAW file
  string strMergedFile = strTempDir + "merged.raw";

  bool bIsMerged = false;
  MasterController *MCtlr = &(Controller::Instance());
  if (bSignedG) {
    if (bIsFloatG) {
      switch (iComponentSizeG) {
        case 32 : {
                    DataMerger<float> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
        case 64 : {
                    DataMerger<double> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
      }
    } else {
      switch (iComponentSizeG) {
        case 8  : {
                    DataMerger<char> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
        case 16 : {
                    DataMerger<short> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
        case 32 : {
                    DataMerger<int> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
        case 64 : {
                    DataMerger<int64_t> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
      }
    }
  } else {
    if (bIsFloatG) {
      // unsigned float ??? :-)
      T_ERROR("Don't know how to handle unsigned float data.");
      return false;
    } else {
      switch (iComponentSizeG) {
        case 8  : {
                    DataMerger<unsigned char> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
        case 16 : {
                    DataMerger<unsigned short> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
        case 32 : {
                    DataMerger<unsigned int> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
        case 64 : {
                    DataMerger<UINT64> d(vIntermediateFiles, strMergedFile, vVolumeSizeG.volume()*iComponentCountG, MCtlr, bUseMaxMode);
                    bIsMerged = d.IsOK();
                    break;
                  }
      }
    }
  }


  for (size_t i = 0;i<vIntermediateFiles.size();i++) {
    if (vIntermediateFiles[i].bDelete && SysTools::FileExists(vIntermediateFiles[i].strFilename))
      remove(vIntermediateFiles[i].strFilename.c_str());
  }
  if (!bIsMerged) return false;


  // convert that single RAW file to the target data
  string strExtTarget = SysTools::ToUpperCase(SysTools::GetExt(strTargetFilename));
  bool bTargetCreated = false;
  if (strExtTarget == "UVF") {
    bTargetCreated = RAWConverter::ConvertRAWDataset(strMergedFile, strTargetFilename, strTempDir, 0,
                                       iComponentSizeG, iComponentCountG, bConvertEndianessG, bSignedG,
                                       bIsFloatG, vVolumeSizeG, vVolumeAspectG, strTitleG, SysTools::GetFilename(strMergedFile));

  } else {
    for (size_t k = 0;k<m_vpConverters.size();k++) {
      const std::vector<std::string>& vStrSupportedExtTarget = m_vpConverters[k]->SupportedExt();
      for (size_t l = 0;l<vStrSupportedExtTarget.size();l++) {
        if (vStrSupportedExtTarget[l] == strExtTarget) {
          bTargetCreated = m_vpConverters[k]->ConvertToNative(strMergedFile, strTargetFilename, 0,
                                                              iComponentSizeG, iComponentCountG, bSignedG, bIsFloatG,
                                                              vVolumeSizeG, vVolumeAspectG, bNoUserInteraction);
          if (bTargetCreated) break;
        }
      }
      if (bTargetCreated) break;
    }
  }
  remove(strMergedFile.c_str());
  return bTargetCreated;

}


bool IOManager::ConvertDataset(const std::string& strFilename,
                               const std::string& strTargetFilename,
                               const std::string& strTempDir,
                               bool bNoUserInteraction) {


  MESSAGE("Request to convert dataset %s to %s received.",
          strFilename.c_str(), strTargetFilename.c_str());

  string strExt = SysTools::ToUpperCase(SysTools::GetExt(strFilename));
  string strExtTarget = SysTools::ToUpperCase(SysTools::GetExt(strTargetFilename));

  MESSAGE("ext: %s, target: %s", strExt.c_str(), strExtTarget.c_str());


  if (strExtTarget == "UVF") {
    for (size_t i = 0;i<m_vpConverters.size();i++) {
      const std::vector<std::string>& vStrSupportedExt = m_vpConverters[i]->SupportedExt();
      for (size_t j = 0;j<vStrSupportedExt.size();j++) {
        MESSAGE("%s supports %s", m_vpConverters[i]->GetDesc().c_str(),
                vStrSupportedExt[j].c_str());
        if (vStrSupportedExt[j] == strExt) {
          if (m_vpConverters[i]->ConvertToUVF(strFilename, strTargetFilename, strTempDir, bNoUserInteraction)) return true;
        }
      }
    }

    if (m_pFinalConverter)
      return m_pFinalConverter->ConvertToUVF(strFilename, strTargetFilename, strTempDir, bNoUserInteraction);
    else
      return false;
  } else {
    UINT64        iHeaderSkip=0;
    UINT64        iComponentSize=0;
    UINT64        iComponentCount=0;
    bool          bConvertEndianess=false;
    bool          bSigned=false;
    bool          bIsFloat=false;
    UINTVECTOR3   vVolumeSize(0,0,0);
    FLOATVECTOR3  vVolumeAspect(0,0,0);
    string        strTitle = "";
    string        strSource = "";
    UVFTables::ElementSemanticTable eType = UVFTables::ES_UNDEFINED;
    string        strIntermediateFile = "";
    bool          bDeleteIntermediateFile = false;

    bool bRAWCreated = false;

    if (strExt == "UVF") {
      UVFDataset v(strFilename,false);
      if (!v.IsOpen()) return false;

      UINT64 iLODLevel = 0; // always extract the highest quality here

      iHeaderSkip = 0;
      iComponentSize = v.GetInfo().GetBitWidth();
      iComponentCount = v.GetInfo().GetComponentCount();
      bConvertEndianess = !v.GetInfo().IsSameEndianness();
      bSigned = v.GetInfo().GetIsSigned();
      bIsFloat = v.GetInfo().GetIsFloat();
      vVolumeSize = UINTVECTOR3(v.GetInfo().GetDomainSize(iLODLevel));
      vVolumeAspect = FLOATVECTOR3(v.GetInfo().GetScale());
      eType             = UVFTables::ES_UNDEFINED;  /// \todo grab this data from the UVF file
      strTitle          = "UVF data";               /// \todo grab this data from the UVF file
      strSource         = SysTools::GetFilename(strFilename);

      strIntermediateFile = strTempDir + strFilename +".raw";
      bDeleteIntermediateFile = true;

      if (!v.Export(iLODLevel, strIntermediateFile, false)) {
        if (SysTools::FileExists(strIntermediateFile)) {
          RAWConverter::Remove(strIntermediateFile.c_str(),
                           Controller::Debug::Out());
        }
        return false;
      } else bRAWCreated = true;

    } else {
      for (size_t i = 0;i<m_vpConverters.size();i++) {
        const std::vector<std::string>& vStrSupportedExt = m_vpConverters[i]->SupportedExt();
        for (size_t j = 0;j<vStrSupportedExt.size();j++) {
          if (vStrSupportedExt[j] == strExt) {
            bRAWCreated = m_vpConverters[i]->ConvertToRAW(strFilename, strTempDir, bNoUserInteraction,
                                            iHeaderSkip, iComponentSize, iComponentCount, bConvertEndianess, bSigned, bIsFloat, vVolumeSize, vVolumeAspect,
                                            strTitle, eType, strIntermediateFile, bDeleteIntermediateFile);
            if (bRAWCreated) break;
          }
        }
        if (bRAWCreated) break;
      }

      if (!bRAWCreated && m_pFinalConverter) {
        bRAWCreated = m_pFinalConverter->ConvertToRAW(strFilename, strTempDir, bNoUserInteraction,
                                        iHeaderSkip, iComponentSize, iComponentCount, bConvertEndianess, bSigned, bIsFloat, vVolumeSize, vVolumeAspect,
                                        strTitle, eType, strIntermediateFile, bDeleteIntermediateFile);
      }
    }
    if (!bRAWCreated) return false;

    bool bTargetCreated = false;
    for (size_t k = 0;k<m_vpConverters.size();k++) {
      const std::vector<std::string>& vStrSupportedExtTarget = m_vpConverters[k]->SupportedExt();
      for (size_t l = 0;l<vStrSupportedExtTarget.size();l++) {
        if (vStrSupportedExtTarget[l] == strExtTarget) {
          bTargetCreated =
            m_vpConverters[k]->ConvertToNative(strIntermediateFile,
                                               strTargetFilename,
                                               iHeaderSkip, iComponentSize,
                                               iComponentCount, bSigned,
                                               bIsFloat, vVolumeSize,
                                               vVolumeAspect,
                                               bNoUserInteraction);
          if (bTargetCreated) break;
        }
      }
      if (bTargetCreated) break;
    }
    if (bDeleteIntermediateFile) remove(strIntermediateFile.c_str());
    if (bTargetCreated) return true;
  }
  return false;
}

UVFDataset* IOManager::ConvertDataset(FileStackInfo* pStack, const std::string& strTargetFilename, const std::string& strTempDir, AbstrRenderer* requester) {
  if (!ConvertDataset(pStack, strTargetFilename, strTempDir)) return NULL;
  return dynamic_cast<UVFDataset*>(LoadDataset(strTargetFilename, requester));
}

UVFDataset* IOManager::ConvertDataset(const std::string& strFilename, const std::string& strTargetFilename, const std::string& strTempDir, AbstrRenderer* requester) {
  if (!ConvertDataset(strFilename, strTargetFilename, strTempDir)) return NULL;
  return dynamic_cast<UVFDataset*>(LoadDataset(strTargetFilename, requester));
}

Dataset* IOManager::LoadDataset(const std::string& strFilename,
                                AbstrRenderer* requester) {
  return Controller::Instance().MemMan()->LoadDataset(strFilename, requester);
}

bool MCBrick(LargeRAWFile* pSourceFile, const std::vector<UINT64> vBrickSize, const std::vector<UINT64> vBrickOffset, void* pUserContext ) {
    MCData* pMCData = (MCData*)pUserContext;
    return pMCData->PerformMC(pSourceFile, vBrickSize, vBrickOffset);
}

bool IOManager::ExtractIsosurface(const UVFDataset* pSourceData, UINT64 iLODlevel, double fIsovalue, const DOUBLEVECTOR3& vfRescaleFactors, const std::string& strTargetFilename, const std::string& strTempDir) {
  if (pSourceData->GetInfo().GetComponentCount() != 1) {
    T_ERROR("Isosurface extraction only supported for scalar volumes.");
    return false;
  }

  string strTempFilename = strTempDir + SysTools::GetFilename(strTargetFilename)+".tmp_raw";
  MCData* pMCData = NULL;

  bool   bFloatingPoint  = pSourceData->GetInfo().GetIsFloat();
  bool   bSigned         = pSourceData->GetInfo().GetIsSigned();
  UINT64  iComponentSize = pSourceData->GetInfo().GetBitWidth();
  FLOATVECTOR3 vScale    = FLOATVECTOR3(pSourceData->GetInfo().GetScale() * vfRescaleFactors);

  if (bFloatingPoint) {
    if (bSigned) {
      switch (iComponentSize) {
        case 32 : pMCData = new MCDataTemplate<float>(strTargetFilename, float(fIsovalue), vScale); break;
        case 64 : pMCData = new MCDataTemplate<double>(strTargetFilename, double(fIsovalue), vScale); break;
      }
    }
  } else {
    if (bSigned) {
      switch (iComponentSize) {
        case  8 : pMCData = new MCDataTemplate<char>(strTargetFilename, char(fIsovalue), vScale); break;
        case 16 : pMCData = new MCDataTemplate<short>(strTargetFilename, short(fIsovalue), vScale); break;
        case 32 : pMCData = new MCDataTemplate<int>(strTargetFilename, int(fIsovalue), vScale); break;
        case 64 : pMCData = new MCDataTemplate<int64_t>(strTargetFilename, int64_t(fIsovalue), vScale); break;
      }
    } else {
      switch (iComponentSize) {
        case  8 : pMCData = new MCDataTemplate<unsigned char>(strTargetFilename, (unsigned char)(fIsovalue), vScale); break;
        case 16 : pMCData = new MCDataTemplate<unsigned short>(strTargetFilename, (unsigned short)(fIsovalue), vScale); break;
        case 32 : pMCData = new MCDataTemplate<UINT32>(strTargetFilename, UINT32(fIsovalue), vScale); break;
        case 64 : pMCData = new MCDataTemplate<UINT64>(strTargetFilename, UINT64(fIsovalue), vScale); break;
      }
    }
  }

  if (!pMCData) {
    T_ERROR("Unsupported data format.");
    return false;
  }

  bool bResult = pSourceData->Export(iLODlevel, strTempFilename, false, &MCBrick, (void*)pMCData, 1);

  if (SysTools::FileExists(strTempFilename)) remove (strTempFilename.c_str());
  delete pMCData;

  if (bResult)
    return true;
  else {
    remove (strTargetFilename.c_str());
    T_ERROR("Export call failed.");
    return false;
  }
}


bool IOManager::ExportDataset(const UVFDataset* pSourceData, UINT64 iLODlevel, const std::string& strTargetFilename, const std::string& strTempDir) {
  // find the right converter to handle the output
  string strExt = SysTools::ToUpperCase(SysTools::GetExt(strTargetFilename));
  AbstrConverter* pExporter = NULL;
  for (size_t i = 0;i<m_vpConverters.size();i++) {
    const std::vector<std::string>& vStrSupportedExt = m_vpConverters[i]->SupportedExt();
    for (size_t j = 0;j<vStrSupportedExt.size();j++) {
      if (vStrSupportedExt[j] == strExt) {
        pExporter = m_vpConverters[i];
        break;
      }
    }
    if (pExporter) break;
  }

  if (!pExporter) {
    T_ERROR("Unknown file extension %s.", strExt.c_str());
    return false;
  }

  string strTempFilename = strTempDir + SysTools::GetFilename(strTargetFilename)+".tmp_raw";
  bool bRAWCreated = pSourceData->Export(iLODlevel, strTempFilename, false);

  if (!bRAWCreated) {
    T_ERROR("Unable to write temp file %s", strTempFilename.c_str());
    return false;
  }

  MESSAGE("Writing Target Dataset");

  bool bTargetCreated = pExporter->ConvertToNative(
                                strTempFilename, strTargetFilename, 0,
                                pSourceData->GetInfo().GetBitWidth(),
                                pSourceData->GetInfo().GetComponentCount(),
                                pSourceData->GetInfo().GetIsSigned(),
                                pSourceData->GetInfo().GetIsFloat(),
                                UINTVECTOR3(pSourceData->GetInfo().GetDomainSize(iLODlevel)),
                                FLOATVECTOR3(pSourceData->GetInfo().GetRescaleFactors()),
                                false);

  remove(strTempFilename.c_str());

  if (!bTargetCreated) {
    T_ERROR("Unable to write target file %s", strTargetFilename.c_str());
    return false;
  }

  MESSAGE("Done!");

  return bTargetCreated;
}


bool IOManager::NeedsConversion(const std::string& strFilename,
                                bool& bChecksumFail) const {
  wstring wstrFilename(strFilename.begin(), strFilename.end());
  return !UVF::IsUVFFile(wstrFilename, bChecksumFail);
}

bool IOManager::NeedsConversion(const std::string& strFilename) const {
  wstring wstrFilename(strFilename.begin(), strFilename.end());
  return !UVF::IsUVFFile(wstrFilename);
}


std::string IOManager::GetLoadDialogString() const {
  string strDialog = "All known Files (*.uvf ";
  map<string,string> descPairs;

  // first create the show all text entry
  for (size_t i = 0;i<m_vpConverters.size();i++) {
    for (size_t j = 0;j<m_vpConverters[i]->SupportedExt().size();j++) {
      string strExt = SysTools::ToLowerCase(m_vpConverters[i]->SupportedExt()[j]);
      if (descPairs.count(strExt) == 0) {
        strDialog = strDialog + "*." + strExt + " ";
        descPairs[strExt] = m_vpConverters[i]->GetDesc();
      }
    }
  }
  strDialog += ");;Universal Volume Format (*.uvf);;";

  // seperate entries
  for (size_t i = 0;i<m_vpConverters.size();i++) {
    strDialog += m_vpConverters[i]->GetDesc() + " (";
    for (size_t j = 0;j<m_vpConverters[i]->SupportedExt().size();j++) {
      string strExt = SysTools::ToLowerCase(m_vpConverters[i]->SupportedExt()[j]);
      strDialog += "*." + strExt;
      if (j<m_vpConverters[i]->SupportedExt().size()-1)
        strDialog += " ";
    }
    strDialog += ");;";
  }

  strDialog += "All Files (*.*)";

  return strDialog;
}

std::string IOManager::GetExportDialogString() const {
  std::string strDialog;
  // seperate entries
  for (size_t i = 0;i<m_vpConverters.size();i++) {
    for (size_t j = 0;j<m_vpConverters[i]->SupportedExt().size();j++) {
      if (m_vpConverters[i]->CanExportData()) {
        string strExt = SysTools::ToLowerCase(m_vpConverters[i]->SupportedExt()[j]);
        strDialog += m_vpConverters[i]->GetDesc() + " (*." + strExt + ");;";
      }
    }
  }

  return strDialog;
}



bool IOManager::AnalyzeDataset(const std::string& strFilename, RangeInfo& info, const std::string& strTempDir) {
  // find the right converter to handle the dataset
  string strExt = SysTools::ToUpperCase(SysTools::GetExt(strFilename));

  if (strExt == "UVF") {
    UVFDataset v(strFilename,false);
    if (!v.IsOpen()) return false;

    UINT64 iComponentCount = v.GetInfo().GetComponentCount();
    bool bSigned = v.GetInfo().GetIsSigned();
    bool bIsFloat = v.GetInfo().GetIsFloat();

    if (iComponentCount != 1) return false;  // only scalar data supported at the moment
    const Histogram1D& pHist = v.Get1DHistogram();

    // as our UVFs are always quantized to either 8bit or 16bit right now only the
    // nofloat + unsigned path is taken, the others are for future extensions
    if (bIsFloat) {
      info.m_iValueType = 0;
      info.m_fRange.first = 0.0;
      info.m_fRange.second = double(pHist.GetFilledSize()-1);
    } else {
      if (bSigned) {
        info.m_iValueType = 1;
        info.m_iRange.first = 0;
        info.m_iRange.second = int64_t(pHist.GetFilledSize()-1);
      } else {
        info.m_iValueType = 2;
        info.m_uiRange.first = 0;
        info.m_uiRange.second = UINT64(pHist.GetFilledSize()-1);
      }
    }

    info.m_vAspect = FLOATVECTOR3(v.GetInfo().GetScale());
    info.m_vDomainSize = UINTVECTOR3(v.GetInfo().GetDomainSize());
    info.m_iComponentSize = v.GetInfo().GetBitWidth();

    return true;
  } else {
    bool bAnalyzed = false;
    for (size_t i = 0;i<m_vpConverters.size();i++) {
      const std::vector<std::string>& vStrSupportedExt = m_vpConverters[i]->SupportedExt();
      for (size_t j = 0;j<vStrSupportedExt.size();j++) {
        if (vStrSupportedExt[j] == strExt) {
          bAnalyzed = m_vpConverters[i]->Analyze(strFilename, strTempDir, false, info);
          if (bAnalyzed) break;
        }
      }
      if (bAnalyzed) break;
    }

    if (!bAnalyzed && m_pFinalConverter) {
      bAnalyzed = m_pFinalConverter->Analyze(strFilename, strTempDir, false, info);
    }

    return bAnalyzed;
  }
}
