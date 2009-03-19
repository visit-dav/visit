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
  \file    GPUMemManDataStructs.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/

#pragma once

#ifndef GPUMEMMANDATASTRUCTS_H
#define GPUMEMMANDATASTRUCTS_H

#include <deque>
#include <string>
#include "../AbstrRenderer.h"
#include "../GLTexture1D.h"
#include "../GLTexture2D.h"
#include "../GLTexture3D.h"
#include "../GLFBOTex.h"
#include "../GLSLProgram.h"
#include "../../IO/VolumeDataset.h"
#include "../../IO/TransferFunction1D.h"
#include "../../IO/TransferFunction2D.h"

typedef std::deque< AbstrRenderer* > AbstrRendererList;
typedef AbstrRendererList::iterator AbstrRendererListIter;

// volume datasets
class VolDataListElem {
public:
  VolDataListElem(VolumeDataset* _pVolumeDataset, AbstrRenderer* pUser) :
    pVolumeDataset(_pVolumeDataset) 
  {
    qpUser.push_back(pUser);
  }

  VolumeDataset*  pVolumeDataset;
  AbstrRendererList qpUser;
};
typedef std::deque<VolDataListElem> VolDataList;
typedef VolDataList::iterator VolDataListIter;

// simple textures
class SimpleTextureListElem {
public:
  SimpleTextureListElem(unsigned int _iAccessCounter, GLTexture2D* _pTexture, std::string _strFilename) :
    iAccessCounter(_iAccessCounter), 
    pTexture(_pTexture), 
    strFilename(_strFilename)
  {}

  unsigned int  iAccessCounter;
  GLTexture2D*  pTexture;
  std::string    strFilename;
};
typedef std::deque<SimpleTextureListElem> SimpleTextureList;
typedef SimpleTextureList::iterator SimpleTextureListIter;

// 1D transfer functions
class Trans1DListElem {
public:
  Trans1DListElem(TransferFunction1D* _pTransferFunction1D, GLTexture1D* _pTexture, AbstrRenderer* pUser) :
    pTransferFunction1D(_pTransferFunction1D),
    pTexture(_pTexture)
  {
    qpUser.push_back(pUser);
  }

  TransferFunction1D*  pTransferFunction1D;
  GLTexture1D*    pTexture;
  AbstrRendererList  qpUser;
};
typedef std::deque<Trans1DListElem> Trans1DList;
typedef Trans1DList::iterator Trans1DListIter;

// 2D transfer functions
class Trans2DListElem {
public:
  Trans2DListElem(TransferFunction2D* _pTransferFunction2D, GLTexture2D* _pTexture, AbstrRenderer* pUser) :
    pTransferFunction2D(_pTransferFunction2D),
    pTexture(_pTexture)
  {
    qpUser.push_back(pUser);
  }

  TransferFunction2D*  pTransferFunction2D;
  GLTexture2D*    pTexture;
  AbstrRendererList  qpUser;
};
typedef std::deque<Trans2DListElem> Trans2DList;
typedef Trans2DList::iterator Trans2DListIter;

// 3D textures
class Texture3DListElem {
public:
  Texture3DListElem(VolumeDataset* _pDataset, const std::vector<UINT64>& _vLOD, const std::vector<UINT64>& _vBrick, bool bIsPaddedToPowerOfTwo, UINT64 iIntraFrameCounter, UINT64 iFrameCounter);
  ~Texture3DListElem();
  bool Equals(VolumeDataset* _pDataset, const std::vector<UINT64>& _vLOD, const std::vector<UINT64>& _vBrick, bool bIsPaddedToPowerOfTwo);
  bool Replace(VolumeDataset* _pDataset, const std::vector<UINT64>& _vLOD, const std::vector<UINT64>& _vBrick, bool bIsPaddedToPowerOfTwo, UINT64 iIntraFrameCounter, UINT64 iFrameCounter);
  bool BestMatch(const std::vector<UINT64>& vDimension, bool bIsPaddedToPowerOfTwo, UINT64& iIntraFrameCounter, UINT64& iFrameCounter);
  GLTexture3D* Access(UINT64& iIntraFrameCounter, UINT64& iFrameCounter);

  bool LoadData();
  void  FreeData();
  bool CreateTexture(bool bDeleteOldTexture=true);
  void  FreeTexture();

  unsigned char*      pData;
  GLTexture3D*        pTexture;
  VolumeDataset*      pDataset;
  unsigned int        iUserCount;

  UINT64 GetIntraFrameCounter() {return m_iIntraFrameCounter;}
  UINT64 GetFrameCounter() {return m_iFrameCounter;}

private:
  bool Match(const std::vector<UINT64>& vDimension);

  UINT64 m_iIntraFrameCounter;
  UINT64 m_iFrameCounter;

  std::vector<UINT64> vLOD;
  std::vector<UINT64> vBrick;
  bool m_bIsPaddedToPowerOfTwo;

};
typedef std::deque<Texture3DListElem*> Texture3DList;
typedef Texture3DList::iterator Texture3DListIter;

// framebuffer objects
class FBOListElem {
public:
  FBOListElem(GLFBOTex* _pFBOTex) : pFBOTex(_pFBOTex)
  {}
  FBOListElem(MasterController* pMasterController, GLenum minfilter, GLenum magfilter, GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat, unsigned int iSizePerElement, bool bHaveDepth, int iNumBuffers) :
    pFBOTex(new GLFBOTex(pMasterController, minfilter, magfilter, wrapmode, width, height, intformat, iSizePerElement, bHaveDepth, iNumBuffers))
  {}

  ~FBOListElem()
  {
    delete pFBOTex;
  }

  GLFBOTex* pFBOTex;
};
typedef std::deque<FBOListElem*> FBOList;
typedef FBOList::iterator FBOListIter;


// framebuffer objects
class GLSLListElem {
public:
  GLSLListElem(MasterController* pMasterController, const std::string& _strVSFile, const std::string& _strFSFile) :
    strVSFile(_strVSFile), strFSFile(_strFSFile),
    iAccessCounter(1),
    pGLSLProgram(new GLSLProgram(pMasterController, _strVSFile.c_str(), _strFSFile.c_str()))
  {
    if (!pGLSLProgram->IsValid()) {
      delete pGLSLProgram;
      pGLSLProgram = NULL;
    }
  }

  ~GLSLListElem()
  {
    delete pGLSLProgram;
  }

  std::string strVSFile; 
  std::string strFSFile;
  unsigned int iAccessCounter;
  GLSLProgram* pGLSLProgram;
};
typedef std::deque<GLSLListElem*> GLSLList;
typedef GLSLList::iterator GLSLListIter;

#endif // GPUMEMMANDATASTRUCTS_H
