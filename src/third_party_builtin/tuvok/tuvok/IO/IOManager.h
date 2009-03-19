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
  \file    IOManager.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \version  1.0
  \date    August 2008
*/


#pragma once

#ifndef IOMANAGER_H
#define IOMANAGER_H

#include <string>
#include "../Renderer/AbstrRenderer.h"
#include "../IO/DirectoryParser.h"
#include "../IO/UVF/UVF.h"

#define BRICKSIZE 256
#define BRICKOVERLAP 4

class MasterController;

class IOManager {
public:
  IOManager(MasterController* masterController);
  ~IOManager();

  std::vector<FileStackInfo*> ScanDirectory(std::string strDirectory);
  bool ConvertDataset(FileStackInfo* pStack, const std::string& strTargetFilename);
  bool ConvertDataset(const std::string& strFilename, const std::string& strTargetFilename);
  VolumeDataset* ConvertDataset(FileStackInfo* pStack, const std::string& strTargetFilename, AbstrRenderer* requester);
  VolumeDataset* ConvertDataset(const std::string& strFilename, const std::string& strTargetFilename, AbstrRenderer* requester);
  VolumeDataset* LoadDataset(const std::string& strFilename, AbstrRenderer* requester);
  bool NeedsConversion(const std::string& strFilename, bool& bChecksumFail);
  bool NeedsConversion(const std::string& strFilename);

private:  
  MasterController* m_pMasterController;

  bool ConvertNHDRDataset(const std::string& strFilename, const std::string& strTargetFilename);
  bool ConvertDATDataset(const std::string& strFilename, const std::string& strTargetFilename);
  bool ConvertRAWDataset(const std::string& strFilename, const std::string& strTargetFilename, UINT64 iHeaderSkip,
				                 UINT64 iComponentSize, UINT64 iComponentCount, bool bConvertEndianness,
				                 bool bSigned, UINTVECTOR3 vVolumeSize,FLOATVECTOR3 vVolumeAspect, 
                         std::string strDesc, std::string strSource="", UVFTables::ElementSemanticTable eType=UVFTables::ES_UNDEFINED);


  const std::string QuantizeShortTo12Bits(UINT64 iHeaderSkip, const std::string& strFilename, const std::string& strTargetFilename, size_t iSize);
  const std::string QuantizeFloatTo12Bits(UINT64 iHeaderSkip, const std::string& strFilename, const std::string& strTargetFilename, size_t iSize);

};

#endif // IOMANAGER_H
