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

//!    File   : ImageVis3D.cpp
//!    Author : Jens Krueger
//!             SCI Institute
//!             University of Utah
//!    Date   : September 2008
//
//!    Copyright (C) 2008 SCI Institute


#pragma once

#ifndef MASTERCONTROLLER_H
#define MASTERCONTROLLER_H

#include "../StdTuvokDefines.h"
#include <vector>
#include <string>

#include "../DebugOut/MultiplexOut.h"
#include "../DebugOut/ConsoleOut.h"

#include "../Renderer/AbstrRenderer.h"
#include "../Renderer/GPUMemMan/GPUMemManDataStructs.h"

#include "../Scripting/Scriptable.h"
class IOManager;
class GPUMemMan;
class Scripting;
class SystemInfo;

/** \class MasterController
 * Centralized controller for ImageVis3D.
 *
 * MasterController is a router for all of the components of
 * ImageVis3D.  Modules only depend on / utilize the controller,
 * never other modules.  For example, the GUI does not inform the
 * renderer that a re-render is necessary when a window is resized.
 * Instead, it informs the abstract interface through this controller,
 * and the renderer implementation decides on its own how it wants to
 * handle the resize event. */
class MasterController : public Scriptable {
public:
  enum EVolumeRendererType {
    OPENGL_SBVR = 0,
    OPENGL_RAYCASTER,
    DIRECTX_SBVR,
    DIRECTX_RAYCASTER
  };

  /// Defaults to using a Console-based debug output stream.
  MasterController();
  virtual ~MasterController();

  /// Create a new renderer.
  AbstrRenderer* RequestNewVolumerenderer(EVolumeRendererType eRendererType,
                                          bool bUseOnlyPowerOfTwo,
                                          bool bDownSampleTo8Bits,
                                          bool bDisableBorder,
                                          bool simple);
  /// Indicate that a renderer is no longer needed.
  void ReleaseVolumerenderer(AbstrRenderer* pVolumeRenderer);

  /// Add another debug output
  /// \param debugOut      the new stream
  void AddDebugOut(AbstrDebugOut* debugOut);

  /// Removes the given debug output stream.
  /// The stream must be the currently connected/used one.
  void RemoveDebugOut(AbstrDebugOut* debugOut);

  /// Access the currently-active debug stream.
  ///@{
  AbstrDebugOut* DebugOut();
  const AbstrDebugOut *DebugOut() const;
  ///@}

  /// The GPU memory manager moves data from CPU to GPU memory, and
  /// removes data from GPU memory.
  ///@{
  GPUMemMan*       MemMan()       { return m_pGPUMemMan; }
  const GPUMemMan* MemMan() const { return m_pGPUMemMan; }
  ///@}

  /// The IO manager is responsible for loading data into host memory.
  ///@{
  IOManager*       IOMan()       { return m_pIOManager;}
  const IOManager* IOMan() const { return m_pIOManager;}
  ///@}

  /// System information is for looking up host parameters, such as the
  /// amount of memory available.
  ///@{
  SystemInfo*       SysInfo()       { return m_pSystemInfo; }
  const SystemInfo* SysInfo() const { return m_pSystemInfo; }
  ///@}

  Scripting*       ScriptEngine()       { return m_pScriptEngine; }
  const Scripting* ScriptEngine() const { return m_pScriptEngine; }

  /// \todo this should return a pointer to memory.
  void Filter(std::string datasetName,
              UINT32 filter,
              void *var0 = 0, void *var1 = 0,
              void *var2 = 0, void *var3 = 0 );

  // Scriptable implementation
  virtual void RegisterCalls(Scripting* pScriptEngine);
  virtual bool Execute(const std::string& strCommand,
                       const std::vector< std::string >& strParams,
                       std::string& strMessage);

private:
  SystemInfo*    m_pSystemInfo;
  GPUMemMan*     m_pGPUMemMan;
  IOManager*     m_pIOManager;
  MultiplexOut   m_DebugOut;
  ConsoleOut     m_DefaultOut;
  Scripting*     m_pScriptEngine;
  bool           m_bDeleteDebugOutOnExit;

  AbstrRendererList m_vVolumeRenderer;
};

#endif // MASTERCONTROLLER_H
