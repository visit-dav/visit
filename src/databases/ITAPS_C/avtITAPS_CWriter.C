/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                            avtITAPS_CWriter.C                             //
// ************************************************************************* //

#include <avtITAPS_CWriter.h>
#include <avtITAPS_CUtility.h>

#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

#include <iMesh.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
//#include <vtkDelaunay3D.h>
#include <vtkPointData.h>
#include <vtkType.h>

#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;
using namespace avtITAPS_CUtility;

// ****************************************************************************
//  Method: avtITAPS_CWriter constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

avtITAPS_CWriter::avtITAPS_CWriter(DBOptionsAttributes *dbopts)
{
#if defined(ITAPS_MOAB)
    saveOptions = "MOAB";
    formatExtension = "h5m";
    for (int i = 0; dbopts != 0 && i < dbopts->GetNumberOfOptions(); ++i)
    {
        if (dbopts->GetName(i) == "Format")
        {
            switch (dbopts->GetEnum("Format"))
            {
                case 0: break; // default case 
                case 1: saveOptions = "EXODUS"; formatExtension = "exoII"; break;
                case 2: saveOptions = "VTK";    formatExtension = "vtk"; break;
                case 3: saveOptions = "SLAC";   formatExtension = "slac"; break;
                case 4: saveOptions = "GMV";    formatExtension = "gmv"; break;
                case 5: saveOptions = "ANSYS";  formatExtension = "ans"; break;
                case 6: saveOptions = "GMSH";   formatExtension = "gmsh"; break;
                case 7: saveOptions = "STL";    formatExtension = "stl"; break;
            }
        }
    }
#elif defined(ITAPS_GRUMMP)
    formatExtension = ""; // grummp seems to append '.vmesh' always
#else
    formatExtension = "unk";
#endif
}

// ****************************************************************************
//  Method: avtITAPS_CWriter destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

avtITAPS_CWriter::~avtITAPS_CWriter()
{
}


// ****************************************************************************
//  Method: avtITAPS_CWriter::OpenFile
//
//  Purpose:
//
//  Programmer: November 20, 2008 
//  Creation:   September 11, 2003
//
// ****************************************************************************

void
avtITAPS_CWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;
    dir ="";
    // find dir if provided
    int idx = stem.rfind("/");
    if ( idx != string::npos )
    {
        int stem_len = stem.size() - (idx+1) ;
        dir  = stem.substr(0,idx+1);
        stem = stem.substr(idx+1,stem_len);
    }
}

// ****************************************************************************
//  Method: avtITAPS_CWriter::WriteHeaders
//
//  Purpose:
//      This will write out the multi-vars for the ITAPS_C constructs.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::WriteHeaders(const avtDatabaseMetaData *md,
                            vector<string> &scalars, vector<string> &vectors,
                            vector<string> &materials)
{
    const avtMeshMetaData *mmd = md->GetMesh(0);

}

// ****************************************************************************
//  Function: WriteMesh 
//
//  Purpose: Builds up the mesh description in iMesh. Note, I/O doesn't occur
//  until iMesh_save is called.
//
//  Programmer: Mark C. Miller 
//  Creation:   December 1, 2008 
//
// ****************************************************************************
static void
WriteMesh(vtkDataSet *_ds, int chunk,
    iMesh_Instance itapsMesh, iBase_EntitySetHandle rootSet,
    iBase_EntityHandle **pntHdls, iBase_EntityHandle **cellHdls)
{
    int i,j;
    vtkDataSet *ds = _ds;

    *pntHdls = 0;
    *cellHdls = 0;

//#if defined(ITAPS_GRUMMP)
#if 0
    vtkDelaunay3D *d3d = vtkDelaunay3D::New();
    d3d->SetInput(_ds);
    d3d->Update();
    ds = (vtkDataSet*) d3d->GetOutput();
//#warning FREE SOME STUFF HERE
#endif

    try
    {
        // Create the entity set representing this chunk 
        iBase_EntitySetHandle chunkSet;
        iMesh_createEntSet(itapsMesh, 0, &chunkSet, &itapsError);
        CheckITAPSError(itapsMesh, iMesh_createEntSet, NoL);
    
        // Add the nodes of this mesh as vertices of the iMesh instance.
        // Note that vertices can only ever be created or live in the instance
        // itself and not in entity sets of the instance. Though, I think we
        // can create 'links' to vertices from a given entity set using 'add'
        // method(s). We should really use the 'Arr' versions of this method
        // but initial coding is simplest to use individual entity methods.
        //
        // In fact, it would appear as though everything is, by fiat, created
        // in the iMesh instance itself (root set), and then can be 'moved' by
        // adding to another set and removing from root?
        int npts = ds->GetNumberOfPoints(); 
        iBase_EntityHandle *ptHdls = new iBase_EntityHandle[npts];
        for (i = 0; i < npts; i++)
        {
            double pt[3];
            ds->GetPoint(i, pt);
    
            // create initial Vtx entity
            iMesh_createVtx(itapsMesh, pt[0], pt[1], pt[2], &ptHdls[i], &itapsError);
            if (i<5) CheckITAPSError(itapsMesh, iMesh_createVtx, NoL);
        }
        *pntHdls = ptHdls;
    
        // add just created Vtx entites to chunkSet
        iMesh_addEntArrToSet(itapsMesh, ptHdls, npts, &chunkSet, &itapsError);
        CheckITAPSError(itapsMesh, iMesh_addEntArrToSet, NoL);
    
        // remove just created Vtx entities from rootSet, ok?
        //iMesh_rmvEntArrFromSet(itapsMesh, ptHdls, npts, &rootSet, &itapsError);
        //CheckITAPSError(itapsMesh, iMesh_rmvEntArrFromSet, NoL);
    
        int ncells = ds->GetNumberOfCells();
        iBase_EntityHandle *znHdls = new iBase_EntityHandle[ncells];
        for (i = 0; i < ncells; i++)
        {
            vtkCell *theCell = ds->GetCell(i);
    
            int status;
            int topo = VTKZoneTypeToITAPSEntityTopology(theCell->GetCellType());
            int ncellPts = theCell->GetNumberOfPoints();
            iBase_EntityHandle *cellPtEnts = new iBase_EntityHandle[ncellPts];
            for (int j = 0; j < ncellPts; j++)
                cellPtEnts[j] = ptHdls[theCell->GetPointId(j)];
    
            iMesh_createEnt(itapsMesh, topo, cellPtEnts, ncellPts, &znHdls[i], &status, &itapsError);
            if (i<5) CheckITAPSError(itapsMesh, iMesh_createEnt, NoL);
        }
        *cellHdls = znHdls;
    
        // add just created cell entites to chunkSet
        iMesh_addEntArrToSet(itapsMesh, znHdls, ncells, &chunkSet, &itapsError);
        CheckITAPSError(itapsMesh, iMesh_addEntArrToSet, NoL);
    
        // remove just created cell entities from rootSet, ok?
        //iMesh_rmvEntArrFromSet(itapsMesh, znHdls, ncells, &rootSet, &itapsError);
        //CheckITAPSError(itapsMesh, iMesh_rmvEntArrFromSet, NoL);

    }
    catch (iBase_Error TErr)
    {
        char msg[512];
        char desc[256];
        desc[0] = '\0';
        int tmpError = itapsError;
#if !defined(ITAPS_GRUMMP)
        iMesh_getDescription(itapsMesh, desc, &itapsError, sizeof(desc));
#endif
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) \"%s\""
            "\nUnable to open file!", tmpError, desc);
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }
funcEnd: ;
}

// ****************************************************************************
//  Function: vtkDataTypeToITAPSTagType 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 1, 2008 
//
// ****************************************************************************
static int
vtkDataTypeToITAPSTagType(int t)
{
    switch (t)
    {
        case VTK_CHAR:           
        case VTK_UNSIGNED_CHAR:  return iBase_BYTES;

        case VTK_SHORT:
        case VTK_UNSIGNED_SHORT:
        case VTK_INT:
        case VTK_UNSIGNED_INT:
        case VTK_LONG:
        case VTK_UNSIGNED_LONG:
        case VTK_ID_TYPE:        return iBase_INTEGER;

        case VTK_FLOAT:          
        case VTK_DOUBLE:         return iBase_DOUBLE;
    }
    return iBase_BYTES;
}

// ****************************************************************************
//  Function: ConvertTypeAndStorageOrder
//
//  Programmer: Mark C. Miller 
//  Creation:   December 6, 2008 
//
// ****************************************************************************
template <class iT, class oT>
static void ConvertTypeAndStorageOrder(const iT *const ibuf, int npts, int ncomps, int sorder, oT *obuf)
{
    int i, j;
    if (sorder == iBase_INTERLEAVED)
    {
        for (i = 0; i < npts; i++)
        {
            for (j = 0; j < ncomps; j++)
            {
                obuf[i*ncomps+j] = (oT) ibuf[i*ncomps+j];
            }
        }
    }
    else
    {
        for (j = 0; j < ncomps; j++)
        {
            for (i = 0; i < npts; i++)
            {
                obuf[j*ncomps+i] = (oT) ibuf[i*ncomps+j];
            }
        }
    }
}

// ****************************************************************************
//  Function: ConvertTypeAndStorageOrder
//
//  Programmer: Mark C. Miller 
//  Creation:   December 5, 2008 
//
// ****************************************************************************
template <class T>
static void ConvertTypeAndStorageOrder(vtkDataArray *arr, int sorder, T **buf) 
{
    int npts = arr->GetNumberOfTuples();
    int ncomps = arr->GetNumberOfComponents();
    void *p = arr->GetVoidPointer(0);
    T *newbuf = new T[npts*ncomps];

    switch (arr->GetDataType())
    {
        case VTK_CHAR: ConvertTypeAndStorageOrder((char*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_UNSIGNED_CHAR:ConvertTypeAndStorageOrder((unsigned char*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_SHORT: ConvertTypeAndStorageOrder((short*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_UNSIGNED_SHORT: ConvertTypeAndStorageOrder((unsigned short*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_INT: ConvertTypeAndStorageOrder((int*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_UNSIGNED_INT: ConvertTypeAndStorageOrder((unsigned int*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_LONG: ConvertTypeAndStorageOrder((long*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_UNSIGNED_LONG: ConvertTypeAndStorageOrder((unsigned long*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_ID_TYPE: ConvertTypeAndStorageOrder((vtkIdType*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_FLOAT:  ConvertTypeAndStorageOrder((float*) p, npts, ncomps, sorder, newbuf); break;
        case VTK_DOUBLE: ConvertTypeAndStorageOrder((double*) p, npts, ncomps, sorder, newbuf); break;
    }

    *buf = newbuf;
}

// ****************************************************************************
//  Function: ConvertTypeAndStorageOrder
//
//  Purpose: Iterate over all point and cell variables, skipping VisIt
//  'internal' variables and build up tags on the appropriate entities in
//  iMesh.
//
//  Programmer: Mark C. Miller 
//  Creation:   December 5, 2008 
//
// ****************************************************************************
static void
WriteVariables(vtkDataSet *ds, int chunk,
    iMesh_Instance itapsMesh, iBase_EntityHandle rootSet,
    iBase_EntityHandle *ptHdls, iBase_EntityHandle *clHdls)
{
    try
    {
        int sorder;
        iMesh_getDfltStorage(itapsMesh, &sorder, &itapsError);

        for (int pass = 0; pass < 2; pass++)
        {
            vtkPointData *pd = 0;
            vtkCellData *cd = 0;
    
            if (pass == 0)
               pd = ds->GetPointData();
            else
               cd = ds->GetCellData();
    
            iBase_EntityHandle *eHdls = pd ? ptHdls : clHdls;
            int narrays = pd ? pd->GetNumberOfArrays() : cd->GetNumberOfArrays();
            int nents = pd ? ds->GetNumberOfPoints() : ds->GetNumberOfCells();
    
            for (int i = 0 ; i < narrays ; i++)
            {
                vtkDataArray *arr = pd ? pd->GetArray(i) : cd->GetArray(i);
        
                // ignore internal VisIt data
                if (strstr(arr->GetName(), "vtk") != NULL)
                    continue;
                if (strstr(arr->GetName(), "avt") != NULL)
                    continue;
    
                int npts = arr->GetNumberOfTuples();
                int ncomps = arr->GetNumberOfComponents();
                int iType = vtkDataTypeToITAPSTagType(arr->GetDataType());
                iBase_TagHandle varTag;
                iMesh_createTag(itapsMesh, arr->GetName(), ncomps, iType, &varTag,
                    &itapsError, strlen(arr->GetName()));
                CheckITAPSError(itapsMesh, iMesh_createTag, NoL);

                switch (iType)
                {
                    case iBase_DOUBLE:
                    {
                        if (arr->GetDataType() == VTK_DOUBLE && sorder == iBase_INTERLEAVED)
                        {
                            // Fastrack because in this case iMesh can digest VTK-native
                            iMesh_setDblArrData(itapsMesh, eHdls, nents, varTag,
                                (double *) arr->GetVoidPointer(0), npts*ncomps, &itapsError);
                            CheckITAPSError(itapsMesh, iMesh_setDblArrData, NoL);
                        }
                        else
                        {
                            double *buf;
                            ConvertTypeAndStorageOrder(arr, sorder, &buf);
                            iMesh_setDblArrData(itapsMesh, eHdls, nents, varTag, buf,
                                npts*ncomps, &itapsError);
                            delete [] buf;
                            CheckITAPSError(itapsMesh, iMesh_setDblArrData, NoL);
                        }
                        break;
                    }
                    case iBase_INTEGER:
                    {
                        if (arr->GetDataType() == VTK_INT && sorder == iBase_INTERLEAVED)
                        {
                            // Fastrack because in this case iMesh can digest VTK-native
                            iMesh_setIntArrData(itapsMesh, eHdls, nents, varTag,
                                (int *) arr->GetVoidPointer(0), npts*ncomps, &itapsError);
                            CheckITAPSError(itapsMesh, iMesh_setIntArrData, NoL);
                        }
                        else
                        {
                            int *buf;
                            ConvertTypeAndStorageOrder(arr, sorder, &buf);
                            iMesh_setIntArrData(itapsMesh, eHdls, nents, varTag, buf,
                                npts*ncomps, &itapsError);
                            delete [] buf;
                            CheckITAPSError(itapsMesh, iMesh_setIntArrData, NoL);
                        }
                        break;
                    }
                    case iBase_BYTES:
                    {
                        if (arr->GetDataType() == VTK_CHAR && sorder == iBase_INTERLEAVED)
                        {
                            // Fastrack because in this case iMesh can digest VTK-native
                            iMesh_setArrData(itapsMesh, eHdls, nents, varTag,
                                (char *) arr->GetVoidPointer(0), npts*ncomps, &itapsError);
                            CheckITAPSError(itapsMesh, iMesh_setArrData, NoL);
                        }
                        else
                        {
                            char *buf;
                            ConvertTypeAndStorageOrder(arr, sorder, &buf);
                            iMesh_setArrData(itapsMesh, eHdls, nents, varTag, buf,
                                npts*ncomps, &itapsError);
                            delete [] buf;
                            CheckITAPSError(itapsMesh, iMesh_setArrData, NoL);
                        }
                        break;
                    }
                }
            }
        }
    }
    catch (iBase_Error TErr)
    {
        char msg[512];
        char desc[256];
        desc[0] = '\0';
        int tmpError = itapsError;
#if !defined(ITAPS_GRUMMP)
        iMesh_getDescription(itapsMesh, desc, &itapsError, sizeof(desc));
#endif
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) \"%s\""
            "\nUnable to open file!", tmpError, desc);
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }
funcEnd: ;
}


// ****************************************************************************
//  Method: avtITAPS_CWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    int i;

    char dummyStr[32];
    iMesh_Instance itapsMesh;
    iMesh_newMesh(dummyStr, &itapsMesh, &itapsError, 0);
    CheckITAPSError(itapsMesh, iMesh_newMesh, NoL);

    iBase_EntitySetHandle rootSet;
    iMesh_getRootSet(itapsMesh, &rootSet, &itapsError);
    CheckITAPSError(itapsMesh, iMesh_getRootSet, NoL);

    // Create mesh description in iMesh instance
    iBase_EntityHandle *ptHdls, *clHdls;
    WriteMesh(ds, chunk, itapsMesh, rootSet, &ptHdls, &clHdls);

    // Create variables (tags) in iMesh instance
    WriteVariables(ds, chunk, itapsMesh, rootSet, ptHdls, clHdls);

    if (ptHdls)
        delete [] ptHdls;
    if (clHdls)
        delete [] clHdls;

    // Ok, write the iMesh instance to a file
    try
    {

        // save the file
        string fname = dir + stem;
        char filename[1024];
        if (formatExtension != "")
            sprintf(filename, "%s.%d.%s", fname.c_str(), chunk, formatExtension.c_str());
        else
            sprintf(filename, "%s.%d", fname.c_str(), chunk);
        iMesh_save(itapsMesh, rootSet, filename, saveOptions.c_str(), &itapsError,
            strlen(filename), saveOptions.size());
        CheckITAPSError(itapsMesh, iMesh_save, NoL);

    }
    catch (iBase_Error TErr)
    {
        char msg[512];
        char desc[256];
        desc[0] = '\0';
        int tmpError = itapsError;
#if !defined(ITAPS_GRUMMP)
        iMesh_getDescription(itapsMesh, desc, &itapsError, sizeof(desc));
#endif
        SNPRINTF(msg, sizeof(msg), "Encountered ITAPS error (%d) \"%s\""
            "\nUnable to open file!", tmpError, desc);
        if (!avtCallback::IssueWarning(msg))
            cerr << msg << endl;
    }
funcEnd: ;
}


// ****************************************************************************
//  Method: avtITAPS_CWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 20, 2008 
//
// ****************************************************************************

void
avtITAPS_CWriter::CloseFile(void)
{
}
