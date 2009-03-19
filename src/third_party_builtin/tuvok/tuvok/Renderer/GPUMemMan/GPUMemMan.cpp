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
  \file    GPUMemMan.h
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    August 2008
*/

#include <cassert>
#include <algorithm>
#include <functional>
#include "GPUMemMan.h"
#include <QtGui/QImage>
#include <QtOpenGL/QGLWidget>
#include <QtGui/QImageReader>
#include <Controller/Controller.h>
#include "../../Basics/SystemInfo.h"

using namespace std;

GPUMemMan::GPUMemMan(MasterController* masterController) :
  m_MasterController(masterController),
  m_SystemInfo(masterController->SysInfo()),
  m_iAllocatedGPUMemory(0),
  m_iAllocatedCPUMemory(0),
  m_iFrameCounter(0)
{

}

GPUMemMan::~GPUMemMan() {
  // Can't access the controller through the singleton; this destructor is
  // called during MC's destructor!  Since the MC is dying, we shouldn't rely
  // on anything within it being valid, but as a bit of a hack we'll grab the
  // active debug output anyway.  This works because we know that the debug
  // outputs will be deleted last -- after the memory manager.
  AbstrDebugOut &dbg = *(m_MasterController->DebugOut());
  for (VolDataListIter i = m_vpVolumeDatasets.begin();i<m_vpVolumeDatasets.end();i++) {
    dbg.Warning(_func_, "Detected unfreed dataset %s.",
                i->pVolumeDataset->Filename().c_str());
    delete i->pVolumeDataset;
  }

  for (SimpleTextureListIter i = m_vpSimpleTextures.begin();i<m_vpSimpleTextures.end();i++) {
    dbg.Warning(_func_, "Detected unfreed SimpleTexture %s.",
                i->strFilename.c_str());

    m_iAllocatedGPUMemory -= i->pTexture->GetCPUSize();
    m_iAllocatedCPUMemory -= i->pTexture->GetGPUSize();

    delete i->pTexture;
  }

  for (Trans1DListIter i = m_vpTrans1DList.begin();i<m_vpTrans1DList.end();i++) {
    dbg.Warning(_func_, "Detected unfreed 1D Transferfunction.");

    m_iAllocatedGPUMemory -= i->pTexture->GetCPUSize();
    m_iAllocatedCPUMemory -= i->pTexture->GetGPUSize();

    delete i->pTexture;
    delete i->pTransferFunction1D;
  }

  for (Trans2DListIter i = m_vpTrans2DList.begin();i<m_vpTrans2DList.end();i++) {
    dbg.Warning(_func_, "Detected unfreed 2D Transferfunction.");

    m_iAllocatedGPUMemory -= i->pTexture->GetCPUSize();
    m_iAllocatedCPUMemory -= i->pTexture->GetGPUSize();

    delete i->pTexture;
    delete i->pTransferFunction2D;
  }

  for (Texture3DListIter i = m_vpTex3DList.begin();i<m_vpTex3DList.end();i++) {
    dbg.Warning(_func_, "Detected unfreed 3D texture.");

    m_iAllocatedGPUMemory -= (*i)->pTexture->GetCPUSize();
    m_iAllocatedCPUMemory -= (*i)->pTexture->GetGPUSize();

    delete (*i);
  }

  for (FBOListIter i = m_vpFBOList.begin();i<m_vpFBOList.end();i++) {
    dbg.Warning(_func_, "Detected unfreed FBO.");

    m_iAllocatedGPUMemory -= (*i)->pFBOTex->GetCPUSize();
    m_iAllocatedCPUMemory -= (*i)->pFBOTex->GetGPUSize();

    delete (*i);
  }

  for (GLSLListIter i = m_vpGLSLList.begin();i<m_vpGLSLList.end();i++) {
    dbg.Warning(_func_, "Detected unfreed GLSL program.");

    m_iAllocatedGPUMemory -= (*i)->pGLSLProgram->GetCPUSize();
    m_iAllocatedCPUMemory -= (*i)->pGLSLProgram->GetGPUSize();

    delete (*i);
  }

  assert(m_iAllocatedGPUMemory == 0);
  assert(m_iAllocatedCPUMemory == 0);
}

// ******************** Datasets

VolumeDataset* GPUMemMan::LoadDataset(const string& strFilename, AbstrRenderer* requester) {
  for (VolDataListIter i = m_vpVolumeDatasets.begin();i<m_vpVolumeDatasets.end();i++) {
    if (i->pVolumeDataset->Filename() == strFilename) {
      MESSAGE("Reusing %s", strFilename.c_str());
      i->qpUser.push_back(requester);
      return i->pVolumeDataset;
    }
  }

  MESSAGE("Loading %s", strFilename.c_str());
  VolumeDataset* dataset = new VolumeDataset(strFilename, false);  // PRECONDITION: we assume that the file has been verified before
  if (dataset->IsOpen()) {
    m_vpVolumeDatasets.push_back(VolDataListElem(dataset, requester));
    return dataset;
  } else {
    T_ERROR("Error opening dataset %s", strFilename.c_str());
    return NULL;
  }
}

// Functor to find the VolDataListElem which holds the VolumeDataset given to
// the constructor.
struct find_ds : public std::unary_function<VolDataListElem, bool> {
  find_ds(const VolumeDataset *vds) : _ds(vds) { }
  bool operator()(const VolDataListElem &cmp) const {
    return cmp.pVolumeDataset == _ds;
  }
  private: const VolumeDataset *_ds;
};

void GPUMemMan::FreeDataset(VolumeDataset* pVolumeDataset,
                            AbstrRenderer* requester) {
  // store a name conditional for later logging
  const std::string ds_name = pVolumeDataset ? pVolumeDataset->Filename()
                                             : std::string("(null ds)");

  // find the dataset this refers to in our internal list
  VolDataListIter vol_ds = std::find_if(m_vpVolumeDatasets.begin(),
                                        m_vpVolumeDatasets.end(),
                                        find_ds(pVolumeDataset));

  // Don't access the singleton; see comment in the destructor.
  AbstrDebugOut &dbg = *(m_MasterController->DebugOut());
  if(vol_ds == m_vpVolumeDatasets.end()) {
    dbg.Warning(_func_,"Dataset '%s' not found or not being used by requester",
                ds_name.c_str());
    return;
  }

  // search for a renderer that the dataset is using
  AbstrRendererListIter renderer = std::find(vol_ds->qpUser.begin(),
                                             vol_ds->qpUser.end(), requester);
  // bail out if there doesn't appear to be a link between the DS and a
  // renderer.
  if(renderer == vol_ds->qpUser.end()) {
    dbg.Warning(_func_, "Dataset %s does not seem to be associated "
                        "with a renderer.", ds_name.c_str());
    return;
  }

  // remove it from the list of renderers which use this DS; if this brings the
  // `reference count' of the DS to 0, delete it.
  vol_ds->qpUser.erase(renderer);
  if(vol_ds->qpUser.empty()) {
    dbg.Message(_func_,"Cleaning up all 3D textures associated to dataset %s",
                ds_name.c_str());
    FreeAssociatedTextures(pVolumeDataset);
    dbg.Message(_func_,"Released Dataset %s", ds_name.c_str());
    delete pVolumeDataset;
    m_vpVolumeDatasets.erase(vol_ds);
  } else {
    dbg.Message(_func_,"Decreased access count but dataset %s is still "
                " in use by another subsystem", ds_name.c_str());
  }
}

// ******************** Simple Textures

GLTexture2D* GPUMemMan::Load2DTextureFromFile(const string& strFilename) {
  for (SimpleTextureListIter i = m_vpSimpleTextures.begin();i<m_vpSimpleTextures.end();i++) {
    if (i->strFilename == strFilename) {
      MESSAGE("Reusing %s", strFilename.c_str());
      i->iAccessCounter++;
      return i->pTexture;
    }
  }

  QImage image;
  if (!image.load(strFilename.c_str())) {
    T_ERROR("Unable to load file %s", strFilename.c_str());
    return NULL;
  }
  MESSAGE("Loaded %s, now creating OpenGL resources ..", strFilename.c_str());

  QImage glimage = QGLWidget::convertToGLFormat(image);

  GLTexture2D* tex = new GLTexture2D(glimage.width(),glimage.height(), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4, glimage.bits(), GL_LINEAR, GL_LINEAR);

  m_iAllocatedGPUMemory += tex->GetCPUSize();
  m_iAllocatedCPUMemory += tex->GetGPUSize();

  m_vpSimpleTextures.push_back(SimpleTextureListElem(1,tex,strFilename));
  return tex;
}


void GPUMemMan::FreeTexture(GLTexture2D* pTexture) {
  for (SimpleTextureListIter i = m_vpSimpleTextures.begin();i<m_vpSimpleTextures.end();i++) {
    if (i->pTexture == pTexture) {
      i->iAccessCounter--;
      if (i->iAccessCounter == 0) {
        MESSAGE("Deleted texture %s", i->strFilename.c_str());

        m_iAllocatedGPUMemory -= i->pTexture->GetCPUSize();
        m_iAllocatedCPUMemory -= i->pTexture->GetGPUSize();

        i->pTexture->Delete();
        m_vpSimpleTextures.erase(i);
      } else {
        MESSAGE("Decreased access count but the texture %s is still in use by another subsystem",i->strFilename.c_str());
      }
      return;
    }
  }
  WARNING("Texture not found");
}

// ******************** 1D Trans

void GPUMemMan::Changed1DTrans(AbstrRenderer* requester, TransferFunction1D* pTransferFunction1D) {
  MESSAGE("Sending change notification for 1D transfer function");

  pTransferFunction1D->ComputeNonZeroLimits();

  for (Trans1DListIter i = m_vpTrans1DList.begin();i<m_vpTrans1DList.end();i++) {
    if (i->pTransferFunction1D == pTransferFunction1D) {
      for (AbstrRendererListIter j = i->qpUser.begin();j<i->qpUser.end();j++) {
        if (*j != requester) (*j)->Changed1DTrans();
      }
    }
  }
}

void GPUMemMan::GetEmpty1DTrans(size_t iSize, AbstrRenderer* requester,
                                TransferFunction1D** ppTransferFunction1D,
                                GLTexture1D** tex) {
  MESSAGE("Creating new empty 1D transfer function");
  assert(iSize > 0); // if not, our TF would be *really* empty :)
  *ppTransferFunction1D = new TransferFunction1D(iSize);
  (*ppTransferFunction1D)->SetStdFunction();

  unsigned char* pcData = NULL;
  (*ppTransferFunction1D)->GetByteArray(&pcData);
  *tex = new GLTexture1D(UINT32((*ppTransferFunction1D)->GetSize()), GL_RGBA8,
                         GL_RGBA, GL_UNSIGNED_BYTE, 4, pcData);
  delete [] pcData;

  m_iAllocatedGPUMemory += (*tex)->GetCPUSize();
  m_iAllocatedCPUMemory += (*tex)->GetGPUSize();

  m_vpTrans1DList.push_back(Trans1DListElem(*ppTransferFunction1D, *tex,
                                            requester));
}

void GPUMemMan::Get1DTransFromFile(const string& strFilename, AbstrRenderer* requester, TransferFunction1D** ppTransferFunction1D, GLTexture1D** tex) {
  MESSAGE("Loading 1D transfer function from file");
  *ppTransferFunction1D = new TransferFunction1D(strFilename);

  unsigned char* pcData = NULL;
  (*ppTransferFunction1D)->GetByteArray(&pcData);
  *tex = new GLTexture1D(UINT32((*ppTransferFunction1D)->GetSize()), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,4,pcData);
  delete [] pcData;

  m_iAllocatedGPUMemory += (*tex)->GetCPUSize();
  m_iAllocatedCPUMemory += (*tex)->GetGPUSize();

  m_vpTrans1DList.push_back(Trans1DListElem(*ppTransferFunction1D, *tex, requester));
}

GLTexture1D* GPUMemMan::Access1DTrans(TransferFunction1D* pTransferFunction1D, AbstrRenderer* requester) {
  for (Trans1DListIter i = m_vpTrans1DList.begin();i<m_vpTrans1DList.end();i++) {
    if (i->pTransferFunction1D == pTransferFunction1D) {
      MESSAGE("Accessing 1D transferfunction");
      i->qpUser.push_back(requester);
      return i->pTexture;
    }
  }

  T_ERROR("Unable to find 1D transferfunction");
  return NULL;
}

void GPUMemMan::Free1DTrans(TransferFunction1D* pTransferFunction1D, AbstrRenderer* requester) {
  AbstrDebugOut &dbg = *(m_MasterController->DebugOut());
  for (Trans1DListIter i = m_vpTrans1DList.begin();i<m_vpTrans1DList.end();i++) {
    if (i->pTransferFunction1D == pTransferFunction1D) {
      for (AbstrRendererListIter j = i->qpUser.begin();j<i->qpUser.end();j++) {
        if (*j == requester) {
          i->qpUser.erase(j);
          if (i->qpUser.empty()) {
            dbg.Message(_func_, "Released 1D TF");

            m_iAllocatedGPUMemory -= i->pTexture->GetCPUSize();
            m_iAllocatedCPUMemory -= i->pTexture->GetGPUSize();

            delete i->pTransferFunction1D;
            i->pTexture->Delete();
            delete i->pTexture;
            m_vpTrans1DList.erase(i);
          } else {
            dbg.Message(_func_, "Decreased access count, but 1D TF is still "
                                "in use by another subsystem.");
          }
          return;
        }
      }
    }
  }
  dbg.Warning(_func_, "1D TF not found not in use by requester.");
}

// ******************** 2D Trans

void GPUMemMan::Changed2DTrans(AbstrRenderer* requester, TransferFunction2D* pTransferFunction2D) {
  MESSAGE("Sending change notification for 2D transfer function");

  pTransferFunction2D->InvalidateCache();

  pTransferFunction2D->ComputeNonZeroLimits();

  for (Trans2DListIter i = m_vpTrans2DList.begin();i<m_vpTrans2DList.end();i++) {
    if (i->pTransferFunction2D == pTransferFunction2D) {
      for (AbstrRendererListIter j = i->qpUser.begin();j<i->qpUser.end();j++) {
        if (*j != requester) (*j)->Changed2DTrans();
      }
    }
  }

}

void GPUMemMan::GetEmpty2DTrans(const VECTOR2<size_t>& iSize, AbstrRenderer* requester, TransferFunction2D** ppTransferFunction2D, GLTexture2D** tex) {
  MESSAGE("Creating new empty 2D transfer function");
  *ppTransferFunction2D = new TransferFunction2D(iSize);

  unsigned char* pcData = NULL;
  (*ppTransferFunction2D)->GetByteArray(&pcData);
  *tex = new GLTexture2D(UINT32(iSize.x), UINT32(iSize.y), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,4,pcData);
  delete [] pcData;

  m_iAllocatedGPUMemory += (*tex)->GetCPUSize();
  m_iAllocatedCPUMemory += (*tex)->GetGPUSize();

  m_vpTrans2DList.push_back(Trans2DListElem(*ppTransferFunction2D, *tex, requester));
}

void GPUMemMan::Get2DTransFromFile(const string& strFilename, AbstrRenderer* requester, TransferFunction2D** ppTransferFunction2D, GLTexture2D** tex) {
  MESSAGE("Loading 2D transfer function from file");
  *ppTransferFunction2D = new TransferFunction2D(strFilename);

  unsigned char* pcData = NULL;
  (*ppTransferFunction2D)->GetByteArray(&pcData);
  *tex = new GLTexture2D(UINT32((*ppTransferFunction2D)->GetSize().x), UINT32((*ppTransferFunction2D)->GetSize().y), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,4,pcData);
  delete [] pcData;

  m_iAllocatedGPUMemory += (*tex)->GetCPUSize();
  m_iAllocatedCPUMemory += (*tex)->GetGPUSize();

  m_vpTrans2DList.push_back(Trans2DListElem(*ppTransferFunction2D, *tex, requester));
}

GLTexture2D* GPUMemMan::Access2DTrans(TransferFunction2D* pTransferFunction2D, AbstrRenderer* requester) {
  for (Trans2DListIter i = m_vpTrans2DList.begin();i<m_vpTrans2DList.end();i++) {
    if (i->pTransferFunction2D == pTransferFunction2D) {
      MESSAGE("Accessing 2D transferfunction");
      i->qpUser.push_back(requester);
      return i->pTexture;
    }
  }

  T_ERROR("Unable to find 2D transferfunction");
  return NULL;
}

void GPUMemMan::Free2DTrans(TransferFunction2D* pTransferFunction2D, AbstrRenderer* requester) {
  AbstrDebugOut &dbg = *(m_MasterController->DebugOut());
  for (Trans2DListIter i = m_vpTrans2DList.begin();i<m_vpTrans2DList.end();i++) {
    if (i->pTransferFunction2D == pTransferFunction2D) {
      for (AbstrRendererListIter j = i->qpUser.begin();j<i->qpUser.end();j++) {
        if (*j == requester) {
          i->qpUser.erase(j);
          if (i->qpUser.empty()) {
            dbg.Message(_func_, "Released 2D TF");

            m_iAllocatedGPUMemory -= i->pTexture->GetCPUSize();
            m_iAllocatedCPUMemory -= i->pTexture->GetGPUSize();

            delete i->pTransferFunction2D;
            i->pTexture->Delete();
            delete i->pTexture;

            m_vpTrans2DList.erase(i);
          } else {
            dbg.Message(_func_, "Decreased access count, but 2D TF is still "
                                "in use by another subsystem.");
          }
          return;
        }
      }
    }
  }
  dbg.Warning(_func_, "2D TF not found or not in use by requester.");
}

// ******************** 3D Textures

bool GPUMemMan::IsResident(const VolumeDataset* pDataset,
                           const vector<UINT64>& vLOD,
                           const vector<UINT64>& vBrick,
                           bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits,
                           bool bDisableBorder) const {
  for(Texture3DListConstIter i = m_vpTex3DList.begin();
      i < m_vpTex3DList.end(); ++i) {
    if((*i)->Equals(pDataset, vLOD, vBrick, bUseOnlyPowerOfTwo,
                    bDownSampleTo8Bits, bDisableBorder)) {
      return true;
    }
  }
  return false;
}


GLTexture3D* GPUMemMan::Get3DTexture(VolumeDataset* pDataset,
                                     const vector<UINT64>& vLOD,
                                     const vector<UINT64>& vBrick,
                                     bool bUseOnlyPowerOfTwo,
                                     bool bDownSampleTo8Bits,
                                     bool bDisableBorder,
                                     UINT64 iIntraFrameCounter,
                                     UINT64 iFrameCounter) {
  for (Texture3DListIter i = m_vpTex3DList.begin();
       i < m_vpTex3DList.end(); i++) {
    if ((*i)->Equals(pDataset, vLOD, vBrick, bUseOnlyPowerOfTwo,
                     bDownSampleTo8Bits, bDisableBorder)) {
      MESSAGE("Reusing 3D texture");
      return (*i)->Access(iIntraFrameCounter, iFrameCounter);
    }
  }

  const vector<UINT64> vSize = pDataset->GetInfo()->GetBrickSizeND(vLOD, vBrick);
  UINT64 iBitWidth  = pDataset->GetInfo()->GetBitWidth();
  UINT64 iCompCount = pDataset->GetInfo()->GetComponentCount();

  UINT64 iNeededCPUMemory = iBitWidth * iCompCount * vSize[0];
  for (size_t i = 1;i<vSize.size();i++) iNeededCPUMemory *= vSize[i];

  // for OpenGL we ignore the GPU memory load and let GL do the paging
  if (m_iAllocatedCPUMemory + iNeededCPUMemory > m_SystemInfo->GetMaxUsableCPUMem()) {
    MESSAGE("Not enough memory for texture %i x %i x %i (%ibit * %i), paging ...",
            int(vSize[0]), int(vSize[1]), int(vSize[2]), int(iBitWidth),
            int(iCompCount));

    // search for best brick to replace with this brick
    UINT64 iTargetFrameCounter = UINT64_INVALID;
    UINT64 iTargetIntraFrameCounter = UINT64_INVALID;
    Texture3DListIter iBestMatch;
    for (Texture3DListIter i = m_vpTex3DList.begin();i<m_vpTex3DList.end();i++) {
      if ((*i)->BestMatch(vSize, bUseOnlyPowerOfTwo, bDownSampleTo8Bits,
                          bDisableBorder, iTargetIntraFrameCounter,
                          iTargetFrameCounter)) iBestMatch = i;
    }

    if (iTargetFrameCounter != UINT64_INVALID) {
      // found a suitable brick that can be replaced
      MESSAGE("  Found suitable target brick from frame %i with intraframe "
              " counter %i (current frame %i / current intraframe %i)",
              int(iTargetFrameCounter), int(iTargetIntraFrameCounter),
              int(iFrameCounter), int(iIntraFrameCounter));
      (*iBestMatch)->Replace(pDataset, vLOD, vBrick, bUseOnlyPowerOfTwo,
                             bDownSampleTo8Bits, bDisableBorder,
                             iIntraFrameCounter, iFrameCounter);
      (*iBestMatch)->iUserCount++;
      return (*iBestMatch)->pTexture;
    } else {
      MESSAGE("  No suitable brick found. Randomly deleting bricks until this"
              " brick fits into memory");

      // no suitable brick found -> randomly delete bricks until this brick fits into memory
      while (m_iAllocatedCPUMemory + iNeededCPUMemory > m_SystemInfo->GetMaxUsableCPUMem()) {

        if (m_vpTex3DList.empty()) {
          // we do not have enough memory to page in even a single block...
          T_ERROR("Not enough memory to page a brick into memory, "
                  "aborting (MaxMem=%ikb, NeededMem=%ikb).",
                  int(m_SystemInfo->GetMaxUsableCPUMem()/1024),
                  int(iNeededCPUMemory/1024));
          return NULL;
        }

        // theoretically this means that if we have more than MAX_RAND bricks
        // we always pick the first, but who cares ...
        size_t iIndex = size_t(rand()%m_vpTex3DList.size());

        if (m_vpTex3DList[iIndex]->iUserCount == 0) {
          MESSAGE("   Deleting texture %i", int(iIndex));
          Delete3DTexture(iIndex);
        }
      }
    }
  }

  MESSAGE("Creating new texture %i x %i x %i, bitsize=%i, componentcount=%i", int(vSize[0]), int(vSize[1]), int(vSize[2]), int(iBitWidth), int(iCompCount));

  Texture3DListElem* pNew3DTex = new Texture3DListElem(pDataset, vLOD, vBrick, bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder, iIntraFrameCounter, iFrameCounter, m_MasterController);

  if (pNew3DTex->pTexture == NULL) {
    T_ERROR("Failed to create OpenGL texture.");
    delete pNew3DTex;
    return NULL;
  }
  pNew3DTex->iUserCount = 1;

  m_iAllocatedGPUMemory += pNew3DTex->pTexture->GetCPUSize();
  m_iAllocatedCPUMemory += pNew3DTex->pTexture->GetGPUSize();

  m_vpTex3DList.push_back(pNew3DTex);
  return (*(m_vpTex3DList.end()-1))->pTexture;
}

void GPUMemMan::Release3DTexture(GLTexture3D* pTexture) {
  for (size_t i = 0;i<m_vpTex3DList.size();i++) {
    if (m_vpTex3DList[i]->pTexture == pTexture) {
      if (m_vpTex3DList[i]->iUserCount > 0) {
          m_vpTex3DList[i]->iUserCount--;
      } else {
        WARNING("Attempting to release a 3D texture that is not in use.");
      }
    }
  }
}

void GPUMemMan::Delete3DTexture(size_t iIndex) {

  m_iAllocatedGPUMemory -= m_vpTex3DList[iIndex]->pTexture->GetCPUSize();
  m_iAllocatedCPUMemory -= m_vpTex3DList[iIndex]->pTexture->GetGPUSize();

  if ( m_vpTex3DList[iIndex]->iUserCount != 0) {
    WARNING("Freeing used 3D texture!");
  }

  delete m_vpTex3DList[iIndex];

  m_vpTex3DList.erase(m_vpTex3DList.begin()+iIndex);
}

void GPUMemMan::FreeAssociatedTextures(VolumeDataset* pDataset) {
  for (size_t i = 0;i<m_vpTex3DList.size();i++) {
    if (m_vpTex3DList[i]->pDataset == pDataset) {

      MESSAGE("Deleting a 3D texture of size %i x %i x %i",
              m_vpTex3DList[i]->pTexture->GetSize().x,
              m_vpTex3DList[i]->pTexture->GetSize().y,
              m_vpTex3DList[i]->pTexture->GetSize().z);

      Delete3DTexture(i);
      i--;
    }
  }
}


void GPUMemMan::MemSizesChanged() {
  if (m_iAllocatedCPUMemory > m_SystemInfo->GetMaxUsableCPUMem()) {
      /// \todo CPU free resources to match max mem requirements
  }

  if (m_iAllocatedGPUMemory > m_SystemInfo->GetMaxUsableGPUMem()) {
      /// \todo GPU free resources to match max mem requirements
  }
}


GLFBOTex* GPUMemMan::GetFBO(GLenum minfilter, GLenum magfilter, GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat, UINT32 iSizePerElement, bool bHaveDepth, int iNumBuffers) {

  MESSAGE("Creating new FBO of size %i x %i", int(width), int(height));

  FBOListElem* e = new FBOListElem(m_MasterController,minfilter, magfilter, wrapmode, width, height, intformat, iSizePerElement, bHaveDepth, iNumBuffers);

  m_vpFBOList.push_back(e);

  m_iAllocatedGPUMemory += e->pFBOTex->GetCPUSize();
  m_iAllocatedCPUMemory += e->pFBOTex->GetGPUSize();

  return e->pFBOTex;
}

void GPUMemMan::FreeFBO(GLFBOTex* pFBO) {
  for (size_t i = 0;i<m_vpFBOList.size();i++) {
    if (m_vpFBOList[i]->pFBOTex == pFBO) {
      MESSAGE("Freeing FBO ");
      m_iAllocatedGPUMemory -= m_vpFBOList[i]->pFBOTex->GetCPUSize();
      m_iAllocatedCPUMemory -= m_vpFBOList[i]->pFBOTex->GetGPUSize();

      delete m_vpFBOList[i];

      m_vpFBOList.erase(m_vpFBOList.begin()+i);
      return;
    }
  }
  WARNING("FBO to free not found.");
}

GLSLProgram* GPUMemMan::GetGLSLProgram(const string& strVSFile, const string& strFSFile) {
  for (GLSLListIter i = m_vpGLSLList.begin();i<m_vpGLSLList.end();i++) {
    if ((*i)->strVSFile == strVSFile && (*i)->strFSFile == strFSFile) {
      MESSAGE("Reusing GLSL program from the VS %s and the FS %s", (*i)->strVSFile.c_str(), (*i)->strFSFile.c_str());
      (*i)->iAccessCounter++;
      return (*i)->pGLSLProgram;
    }
  }

  MESSAGE("Creating new GLSL program from the VS %s and the FS %s", strVSFile.c_str(), strFSFile.c_str());

  GLSLListElem* e = new GLSLListElem(m_MasterController, strVSFile, strFSFile);

  if (e->pGLSLProgram != NULL) {

    m_vpGLSLList.push_back(e);

    m_iAllocatedGPUMemory += e->pGLSLProgram->GetCPUSize();
    m_iAllocatedCPUMemory += e->pGLSLProgram->GetGPUSize();

    return e->pGLSLProgram;
  } else {
    T_ERROR("Failed to created GLSL program from the VS %s and the FS %s", strVSFile.c_str(), strFSFile.c_str());
    return NULL;
  }
}

void GPUMemMan::FreeGLSLProgram(GLSLProgram* pGLSLProgram) {
  if (pGLSLProgram == NULL) return;

  for (size_t i = 0;i<m_vpGLSLList.size();i++) {
    if (m_vpGLSLList[i]->pGLSLProgram == pGLSLProgram) {
      m_vpGLSLList[i]->iAccessCounter--;
      if (m_vpGLSLList[i]->iAccessCounter == 0) {

        MESSAGE("Freeing GLSL program");
        m_iAllocatedGPUMemory -= m_vpGLSLList[i]->pGLSLProgram->GetCPUSize();
        m_iAllocatedCPUMemory -= m_vpGLSLList[i]->pGLSLProgram->GetGPUSize();

        delete m_vpGLSLList[i];

        m_vpGLSLList.erase(m_vpGLSLList.begin()+i);
      } else MESSAGE("Decreased access counter but kept GLSL program in memory.");
      return;
    }
  }
  WARNING("GLSL program to free not found.");
}

UINT64 GPUMemMan::GetCPUMem() const {return m_SystemInfo->GetCPUMemSize();}
UINT64 GPUMemMan::GetGPUMem() const {return m_SystemInfo->GetGPUMemSize();}
UINT32 GPUMemMan::GetBitWithMem() const {
  return m_SystemInfo->GetProgrammBitWith();
}
UINT32 GPUMemMan::GetNumCPUs() const {return m_SystemInfo->GetNumberOfCPUs();}
