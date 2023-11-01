// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
void pylibsim_setWorkerProcessCallback(PyObject*);

int pylibsim_invokeBroadcastIntCallback(int*,int);
int pylibsim_invokeBroadcastStringCallback(char*,int,int);
void pylibsim_invokeWorkerProcessCallback(void);

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
void pylibsim_invoke_v_F_pc_pv(char*,void*);
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
