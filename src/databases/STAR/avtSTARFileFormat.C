/*****************************************************************************
*
* Copyright (c) 2010, University of New Hampshire Computer Science Department
* All rights reserved.
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

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  File:        avtSTARFileFormat.C                                         //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// C++ includes
#include <string>
using std::string;

// visit includes
#include <avtSTARFileFormat.h>
#include <avtDatabaseMetaData.h>
#include <avtMeshMetaData.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <Expression.h>
#include <InvalidVariableException.h>

// vtk includes
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

// stardata includes
#include "MultiresFileReader.h"
#include "MultiresMultifileReader.h"
#include "MultiresGrid.h"
#include "ResolutionMap.h"

// local includes
#include "avtResolutionSelection.h"
#include "avtResolutionCommand.h"
#include "avtResolutionInfo.h"

#ifdef DO_TIMINGS
#include <StackTimer.h>
#endif

/* ========================================================================= */
/**
 *      Constructs a plugin and initializes the file passed in
 *      by reading the metadata header.
 **/

avtSTARFileFormat::avtSTARFileFormat(const char *filename)
    : avtMTMDFileFormat(filename), mFilename(filename)
{
    TRACE(filename);
    DEBUG("entered plugin for STAR: filename='%s'", filename);
    INFO("STAR Database loading file '%s'",splitPathName(mFilename)[1].c_str());

    mDataManager = NULL;
    mMultiresGrid = NULL;
    mNumFiles = 0;

    string extension = splitFileName(filename)[1];

    if(extension == "mrd") {
        mDataManager = new MultiresFileReader(filename);
        mNumFiles = 1;
    }
    else if(extension == "mrm") {
        mDataManager = new MultiresMultifileReader(filename);
        mNumFiles = mDataManager->numFiles();
    }
    else {
        ERROR("Unknown file extension in file '%s'", filename);
        exit(-1);
    }

    int numResolutions = mDataManager->numResolutions();

    // set the current resolution to the lowest possible...
    mResolution = numResolutions - 1;

    // and for UMR data, set every chunk to the lowest possible resolution
    int xchunks   = mDataManager->numXchunks(mResolution);
    int ychunks   = mDataManager->numYchunks(mResolution);
    int zchunks   = mDataManager->numZchunks(mResolution);
    int numChunks = xchunks * ychunks * zchunks;

    mResolutionMap = new ResolutionMap(numChunks, numResolutions);
    mResolutionMap->loadUniformResMap(mResolution);
}

/* ========================================================================= */
/**
 *      Returns the number of files in the simulation.
 *      In future versions where we want interpolated, 
 *      or virtual, time steps, the number of files may not
 *      be equal to the number of timesteps.
 *
 *      For example, if the density directory had 2 files:
 *
 *          jcd0004.rr.000060.mrd  
 *          jcd0004.rr.000120.mrd
 *
 *      The number of time steps would be 120, for which
 *      only 2 were saved to disk, timestep 60 and timestep 120.
 **/

int avtSTARFileFormat::GetNTimesteps(void)
{
    TRACE();
    DEBUG("returning '%d' timesteps", mNumFiles);

    return mNumFiles;
}

/* ========================================================================= */

void avtSTARFileFormat::ActivateTimestep(int ts)
{
    TRACE();
    // not sure what to do here
    
    DEBUG("ActivateTimestep called for '%d'", ts);
}

/* ========================================================================= */

void avtSTARFileFormat::GetCycles(vector<int>& cycles)
{
    TRACE(cycles);

    if(mDataManager != NULL) {
        cycles = mDataManager->timeStepList();
    }
}

/* ========================================================================= */

void avtSTARFileFormat::GetTimes(vector<double>& times)
{
    TRACE(times);

    if(mDataManager != NULL) {
        vector<int> timesteps = mDataManager->timeStepList();

        times.resize(timesteps.size());

        for(unsigned int i=0; i<timesteps.size(); i++) 
            times[i] = (float)timesteps[i];
    }
}

/* ========================================================================= */
/**
 *    When VisIt is done focusing on a particular timestep, it asks that
 *    timestep to free up any resources (memory, file descriptors) that
 *    it has associated with it.  This method is the mechanism for doing
 *    that.
 *
 *    I think this function is called to free up stuff *in between*
 *    timesteps.  We need to keep the current grid and metadata
 *    object files alive between timesteps but delete any
 *    active data.
 **/

void avtSTARFileFormat::FreeUpResources(void)
{
    TRACE();

    /*
    DEBUG("Free memory for var=%s, time=%d",
            mCurrentVarName.c_str(), mCurrentTimeState);

    if(mDataManager != NULL) 
        mDataManager->freeRawDataMemory(mCurrentVarName, mCurrentTimeState);
    */
}

/* ========================================================================= */
/**
 *    This needs to be false for a multiresolution database.
 *    When changing the resolution, if visit were to cache 
 *    the variables (i.e., if this method returned true), then
 *    it will not know that the resolution had been changed.  
 *
 *    By returning false, Visit will not cache variables, so
 *    that when the resolution is changed, this object will
 *    be called (@see GetVar()) to re-get the data at the 
 *    lower resolution.
 **/

bool avtSTARFileFormat::CanCacheVariable(const char *varname)
{
    TRACE(varname);

    return false;
}

/* ========================================================================= */
/**
 *      Tells Visit whether to call PopulateDatabaseMetaData for
 *      each time step or not.
 **/

bool avtSTARFileFormat::HasInvariantMetaData(void) const
{ 
    TRACE();
    DEBUG("entered, returning false");

    // FIXME if the chunking scheme varies, this needs to return true
    return false; 
}

/* ========================================================================= */
/**
 *    This database meta-data object is like a table of contents for the
 *    file.  By populating it, you are telling the rest of VisIt what
 *    information it can request from you.
 **/

void avtSTARFileFormat::
PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    TRACE(md,timeState);
    NOTNULL(mDataManager);
    BOUNDS_CHECK(mResolution, 0, mDataManager->numResolutions());
    DEBUG("md=%p, timeState=%d", md, timeState);

    const double* extents = NULL;       // not sure why this is NULL
    int numBlocks = mDataManager->numXchunks(mResolution) *
                    mDataManager->numYchunks(mResolution) *
                    mDataManager->numZchunks(mResolution);

    int blockOrigin = 0;                // must be 0 for Single Domain?
    int numDims = 3;
    int topologyDimension = 3;
    const char* meshname = "ggcm";

    AddMeshToMetaData(md,
                      meshname,
                      AVT_RECTILINEAR_MESH,
                      extents,
                      numBlocks,
                      blockOrigin,
                      numDims,
                      topologyDimension);

    // assuming there is only one mesh...
    avtMeshMetaData& mmd = md->GetMeshes(0);
    mmd.LODs = mDataManager->numResolutions() - 1;
    debug4 << "Telling VisIt we have " << mmd.LODs << " levels of detail.\n";

    for(int i=0; i<mDataManager->numVariables(); i++) {
        string varname = mDataManager->variableNameAt(i);

        if(mDataManager->isVectorComponent(varname)) {
            DEBUG("Adding vector component '%s'", varname.c_str());

            // following set of commands equivalent to AddScalarVarToMetaData()
            avtScalarMetaData *smd = new avtScalarMetaData();
            smd->name = varname.c_str();
            smd->meshName = meshname;
            smd->centering = AVT_NODECENT;
            // end equivalent to calling AddScalarVarToMetaData()
            
            smd->hideFromGUI = true; // dont put vector components on menu
            md->Add(smd);
        }
        else if(mDataManager->isScalar(varname)) {
            DEBUG("Adding scalar '%s'", varname.c_str());
            AddScalarVarToMetaData(md, varname.c_str(), meshname, AVT_NODECENT);

            if(mDataManager->hasMinVal() && mDataManager->hasMaxVal()) {
                double minmax[2];
                minmax[0] = mDataManager->minVal();
                minmax[1] = mDataManager->maxVal();
                md->SetExtents(varname.c_str(), minmax);
            }
        }
        else if(mDataManager->isVector(varname)) {
            DEBUG("Adding vector '%s'", varname.c_str());
            AddVectorVarToMetaData(md, varname.c_str(), meshname, AVT_NODECENT, 
                               numDims);
        }
        else if(mDataManager->isTensor(varname)) {
            ERROR("Tensors not supported yet");
        }
    }

    for(int i=0; i<mDataManager->numVectorExpressions(); i++) {
        string expression = mDataManager->vectorExpressionAt(i);

        vector<string> components = split(expression, " \t\n,{};");
        string name = components[0].erase(components[0].length()-1);

        Expression vectorvar;
        vectorvar.SetName(name);
        vectorvar.SetDefinition(expression);
        vectorvar.SetType(Expression::VectorMeshVar);

        DEBUG("adding vector '%s' with expression '%s'", 
                    name.c_str(), expression.c_str());
        md->AddExpression(&vectorvar);
    }

    vector<int> cycles;
    GetCycles(cycles);
    md->SetCycles(cycles);
    //md->SetCyclesAreAccurate(true);

    vector<double> times;
    GetTimes(times);
    md->SetTimes(times);
    //md->SetTimesAreAccurate(true);
    md->SetTemporalExtents(0, GetNTimesteps()-1);

    addDerivedVariables(md, timeState);
}

/* ========================================================================= */
/**
 *    Gets the mesh associated with this file.  The mesh is returned as a
 *    derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
 *    vtkUnstructuredGrid, etc).
 *
 *    @param timestate   The index of the timestate.  If GetNTimesteps 
 *                       returned 'N' time steps, this is guaranteed 
 *                       to be between 0 and N-1.
 *    @param domain      The index of the domain.  If there are NDomains, 
 *                       this value is guaranteed to be between 0 and 
 *                       NDomains-1, regardless of block origin.
 *    @param meshname    The name of the mesh of interest.  This can be 
 *                       ignored if there is only one mesh.
 **/

vtkDataSet* avtSTARFileFormat::
GetMesh(int timestate, int domain, const char *meshname)
{
    TRACE(timestate, domain, meshname);
    NOTNULL(mDataManager);

    if(mMultiresGrid == NULL) {
        string gridfile = mDataManager->gridFilename();

        bool gridFileExists = fileExists(gridfile.c_str());
        if(!gridFileExists) {
            ERROR("Grid file '%s' does not exist.  Creating a default grid.",
                    gridfile.c_str());
        }

        if(gridfile == "" || !gridFileExists) {
            int fullwidth = mDataManager->width(0);
            int fullheight = mDataManager->height(0);
            int fulldepth = mDataManager->depth(0);
            mMultiresGrid = new MultiresGrid(fullwidth, fullheight, fulldepth);
        }
        else {
            mMultiresGrid = new MultiresGrid(gridfile.c_str());
        }
        
        mMultiresGrid->generateLowRes(mDataManager->numResolutions());
    }
    
    int resolution = mResolutionMap->resolutionForChunk(domain);

    MultiresGrid* grid = mMultiresGrid->atResolution(resolution);

    int width   = mDataManager->chunkwidth(resolution);
    int height  = mDataManager->chunkheight(resolution);
    int depth   = mDataManager->chunkdepth(resolution);
    int xchunks = mDataManager->numXchunks(resolution);
    int ychunks = mDataManager->numYchunks(resolution);
    int zchunks = mDataManager->numZchunks(resolution);

    const float* xcoords = grid->xcoordsChunk(domain, xchunks,ychunks,zchunks);
    const float* ycoords = grid->ycoordsChunk(domain, xchunks,ychunks,zchunks);
    const float* zcoords = grid->zcoordsChunk(domain, xchunks,ychunks,zchunks);

    bool DO_NOT_DELETE = true;
    vtkFloatArray* gridvals[3];

    gridvals[0] = vtkFloatArray::New();   // x component
    gridvals[1] = vtkFloatArray::New();   // y component
    gridvals[2] = vtkFloatArray::New();   // z component
    gridvals[0]->SetNumberOfComponents(1);
    gridvals[1]->SetNumberOfComponents(1);
    gridvals[2]->SetNumberOfComponents(1);
    gridvals[0]->SetArray((float*)xcoords, width, DO_NOT_DELETE);
    gridvals[1]->SetArray((float*)ycoords, height, DO_NOT_DELETE);
    gridvals[2]->SetArray((float*)zcoords, depth, DO_NOT_DELETE);

    vtkRectilinearGrid* mesh = vtkRectilinearGrid::New();
    mesh->SetXCoordinates(gridvals[0]);
    mesh->SetYCoordinates(gridvals[1]);
    mesh->SetZCoordinates(gridvals[2]);
    mesh->SetExtent(0, width-1, 0, height-1, 0, depth-1);
    mesh->SetWholeExtent(0, width-1, 0, height-1, 0, depth-1);

    DEBUG("size=[%d,%d,%d]", width, height, depth);

    return mesh;
}

/* ========================================================================= */
/**
 *      Gets a scalar variable associated with this file.  Although VTK has
 *      support for many different types, the best bet is vtkFloatArray, since
 *      that is supported everywhere through VisIt.
 *
 *      @param timestate  The index of the timestate.  If GetNTimesteps 
 *                        returned 'N' time steps, this is guaranteed 
 *                        to be between 0 and N-1.
 *      @param domain     The index of the domain.  If there are NDomains, 
 *                        this value is guaranteed to be between 0 
 *                        and NDomains-1, regardless of block origin.
 *      @param varname    The name of the variable requested.
 **/

vtkDataArray* avtSTARFileFormat::
GetVar(int timestate, int domain, const char *varname)
{
    // {{{ preconditions && debug
    TRACE(timestate,domain,varname);
    NOTNULL(mDataManager);
    NOTNULL(mResolutionMap);
    BOUNDS_CHECK(mResolution,0,mDataManager->numResolutions());
    DEBUG("timestate=%d, domain=%d, varname='%s'", timestate, domain, varname);
    DEBUG("current uniform resolution is '%d'", mResolution);
    DEBUG("domain='%d', resolution='%d', size=[%d,%d,%d]", 
          domain, mResolutionMap->resolutionForChunk(domain),
          mDataManager->chunkwidth(mResolutionMap->resolutionForChunk(domain)),
          mDataManager->chunkheight(mResolutionMap->resolutionForChunk(domain)),
          mDataManager->chunkdepth(mResolutionMap->resolutionForChunk(domain)));
    // }}}

    const float* data = NULL;
    mCurrentVarName = varname;
    mCurrentTimeState = timestate;

    int resolution = mResolutionMap->resolutionForChunk(domain);

    // get the size for a domain (chunk)
    int width  = mDataManager->chunkwidth(resolution);
    int height = mDataManager->chunkheight(resolution);
    int depth  = mDataManager->chunkdepth(resolution);
    data       = mDataManager->rawData(varname, resolution, timestate, domain);

    bool DO_NOT_DELETE = true;
    vtkFloatArray* vtkdata = vtkFloatArray::New();
    vtkdata->SetName(varname);
    vtkdata->SetNumberOfComponents(1);
    vtkdata->SetNumberOfTuples(width*height*depth);
    vtkdata->SetArray((float*)data, width*height*depth, DO_NOT_DELETE);

    return vtkdata;
}

/* ========================================================================= */
/**     
 *      Gets a vector variable associated with this file.  Although VTK has
 *      support for many different types, the best bet is vtkFloatArray, since
 *      that is supported everywhere through VisIt.
 *
 *      @param timestate  The index of the timestate.  If GetNTimesteps 
 *                        returned 'N' time steps, this is guaranteed 
 *                        to be between 0 and N-1.
 *      @param domain     The index of the domain.  If there are NDomains, 
 *                        this value is guaranteed to be between 0 
 *                        and NDomains-1, regardless of block origin.
 *      @param varname    The name of the variable requested.
 *
 **/

vtkDataArray* avtSTARFileFormat::
GetVectorVar(int timestate, int domain, const char *varname)
{
    TRACE(timestate,domain,varname);
    NOTNULL(mDataManager);
    BOUNDS_CHECK(mResolution,0,mDataManager->numResolutions());
    DEBUG("timestate=%d, domain=%d, varname='%s'", timestate, domain, varname);
    DEBUG("current resolution is '%d'", mResolution);

    mCurrentVarName = varname;
    mCurrentTimeState = timestate;

    int resolution = mResolutionMap->resolutionForChunk(domain);

    int width    = mDataManager->chunkwidth(resolution);
    int height   = mDataManager->chunkheight(resolution);
    int depth    = mDataManager->chunkdepth(resolution);
    const float* data = 
        mDataManager->rawData(varname, resolution, timestate, domain);

    bool DO_NOT_DELETE = true;
    vtkFloatArray* vtkdata = vtkFloatArray::New();
    vtkdata->SetName(varname);
    vtkdata->SetNumberOfComponents(3);
    vtkdata->SetNumberOfTuples(width*height*depth);
    vtkdata->SetArray((float*)data, width*height*depth, DO_NOT_DELETE);

    DEBUG("size was [%d,%d,%d]", width, height, depth);

    return vtkdata;
}

/* ========================================================================= */
/**
 *      This is part of the visit 'contract' code to receive 
 *      messages from the MultiresControl plugin.
 *
 *      This method listens for messages to change the resolution.
 *      The sender is, in this case, the MultiresControl plugin. 
 *
 *      See method avtMultiresControlFilter::ModifyContract.
 *
 **/

void avtSTARFileFormat::
RegisterDataSelections(const vector<avtDataSelection_p>& sels, 
                       vector<bool>*                     selectionsApplied)
{
    TRACE(sels, selectionsApplied);
    DEBUG("entered, sels.size() is '%d'", (int)sels.size());

    for(unsigned int i=0; i<sels.size(); i++) {
        string type = sels[i]->GetType();
        DEBUG("type[%d]='%s'", i, type.c_str());
        DEBUG("selectionsApplied[%d]='%s'", 
                i, (*selectionsApplied)[i]?"true":"false");

        if(type == "avtResolutionSelection") {
            avtResolutionSelection* res = (avtResolutionSelection*) *sels[i];
            DEBUG("changing resolution to '%d'", res->resolution());
            changeResolution(res->resolution());
            // FIXME when changing the resolution need to change the
            // chunking scheme and the resolution map possibly
            (*selectionsApplied)[i] = true;
        }
        else if(type == "avtResolutionCommand") {
            avtResolutionCommand* cmd = (avtResolutionCommand*) *sels[i];
            DEBUG("got command '%s'", cmd->command().c_str());

            processCommand(cmd->command());
        }
        else if(type == "avtResolutionInfo") {
            avtResolutionInfo* info = (avtResolutionInfo*) *sels[i];
            DEBUG("got info, filling it in with sizes");

            for(int i=0; i<mDataManager->numResolutions(); i++) {
                int width  = mDataManager->width(i);
                int height = mDataManager->height(i);
                int depth  = mDataManager->depth(i);
                info->addSize(width, height, depth, i);
            }
        }
        //else {
        //    WARNING("This DB ignores avtDataSelection of type '%s'", 
        //            type.c_str());
        //}
    }
}

/* ========================================================================= */
/**
 *      Sets the current resolution the user specified choice.
 **/

void avtSTARFileFormat::changeResolution(int resolution)
{
    TRACE(nodeId);

    if(mDataManager != NULL) {
        int maxResolution = mDataManager->numResolutions()-1;

        if(mResolution >=0 && resolution <= maxResolution) {
            mResolution = resolution;

            mResolutionMap->loadUniformResMap(resolution);
        }
        else {
            WARNING("Cannot change to resolution '%d', available "
                    "resolutions are [0...%d]", resolution, maxResolution);
        }
    }
}
/* ========================================================================= */
/**
 *      Commands must begin with a '>' character.
 *
 *      Current commands are:
 *          norender <domain>
 *          useAR [true|false]
 *          help
 **/

void avtSTARFileFormat::processCommand(string command)
{
    TRACE(nodeId);

    if(command.length()>=1 && command[0]=='>') {
        vector<string> tokens = split(command, " \t\n=>");

        if(tokens.size() >= 1 ) {
            if(tokens[0] == "norender" && tokens.size() >= 2) {
                WARNING("Command 'norender' not supported.... yet");
            }
            if(tokens[0] == "useAR" && tokens.size() >= 2) {
                if(tokens[1] == "true")
                    mResolutionMap->loadRandomMap();
                else
                    mResolutionMap->loadUniformResMap(mResolution);
            }
            if(tokens[0] == "help") {
                fprintf(stderr,"\nCommands must begin with '>'. ");
                fprintf(stderr,"Commands are:\n");
                fprintf(stderr,"    > norender <domain>\n");
                fprintf(stderr,"    > useAR=[true|false]\n");
                fprintf(stderr,"    > help\n\n");
            }
            else {
                WARNING("Unknown command '%s'", tokens[0].c_str());
            }
        }
    }
}

/* ========================================================================= */
/**
 *      Adds derived variables to the menu in visit using the
 *      expression mechanism in visit.  Derived variables are
 *      calculated at runtime from the 11 primary variables
 *      (pp,rr,bx,by,bz,vx,vy,vz,xjx,xjy,xjz).  It's slow
 *      but handy.  
 *
 *      A global table of derived variables is used to 
 *      define everything in a single place.  @See DerivedVariable,
 *      @see variableTable.  This method iterates through
 *      each entry in the table and adds them to the
 *      expression metadata.
 **/

#include "DerivedVariables.h"

void avtSTARFileFormat::
addDerivedVariables(avtDatabaseMetaData *md, int timeState)
{
    TRACE(md, timestate);
    NOTNULL(mDataManager);

    // if all 11 primary variables there, add the derived variables too
    if(mDataManager->numVariables()==11) {
        Expression e;
        string submenu = "=Derived Variables /";

        int numEntries = sizeof DerivedVariableTable/
                         sizeof DerivedVariableTable[0];

        for(int i=0; i<numEntries; i++) {
            e.SetName(submenu + DerivedVariableTable[i].name);
            e.SetDefinition(DerivedVariableTable[i].expression);
            e.SetType(DerivedVariableTable[i].expressionType);

            DEBUG("adding derived variable '%s'='%s'",
                    DerivedVariableTable[i].name.c_str(),
                    DerivedVariableTable[i].expression.c_str());

            md->AddExpression(&e);
        }
    }
}

/* ========================================================================= */

avtSTARFileFormat::~avtSTARFileFormat()
{
    DEBUG("Destroying plugin");
    DEBUG("deleting MultiresFileReader, MultiresGrid memory");

    delete mDataManager;
    delete mMultiresGrid;

    mDataManager = NULL;
    mMultiresGrid = NULL;
}

/* ========================================================================= */
