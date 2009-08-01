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
#include <numeric>
#include <typeinfo>
// normally we'd include Qt headers first, but we want to make sure to get GLEW
// before GL in this special case.
#include "GPUMemMan.h"
#ifndef TUVOK_NO_QT
# include <QtGui/QImage>
# include <QtOpenGL/QGLWidget>
#endif

#include "Basics/SystemInfo.h"
#include "Controller/Controller.h"
#include "GPUMemManDataStructs.h"
#include "IO/uvfDataset.h"
#include "IO/uvfMetadata.h"
#include "Renderer/AbstrRenderer.h"
#include "Renderer/GL/GLError.h"
#include "Renderer/GL/GLTexture1D.h"
#include "Renderer/GL/GLTexture2D.h"
#include "Renderer/GL/GLTexture3D.h"

using namespace std;
using namespace tuvok;

#include <IO/IOManager.h>

GPUMemMan::GPUMemMan(MasterController* masterController) :
  m_MasterController(masterController),
  m_SystemInfo(masterController->SysInfo()),
  m_iAllocatedGPUMemory(0),
  m_iAllocatedCPUMemory(0),
  m_iFrameCounter(0)
{
  m_vUploadHub.resize(INCORESIZE*4);
  m_iAllocatedCPUMemory = INCORESIZE*4;
}

GPUMemMan::~GPUMemMan() {
  // Can't access the controller through the singleton; this destructor is
  // called during MC's destructor!  Since the MC is dying, we shouldn't rely
  // on anything within it being valid, but as a bit of a hack we'll grab the
  // active debug output anyway.  This works because we know that the debug
  // outputs will be deleted last -- after the memory manager.
  AbstrDebugOut &dbg = *(m_MasterController->DebugOut());
  for (VolDataListIter i = m_vpVolumeDatasets.begin();i<m_vpVolumeDatasets.end();i++) {
    try {
      dbg.Warning(_func_, "Detected unfreed dataset %s.",
                  dynamic_cast<UVFDataset&>(*(i->pVolumeDataset)).Filename().c_str());
    } catch(std::bad_cast) {
      dbg.Warning(_func_, "Detected unfreed dataset %p.", i->pVolumeDataset);
    }
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

  m_vUploadHub.resize(0);
  m_iAllocatedCPUMemory -= INCORESIZE*4;

  assert(m_iAllocatedGPUMemory == 0);
  assert(m_iAllocatedCPUMemory == 0);
}

// ******************** Datasets

Dataset* GPUMemMan::LoadDataset(const string& strFilename,
                                AbstrRenderer* requester) {
  // We want to reuse datasets which have already been loaded.  Yet we have a
  // list of `Dataset's, not `UVFDataset's, and so therefore we can't rely on
  // each element of the list having a file backing it up.
  //
  // Yet they all will; this method is never going to get called for datasets
  // which are given from clients via an in-memory transfer.  Thus nothing is
  // ever going to get added to the list which isn't a UVFDataset.
  //
  // We could make the list be composed only of UVFDatasets.  Eventually we'd
  // like to do dataset caching for any client though, not just ImageVis3D.
  for (VolDataListIter i = m_vpVolumeDatasets.begin();i<m_vpVolumeDatasets.end();i++) {
    // Given the above, this cast is guaranteed to succeed.
    UVFDataset *dataset = dynamic_cast<UVFDataset*>(i->pVolumeDataset);
    assert(dataset != NULL);

    if (dataset->Filename() == strFilename) {
      MESSAGE("Reusing %s", strFilename.c_str());
      i->qpUser.push_back(requester);
      return i->pVolumeDataset;
    }
  }

  MESSAGE("Loading %s", strFilename.c_str());
  // we assume the file has already been verified
  UVFDataset* dataset = new UVFDataset(strFilename, false);

  if (dataset->IsOpen()) {
    m_vpVolumeDatasets.push_back(VolDataListElem(dataset, requester));
    return dataset;
  } else {
    T_ERROR("Error opening dataset %s", strFilename.c_str());
    return NULL;
  }
}

// Functor to find the VolDataListElem which holds the Dataset given in
// the constructor.
struct find_ds : public std::unary_function<VolDataListElem, bool> {
  find_ds(const Dataset* vds) : _ds(vds) { }
  bool operator()(const VolDataListElem &cmp) const {
    return cmp.pVolumeDataset == _ds;
  }
  private: const Dataset* _ds;
};

void GPUMemMan::FreeDataset(Dataset* pVolumeDataset,
                            AbstrRenderer* requester) {
  // store a name conditional for later logging
  std::string ds_name;
  try {
    const UVFDataset& ds = dynamic_cast<UVFDataset&>(*pVolumeDataset);
    ds_name = ds.Filename();
  } catch(std::bad_cast) {
    ds_name = "(unnamed dataset)";
  }

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
  // reference count of the DS to 0, delete it.
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

#ifndef TUVOK_NO_QT
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
#else
  T_ERROR("No Qt support!");
  return NULL;
#endif
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

  std::vector<unsigned char> vTFData;
  (*ppTransferFunction1D)->GetByteArray(vTFData);
  *tex = new GLTexture1D(UINT32((*ppTransferFunction1D)->GetSize()), GL_RGBA8,
                         GL_RGBA, GL_UNSIGNED_BYTE, 4, &vTFData.at(0));

  m_iAllocatedGPUMemory += (*tex)->GetCPUSize();
  m_iAllocatedCPUMemory += (*tex)->GetGPUSize();

  m_vpTrans1DList.push_back(Trans1DListElem(*ppTransferFunction1D, *tex,
                                            requester));
}

void GPUMemMan::Get1DTransFromFile(const string& strFilename, AbstrRenderer* requester, TransferFunction1D** ppTransferFunction1D, GLTexture1D** tex, size_t iSize) {
  MESSAGE("Loading 1D transfer function from file");
  *ppTransferFunction1D = new TransferFunction1D(strFilename);

  if (iSize != 0 && (*ppTransferFunction1D)->GetSize() != iSize) (*ppTransferFunction1D)->Resample(iSize);

  std::vector<unsigned char> vTFData;
  (*ppTransferFunction1D)->GetByteArray(vTFData);
  *tex = new GLTexture1D(UINT32((*ppTransferFunction1D)->GetSize()), GL_RGBA8,
                         GL_RGBA, GL_UNSIGNED_BYTE, 4, &vTFData.at(0));

  m_iAllocatedGPUMemory += (*tex)->GetCPUSize();
  m_iAllocatedCPUMemory += (*tex)->GetGPUSize();

  m_vpTrans1DList.push_back(Trans1DListElem(*ppTransferFunction1D, *tex, requester));
}

std::pair<TransferFunction1D*, GLTexture1D*>
GPUMemMan::SetExternal1DTrans(const std::vector<unsigned char>& rgba,
                              AbstrRenderer* requester)
{
  const size_t sz = rgba.size() / 4; // RGBA, i.e. 4 components.
  MESSAGE("Setting %u element 1D TF from external source.",
          static_cast<UINT32>(sz));
  assert(!rgba.empty());

  TransferFunction1D *tf1d = new TransferFunction1D(sz);
  tf1d->Set(rgba);

  GLTexture1D *tex = new GLTexture1D(static_cast<UINT32>(tf1d->GetSize()),
                                     GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4,
                                     &rgba.at(0));
  m_iAllocatedGPUMemory += tex->GetCPUSize();
  m_iAllocatedCPUMemory += tex->GetGPUSize();

  m_vpTrans1DList.push_back(Trans1DListElem(tf1d, tex, requester));

  return std::make_pair(tf1d, tex);
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

void GPUMemMan::Get2DTransFromFile(const string& strFilename, AbstrRenderer* requester, TransferFunction2D** ppTransferFunction2D, GLTexture2D** tex, const VECTOR2<size_t>& vSize) {
  MESSAGE("Loading 2D transfer function from file");
  *ppTransferFunction2D = new TransferFunction2D(strFilename);

  if ((vSize.x != 0 || vSize.y != 0) &&
      (*ppTransferFunction2D)->GetSize() != vSize) {
    (*ppTransferFunction2D)->Resample(vSize);
  }

  unsigned char* pcData = NULL;
  (*ppTransferFunction2D)->GetByteArray(&pcData);
  *tex = new GLTexture2D(UINT32((*ppTransferFunction2D)->GetSize().x),
                         UINT32((*ppTransferFunction2D)->GetSize().y),
                         GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,4,pcData);
  delete [] pcData;

  m_iAllocatedGPUMemory += (*tex)->GetCPUSize();
  m_iAllocatedCPUMemory += (*tex)->GetGPUSize();

  m_vpTrans2DList.push_back(Trans2DListElem(*ppTransferFunction2D, *tex,
                                            requester));
}

GLTexture2D* GPUMemMan::Access2DTrans(TransferFunction2D* pTransferFunction2D,
                                      AbstrRenderer* requester) {
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

void GPUMemMan::Free2DTrans(TransferFunction2D* pTransferFunction2D,
                            AbstrRenderer* requester) {
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
            dbg.Message(_func_, "Decreased access count, "
                        "but 2D TF is still in use by another subsystem.");
          }
          return;
        }
      }
    }
  }
  dbg.Warning(_func_, "2D TF not found or not in use by requester.");
}

// ******************** 3D Textures

bool GPUMemMan::IsResident(const Dataset* pDataset,
                           const vector<UINT64>& vLOD,
                           const vector<UINT64>& vBrick,
                           bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits,
                           bool bDisableBorder) const {
  for(Texture3DListConstIter i = m_vpTex3DList.begin();
      i < m_vpTex3DList.end(); ++i) {
    if((*i)->Equals(pDataset, vLOD, vBrick, bUseOnlyPowerOfTwo,
                    bDownSampleTo8Bits, bDisableBorder,
                    CTContext::Current())) {
      return true;
    }
  }
  return false;
}

/// Calculates the amount of memory the given brick will take up.
/// Slightly complicated because we might have an N-dimensional brick.
static UINT64
required_cpu_memory(const Metadata& md,
                    const std::vector<UINT64>& vLOD,
                    const std::vector<UINT64>& vBrick)
{
  UINT64 mem = 1;
  try {
    const UVFMetadata& umd = dynamic_cast<const UVFMetadata&>(md);
    const std::vector<UINT64> vSize = umd.GetBrickSizeND(vLOD, vBrick);
    mem = std::accumulate(vSize.begin(), vSize.end(), UINT64(1),
                          std::multiplies<UINT64>());
  } catch(const std::bad_cast&) {
    const size_t lod = vLOD[0];
    const UINT64VECTOR3 brick(vBrick[0],vBrick[1],vBrick[2]);
    const UINT64VECTOR3 sz = md.GetBrickSize(Metadata::BrickKey(lod, brick));
    mem = sz[0] * sz[1] * sz[2];
  }
  mem *= md.GetBitWidth();
  mem *= md.GetComponentCount();
  return mem;
}

/// Searches the texture list for a texture which matches the given criterion.
/// @return matching texture, or lst.end() if no texture matches.
static Texture3DListIter
find_closest_texture(Texture3DList &lst, const std::vector<UINT64> vSize,
                     bool use_pot, bool downsample, bool disable_border)
{
  UINT64 iTargetFrameCounter = UINT64_INVALID;
  UINT64 iTargetIntraFrameCounter = UINT64_INVALID;

  Texture3DListIter iBestMatch = lst.end();
  for (Texture3DListIter i=lst.begin(); i < lst.end(); ++i) {
    if ((*i)->BestMatch(vSize, use_pot, downsample, disable_border,
                        iTargetIntraFrameCounter, iTargetFrameCounter,
                        CTContext::Current())) {
      iBestMatch = i;
    }
  }
  if(iBestMatch != lst.end()) {
    MESSAGE("  Found suitable target brick from frame %i with intraframe "
            "counter %i.", int(iTargetFrameCounter),
            int(iTargetIntraFrameCounter));
  }
  return iBestMatch;
}


// We use our own function instead of a functor because we're searching through
// a list of 3D textures, which are noncopyable.  A copy operation on our 3d
// texes would be expensive.
template <typename ForwIter> static ForwIter
find_brick_with_usercount(ForwIter first, const ForwIter last,
                          UINT32 user_count)
{
  while(first != last && (*first)->iUserCount != user_count) {
    ++first;
  }
  return first;
}

// Gets rid of *all* unused bricks.  Returns the number of bricks it deleted.
size_t GPUMemMan::DeleteUnusedBricks() {
  size_t removed = 0;
  // This is a bit harsh.  erase() in the middle of a deque invalidates *all*
  // iterators.  So we repeatedly search for unused bricks, until our search
  // comes up empty.
  // That said, erase() at the beginning or end of a deque only invalidates the
  // pointed-to element.  You might consider an optimization where we delete
  // all unused bricks from the beginning and end of the structure, before
  // moving on to the exhaustive+expensive search.
  bool found;
  do {
    found = false;
    const Texture3DListIter& iter = find_brick_with_usercount(
                                      m_vpTex3DList.begin(),
                                      m_vpTex3DList.end(), 0
                                    );
    if(iter != m_vpTex3DList.end()) {
      ++removed;
      found = true;
      Delete3DTexture(iter);
    }
  } while(!m_vpTex3DList.empty() && found);

  MESSAGE("Got rid of %u unused bricks.", static_cast<unsigned int>(removed));
  return removed;
}

// We don't have enough CPU memory to load something.  Get rid of a brick.
void GPUMemMan::DeleteArbitraryBrick() {
  assert(!m_vpTex3DList.empty());

  // Identify the least used brick.  The 128 is an arbitrary choice.  We want
  // it to be high enough to hit every conceivable number of users for a brick.
  // We don't want to use 2^32 though, because then the application would feel
  // like it hung if we had some other bug.
  for(UINT32 in_use_by=0; in_use_by < 128; ++in_use_by) {
    const Texture3DListIter& iter = find_brick_with_usercount(
                                      m_vpTex3DList.begin(),
                                      m_vpTex3DList.end(), in_use_by
                                    );
    if(iter != m_vpTex3DList.end()) {
      MESSAGE("  Deleting texture %d",
              std::distance(iter, m_vpTex3DList.begin()));
      Delete3DTexture(iter);
      return;
    }
  }
  WARNING("All bricks are (heavily) in use: "
          "cannot make space for a new brick.");
}

GLTexture3D* GPUMemMan::Get3DTexture(Dataset* pDataset,
                                     const vector<UINT64>& vLOD,
                                     const vector<UINT64>& vBrick,
                                     bool bUseOnlyPowerOfTwo,
                                     bool bDownSampleTo8Bits,
                                     bool bDisableBorder,
                                     UINT64 iIntraFrameCounter,
                                     UINT64 iFrameCounter) {
  // It can occur that we can create the brick in CPU memory but OpenGL must
  // perform a texture copy to obtain the texture.  If that happens, we'll
  // delete any brick and then try again.
  do {
    try {
      return this->AllocOrGet3DTexture(pDataset, vLOD, vBrick,
                                       bUseOnlyPowerOfTwo, bDownSampleTo8Bits,
                                       bDisableBorder,
                                       iIntraFrameCounter, iFrameCounter);
    } catch(tuvok::OutOfMemory&) { // Texture allocation failed.
      // If texture allocation failed and we had no bricks loaded, then the
      // system must be extremely memory limited.  Make a note and then bail.
      if(m_vpTex3DList.empty()) {
        T_ERROR("This system does not have enough memory to render a brick.");
        return NULL;
      }
      // Delete all bricks that aren't used.  If that ends up being nothing,
      // then we're pretty screwed.  Stupidly choose a brick in that case.
      if(0 == DeleteUnusedBricks()) {
        WARNING("No bricks unused.  Falling back to "
                "deleting bricks that ARE in use!");
        // Delete up to 4 bricks.  We want to delete multiple bricks here
        // because we'll temporarily need copies of the bricks in memory.
        for(size_t i=0; i < 4 && !m_vpTex3DList.empty(); ++i) {
          DeleteArbitraryBrick();
        }
      }
    }
  } while(!m_vpTex3DList.empty());
  // Can't happen, but to quiet compilers:
  return NULL;
}

GLTexture3D* GPUMemMan::AllocOrGet3DTexture(Dataset* pDataset,
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
                     bDownSampleTo8Bits, bDisableBorder,
                     CTContext::Current())) {
      MESSAGE("Reusing 3D texture");
      return (*i)->Access(iIntraFrameCounter, iFrameCounter);
    }
  }

  UINT64 iNeededCPUMemory = required_cpu_memory(pDataset->GetInfo(),
                                                vLOD, vBrick);

  const UINT64VECTOR3 bk(vBrick[0], vBrick[1], vBrick[2]);
  const UINT64VECTOR3 sz = pDataset->GetInfo().GetBrickSize(
                              Metadata::BrickKey(vLOD[0], bk)
                           );
  const UINT64 iBitWidth = pDataset->GetInfo().GetBitWidth();
  const UINT64 iCompCount = pDataset->GetInfo().GetComponentCount();

  // for OpenGL we ignore the GPU memory load and let GL do the paging
  if (m_iAllocatedCPUMemory + iNeededCPUMemory >
      m_SystemInfo->GetMaxUsableCPUMem()) {
    MESSAGE("Not enough memory for texture %i x %i x %i (%ibit * %i), "
            "paging ...", int(sz[0]), int(sz[1]), int(sz[2]),
            int(iBitWidth), int(iCompCount));

    // search for best brick to replace with this brick
    const UVFMetadata& umd = dynamic_cast<const UVFMetadata&>
                                         (pDataset->GetInfo());
    const std::vector<UINT64> vSize = umd.GetBrickSizeND(vLOD, vBrick);
    Texture3DListIter iBestMatch = find_closest_texture(m_vpTex3DList, vSize,
                                                        bUseOnlyPowerOfTwo,
                                                        bDownSampleTo8Bits,
                                                        bDisableBorder);
    if (iBestMatch != m_vpTex3DList.end()) {
      // found a suitable brick that can be replaced
      (*iBestMatch)->Replace(pDataset, vLOD, vBrick, bUseOnlyPowerOfTwo,
                             bDownSampleTo8Bits, bDisableBorder,
                             iIntraFrameCounter, iFrameCounter,
                             CTContext::Current(), m_vUploadHub);
      (*iBestMatch)->iUserCount++;
      return (*iBestMatch)->pTexture;
    } else {
      // We know the brick doesn't fit in memory, and we know there's no
      // existing texture which matches enough that we could overwrite it with
      // this one.  There's little we can do at this point ...
      MESSAGE("  No suitable brick found. Randomly deleting bricks until this"
              " brick fits into memory");

      while (m_iAllocatedCPUMemory + iNeededCPUMemory >
             m_SystemInfo->GetMaxUsableCPUMem()) {
        if (m_vpTex3DList.empty()) {
          // we do not have enough memory to page in even a single block...
          T_ERROR("Not enough memory to page a single brick into memory, "
                  "aborting (MaxMem=%ikb, NeededMem=%ikb).",
                  int(m_SystemInfo->GetMaxUsableCPUMem()/1024),
                  int(iNeededCPUMemory/1024));
          return NULL;
        }

        DeleteArbitraryBrick();
      }
    }
  }

  MESSAGE("Creating new texture %i x %i x %i, bitsize=%i, componentcount=%i",
          int(sz[0]), int(sz[1]), int(sz[2]), int(iBitWidth), int(iCompCount));

  Texture3DListElem* pNew3DTex = new Texture3DListElem(pDataset, vLOD, vBrick,
                                                       bUseOnlyPowerOfTwo,
                                                       bDownSampleTo8Bits,
                                                       bDisableBorder,
                                                       iIntraFrameCounter,
                                                       iFrameCounter,
                                                       m_MasterController,
                                                       CTContext::Current(),
                                                       m_vUploadHub);
  if (pNew3DTex->pTexture == NULL) {
    T_ERROR("Failed to create OpenGL texture.");
    delete pNew3DTex;
    return NULL;
  }
  MESSAGE("texture created.");
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

void GPUMemMan::Delete3DTexture(const Texture3DListIter &tex) {
  m_iAllocatedGPUMemory -= (*tex)->pTexture->GetCPUSize();
  m_iAllocatedCPUMemory -= (*tex)->pTexture->GetGPUSize();

  if((*tex)->iUserCount != 0) {
    WARNING("Freeing used 3D texture!");
  }
  delete *tex;
  m_vpTex3DList.erase(tex);
}

void GPUMemMan::Delete3DTexture(size_t iIndex) {
  this->Delete3DTexture(m_vpTex3DList.begin() + iIndex);
}

void GPUMemMan::FreeAssociatedTextures(Dataset* pDataset) {
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


GLFBOTex* GPUMemMan::GetFBO(GLenum minfilter, GLenum magfilter,
                            GLenum wrapmode, GLsizei width, GLsizei height,
                            GLenum intformat, UINT32 iSizePerElement,
                            bool bHaveDepth, int iNumBuffers) {
  MESSAGE("Creating new FBO of size %i x %i", int(width), int(height));

  UINT64 m_iCPUMemEstimate = GLFBOTex::EstimateCPUSize(width, height,
                                                       iSizePerElement,
                                                       bHaveDepth, iNumBuffers);

  // if we are running out of mem, kick out bricks to create room for the FBO
  while (m_iAllocatedCPUMemory + m_iCPUMemEstimate >
         m_SystemInfo->GetMaxUsableCPUMem() && m_vpTex3DList.size() > 0) {
    MESSAGE("Not enough memory for FBO %i x %i (%ibit * %i), "
            "paging out bricks ...", int(width), int(height),
            int(iSizePerElement), int(iNumBuffers));

    // search for best brick to replace with this brick
    UINT64 iMinTargetFrameCounter;
    UINT64 iMaxTargetIntraFrameCounter;
    (*m_vpTex3DList.begin())->GetCounters(iMaxTargetIntraFrameCounter,
                                          iMinTargetFrameCounter);
    size_t iIndex = 0;
    size_t iBestIndex = 0;

    for (Texture3DListIter i = m_vpTex3DList.begin()+1;i<m_vpTex3DList.end();i++) {
      UINT64 iTargetFrameCounter = UINT64_INVALID;
      UINT64 iTargetIntraFrameCounter = UINT64_INVALID;
      (*i)->GetCounters(iTargetIntraFrameCounter, iTargetFrameCounter);
      iIndex++;

      if (iTargetFrameCounter < iMinTargetFrameCounter) {
        iMinTargetFrameCounter = iTargetFrameCounter;
        iMaxTargetIntraFrameCounter = iTargetIntraFrameCounter;
        iBestIndex = iIndex;
      } else {
        if (iTargetFrameCounter == iMinTargetFrameCounter &&
            iTargetIntraFrameCounter > iMaxTargetIntraFrameCounter) {
          iMaxTargetIntraFrameCounter = iTargetIntraFrameCounter;
          iBestIndex = iIndex;
        }
      }
    }
    MESSAGE("   Deleting texture %i", int(iBestIndex));
    Delete3DTexture(iBestIndex);
  }


  FBOListElem* e = new FBOListElem(m_MasterController, minfilter, magfilter,
                                   wrapmode, width, height, intformat,
                                   iSizePerElement, bHaveDepth, iNumBuffers);
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

GLSLProgram* GPUMemMan::GetGLSLProgram(const string& strVSFile,
                                       const string& strFSFile) {
  for (GLSLListIter i = m_vpGLSLList.begin();i<m_vpGLSLList.end();i++) {
    if ((*i)->strVSFile == strVSFile && (*i)->strFSFile == strFSFile) {
      MESSAGE("Reusing GLSL program from the VS %s and the FS %s",
              (*i)->strVSFile.c_str(), (*i)->strFSFile.c_str());
      (*i)->iAccessCounter++;
      return (*i)->pGLSLProgram;
    }
  }

  MESSAGE("Creating new GLSL program from the VS %s and the FS %s",
          SysTools::GetFilename(strVSFile).c_str(),
          SysTools::GetFilename(strFSFile).c_str());

  GLSLListElem* e = new GLSLListElem(m_MasterController, strVSFile, strFSFile);

  if (e->pGLSLProgram != NULL) {
    m_vpGLSLList.push_back(e);

    m_iAllocatedGPUMemory += e->pGLSLProgram->GetCPUSize();
    m_iAllocatedCPUMemory += e->pGLSLProgram->GetGPUSize();

    return e->pGLSLProgram;
  } else {
    T_ERROR("Failed to created GLSL program from the VS %s and the FS %s",
            strVSFile.c_str(), strFSFile.c_str());
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
      } else {
        MESSAGE("Decreased access counter but kept GLSL program in memory.");
      }
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
