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
  \file    GLFBOTex.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#pragma once

#ifndef GPUMEMMAN_H
#define GPUMEMMAN_H

#include "../../StdTuvokDefines.h"
#include "GPUMemManDataStructs.h"

class MasterController;
class SystemInfo;

class GPUMemMan {
  public:
    GPUMemMan(MasterController* masterController);
    virtual ~GPUMemMan();

    VolumeDataset* LoadDataset(const std::string& strFilename,
                               AbstrRenderer* requester);
    void FreeDataset(VolumeDataset* pVolumeDataset, AbstrRenderer* requester);

    void Changed1DTrans(AbstrRenderer* requester,
                        TransferFunction1D* pTransferFunction1D);
    void GetEmpty1DTrans(size_t iSize, AbstrRenderer* requester,
                         TransferFunction1D** ppTransferFunction1D,
                         GLTexture1D** tex);
    void Get1DTransFromFile(const std::string& strFilename,
                            AbstrRenderer* requester,
                            TransferFunction1D** ppTransferFunction1D,
                            GLTexture1D** tex);
    GLTexture1D* Access1DTrans(TransferFunction1D* pTransferFunction1D,
                               AbstrRenderer* requester);
    void Free1DTrans(TransferFunction1D* pTransferFunction1D,
                     AbstrRenderer* requester);

    void Changed2DTrans(AbstrRenderer* requester,
                        TransferFunction2D* pTransferFunction2D);
    void GetEmpty2DTrans(const VECTOR2<size_t>& iSize,
                         AbstrRenderer* requester,
                         TransferFunction2D** ppTransferFunction2D,
                         GLTexture2D** tex);
    void Get2DTransFromFile(const std::string& strFilename,
                            AbstrRenderer* requester,
                            TransferFunction2D** ppTransferFunction2D,
                            GLTexture2D** tex);
    GLTexture2D* Access2DTrans(TransferFunction2D* pTransferFunction2D,
                               AbstrRenderer* requester);
    void Free2DTrans(TransferFunction2D* pTransferFunction2D,
                     AbstrRenderer* requester);

    GLTexture2D* Load2DTextureFromFile(const std::string& strFilename);
    void FreeTexture(GLTexture2D* pTexture);

    GLTexture3D* Get3DTexture(VolumeDataset* pDataset,
                              const std::vector<UINT64>& vLOD,
                              const std::vector<UINT64>& vBrick,
                              bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits,
                              bool bDisableBorder, UINT64 iIntraFrameCounter,
                              UINT64 iFrameCounter);
    bool IsResident(const VolumeDataset* pDataset,
                    const std::vector<UINT64>& vLOD,
                    const std::vector<UINT64>& vBrick, bool bUseOnlyPowerOfTwo,
                    bool bDownSampleTo8Bits, bool bDisableBorder) const;

    void Release3DTexture(GLTexture3D* pTexture);

    GLFBOTex* GetFBO(GLenum minfilter, GLenum magfilter, GLenum wrapmode,
                     GLsizei width, GLsizei height, GLenum intformat,
                     UINT32 iSizePerElement, bool bHaveDepth=false,
                     int iNumBuffers=1);
    void FreeFBO(GLFBOTex* pFBO);

    GLSLProgram* GetGLSLProgram(const std::string& strVSFile,
                                const std::string& strFSFile);
    void FreeGLSLProgram(GLSLProgram* pGLSLProgram);

    void MemSizesChanged();

    // ok, i know this UINT64 could theoretically overflow but lets assume the
    // universe collapses before that happens
    // Seems likely. -- TJF
    UINT64 UpdateFrameCounter() {return ++m_iFrameCounter;}

    /// system statistics
    ///@{
    UINT64 GetCPUMem() const;
    UINT64 GetGPUMem() const;
    UINT64 GetAllocatedCPUMem() const {return m_iAllocatedCPUMemory;}
    UINT64 GetAllocatedGPUMem() const {return m_iAllocatedGPUMemory;}
    UINT32 GetBitWithMem() const;
    UINT32 GetNumCPUs() const;
    ///@}

  private:
    VolDataList       m_vpVolumeDatasets;
    SimpleTextureList m_vpSimpleTextures;
    Trans1DList       m_vpTrans1DList;
    Trans2DList       m_vpTrans2DList;
    Texture3DList     m_vpTex3DList;
    FBOList           m_vpFBOList;
    GLSLList          m_vpGLSLList;
    MasterController* m_MasterController;
    SystemInfo*       m_SystemInfo;

    UINT64            m_iAllocatedGPUMemory;
    UINT64            m_iAllocatedCPUMemory;
    UINT64            m_iFrameCounter;

    void FreeAssociatedTextures(VolumeDataset* pDataset);
    void Delete3DTexture(size_t iIndex);
};

#endif // GPUMEMMAN_H
