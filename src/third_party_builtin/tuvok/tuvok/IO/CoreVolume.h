/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/
#pragma once

#ifndef TUVOK_CORE_VOLUME_H
#define TUVOK_CORE_VOLUME_H

#include "VolumeDataset.h"
#include "../Controller/Controller.h"

/** A CoreVolume is a VolumeDataset which is entirely in core -- no paging can
 * be done for rendering the dataset.  The intent is that CoreVolumes only have
 * a single LOD. */
class CoreVolume : public VolumeDataset {
public:
  CoreVolume();
  virtual ~CoreVolume();

  /// These functions make no sense given the purpose of this class.
  /// .. of course this indicates we should rethink the hierarchy.
  ///@{
  virtual bool IsOpen() const { return true; }
  virtual std::string Filename() const { return std::string(""); }
  virtual bool Export(UINT64, const std::string&, bool, 
                      bool (*)(LargeRAWFile* pSourceFile,
                               const std::vector<UINT64> vBrickSize,
                               const std::vector<UINT64> vBrickOffset,
                               void* pUserContext)=NULL,
                      void* = NULL,
                      UINT64 =0) const {
    T_ERROR("Cannot export in-core dataset!");
    return false;
  }
  ///@}

  virtual bool GetBrick(unsigned char** ppData,
                        const std::vector<UINT64>& vLOD,
                        const std::vector<UINT64>& vBrick) const;
  virtual float GetMaxGradMagnitude() const;

  void SetHistogram(const std::vector<UINT32>&);
  void SetHistogram(const std::vector<std::vector<UINT32> >&);

  void SetInfo(VolumeDatasetInfo *vds_i) { m_pVolumeDatasetInfo = vds_i; }

//  void SetData(unsigned short*, size_t len);
  void SetData(float*, size_t len);
  void SetData(unsigned char*, size_t len);
  void SetGradientMagnitude(float *gmn, size_t len);

protected:
  /// Should the data change and the client isn't going to supply a histogram,
  /// we should supply one ourself.
  void Recalculate1DHistogram();

private:
  std::vector<unsigned char> m_vScalar;
  std::vector<float> m_vGradientMagnitude;
};

#endif // TUVOK_CORE_VOLUME_H
