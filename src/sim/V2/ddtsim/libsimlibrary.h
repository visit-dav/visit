/*****************************************************************************
*
* Copyright (c) 2011, Allinea
* All rights reserved.
*
* This file has been contributed to the VisIt project, which is
* Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
* https://visit.llnl.gov/.  The full copyright notice is contained in the 
* file COPYRIGHT located at the root of the VisIt distribution or at 
* http://www.llnl.gov/visit/copyright.html.
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

#ifndef LIBSIMLIBRARY_H
#define LIBSIMLIBRARY_H

#include "VisItInterfaceTypes_V2.h"

#ifndef VISIT_STATIC
  #include "dllibrary.h"
#else
  #include "VisItControlInterface_V2.h"
  #include "VisItDataInterface_V2.h"
#endif


class LibsimLibrary
{
public:   
#ifndef VISIT_STATIC
    LibsimLibrary(const char* lib);
    bool isLoaded() { return mLibsim.isLoaded(); }

    void SetSlaveProcessCallback(void (*callback)()) {
        ((void (*)(void (*)())) mLibsim.symbol("VisItSetSlaveProcessCallback")) (callback); }

    void SetBroadcastIntFunction(int (*callback)(int*, int)) {
        ((void (*)(int (*)(int*, int))) mLibsim.symbol("VisItSetBroadcastIntFunction")) (callback); }

    void SetBroadcastStringFunction(int (*callback)(char*, int, int)) {
        ((void (*)(int (*)(char*, int, int))) mLibsim.symbol("VisItSetBroadcastStringFunction")) (callback); }

    void SetCommandCallback(void (*callback)(const char *, const char *, void *), void *cbdata) {
        ((void (*)(void (*)(const char *, const char *, void *),void*)) mLibsim.symbol("VisItSetCommandCallback")) (callback, cbdata); }

    int SetGetDomainList(visit_handle (*callback)(const char *, void *), void *cbdata) {
        return ((int (*)(visit_handle (*)(const char *, void *),void*)) mLibsim.symbol("VisItSetGetDomainList")) (callback, cbdata); }

    int SetGetMetaData(visit_handle (*callback)(void *), void *cbdata) {
        return ((int (*)(visit_handle (*)(void *),void*)) mLibsim.symbol("VisItSetGetMetaData")) (callback, cbdata); }

    int SetGetMesh(visit_handle (*callback)(int, const char *, void *), void *cbdata) {
        return ((int (*)(visit_handle (*)(int, const char *, void *),void*)) mLibsim.symbol("VisItSetGetMesh")) (callback, cbdata); }

    int SetGetVariable(visit_handle (*callback)(int, const char *, void *), void *cbdata) {
        return ((int (*)(visit_handle (*)(int, const char *, void *),void*)) mLibsim.symbol("VisItSetGetVariable")) (callback, cbdata); }

    int   InitializeSocketAndDumpSimFile(const char *name, const char *comment, const char *path,
            const char *inputfile, const char *guifile, const char *absoluteFilename) {
                return ((int (*)(const char*,const char*,const char*,const char*,const char*,const char*)) 
                    mLibsim.symbol("VisItInitializeSocketAndDumpSimFile"))(
                        name,comment,path,inputfile,guifile,absoluteFilename); }

    int   AttemptToCompleteConnection() {return ((int (*)()) mLibsim.symbol("VisItAttemptToCompleteConnection"))(); }
    void  Disconnect()           { ((void (*)()) mLibsim.symbol("VisItDisconnect"))(); }
    char* GetLastError()         { return ((char* (*)()) mLibsim.symbol("VisItGetLastError"))(); }
    int   SetupEnvironment()     { return ((int (*)()) mLibsim.symbol("VisItSetupEnvironment"))(); }
    void  SetParallel(int b)     { ((void (*)(int)) mLibsim.symbol("VisItSetParallel"))(b); }
    void  SetParallelRank(int r) { ((void (*)(int)) mLibsim.symbol("VisItSetParallelRank"))(r); }
    void  SetDirectory(char *dir) { ((void (*)(char *)) mLibsim.symbol("VisItSetDirectory"))(dir); }
    void  OpenTraceFile(char *file) { ((void (*)(char *)) mLibsim.symbol("VisItOpenTraceFile"))(file); }
    void  CloseTraceFile() { ((void (*)()) mLibsim.symbol("VisItCloseTraceFile"))(); }

    // Cache and reuse pointers to frequently used functions
    void UpdatePlots()     { mVisItUpdatePlots(); }
    void TimeStepChanged() { mVisItTimeStepChanged(); }
    int DetectInput(int blocking, int consoledesc) {
        return mVisItDetectInput(blocking,consoledesc); }
    int ProcessEngineCommand() {
        return mVisItProcessEngineCommand(); }

    int VariableData_alloc(visit_handle *h) {
        return mVariableData_alloc(h); }
    int DomainList_alloc(visit_handle *h) {
        return mDomainList_alloc(h); }
    int SimulationMetaData_alloc(visit_handle *h) {
        return mVisIt_SimulationMetaData_alloc(h); }
    int SimulationMetaData_setMode(visit_handle h,int mode) {
        return mVisIt_SimulationMetaData_setMode(h,mode); }
    int SimulationMetaData_setCycleTime(visit_handle h,int cycle,double time) {
        return mVisIt_SimulationMetaData_setCycleTime(h,cycle,time); }
    int SimulationMetaData_addGenericCommand(visit_handle h,visit_handle c) {
        return mVisIt_SimulationMetaData_addGenericCommand(h,c); }
    int SimulationMetaData_addMesh(visit_handle h,visit_handle m) {
        return mVisIt_SimulationMetaData_addMesh(h,m); }
    int SimulationMetaData_addVariable(visit_handle h,visit_handle v) {
        return mVisIt_SimulationMetaData_addVariable(h,v); }

    int CommandMetaData_alloc(visit_handle *h) {
        return mVisIt_CommandMetaData_alloc(h); }
    int CommandMetaData_setName(visit_handle h,const char *name) {
        return mVisIt_CommandMetaData_setName(h,name); }

    int MeshMetaData_alloc(visit_handle *h) {
        return mVisIt_MeshMetaData_alloc(h); }
    int MeshMetaData_setName(visit_handle h,const char *name) {
        return mVisIt_MeshMetaData_setName(h,name); }
    int MeshMetaData_setMeshType(visit_handle h,int type) {
        return mVisIt_MeshMetaData_setMeshType(h,type); }
    int MeshMetaData_setTopologicalDimension(visit_handle h,int dim) {
        return mVisIt_MeshMetaData_setTopologicalDimension(h,dim); }
    int MeshMetaData_setSpatialDimension(visit_handle h,int dim) {
        return mVisIt_MeshMetaData_setSpatialDimension(h,dim); }
    int MeshMetaData_setNumDomains(visit_handle h,int dom) {
        return mVisIt_MeshMetaData_setNumDomains(h,dom); }
    int MeshMetaData_setXUnits(visit_handle h,const char *name) {
        return mVisIt_MeshMetaData_setXUnits(h,name); }
    int MeshMetaData_setYUnits(visit_handle h,const char *name) {
        return mVisIt_MeshMetaData_setYUnits(h,name); }
    int MeshMetaData_setZUnits(visit_handle h,const char *name) {
        return mVisIt_MeshMetaData_setZUnits(h,name); }
    int MeshMetaData_setXLabel(visit_handle h,const char *name) {
        return mVisIt_MeshMetaData_setXLabel(h,name); }
    int MeshMetaData_setYLabel(visit_handle h,const char *name) {
        return mVisIt_MeshMetaData_setYLabel(h,name); }
    int MeshMetaData_setZLabel(visit_handle h,const char *name) {
        return mVisIt_MeshMetaData_setZLabel(h,name); }

    int VariableMetaData_alloc(visit_handle *h) {
        return mVisIt_VariableMetaData_alloc(h); }
    int VariableMetaData_setName(visit_handle h,const char *name) {
        return mVisIt_VariableMetaData_setName(h,name); }
    int VariableMetaData_setMeshName(visit_handle h,const char *name) {
        return mVisIt_VariableMetaData_setMeshName(h,name); }
    int VariableMetaData_setUnits(visit_handle h,const char *name) {
        return mVisIt_VariableMetaData_setUnits(h,name); }
    int VariableMetaData_setType(visit_handle h,int type) {
        return mVisIt_VariableMetaData_setType(h,type); }
    int VariableMetaData_setCentering(visit_handle h,int c) {
        return mVisIt_VariableMetaData_setCentering(h,c); }
    int DomainList_setDomains(visit_handle h,int allDoms,visit_handle myDoms) {
        return mDomainList_setDomains(h,allDoms,myDoms); }
    int VariableData_setDataI(visit_handle h,int owner,int nComps,int nTuples,int *ptr) {
        return mVariableData_setDataI(h,owner,nComps,nTuples,ptr); }
    int VariableData_setDataF(visit_handle h,int owner,int nComps,int nTuples,float *ptr) {
        return mVariableData_setDataF(h,owner,nComps,nTuples,ptr); }
    int VariableData_setDataD(visit_handle h,int owner,int nComps,int nTuples,double *ptr) {
        return mVariableData_setDataD(h,owner,nComps,nTuples,ptr); }
    int VariableData_setDataC(visit_handle h,int owner,int nComps,int nTuples,char *ptr) {
        return mVariableData_setDataC(h,owner,nComps,nTuples,ptr); }

    int RectilinearMesh_alloc(visit_handle *h) {
        return mVisIt_RectilinearMesh_alloc(h); }
    int RectilinearMesh_setCoordsXY(visit_handle h,visit_handle x,visit_handle y) {
        return mVisIt_RectilinearMesh_setCoordsXY(h,x,y); }
    int RectilinearMesh_setCoordsXYZ(visit_handle h,visit_handle x,visit_handle y,visit_handle z) {
        return mVisIt_RectilinearMesh_setCoordsXYZ(h,x,y,z); }
    int RectilinearMesh_setBaseIndex(visit_handle h,int base_index[3]) {
        return mVisIt_RectilinearMesh_setBaseIndex(h,base_index); }

    int PointMesh_alloc(visit_handle *h) {
        return mVisIt_PointMesh_alloc(h); }
    int PointMesh_setCoordsXY(visit_handle h,visit_handle x,visit_handle y) {
        return mVisIt_PointMesh_setCoordsXY(h,x,y); }
    int PointMesh_setCoordsXYZ(visit_handle h,visit_handle x,visit_handle y,visit_handle z) {
        return mVisIt_PointMesh_setCoordsXYZ(h,x,y,z); }
#else
        LibsimLibrary() {}
    LibsimLibrary(const char* lib) {}
    bool isLoaded() { return true; }

    void SetSlaveProcessCallback(void (*callback)())                  { VisItSetSlaveProcessCallback(callback); }
    void SetBroadcastIntFunction(int (*callback)(int*, int))          { VisItSetBroadcastIntFunction(callback); }
    void SetBroadcastStringFunction(int (*callback)(char*, int, int)) { VisItSetBroadcastStringFunction(callback); }
    void SetCommandCallback(void (*callback)(const char *, const char *, void *), void *cbdata) {
        VisItSetCommandCallback(callback, cbdata); }
    int SetGetDomainList(visit_handle (*callback)(const char *, void *), void *cbdata)    { return VisItSetGetDomainList(callback, cbdata); }
    int SetGetMetaData(visit_handle (*callback)(void *), void *cbdata)                    { return VisItSetGetMetaData(callback, cbdata); }
    int SetGetMesh(visit_handle (*callback)(int, const char *, void *), void *cbdata)     { return VisItSetGetMesh(callback, cbdata); }
    int SetGetVariable(visit_handle (*callback)(int, const char *, void *), void *cbdata) { return VisItSetGetVariable(callback, cbdata); }
    int   InitializeSocketAndDumpSimFile(const char *name, const char *comment, const char *path,
            const char *inputfile, const char *guifile, const char *absoluteFilename) {
        return VisItInitializeSocketAndDumpSimFile(name,comment,path,inputfile,guifile,absoluteFilename); }
    int   AttemptToCompleteConnection()             { return VisItAttemptToCompleteConnection(); }
    void  Disconnect()                              { VisItDisconnect(); }
    char* GetLastError()                            { return VisItGetLastError(); }
    int   SetupEnvironment()                        { return VisItSetupEnvironment(); }
    void  SetParallel(int b)                        { VisItSetParallel(b); }
    void  SetParallelRank(int r)                    { VisItSetParallelRank(r); }
    void  SetDirectory(char *dir)                   { VisItSetDirectory(dir); }
    void  OpenTraceFile(char *file)                 { VisItOpenTraceFile(file); }
    void  CloseTraceFile()                          { VisItCloseTraceFile(); }
    void UpdatePlots()                              { VisItUpdatePlots(); }
    void TimeStepChanged()                          { VisItTimeStepChanged(); }
    int DetectInput(int blocking, int consoledesc)  { return VisItDetectInput(blocking,consoledesc); }
    int ProcessEngineCommand()                      { return VisItProcessEngineCommand(); }
    int VariableData_alloc(visit_handle *h)         { return VariableData_alloc(h); }
    int DomainList_alloc(visit_handle *h)           { return VisIt_DomainList_alloc(h); }
    int SimulationMetaData_alloc(visit_handle *h)   { return VisIt_SimulationMetaData_alloc(h); }
    int SimulationMetaData_setMode(visit_handle h,int mode)                   { return VisIt_SimulationMetaData_setMode(h,mode); }
    int SimulationMetaData_setCycleTime(visit_handle h,int cycle,double time) { return VisIt_SimulationMetaData_setCycleTime(h,cycle,time); }
    int SimulationMetaData_addGenericCommand(visit_handle h,visit_handle c)   { return VisIt_SimulationMetaData_addGenericCommand(h,c); }
    int SimulationMetaData_addMesh(visit_handle h,visit_handle m)             { return VisIt_SimulationMetaData_addMesh(h,m); }
    int SimulationMetaData_addVariable(visit_handle h,visit_handle v)         { return VisIt_SimulationMetaData_addVariable(h,v); }
    int CommandMetaData_alloc(visit_handle *h)                                { return VisIt_CommandMetaData_alloc(h); }
    int CommandMetaData_setName(visit_handle h,const char *name)              { return VisIt_CommandMetaData_setName(h,name); }
    int MeshMetaData_alloc(visit_handle *h)                                   { return VisIt_MeshMetaData_alloc(h); }
    int MeshMetaData_setName(visit_handle h,const char *name)                 { return VisIt_MeshMetaData_setName(h,name); }
    int MeshMetaData_setMeshType(visit_handle h,int type)                     { return VisIt_MeshMetaData_setMeshType(h,type); }
    int MeshMetaData_setTopologicalDimension(visit_handle h,int dim)          { return VisIt_MeshMetaData_setTopologicalDimension(h,dim); }
    int MeshMetaData_setSpatialDimension(visit_handle h,int dim)              { return VisIt_MeshMetaData_setSpatialDimension(h,dim); }
    int MeshMetaData_setNumDomains(visit_handle h,int dom)      { return VisIt_MeshMetaData_setNumDomains(h,dom); }
    int MeshMetaData_setXUnits(visit_handle h,const char *name) { return VisIt_MeshMetaData_setXUnits(h,name); }
    int MeshMetaData_setYUnits(visit_handle h,const char *name) { return VisIt_MeshMetaData_setYUnits(h,name); }
    int MeshMetaData_setZUnits(visit_handle h,const char *name) { return VisIt_MeshMetaData_setZUnits(h,name); }
    int MeshMetaData_setXLabel(visit_handle h,const char *name) { return VisIt_MeshMetaData_setXLabel(h,name); }
    int MeshMetaData_setYLabel(visit_handle h,const char *name) { return VisIt_MeshMetaData_setYLabel(h,name); }
    int MeshMetaData_setZLabel(visit_handle h,const char *name) { return VisIt_MeshMetaData_setZLabel(h,name); }
    int VariableMetaData_alloc(visit_handle *h)                 { return VisIt_VariableMetaData_alloc(h); }
    int VariableMetaData_setName(visit_handle h,const char *name)     { return VisIt_VariableMetaData_setName(h,name); }
    int VariableMetaData_setMeshName(visit_handle h,const char *name) { return VisIt_VariableMetaData_setMeshName(h,name); }
    int VariableMetaData_setUnits(visit_handle h,const char *name)    { return VisIt_VariableMetaData_setUnits(h,name); }
    int VariableMetaData_setType(visit_handle h,int type)             { return VisIt_VariableMetaData_setType(h,type); }
    int VariableMetaData_setCentering(visit_handle h,int c)           { return VisIt_VariableMetaData_setCentering(h,c); }
    int DomainList_setDomains(visit_handle h,int allDoms,visit_handle myDoms) { return DomainList_setDomains(h,allDoms,myDoms); }
    int VariableData_setDataI(visit_handle h,int owner,int nComps,int nTuples,int *ptr)     { return VariableData_setDataI(h,owner,nComps,nTuples,ptr); }
    int VariableData_setDataF(visit_handle h,int owner,int nComps,int nTuples,float *ptr)   { return VariableData_setDataF(h,owner,nComps,nTuples,ptr); }
    int VariableData_setDataD(visit_handle h,int owner,int nComps,int nTuples,double *ptr)  { return VariableData_setDataD(h,owner,nComps,nTuples,ptr); }
    int VariableData_setDataC(visit_handle h,int owner,int nComps,int nTuples,char *ptr)    { return VariableData_setDataC(h,owner,nComps,nTuples,ptr); }
    int RectilinearMesh_alloc(visit_handle *h)                                              { return VisIt_RectilinearMesh_alloc(h); }
    int RectilinearMesh_setCoordsXY(visit_handle h,visit_handle x,visit_handle y)           { return VisIt_RectilinearMesh_setCoordsXY(h,x,y); }
    int RectilinearMesh_setCoordsXYZ(visit_handle h,visit_handle x,visit_handle y,visit_handle z) { return VisIt_RectilinearMesh_setCoordsXYZ(h,x,y,z); }
    int RectilinearMesh_setBaseIndex(visit_handle h,int base_index[3])                      { return VisIt_RectilinearMesh_setBaseIndex(h,base_index); }
    int PointMesh_alloc(visit_handle *h)                                                    { return VisIt_PointMesh_alloc(h); }
    int PointMesh_setCoordsXY(visit_handle h,visit_handle x,visit_handle y)                 { return VisIt_PointMesh_setCoordsXY(h,x,y); }
    int PointMesh_setCoordsXYZ(visit_handle h,visit_handle x,visit_handle y,visit_handle z) { return VisIt_PointMesh_setCoordsXYZ(h,x,y,z); }
#endif

private:
#ifndef VISIT_STATIC
    DlLibrary mLibsim;
    
    void (*mVisItUpdatePlots)(void);    
    void (*mVisItTimeStepChanged)(void);
    
    int (*mVisItDetectInput)(int,int);
    int (*mVisItProcessEngineCommand)(void);
    
    int (*mVariableData_alloc)(visit_handle*);
    int (*mDomainList_alloc)(visit_handle*);
    
    int (*mVisIt_SimulationMetaData_alloc)(visit_handle*);
    int (*mVisIt_SimulationMetaData_setMode)(visit_handle,int);
    int (*mVisIt_SimulationMetaData_setCycleTime)(visit_handle,int,double);
    int (*mVisIt_SimulationMetaData_addGenericCommand)(visit_handle,visit_handle);
    int (*mVisIt_SimulationMetaData_addMesh)(visit_handle,visit_handle);
    int (*mVisIt_SimulationMetaData_addVariable)(visit_handle,visit_handle);
    
    int (*mVisIt_MeshMetaData_alloc)(visit_handle*);
    int (*mVisIt_MeshMetaData_setName)(visit_handle,const char*);
    int (*mVisIt_MeshMetaData_setMeshType)(visit_handle,int);
    int (*mVisIt_MeshMetaData_setTopologicalDimension)(visit_handle,int);
    int (*mVisIt_MeshMetaData_setSpatialDimension)(visit_handle,int);
    int (*mVisIt_MeshMetaData_setNumDomains)(visit_handle,int);
    int (*mVisIt_MeshMetaData_setXUnits)(visit_handle,const char*);
    int (*mVisIt_MeshMetaData_setYUnits)(visit_handle,const char*);
    int (*mVisIt_MeshMetaData_setZUnits)(visit_handle,const char*);
    int (*mVisIt_MeshMetaData_setXLabel)(visit_handle,const char*);
    int (*mVisIt_MeshMetaData_setYLabel)(visit_handle,const char*);
    int (*mVisIt_MeshMetaData_setZLabel)(visit_handle,const char*);
    
    int (*mVisIt_CommandMetaData_alloc)(visit_handle*);
    int (*mVisIt_CommandMetaData_setName)(visit_handle,const char*);
    
    int (*mVisIt_VariableMetaData_alloc)(visit_handle*);
    int (*mVisIt_VariableMetaData_setName)(visit_handle,const char*);
    int (*mVisIt_VariableMetaData_setMeshName)(visit_handle,const char*);
    int (*mVisIt_VariableMetaData_setUnits)(visit_handle,const char*);
    int (*mVisIt_VariableMetaData_setType)(visit_handle,int);
    int (*mVisIt_VariableMetaData_setCentering)(visit_handle,int);
    
    int (*mDomainList_setDomains)(visit_handle,int,visit_handle);
    int (*mVariableData_setDataI)(visit_handle,int,int,int,int*);
    int (*mVariableData_setDataF)(visit_handle,int,int,int,float*);
    int (*mVariableData_setDataD)(visit_handle,int,int,int,double*);
    int (*mVariableData_setDataC)(visit_handle,int,int,int,char*);
    
    int (*mVisIt_RectilinearMesh_alloc)(visit_handle*);
    int (*mVisIt_RectilinearMesh_setCoordsXY)(visit_handle,visit_handle,visit_handle);
    int (*mVisIt_RectilinearMesh_setCoordsXYZ)(visit_handle,visit_handle,visit_handle,visit_handle);
    int (*mVisIt_RectilinearMesh_setBaseIndex)(visit_handle,int[3]);

    int (*mVisIt_PointMesh_alloc)(visit_handle*);
    int (*mVisIt_PointMesh_setCoordsXY)(visit_handle,visit_handle,visit_handle);
    int (*mVisIt_PointMesh_setCoordsXYZ)(visit_handle,visit_handle,visit_handle,visit_handle);
#endif // VISIT_STATIC
};

#endif // LIBSIMLIBRARY_H
