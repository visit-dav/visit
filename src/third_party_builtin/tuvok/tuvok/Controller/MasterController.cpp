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

//!    File   : MasterController.cpp
//!    Author : Jens Krueger
//!             SCI Institute
//!             University of Utah
//!    Date   : September 2008
//
//!    Copyright (C) 2008 SCI Institute


#include "MasterController.h"
#include "../Basics/SystemInfo.h"
#include "../DebugOut/TextfileOut.h"
#include "../DebugOut/MultiplexOut.h"
#include "../IO/IOManager.h"
#include "../Renderer/GPUMemMan/GPUMemMan.h"
#include "../Renderer/GL/GLRaycaster.h"
#include "../Renderer/GL/GLSBVR.h"

#if defined(_WIN32) && defined(USE_DIRECTX)
#include "../Renderer/DX/DXSBVR.h"
#include "../Renderer/DX/DXRaycaster.h"
#endif

#include "../Scripting/Scripting.h"

MasterController::MasterController()
{
  m_pSystemInfo   = new SystemInfo();
  m_pIOManager    = new IOManager();
  m_pGPUMemMan    = new GPUMemMan(this);
  m_pScriptEngine = new Scripting();
  RegisterCalls(m_pScriptEngine);
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
}


void MasterController::AddDebugOut(AbstrDebugOut* debugOut) {
  if (debugOut != NULL) {
    m_DebugOut.Message(_func_, "Disconnecting from this debug out");

    m_DebugOut.AddDebugOut(debugOut);

    debugOut->Message(_func_, "Connected to this debug out");
  } else {
    m_DebugOut.Warning(_func_,
                       "New debug is a NULL pointer, ignoring it.");
  }
}


void MasterController::RemoveDebugOut(AbstrDebugOut* debugOut) {
  m_DebugOut.RemoveDebugOut(debugOut);
}

/// Access the currently-active debug stream.
AbstrDebugOut* MasterController::DebugOut()
{
  return (m_DebugOut.size() == 0) ? static_cast<AbstrDebugOut*>(&m_DefaultOut)
                                  : static_cast<AbstrDebugOut*>(&m_DebugOut);
}
const AbstrDebugOut *MasterController::DebugOut() const {
  return (m_DebugOut.size() == 0)
           ? static_cast<const AbstrDebugOut*>(&m_DefaultOut)
           : static_cast<const AbstrDebugOut*>(&m_DebugOut);
}

AbstrRenderer* MasterController::
RequestNewVolumerenderer(EVolumeRendererType eRendererType, bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits, bool bDisableBorder) {

  switch (eRendererType) {

  case OPENGL_SBVR :
    m_DebugOut.Message(_func_,"Starting up new renderer (API=OpenGL, Method=Slice Based Volume Rendering)");
    m_vVolumeRenderer.push_back(new GLSBVR(this, bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder));
    return m_vVolumeRenderer[m_vVolumeRenderer.size()-1];

  case OPENGL_RAYCASTER :
    m_DebugOut.Message(_func_,"Starting up new renderer (API=OpenGL, Method=Raycaster)");
    m_vVolumeRenderer.push_back(new GLRaycaster(this, bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder));
    return m_vVolumeRenderer[m_vVolumeRenderer.size()-1];


#if defined(_WIN32) && defined(USE_DIRECTX)
  case DIRECTX_SBVR : 
    m_DebugOut.Message(_func_,"Starting up new renderer (API=DirectX, Method=SBVR)");
    m_vVolumeRenderer.push_back(new DXSBVR(this, bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder));
    return m_vVolumeRenderer[m_vVolumeRenderer.size()-1];

  case DIRECTX_RAYCASTER :
    m_DebugOut.Message(_func_,"Starting up new renderer (API=DirectX, Method=Raycaster)");
    m_vVolumeRenderer.push_back(new DXRaycaster(this, bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder));
    return m_vVolumeRenderer[m_vVolumeRenderer.size()-1];
#else
  case DIRECTX_RAYCASTER :
  case DIRECTX_SBVR : 
    m_DebugOut.Error(_func_,"DirectX 10 renderer not yet implemented. Please select OpenGL as the render API in the settings dialog.");
    return NULL;
#endif

  default :
    m_DebugOut.Error(_func_,"Unsupported Volume renderer requested");
    return NULL;
  };
}


void MasterController::ReleaseVolumerenderer(AbstrRenderer* pVolumeRenderer) {
  for (AbstrRendererListIter i = m_vVolumeRenderer.begin();
       i < m_vVolumeRenderer.end();
       ++i) {

    if (*i == pVolumeRenderer) {
      m_DebugOut.Message(_func_, "Deleting volume renderer");
      delete pVolumeRenderer;

      m_vVolumeRenderer.erase(i);
      return;
    }
  }

  m_DebugOut.Warning(_func_, "requested volume renderer not found");
}


void MasterController::Filter( std::string , UINT32 ,
                              void*, void *, void *, void * ) {
};


void MasterController::RegisterCalls(Scripting* pScriptEngine) {
  pScriptEngine->RegisterCommand(this, "seterrorlog", "on/off", "toggle recording of errors");
  pScriptEngine->RegisterCommand(this, "setwarninglog", "on/off", "toggle recording of warnings");
  pScriptEngine->RegisterCommand(this, "setemessagelog", "on/off", "toggle recording of messages");
  pScriptEngine->RegisterCommand(this, "printerrorlog", "", "print recorded errors");
  pScriptEngine->RegisterCommand(this, "printwarninglog", "", "print recorded errwarningsors");
  pScriptEngine->RegisterCommand(this, "printmessagelog", "", "print recorded messages");
  pScriptEngine->RegisterCommand(this, "clearerrorlog", "", "clear recorded errors");
  pScriptEngine->RegisterCommand(this, "clearwarninglog", "", "clear recorded warnings");
  pScriptEngine->RegisterCommand(this, "clearmessagelog", "", "clear recorded messages");
  pScriptEngine->RegisterCommand(this, "fileoutput", "filename","write debug output to 'filename'");
  pScriptEngine->RegisterCommand(this, "toggleoutput", "on/off on/off on/off on/off","toggle messages, warning, errors, and other output");
}

bool MasterController::Execute(const std::string& strCommand, const std::vector< std::string >& strParams, std::string& strMessage) {
  strMessage = "";
  if (strCommand == "seterrorlog") {
    m_DebugOut.SetListRecordingErrors(strParams[0] == "on");
    if (m_DebugOut.GetListRecordingErrors()) m_DebugOut.printf("current state: true"); else m_DebugOut.printf("current state: false");
    return true;
  }
  if (strCommand == "setwarninglog") {
    m_DebugOut.SetListRecordingWarnings(strParams[0] == "on");
    if (m_DebugOut.GetListRecordingWarnings()) m_DebugOut.printf("current state: true"); else m_DebugOut.printf("current state: false");
    return true;
  }
  if (strCommand == "setmessagelog") {
    m_DebugOut.SetListRecordingMessages(strParams[0] == "on");
    if (m_DebugOut.GetListRecordingMessages()) m_DebugOut.printf("current state: true"); else m_DebugOut.printf("current state: false");
    return true;
  }
  if (strCommand == "printerrorlog") {
    m_DebugOut.PrintErrorList();
    return true;
  }
  if (strCommand == "printwarninglog") {
    m_DebugOut.PrintWarningList();
    return true;
  }
  if (strCommand == "printmessagelog") {
    m_DebugOut.PrintMessageList();
    return true;
  }
  if (strCommand == "clearerrorlog") {
    m_DebugOut.ClearErrorList();
    return true;
  }
  if (strCommand == "clearwarninglog") {
    m_DebugOut.ClearWarningList();
    return true;
  }
  if (strCommand == "clearmessagelog") {
    m_DebugOut.ClearMessageList();
    return true;
  }
  if (strCommand == "toggleoutput") {
    m_DebugOut.SetOutput(strParams[0] == "on",
                           strParams[1] == "on",
                           strParams[2] == "on",
                           strParams[3] == "on");
    return true;
  }
  if (strCommand == "fileoutput") {
    TextfileOut* textout = new TextfileOut(strParams[0]);
    textout->SetShowErrors(m_DebugOut.ShowErrors());
    textout->SetShowWarnings(m_DebugOut.ShowWarnings());
    textout->SetShowMessages(m_DebugOut.ShowMessages());
    textout->SetShowOther(m_DebugOut.ShowOther());

    m_DebugOut.AddDebugOut(textout);
  
    return true;
  }

  return false;
}
