/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             Database_VTK.C                                //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <vtkAppendFilter.h>
#include <vtkContourFilter.h>
#include <vtkCutter.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <SiloObjLib.h>
#include <Database_VTK.h>


// ****************************************************************************
//  Method: Database_VTK constructor
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

Database_VTK::Database_VTK()
{
    nVisitFiles = 0;
    visitFiles  = NULL;
    nMeshFiles   = 0;
    meshFiles    = NULL;
    nStates      = 0;
    nStateFiles  = NULL;
    stateFiles   = NULL;
    stem         = NULL;
    currentState = 0;
}


// ****************************************************************************
//  Method: Database_VTK destructor
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

Database_VTK::~Database_VTK()
{
    if (visitFiles != NULL)
    {
        for (int i = 0 ; i < nVisitFiles ; i++)
        {
            if (visitFiles[i] != NULL)
            {
                delete [] visitFiles[i];
            }
        }
        delete [] visitFiles;
    }
    if (meshFiles != NULL)
    {
        for (int i = 0 ; i < nMeshFiles ; i++)
        {
            if (meshFiles[i] != NULL)
            {
                delete [] meshFiles[i];
            }
        }
        delete [] meshFiles;
    }
    if (stateFiles != NULL)
    {
        for (int i = 0 ; i < nStates ; i++)
        {
            if (stateFiles[i] != NULL)
            {
                for (int j = 0 ; j < nStateFiles[i] ; j++)
                {
                    if (stateFiles[i][j] != NULL)
                    { 
                        delete [] stateFiles[i][j];
                    }
                }
                delete [] stateFiles[i];
            }
        }
        delete [] stateFiles;
    }
    if (nStateFiles != NULL)
    {
        delete [] nStateFiles;
    }
    if (states != NULL)
    {
        for (int i = 0 ; i < nStates ; i++)
        {
            if (states[i] != NULL)
            {
                delete [] states[i];
            }
        }
        delete [] states;
    }
    if (stem != NULL)
    {
        delete [] stem;
    }
}


// ****************************************************************************
//  Method: Database_VTK::Read
//
//  Purpose:
//      Takes a single SiloObj file (presumably the .visit) to read.
//
//  Arguments:
//      visit    A single filename.
//
//  Programmer:  Hank Childs
//  Creation:    September 8, 2000
//
// ****************************************************************************

void
Database_VTK::Read(const char *visit)
{
    Read(1, &visit);
}


// ****************************************************************************
//  Method: Database_VTK::Read
//
//  Purpose:
//      Reads in all of the SILO objects from the files given.
//
//  Arguments:
//      files   A list of files.
//      nFiles  The size of files.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
Database_VTK::Read(int nFiles, const char * const *files)
{
    if (nMeshFiles != 0)
    {
        // HANK
        cerr << "Ignoring specification of new files." << endl;
        return;
    }

    if (nFiles == 1 && strstr(files[0], ".visit") != NULL)
    {
        ReadVisit(files[0]);
    }
    else
    {
        ClassifyFiles(nFiles, files);
    }

    meshTOC.Read(nMeshFiles, meshFiles);

    stateTOC = new StateTableOfContents[nStates];
    for (int i = 0 ; i < nStates ; i++)
    {
        stateTOC[i].Read(nStateFiles[i], stateFiles[i]); 
    }
}


// ****************************************************************************
//  Method: Database_VTK::ReadVisit
//
//  Purpose:
//      Opens the visit file and reads in the names of the mesh and state
//      files.
//
//  Arguments:
//      visit_file     The name of the visit file.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2000
//
// ****************************************************************************

void
Database_VTK::ReadVisit(const char *visit_file)
{
    int  i, j;

    //
    // Silo has a real problem with const -- what to do?
    //
    char  *mfile = const_cast<char *>(visit_file);
    DBfile *dbfile = DBOpen(mfile, DB_UNKNOWN, DB_READ);
    if (dbfile == NULL)
    {
        cerr << "Invalid file \"" << visit_file << "\"." << endl;
        return;
    }

    if (! DBInqVarExists(dbfile, "_files"))
    {
        cerr << "Invalid visit file." << endl;
        return;
    }
   
    //
    // Determine the directory the .visit is lying in.
    //
    char      *p = mfile, *q = NULL;
    bool       hasDir = false;
    while ( (q=strstr(p, "/")) != NULL )
    {
        hasDir = true;
        p = q+1;
    }
    char       visitDir[LONG_STRING];
    strncpy(visitDir, mfile, p-mfile);
    visitDir[p-mfile] = '\0';

    char      *list = (char *) DBGetVar(dbfile, "_files");
    int       nMesh;
    char    **meshes;
    int       nStates;
    int      *nFiles;
    char   ***states;
    ParseVisitFileList(list, nMesh, meshes, nStates, nFiles, states);

    //
    // Put the prepended path in front of all of the file names.
    //
    if (hasDir)
    {
        for (i = 0 ; i < nMesh ; i++)
        {
            char  str[LONG_STRING];
            sprintf(str, "%s%s", visitDir, meshes[i]);
            delete [] meshes[i];
            meshes[i] = CXX_strdup(str);
        }
        for (i = 0 ; i < nStates ; i++)
        {
            for (j = 0 ; j < nFiles[i] ; j++)
            {
                char  str[LONG_STRING];
                sprintf(str, "%s%s", visitDir, states[i][j]);
                delete [] states[i][j];
                states[i][j] = CXX_strdup(str);
            }
        }
    }

    //
    // The stem is needed by some of the lower level routines.  We only need
    // the visit file, a mesh file, and a state file to determine the stem.
    //
    char *stemlist[3];
    stemlist[0] = mfile;
    stemlist[1] = meshes[0];
    stemlist[2] = states[0][0];
    DetermineStem(stemlist, 3);

    nVisitFiles          = 1;
    visitFiles           = new char*[nVisitFiles];
    visitFiles[0]        = CXX_strdup(visit_file);

    nMeshFiles           = nMesh;
    meshFiles            = new char*[nMeshFiles];
    for (i = 0 ; i < nMeshFiles ; i++)
    {
        meshFiles[i] = CXX_strdup(meshes[i]);
    }

    //
    // Its easier to use OrganizeStateFiles than duplicate its functionality.
    //
    int  numStateFiles = 0;
    for (i = 0 ; i < nStates ; i++)
    {
        numStateFiles += nFiles[i];
    }
    char **temp_states = new char*[numStateFiles];
    int    count = 0;
    for (i = 0 ; i < nStates ; i++)
    {
        for (j = 0 ; j < nFiles[i] ; j++)
        {
            temp_states[count] = states[i][j];
            count++;
        }
    }
    OrganizeStateFiles(count, temp_states);

    delete [] temp_states;
    for (i = 0 ; i < nMesh ; i++)
    {
        delete [] meshes[i];
    }
    delete [] meshes;

    for (i = 0 ; i < nStates ; i++)
    {
        for (j = 0 ; j < nFiles[i] ; j++)
        {
            delete [] states[i][j];
        }
        delete [] states[i];
    }
    delete [] nFiles;
    delete [] states;

    //
    // list was created with a malloc, so free it with a "free".
    //
    free(list);

    DBClose(dbfile);
}


// ****************************************************************************
//  Method: Database_VTK::ClassifyFiles
//
//  Purpose:
//      Splits the input file list into groups of visit files, mesh files,
//      and state files.
//
//  Arguments:
//      files   A list of files.
//      nFiles  The number of files.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
Database_VTK::ClassifyFiles(int nFiles, const char * const *files)
{
    if (nFiles <= 0)
    {
        //
        // Degenerative case.
        //
        cerr << "Not given any files." << endl;
        // throw
        return;
    }

    //
    // Create arrays to put the files into.  Overestimate by a factor of
    // 3, but the file list won't be larger than a couple 1000, so this
    // isn't too gross.
    //
    visitFiles           = new char*[nFiles];
    meshFiles            = new char*[nFiles];
    char **tmpStateFiles = new char*[nFiles];
    int   nTmpStateFiles = 0;

    DetermineStem(files, nFiles);

    for (int i = 0 ; i < nFiles ; i++)
    {
        if (strncmp(files[i], stem, stemLength) != 0)
        {
            cerr << "File num is " << i << endl;
            cerr << "Files do not all have the same stem." << endl;
            cerr << "Identified stem as " << stem << ", but does not match "
                 << "with " << files[i] << endl;
            // throw
            return;
        }
       
        if (strstr(files[i]+stemLength, "visit"))
        {
             visitFiles[nVisitFiles] = CXX_strdup(files[i]);
             nVisitFiles++;
        }
        else if (strstr(files[i]+stemLength, "mesh"))
        {
             meshFiles[nMeshFiles] = CXX_strdup(files[i]);
             nMeshFiles++;
        }
        else 
        {
             tmpStateFiles[nTmpStateFiles] = CXX_strdup(files[i]);
             nTmpStateFiles++;
        }

    }   // End 'for' over all the files in the input list.

    OrganizeStateFiles(nTmpStateFiles, tmpStateFiles);
   
    //
    // Clean up memory.
    //
    delete [] tmpStateFiles;
}


// ****************************************************************************
//  Method: Database_VTK::OrganizeStateFiles
//
//  Purpose:
//      Takes a list of state files and determines what the states are and 
//      sorts them into groups by state.
//
//  Arguments:
//      size      number of elements in the list.
//      list      An array of strings where each string is guaranteed to be
//                a state file and each string is a copy that can be stolen
//                without duping.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
Database_VTK::OrganizeStateFiles(int size, const char * const *list)
{
    int   i, j;
    int  *stateIndex = new int[size];
    char  stemState[LONG_STRING];
    bool  matchedPreviousState;

    //
    // Potentially overestimate the number of states.
    //
    states      = new char*[size];
    nStateFiles = new int[size];
    for (i = 0 ; i < size ; i++)
    {
        nStateFiles[i] = 0;
    }

    for (i = 0 ; i < size ; i++)
    {
        matchedPreviousState = false;
        for (j = 0 ; j < nStates ; j++)
        {
            //
            // Create a string that has the stem and the state.
            //
            sprintf(stemState, "%s%s.", stem, states[j]);
            if (strncmp(stemState, list[i], strlen(stemState)) == 0)
            {
                //
                // This file is a member of a pre-existing state.
                //
                matchedPreviousState = true;
                stateIndex[i] = j;
                nStateFiles[j]++;
                break;
            }
        }
        if (matchedPreviousState == false)
        {
            //
            // This file belongs to the new state.
            //
            stateIndex[i] = nStates;

            //
            // Create a new state.
            //
            nStateFiles[nStates]++;
           
            //
            // Assumptions state that every element of the list has the stem
            // to start with.  The state part stops when a '.' is found.
            //
            states[nStates] = CXX_strdup(list[i] + strlen(stem));
            char *p = strstr(states[nStates], ".");
            if (p == NULL)
            {
                cerr << "Could not identify state for " << list[i] << endl;
                // throw
                return;
            }
            int oneToNullOut = p - states[nStates];
            states[nStates][oneToNullOut] = '\0';

            nStates++;
        }
    }
   
    //
    // Now set up the stateFiles data member.
    //
    stateFiles = new char**[nStates];
    for (i = 0 ; i < nStates ; i++)
    {
        stateFiles[i] = new char*[nStateFiles[i]];
        int  count = 0;
        for (j = 0 ; j < size ; j++)
        {
            if (stateIndex[j] == i)
            {
                stateFiles[i][count] = CXX_strdup(list[j]);
                count++;
            }
        }
    }

    //
    // Clean up memory
    //
    delete [] stateIndex;
}


// ****************************************************************************
//  Method: Database_VTK::DetermineStem
//
//  Purpose:
//      Determines the stem from the arguments.
//
//  Arguments:
//      files       A list of files.
//      nFiles      The number of files in `files'.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2000
//
// ****************************************************************************

void
Database_VTK::DetermineStem(const char * const * files, int nFiles)
{
    stemLength = LongestCommonPrefixLength(files, nFiles);
    char str[LONG_STRING];
    strncpy(str, files[0], stemLength);
    str[stemLength] = '\0';
    stem = CXX_strdup(str);
}



// ****************************************************************************
//  Method: Database_VTK::PrintSelf
//
//  Purpose:
//      Prints out the Table of Contents object.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
Database_VTK::PrintSelf()
{
    int   i, j;

    cerr << "Visit Files: " << endl;
    for (i = 0 ; i < nVisitFiles ; i++)
    {
        cerr << "\t" << visitFiles[i] << endl;
    }

    cerr << "Mesh Files: " << endl;
    for (i = 0 ; i < nMeshFiles ; i++)
    {
        cerr << "\t" << meshFiles[i] << endl;
    }

    cerr << "Num states: " << nStates << endl;
    for (i = 0 ; i < nStates ; i++)
    {
        cerr << "\tState:    " << states[i] << endl;
        cerr << "\tNumFiles: " << nStateFiles[i] << endl;
        for (j = 0 ; j < nStateFiles[i] ; j++)
        {
            cerr << "\t\t" << stateFiles[i][j] << endl;
        }
    }
}


// ****************************************************************************
//  Method: Database_VTK::GetDataSet
//
//  Purpose:
//      Finds the domains that satisfy the linear equation specified by
//      params and solution and makes a data set out of those domains.
//
//  Arguments:
//      list     A list of the domains to get for this processor.
//      listN    The size of list.
//      varname  The name of the variable to get.
//
//  Returns:    The data set at the specified domains.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Mar  4 09:19:17 PST 2000
//    I added support for rectilinear and curvilinear meshes.
//
// ****************************************************************************

vtkDataSet *
Database_VTK::GetDataSet(int *list, int listN, const char *varname)
{
    int   i;

    if (listN == 0 || list == NULL)
    {
        // throw
        return NULL;
    }

    vtkDataSet **ds = new vtkDataSet*[listN];

    //
    // Let all of the TOCs get the actual VTK data.
    //
    const char  *meshName = stateTOC[currentState].GetMeshName(varname);
    if (meshName == NULL)
    {
        cerr << "Invalid variable: \"" << varname << "\"." << endl;
        return NULL;
    }

    MESH_TYPE_e  meshType = stateTOC[currentState].GetMeshType(meshName);
    switch (meshType)
    {
      case CURVILINEAR:
        CreateCurvilinearMesh(varname, meshName, list, listN, ds);
        break;
      case RECTILINEAR:
        CreateRectilinearMesh(varname, meshName, list, listN, ds);
        break;
      case UNSTRUCTURED:
        CreateUnstructuredMesh(varname, meshName, list, listN, ds);
        break;
      default:
        cerr << "Unable to resolve mesh type " << meshType << endl;
        // throw
        return NULL;
        /* NOTREACHED */ break; 
    }

    //
    // Create one big dataset.
    //
    vtkAppendFilter  *af = vtkAppendFilter::New();
    for (i = 0 ; i < listN ; i++)
    {
        af->AddInput(ds[i]);
    }

    //
    // Clean up memory
    //
    delete [] ds;

    return af->GetOutput();
}


// ****************************************************************************
//  Method: Database_VTK::CreateUnstructuredMesh
//
//  Purpose:
//      Creates an vtkUnstructuredGrid by creating the grid components 
//      (vtkScalars, vtkPoints) and letting the lower levels populate
//      the components.
//
//  Arguments:
//       varname   The name of the variable of interest.
//       meshname  The name of the mesh that goes with the variable.
//       list      The domains of interest.
//       listN     The number of domains in list.
//       ds        The location to put the created datasets for each domain.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Sep 15 18:16:40 PDT 2000
//    Add support for cell centered data.
//
//    Hank Childs, Thu Oct 19 14:39:06 PDT 2000
//    Clean up memory leaks.
//
// ****************************************************************************

void
Database_VTK::CreateUnstructuredMesh(const char *varname, const char *meshname,
                                     const int *list, int listN, 
                                     vtkDataSet **ds)
{
    int   i;

    int   centering = stateTOC[currentState].GetCentering(varname);

    vtkPoints            **points  = new vtkPoints*[listN];
    vtkScalars           **scalars = new vtkScalars*[listN];
    vtkUnstructuredGrid  **ugrid   = new vtkUnstructuredGrid*[listN];
    for (i = 0 ; i < listN ; i++)
    {
        points[i]  = vtkPoints::New();
        scalars[i] = vtkScalars::New();
        ugrid[i]   = vtkUnstructuredGrid::New();
        ugrid[i]->SetPoints(points[i]);
        points[i]->Delete();
        if (centering == DB_NODECENT)
        {
            ugrid[i]->GetPointData()->SetScalars(scalars[i]);
        }
        else
        {
            ugrid[i]->GetCellData()->SetScalars(scalars[i]);
        }
        scalars[i]->Delete();
    }

    stateTOC[currentState].GetVar(varname, list, listN, scalars);
    stateTOC[currentState].GetCoords(meshname, list, listN, ugrid);
    meshTOC.GetZones(meshname, list, listN, ugrid);

    for (i = 0 ; i < listN ; i++)
    {
        ds[i] = ugrid[i];
    }

    //
    // Clean up memory
    //
    delete [] points;
    delete [] scalars;
    delete [] ugrid;
}


// ****************************************************************************
//  Method: Database_VTK::CreateCurvilinearMesh
//
//  Purpose:
//      Creates an vtkStructuredGrid by creating the grid components 
//      (vtkScalars, vtkPoints) and letting the lower levels populate
//      the components.
//
//  Arguments:
//      varname    The name of the variable.
//      meshname   The name of the mesh.
//      list       The domains of interest.
//      listN      The number of domains in list.
//      ds         Holds the dataset we create.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2000
//
//  Modifications:
//      
//    Hank Childs, Mon Apr  3 15:13:33 PDT 2000
//    Change call to GetCoords to send in the grid instead of the points.
//
//    Hank Childs, Fri Sep 15 18:16:40 PDT 2000
//    Add support for cell centered data.
//
//    Hank Childs, Thu Oct 19 14:39:06 PDT 2000
//    Clean up memory leaks.
//
// ****************************************************************************

void
Database_VTK::CreateCurvilinearMesh(const char *varname, const char *meshname, 
                                    const int *list, int listN, 
                                    vtkDataSet **ds)
{
    int   i;

    int   centering = stateTOC[currentState].GetCentering(varname);

    vtkPoints          **points  = new vtkPoints*[listN];
    vtkScalars         **scalars = new vtkScalars*[listN];
    vtkStructuredGrid  **sgrid   = new vtkStructuredGrid*[listN];
    for (i = 0 ; i < listN ; i++)
    {
        points[i]  = vtkPoints::New();
        scalars[i] = vtkScalars::New();
        sgrid[i]   = vtkStructuredGrid::New();
        sgrid[i]->SetPoints(points[i]);
        points[i]->Delete();
        if (centering == DB_NODECENT)
        {
            sgrid[i]->GetPointData()->SetScalars(scalars[i]);
        }
        else
        {
            sgrid[i]->GetCellData()->SetScalars(scalars[i]);
        }
        scalars[i]->Delete();
    }

    stateTOC[currentState].GetVar(varname, list, listN, scalars);
    stateTOC[currentState].GetCoords(meshname, list, listN, sgrid);

    for (i = 0 ; i < listN ; i++)
    {
        ds[i] = sgrid[i];
    }

    //
    // Clean up memory
    //
    delete [] points;
    delete [] scalars;
    delete [] sgrid;
}


// ****************************************************************************
//  Method: Database_VTK::CreateRectilinearMesh
//
//  Purpose:
//      Creates an vtkRectilinearGrid by creating the grid components 
//      (vtkScalars, vtkPoints) and letting the lower levels populate
//      the components.
//
//  Arguments:
//       varname   The name of the variable of interest.
//       meshname  The name of the mesh that goes with the variable.
//       list      The domains of interest.
//       listN     The number of domains in list.
//       ds        The location to put the created datasets for each domain.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Sep 15 18:16:40 PDT 2000
//    Add support for cell centered data.
//
//    Hank Childs, Thu Oct 19 14:39:06 PDT 2000
//    Clean up memory leaks.
//
// ****************************************************************************

void
Database_VTK::CreateRectilinearMesh(const char *varname, const char *meshname,
                                    const int *list, int listN, 
                                    vtkDataSet **ds)
{
    int   i;

    int   centering = stateTOC[currentState].GetCentering(varname);

    vtkScalars          **scalars = new vtkScalars*[listN];
    vtkRectilinearGrid  **rgrid   = new vtkRectilinearGrid*[listN];
    for (i = 0 ; i < listN ; i++)
    {
        scalars[i] = vtkScalars::New();
        rgrid[i]   = vtkRectilinearGrid::New();
        if (centering == DB_NODECENT)
        {
            rgrid[i]->GetPointData()->SetScalars(scalars[i]);
        }
        else
        {
            rgrid[i]->GetCellData()->SetScalars(scalars[i]);
        }
        scalars[i]->Delete();
    }

    stateTOC[currentState].GetVar(varname, list, listN, scalars);
    stateTOC[currentState].GetCoords(meshname, list, listN, rgrid);

    for (i = 0 ; i < listN ; i++)
    {
        ds[i] = rgrid[i];
    }

    //
    // Clean up memory
    //
    delete [] scalars;
    delete [] rgrid;
}


// ****************************************************************************
//  Method: Database_VTK::GetSpacialMetaData
//
//  Purpose:
//      Gets the interval tree for the coordinates of the mesh associated with
//      the variable specified.
//
//  Arguments:
//      varname     The name of the variable whose mesh's coordinates we want.
//
//  Returns:    A pointer to a constant interval tree.
//
//  Programmer: Hank Childs
//  Creation:   April 12, 2000
//
// ****************************************************************************

const IntervalTree_VTK *
Database_VTK::GetSpacialMetaData(const char *varname)
{
    const char  *meshName = stateTOC[currentState].GetMeshName(varname);
    if (meshName == NULL)
    {
        // throw
        return NULL;
    }

    return stateTOC[currentState].GetMetaData(meshName);
}


// ****************************************************************************
//  Method: Database_VTK::GetVariableMetaData
//
//  Purpose:
//      Gets the interval tree for the variable specified.
//
//  Arguments:
//      varname     The name of the variable whose mesh's coordinates we want.
//
//  Returns:    A pointer to a constant interval tree.
//
//  Programmer: Hank Childs
//  Creation:   April 12, 2000
//
// ****************************************************************************

const IntervalTree_VTK *
Database_VTK::GetVariableMetaData(const char *varname)
{
    return stateTOC[currentState].GetMetaData(varname);
}


// ****************************************************************************
//  Method: Database_VTK::GetNDomains
//
//  Purpose:
//      Takes a variable name, determines which mesh is associated with that
//      domain, and determines how many domains there are in the mesh.
//
//  Arguments:
//      varname     The variable name.
//
//  Returns:        The number of domains in the mesh that varname is defined
//                  on.
//
//  Programmer:     Hank Childs
//  Creation:       August 10, 2000
//
// ****************************************************************************

int
Database_VTK::GetNDomains(const char *varname)
{
    const char  *meshName = stateTOC[currentState].GetMeshName(varname);
    if (meshName == NULL)
    {
        return -1;
    }
    return stateTOC[currentState].GetNDomains(meshName);
}


// ****************************************************************************
//  Method: Database_VTK::SetState
//
//  Purpose:
//      Sets the current state to be the argument.
//
//  Arguments:
//      s       The new state.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2000
//
// ****************************************************************************

void
Database_VTK::SetState(int s)
{
    if (s < 0 || s >= nStates)
    {
        cerr << "Invalid state " << s << endl;
        return;
    }

    currentState = s;
}


