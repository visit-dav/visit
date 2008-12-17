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


#include "MasterController.h"

MasterController::MasterController() :
  m_pDebugOut(new ConsoleOut()),
  m_bDeleteDebugOutOnExit(true)
{
  m_pSystemInfo = new SystemInfo();
  m_pIOManager = new IOManager(this);
  m_pGPUMemMan = new GPUMemMan(this);
}


MasterController::~MasterController() {

  for (AbstrRendererListIter i = m_vVolumeRenderer.begin();
       i<m_vVolumeRenderer.end();
       ++i)
    delete (*i);

  m_vVolumeRenderer.resize(0);

  delete m_pSystemInfo;
  delete m_pIOManager;
  delete m_pGPUMemMan;

  if (m_bDeleteDebugOutOnExit) delete m_pDebugOut;
}


void MasterController::SetDebugOut(AbstrDebugOut* debugOut,
				   bool bDeleteOnExit) {
  if (debugOut != NULL) {
    m_pDebugOut->Message("MasterController::SetDebugOut",
			 "Disconnecting from this debug out");
    if (m_bDeleteDebugOutOnExit )
      delete m_pDebugOut;

    m_bDeleteDebugOutOnExit = bDeleteOnExit;

    m_pDebugOut = debugOut;
    m_pDebugOut->Message("MasterController::SetDebugOut",
			 "Connected to this debug out");

  } else {
    m_pDebugOut->Warning("MasterController::SetDebugOut",
			 "New debug is a NULL pointer, keeping old debug out");
  }
}


void MasterController::RemoveDebugOut(AbstrDebugOut* debugOut) {

  if (debugOut == m_pDebugOut) {
    m_pDebugOut->Message("MasterController::RemoveDebugOut",
			 "Disconnecting from this debug out");
    if (m_bDeleteDebugOutOnExit)
      delete m_pDebugOut;

    m_pDebugOut = new ConsoleOut();

    m_bDeleteDebugOutOnExit = true;
    m_pDebugOut->Message("MasterController::RemoveDebugOut",
			 "Connected to this debug out");

  } else {
    m_pDebugOut->Message("MasterController::RemoveDebugOut",
			 "Not Connected the debug out in question (anymore), doing nothing");
  }
}


AbstrRenderer* MasterController::
RequestNewVolumerenderer(EVolumeRendererType eRendererType, bool bUseOnlyPowerOfTwo) {

  switch (eRendererType) {

  case OPENGL_SBVR :
    m_pDebugOut->Message("MasterController::RequestNewVolumerenderer","Starting up new renderer (API=OpenGL, Method=Slice Based Volume Rendering)");
    m_vVolumeRenderer.push_back(new GLSBVR(this, bUseOnlyPowerOfTwo));
    return m_vVolumeRenderer[m_vVolumeRenderer.size()-1];

  case OPENGL_RAYCASTER :
    m_pDebugOut->Message("MasterController::RequestNewVolumerenderer","Starting up new renderer (API=OpenGL, Method=Raycaster)");
    m_vVolumeRenderer.push_back(new GLRaycaster(this, bUseOnlyPowerOfTwo));
    return m_vVolumeRenderer[m_vVolumeRenderer.size()-1];

  default :
    m_pDebugOut->Error("MasterController::RequestNewVolumerenderer","Unsupported Volume renderer requested");
    return NULL;
  };
}


void MasterController::ReleaseVolumerenderer(AbstrRenderer* pVolumeRenderer) {

  for (AbstrRendererListIter i = m_vVolumeRenderer.begin();
       i<m_vVolumeRenderer.end();
       ++i) {

    if (*i == pVolumeRenderer) {
      m_pDebugOut->Message("MasterController::ReleaseVolumerenderer",
			   "Deleting volume renderer");
      delete pVolumeRenderer;

      m_vVolumeRenderer.erase(i);
      return;
    }
  }

  m_pDebugOut->Warning("MasterController::ReleaseVolumerenderer",
		       "requested volume renderer not found");
}


void MasterController::Filter( std::string , unsigned int , 
                              void*, void *, void *, void * ) {
};
