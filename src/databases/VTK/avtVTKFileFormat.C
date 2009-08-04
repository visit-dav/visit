/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtVTKFileFormat.C                             //
// ************************************************************************* //

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <avtVTKFileFormat.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkVisItDataSetReader.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkFloatArray.h>
#include <vtkVisItXMLImageDataReader.h>
#include <vtkVisItXMLPolyDataReader.h>
#include <vtkVisItXMLRectilinearGridReader.h>
#include <vtkVisItXMLStructuredGridReader.h>
#include <vtkVisItXMLUnstructuredGridReader.h>

#include <snprintf.h>
#include <DebugStream.h>
#include <Expression.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <map>
#include <string>
#include <vector>

using std::string;
//
// Define the static const's
//

const char   *avtVTKFileFormat::MESHNAME="mesh";
const char   *avtVTKFileFormat::VARNAME="VTKVar";


static void GetListOfUniqueCellTypes(vtkUnstructuredGrid *ug, 
                                     vtkUnsignedCharArray *uca);


// ****************************************************************************
//  Method: avtVTKFileFormat constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2001
//
//   Modifications:
//
//     Hank Childs, Tue May 24 12:05:52 PDT 2005
//     Added arguments.
//
//     Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//     Initialized matvarname
//
//     Kathleen Bonnell, Thu Sep 22 15:37:13 PDT 2005 
//     Save the file extension. 
//
//     Kathleen Bonnell, Thu Jun 29 17:30:40 PDT 2006 
//     Add vtk_time, to store time from the VTK file if it is available.
//
//     Hank Childs, Mon Jun 11 21:27:04 PDT 2007
//     Do not assume there is an extension.
//
//     Kathleen Bonnell, Wed Jul  9 17:48:21 PDT 2008
//     Add vtk_cycle, to store cycle from the VTK file if it is available.
//
// ****************************************************************************

avtVTKFileFormat::avtVTKFileFormat(const char *fname, DBOptionsAttributes *) 
    : avtSTSDFileFormat(fname)
{
    dataset = NULL;
    readInDataset = false;
    matvarname = NULL;

    // find the file extension
    int i, start = -1;
    int len = strlen(fname);
    for(i = 0; i < len; i++)
        if(fname[i] == '.')
            start = i;

    if (start != -1)
        extension = string(fname, start+1, len-1);
    else
        extension = "none";

    vtk_time = INVALID_TIME;
    vtk_cycle = INVALID_CYCLE;
}


// ****************************************************************************
//  Method: avtVTKFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Freed matvarname
//
// ****************************************************************************

avtVTKFileFormat::~avtVTKFileFormat()
{
    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }
    if (matvarname != NULL)
    {
        free(matvarname);
        matvarname = NULL;
    }
}


// ****************************************************************************
//  Method: avtVTKFileFormat::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 12 15:52:01 PST 2004
//    Use VisIt's version of the reader, so that all variables can be read
//    into the dataset in one pass.
//
//    Kathleen Bonnell, Thu Mar 11 12:53:12 PST 2004 
//    Convert StructuredPoints datasets into RectilinearGrids. 
//
//    Kathleen Bonnell, Thu Sep 22 15:37:13 PDT 2005 
//    Support vtk xml file formats.
//
//    Kathleen Bonnell, Wed May 17 14:03:29 PDT 2006 
//    Remove call to SetSource(NULL), as it now removes information necessary
//    to the dataset.
//
//    Kathleen Bonnell, Thu Jun 29 17:30:40 PDT 2006 
//    Retrieve TIME from FieldData if available.
//
//    Hank Childs, Mon Jun 11 21:27:04 PDT 2007
//    Add support for files with no extensions.
//
//    Kathleen Bonnell, Fri Feb 29 09:02:44 PST 2008 
//    Support VTK_IMAGE_DATA (returned from vtkXMLImageDataReader), convert it
//    to rectilinear, as we do with StructuredPoints data.
//
//    Sean Ahern, Mon Mar 24 17:19:56 EDT 2008
//    Added better error checking to detect badly formatted VTK files.
//
//    Kathleen Bonnell, Wed Jul  9 18:13:20 PDT 2008
//    Retrieve CYCLE from FieldData if available.
//
// ****************************************************************************

void
avtVTKFileFormat::ReadInDataset(void)
{
    debug4 << "Reading in dataset from VTK file " << filename << endl;

    //
    // This shouldn't ever happen (since we would already have the dataset
    // we are trying to read from the file sitting in memory), but anything
    // to prevent leaks.
    //
    if (dataset != NULL)
    {
        dataset->Delete();
    }

    if (extension == "vtk" || extension == "none")
    {
        if (extension == "none")
            debug1 << "No extension given ... assuming legacy VTK format." 
                   << endl;

        //
        // Create a file reader and set our dataset to be its output.
        //
        vtkVisItDataSetReader *reader = vtkVisItDataSetReader::New();
        reader->ReadAllScalarsOn();
        reader->ReadAllVectorsOn();
        reader->ReadAllTensorsOn();
        reader->SetFileName(filename);
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);

        //
        // Force the read and make sure that the reader is really gone, 
        // so we don't eat up too many file descriptors.
        //
        dataset->Update();
        //dataset->SetSource(NULL);
        reader->Delete();
    }
    else if (extension == "vti")
    {
        vtkVisItXMLImageDataReader *reader = vtkVisItXMLImageDataReader::New();
        reader->SetFileName(filename);
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        dataset->Update();
        //dataset->SetSource(NULL);
        reader->Delete();
    } 
    else if (extension == "vtr") 
    {
        vtkVisItXMLRectilinearGridReader *reader = 
            vtkVisItXMLRectilinearGridReader::New();
        reader->SetFileName(filename);
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        dataset->Update();
        //dataset->SetSource(NULL);
        reader->Delete();
    } 
    else if (extension == "vts")
    {
        vtkVisItXMLStructuredGridReader *reader = 
            vtkVisItXMLStructuredGridReader::New();
        reader->SetFileName(filename);
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        dataset->Update();
        //dataset->SetSource(NULL);
        reader->Delete();
    } 
    else if (extension == "vtp") 
    {
        vtkVisItXMLPolyDataReader *reader = vtkVisItXMLPolyDataReader::New();
        reader->SetFileName(filename);
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        dataset->Update();
        //dataset->SetSource(NULL);
        reader->Delete();
    } 
    else if (extension == "vtu") 
    {
        vtkVisItXMLUnstructuredGridReader *reader = 
            vtkVisItXMLUnstructuredGridReader::New();
        reader->SetFileName(filename);
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, filename);
        }
        dataset->Register(NULL);
        dataset->Update();
        //dataset->SetSource(NULL);
        reader->Delete();
    } 
    else
    {
        EXCEPTION2(InvalidFilesException, filename, 
                   "could not match extension to a VTK file format type");
    }

    vtk_time = INVALID_TIME;
    if (dataset->GetFieldData()->GetArray("TIME") != 0)
    {
        vtk_time = dataset->GetFieldData()->GetArray("TIME")->GetTuple1(0);
    }
    vtk_cycle = INVALID_CYCLE;
    if (dataset->GetFieldData()->GetArray("CYCLE") != 0)
    {
        vtk_cycle = (int)dataset->GetFieldData()->GetArray("CYCLE")->GetTuple1(0);
    }

    if (dataset->GetDataObjectType() == VTK_STRUCTURED_POINTS ||
        dataset->GetDataObjectType() == VTK_IMAGE_DATA)
    {
        //
        // The old dataset passed in will be deleted, a new one will be 
        // returned.
        //
        dataset = ConvertStructuredPointsToRGrid((vtkStructuredPoints*)dataset);
    }
    readInDataset = true;
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetAuxiliaryData
//
//  Programmer: Mark C. Miller 
//  Creation:   September 15, 2005 
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 11:25:32 PST 2008
//    Fix memory leak.
//
// ****************************************************************************

void *
avtVTKFileFormat::GetAuxiliaryData(const char *var,
    const char *type, void *, DestructorFunction &df)
{
    void *rv = NULL;

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        vtkIntArray *matarr = vtkIntArray::SafeDownCast(GetVar(matvarname));

        int ntuples = matarr->GetNumberOfTuples();
        int *matlist = matarr->GetPointer(0);

        int *matnostmp = new int[matnos.size()];
        char **matnamestmp = new char*[matnames.size()];
        for (int i = 0; i < matnos.size(); i++)
        {
            matnostmp[i] = matnos[i];
            matnamestmp[i] = (char*) matnames[i].c_str();
        }

        avtMaterial *mat = new avtMaterial(matnos.size(), //silomat->nmat,
                                           matnostmp,     //silomat->matnos,
                                           matnamestmp,   //silomat->matnames,
                                           1,             //silomat->ndims,
                                           &ntuples,      //silomat->dims,
                                           0,             //silomat->major_order,
                                           matlist,       //silomat->matlist,
                                           0,             //silomat->mixlen,
                                           0,             //silomat->mix_mat,
                                           0,             //silomat->mix_next,
                                           0,             //silomat->mix_zone,
                                           0              //silomat->mix_vf
                                           );

        delete [] matnostmp;
        delete [] matnamestmp;
        matarr->Delete();

        df = avtMaterial::Destruct;
        rv = mat;
    }

    return rv;
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh.  The mesh is actually just the dataset, so return that.
//
//  Arguments:
//      mesh     The desired meshname, this should be MESHNAME.
//
//  Returns:     The mesh as a VTK dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 26 13:33:43 PST 2002
//    Add a reference so that reference counting tricks work.
//
// ****************************************************************************

vtkDataSet *
avtVTKFileFormat::GetMesh(const char *mesh)
{
    debug5 << "Getting mesh from VTK file " << filename << endl;

    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    if (!readInDataset)
    {
        ReadInDataset();
    }

    //
    // The routine that calls this method is going to assume that it can call
    // Delete on what is returned.  That means we better add an extra
    // reference.
    //
    dataset->Register(NULL);
    return dataset;
}


// ****************************************************************************
//  Method: avtVTKFileFormat::GetVar
//
//  Purpose:
//      Gets the variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The varialbe as VTK scalars.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 20 09:23:26 PST 2001
//    Account for vector variable in error checking.
//
//    Hank Childs, Tue Mar 26 13:33:43 PST 2002
//    Add a reference so that reference counting tricks work.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Thu Aug 15 09:07:38 PDT 2002
//    Add support for multiple variables.
//
//    Hank Childs, Sat Mar 19 11:57:19 PST 2005
//    Turn variables with name "internal_var_" back into "avt".
//
// ****************************************************************************

vtkDataArray *
avtVTKFileFormat::GetVar(const char *real_name)
{
    debug5 << "Getting var from VTK file " << filename << endl;

    if (!readInDataset)
    {
        ReadInDataset();
    }

    const char *var = real_name;
    char buffer[1024];
    if (strncmp(var, "internal_var_", strlen("internal_var_")) == 0)
    {
        sprintf(buffer, "avt%s", var + strlen("internal_var_")); 
        var = buffer;
    }

    vtkDataArray *rv = NULL;
    rv = dataset->GetPointData()->GetArray(var);
    if (rv == NULL)
    {
        rv = dataset->GetCellData()->GetArray(var);
    }

    //
    // See if we made up an artificial name for it.
    //
    if (strstr(var, VARNAME) != NULL)
    {
        const char *numstr = var + strlen(VARNAME);
        int num = atoi(numstr);
        int npointvars = dataset->GetPointData()->GetNumberOfArrays();
        if (num < npointvars)
        {
            rv = dataset->GetPointData()->GetArray(num);
        }
        else
        {
            rv = dataset->GetCellData()->GetArray(num-npointvars);
        }
    }

    if (rv == NULL)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    //
    // The routine that calls this method is going to assume that it can call
    // Delete on what is returned.  That means we better add an extra
    // reference.
    //
    rv->Register(NULL);
    return rv;
}


// ****************************************************************************
//  Method: avtVTKFileFormat::GetVectorVar
//
//  Purpose:
//      Gets the vector variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The varialbe as VTK vectors.
//
//  Programmer: Hank Childs
//  Creation:   March 20, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 26 13:33:43 PST 2002
//    Add a reference so that reference counting tricks work.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002 
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Thu Aug 15 09:17:14 PDT 2002
//    Route the vector call through the scalar variable call, since there is
//    now no effective difference between the two.
//
// ****************************************************************************

vtkDataArray *
avtVTKFileFormat::GetVectorVar(const char *var)
{
    //
    // There is no difference between vectors and scalars for this class.
    //
    return GetVar(var);
}


// ****************************************************************************
//  Method: avtVTKFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources.  Since this module does not keep an open file, that
//      only means deleting the dataset.  Since this is all reference counted,
//      there is no worry that we will be deleting something that is being
//      used.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Freed matvarname
//
// ****************************************************************************

void
avtVTKFileFormat::FreeUpResources(void)
{
    debug4 << "VTK file " << filename << " forced to free up resources."
           << endl;

    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }
    if (matvarname != NULL)
    {
        free(matvarname);
        matvarname = NULL;
    }

    readInDataset = false;
}


// ****************************************************************************
//  Method: avtVTKFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Mar 15 11:10:54 PST 2001
//    Better determine if the dataset is 2D or 3D.
//
//    Hank Childs, Tue Mar 20 09:13:30 PST 2001
//    Allow for vector variables.
//
//    Hank Childs, Mon Mar 11 08:52:59 PST 2002
//    Renamed to PopulateDatabaseMetaData.
//
//    Hank Childs, Sun Jun 23 20:50:17 PDT 2002
//    Added support for point meshes.
//
//    Hank Childs, Sat Jun 29 23:08:51 PDT 2002
//    Add support for poly data meshes that are point meshes.
//
//    Hank Childs, Thu Aug 15 09:07:38 PDT 2002
//    Add support for multiple variables.
//
//    Hank Childs, Thu Aug 21 23:28:44 PDT 2003
//    Replace call to GetListOfUniqueCellTypes, which hangs in an infinite
//    loop if there are multiple types of cells.
//
//    Hank Childs, Wed Sep 24 08:02:08 PDT 2003
//    Add support for tensors.
//
//    Hank Childs, Thu Aug 26 08:32:09 PDT 2004
//    Only declare the mesh as 2D if Z=0 for all points.
//
//    Hank Childs, Sat Mar 19 11:57:19 PST 2005
//    Do not return variables with name "avt", since we may want to look
//    at these variables and the generic DB will throw them away.
//
//    Kathleen Bonnell, Wed Jul 13 18:27:05 PDT 2005 
//    Specify whether or not scalar data should be treated as ascii. 
//
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Added support for arrays representing materials
// 
//    Brad Whitlock, Wed Nov 9 10:59:35 PDT 2005
//    Added support for color vectors (ncomps==4).
//
//    Kathleen Bonnell, Fri Feb  3 11:20:02 PST 2006 
//    Added support for MeshCoordType (int in FieldData of dataset,
//    0 == XY, 1 == RZ, 2 == ZR).
//
//    Jeremy Meredith, Mon Aug 28 17:40:47 EDT 2006
//    Added support for unit cell vectors.
//
//    Jeremy Meredith, Thu Apr  2 16:08:16 EDT 2009
//    Added array variable support.
//
//    Brad Whitlock, Fri May 15 16:05:22 PDT 2009
//    I improved the array variable support, adding them for cell data and I
//    added expressions to extract their components. I also added support 
//    label variables.
//
// ****************************************************************************

void
avtVTKFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    int  i;

    if (!readInDataset)
    {
        ReadInDataset();
    }

    int spat = 3;
    int topo = 3;

    avtMeshType type;
    int  vtkType = dataset->GetDataObjectType();
    switch (vtkType)
    {
      case VTK_RECTILINEAR_GRID:
        type = AVT_RECTILINEAR_MESH;
        break;
      case VTK_STRUCTURED_GRID:
        type = AVT_CURVILINEAR_MESH;
        break;
      case VTK_UNSTRUCTURED_GRID:
        type = AVT_UNSTRUCTURED_MESH;
        break;
      case VTK_POLY_DATA:
        topo = 2;
        type = AVT_SURFACE_MESH;
        break;
      default:
        debug1 << "Unable to identify mesh type " << vtkType << endl;
        type = AVT_UNKNOWN_MESH;
        break;
    }

    double bounds[6];
    dataset->GetBounds(bounds);

    if ((bounds[4] == bounds[5]) && (bounds[5] == 0.))
    {
        spat = 2;
        topo = 2;
    }
 
    //
    // See if we have a point mesh.
    //
    if (vtkType == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) dataset;
        vtkUnsignedCharArray *types = vtkUnsignedCharArray::New();
        GetListOfUniqueCellTypes(ugrid, types);

        if (types->GetNumberOfTuples() == 1)
        {
            int myType = (int) types->GetValue(0);
            if (myType == VTK_VERTEX)
            {
                debug5 << "The VTK file format contains all points -- "
                       << "declaring this a point mesh." << endl;
                topo = 0;
            }
        }

        types->Delete();
    }
    else if (vtkType == VTK_POLY_DATA)
    {
        vtkPolyData *pd = (vtkPolyData *) dataset;
        if (pd->GetNumberOfPolys() == 0 && pd->GetNumberOfStrips() == 0)
        {
            if (pd->GetNumberOfLines() > 0)
            {
                topo = 1;
            }
            else
            {
                topo = 0;
            }
        }
    }
 
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = MESHNAME;
    mesh->meshType = type;
    mesh->spatialDimension = spat;
    mesh->topologicalDimension = topo;
    mesh->numBlocks = 1;
    mesh->blockOrigin = 0;
    mesh->SetExtents(bounds);
    if (dataset->GetFieldData()->GetArray("MeshCoordType") != NULL)
    {
        avtMeshCoordType mct = (avtMeshCoordType)
            int(dataset->GetFieldData()->GetArray("MeshCoordType")->
                                                        GetComponent(0, 0));
        mesh->meshCoordType = mct;
        if (mct == AVT_RZ)
        {
            mesh->xLabel = "Z-Axis";
            mesh->yLabel = "R-Axis";
        }
        else if (mct == AVT_ZR)
        {
            mesh->xLabel = "R-Axis";
            mesh->yLabel = "Z-Axis";
        }
    }
    if (dataset->GetFieldData()->GetArray("UnitCellVectors"))
    {
        vtkDataArray *ucv = dataset->GetFieldData()->
                                               GetArray("UnitCellVectors");
        for (int j=0; j<3; j++)
        {
            for (int k=0; k<3; k++)
            {
                mesh->unitCellVectors[j*3+k] = ucv->GetComponent(j*3+k,0);
            }
        }
    }
    md->Add(mesh); 

    int nvars = 0;

    for (i = 0 ; i < dataset->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = dataset->GetPointData()->GetArray(i);
        int ncomp = arr->GetNumberOfComponents();
        const char *name = arr->GetName();
        char buffer[1024];
        char buffer2[1024];
        if (name == NULL || strcmp(name, "") == 0)
        {
            sprintf(buffer, "%s%d", VARNAME, nvars);
            name = buffer;
        }
        if (strncmp(name, "avt", strlen("avt")) == 0)
        {
            sprintf(buffer2, "internal_var_%s", name+strlen("avt"));
            name = buffer2;
        }
        if (ncomp == 1)
        {
            bool ascii = arr->GetDataType() == VTK_CHAR;
            AddScalarVarToMetaData(md, name, MESHNAME, AVT_NODECENT, NULL, ascii);
        }
        else if (ncomp <= 4)
        {
            AddVectorVarToMetaData(md, name, MESHNAME, AVT_NODECENT, ncomp);
        }
        else if (ncomp == 9)
        {
            AddTensorVarToMetaData(md, name, MESHNAME, AVT_NODECENT);
        }
        else
        {
            if(arr->GetDataType() == VTK_UNSIGNED_CHAR ||
               arr->GetDataType() == VTK_CHAR)
            {
                md->Add(new avtLabelMetaData(name, MESHNAME, AVT_NODECENT));
            }
            else
            {
                AddArrayVarToMetaData(md, name, ncomp, MESHNAME, AVT_NODECENT);
                int compnamelen = strlen(name) + 40;
                char *exp_name = new char[compnamelen];
                char *exp_def = new char[compnamelen];
                for(int c = 0; c < ncomp; ++c)
                {
                    SNPRINTF(exp_name, compnamelen, "%s/comp_%d", name, c);
                    SNPRINTF(exp_def,  compnamelen, "array_decompose(<%s>, %d)",  name, c);
                    Expression *e = new Expression;
                    e->SetType(Expression::ScalarMeshVar);
                    e->SetName(exp_name);
                    e->SetDefinition(exp_def);
                    md->AddExpression(e);
                }
                delete [] exp_name;
                delete [] exp_def;
            }
        }
        nvars++;
    }
    for (i = 0 ; i < dataset->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = dataset->GetCellData()->GetArray(i);
        int ncomp = arr->GetNumberOfComponents();
        const char *name = arr->GetName();
        char buffer[1024];
        char buffer2[1024];
        if (name == NULL || strcmp(name, "") == 0)
        {
            sprintf(buffer, "%s%d", VARNAME, nvars);
            name = buffer;
        }
        if (strncmp(name, "avt", strlen("avt")) == 0)
        {
            sprintf(buffer2, "internal_var_%s", name+strlen("avt"));
            name = buffer2;
        }
        if ((arr->GetDataType() == VTK_INT) && (ncomp == 1) &&
            ((strncmp(name, "internal_var_Subsets", strlen("internal_var_Subsets")) == 0) ||
            ((strncmp(name, "material", strlen("material")) == 0))))
        {
            vtkIntArray *iarr = vtkIntArray::SafeDownCast(arr);
            int *iptr = iarr->GetPointer(0);
            std::map<int, bool> valMap;
            int ntuples = iarr->GetNumberOfTuples();
            for (int j = 0; j < ntuples; j++)
                valMap[iptr[j]] = true;
            std::map<int, bool>::const_iterator it;
            for (it = valMap.begin(); it != valMap.end(); it++)
            {
                char tmpname[32];
                SNPRINTF(tmpname, sizeof(tmpname), "%d", it->first);
                matnames.push_back(tmpname);
                matnos.push_back(it->first);
            }

            avtMaterialMetaData *mmd = new avtMaterialMetaData("materials", MESHNAME,
                                               valMap.size(), matnames);
            md->Add(mmd);

            if (strncmp(name, "internal_var_Subsets", strlen("internal_var_Subsets")) == 0)
                matvarname = strdup("internal_var_Subsets");
            else
                matvarname = strdup("material");
        }
        else if (ncomp == 1)
        {
            bool ascii = arr->GetDataType() == VTK_CHAR;
            AddScalarVarToMetaData(md, name, MESHNAME, AVT_ZONECENT, NULL, ascii);
        }
        else if (ncomp <= 4)
        {
            AddVectorVarToMetaData(md, name, MESHNAME, AVT_ZONECENT, ncomp);
        }
        else if (ncomp == 9)
        {
            AddTensorVarToMetaData(md, name, MESHNAME, AVT_ZONECENT);
        }
        else
        {
            if(arr->GetDataType() == VTK_UNSIGNED_CHAR ||
               arr->GetDataType() == VTK_CHAR)
            {
                md->Add(new avtLabelMetaData(name, MESHNAME, AVT_ZONECENT));
            }
            else
            {
                AddArrayVarToMetaData(md, name, ncomp, MESHNAME, AVT_ZONECENT);
                int compnamelen = strlen(name) + 40;
                char *exp_name = new char[compnamelen];
                char *exp_def = new char[compnamelen];
                for(int c = 0; c < ncomp; ++c)
                {
                    SNPRINTF(exp_name, compnamelen, "%s/comp_%d", name, c);
                    SNPRINTF(exp_def,  compnamelen, "array_decompose(<%s>, %d)",  name, c);
                    Expression *e = new Expression;
                    e->SetType(Expression::ScalarMeshVar);
                    e->SetName(exp_name);
                    e->SetDefinition(exp_def);
                    md->AddExpression(e);
                }
                delete [] exp_name;
                delete [] exp_def;
            }
        }
        nvars++;
    }
}


// ****************************************************************************
//  Function: GetListOfUniqueCellTypes
//
//  Purpose:
//     Gets a list of the unique cell types.
//
//  Notes:    This is done externally to the similar method in 
//            vtkUnstructuredGrid, since that method is buggy and can get
//            into an infinite loop.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

static void
GetListOfUniqueCellTypes(vtkUnstructuredGrid *ug, vtkUnsignedCharArray *uca)
{
    int  i;
    bool   haveCellType[256];
    for (i = 0 ; i < 256 ; i++)
        haveCellType[i] = false;

    int ncells = ug->GetNumberOfCells();
    for (i = 0 ; i < ncells ; i++)
        haveCellType[ug->GetCellType(i)] = true;

    int ntypes = 0;
    for (i = 0 ; i < 256 ; i++)
        if (haveCellType[i])
            ntypes++;

    uca->SetNumberOfTuples(ntypes);
    int idx = 0;
    for (i = 0 ; i < 256 ; i++)
        if (haveCellType[i])
        {
            uca->SetValue(idx++, i);
        }
}

// ****************************************************************************
//  Function: ConvertStructuredPointsToRGrid
//
//  Purpose:
//     Constructs a vtkRectilinearGrid from the passed vtkStructuredPoints. 
//
//  Notes:  The passed in dataset will be deleted.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 9, 2004
//
// ****************************************************************************

vtkDataSet *
avtVTKFileFormat::ConvertStructuredPointsToRGrid(vtkStructuredPoints *inSP)
{
    int coordDims[3]; 
    double spacing[3];
    double origin[3];
    inSP->GetDimensions(coordDims);
    inSP->GetSpacing(spacing);
    inSP->GetOrigin(origin);

    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfComponents(1);
    x->SetNumberOfTuples(coordDims[0]);
    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfComponents(1);
    y->SetNumberOfTuples(coordDims[1]);
    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfComponents(1);
    z->SetNumberOfTuples(coordDims[2]);

    vtkRectilinearGrid *outRG = vtkRectilinearGrid::New();
    outRG->SetDimensions(coordDims);
    outRG->SetXCoordinates(x);
    outRG->SetYCoordinates(y);
    outRG->SetZCoordinates(z);
    x->Delete();
    y->Delete();
    z->Delete();

    int i;
    float *ptr = x->GetPointer(0);
    for (i = 0; i < coordDims[0]; i++, ptr++)
        *ptr = origin[0] + i * spacing[0]; 

    ptr = y->GetPointer(0);
    for (i = 0; i < coordDims[1]; i++, ptr++)
        *ptr = origin[1] + i * spacing[1]; 

    ptr = z->GetPointer(0);
    for (i = 0; i < coordDims[2]; i++, ptr++)
        *ptr = origin[2] + i * spacing[2]; 
  
    for (i = 0; i < inSP->GetPointData()->GetNumberOfArrays(); i++)
        outRG->GetPointData()->AddArray(inSP->GetPointData()->GetArray(i));

    for (i = 0; i < inSP->GetCellData()->GetNumberOfArrays(); i++)
        outRG->GetCellData()->AddArray(inSP->GetCellData()->GetArray(i));
   
    inSP->Delete(); 
    return outRG; 
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetCycleFromFilename
//
//  Purpose: Try to get a cycle number from a file name
//
//  Notes: Although all this method does is simply call the format's base
//  class implementation of GuessCycle, doing this is a way for the VTK
//  format to "bless" the guesses that that method makes. Otherwise, VisIt
//  wouldn't know that VTK thinks those guesses are good. See notes in
//  avtSTXXFileFormatInterface::SetDatabaseMetaData for further explanation.
//
//  Programmer: Eric Brugger
//  Creation:   August 12, 2005
//
// ****************************************************************************

int
avtVTKFileFormat::GetCycleFromFilename(const char *f) const
{
    return GuessCycle(f);
}


// ****************************************************************************
//  Method: avtVTKFileFormat::GetTime
//
//  Purpose: Return the time associated with this file
//
//  Programmer: Kathleen Bonnell
//  Creation:   Jun 29, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul  9 18:14:24 PDT 2008
//    Call ReadInDataset if not done already.
//
// ****************************************************************************

double
avtVTKFileFormat::GetTime()
{
    if (INVALID_TIME == vtk_time && !readInDataset)
        ReadInDataset();
    return vtk_time;
}

// ****************************************************************************
//  Method: avtVTKFileFormat::GetCycle
//
//  Purpose: Return the cycle associated with this file
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 9, 2008 
//
//  Modifications:
//
// ****************************************************************************

int
avtVTKFileFormat::GetCycle()
{
    if (INVALID_CYCLE == vtk_cycle && !readInDataset)
        ReadInDataset();
    return vtk_cycle;
}
