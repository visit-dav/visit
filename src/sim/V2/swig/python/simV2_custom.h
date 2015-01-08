/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef simV2_custom_h
#define simV2_custom_h

#include <Python.h>
#include <simV2_PyObject.h>
#include <VisItInterfaceTypes_V2.h>
#include <deque>
#include <utility>

// type used to package objects needed
// to invoke python callback this is
// held by visit and given to the invoker
// in it's callback data argument.
typedef std::pair<simV2_PyObject, simV2_PyObject>
    simV2_CallbackData;

// API for creating callback data that needs to
// be released during a disconnect event
simV2_CallbackData *newCallbackData(
    simV2_PyObject &callback, simV2_PyObject data);

// those which have static variable
void pylibsim_setBroadcastStringCallback(PyObject*);
void pylibsim_setBroadcastIntCallback(PyObject*);
void pylibsim_setSlaveProcessCallback(PyObject*);

int pylibsim_invokeBroadcastIntCallback(int*,int);
int pylibsim_invokeBroadcastStringCallback(char*,int,int);
void pylibsim_invokeSlaveProcessCallback(void);

// invoker which expect pair of callback and data
int pylibsim_invoke_i_F_pi_i_pv(int*,int,void*);
int pylibsim_invoke_i_F_pcc_i_i_pv(char*,int,int,void*);
void pylibsim_invoke_v_F_pv(void*);
void pylibsim_invoke_v_F_pcc_pcc_pv(const char*,const char*,void *);
visit_handle pylibsim_invoke_h_F_pv(void*);
int pylibsim_invoke_i_F_pv(void *);
visit_handle pylibsim_invoke_h_F_i_pcc_pv(int,const char*,void*);
visit_handle pylibsim_invoke_h_F_pcc_pv(const char*, void*);
int pylibsim_invoke_i_F_pcc_pv(const char*,void*);
int pylibsim_invoke_i_F_pcc_i_i_h_h_pv(const char*,int,int,visit_handle,visit_handle,void*);
int pylibsim_invoke_i_F_pcc_pcc_i_h_h_pv(const char *, const char *, int, visit_handle, visit_handle, void *);
void pylibsim_invoke_v_F_i_pv(int,void*);
void pylibsim_invoke_v_F_pv(void *);

// for passing data
int pylibsim_VisIt_VariableData_setDataAsC(visit_handle, int, int, int, PyObject*);
int pylibsim_VisIt_VariableData_setDataAsI(visit_handle, int, int, int, PyObject*);
int pylibsim_VisIt_VariableData_setDataAsF(visit_handle, int, int, int, PyObject*);
int pylibsim_VisIt_VariableData_setDataAsD(visit_handle, int, int, int, PyObject*);

// clean up
void pylibsim_VisItFinalize(void);
void pylibsim_VisItDisconnect(void);
#endif
