/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                            avtVTKFileReader.C                             //
// ************************************************************************* //

#include <limits.h> // for INT_MAX
#include <float.h> // for DBL_MAX

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>
#include <avtMaterial.h>
#include <avtVTKFileReader.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetReader.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLRectilinearGridReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkVisItXMLPDataReader.h>
#include <PVTKParser.h>
#include <VTMParser.h>

#include <snprintf.h>
#include <DebugStream.h>
#include <Expression.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <vtkVisItUtility.h>

#include <map>
#include <string>
#include <vector>

using std::string;
using std::vector;

//
// Define the static const's
//
const char   *avtVTKFileReader::MESHNAME="mesh";
const char   *avtVTKFileReader::VARNAME="VTKVar";


static void GetListOfUniqueCellTypes(vtkUnstructuredGrid *ug,
                                     vtkUnsignedCharArray *uca);

int    avtVTKFileReader::INVALID_CYCLE = -INT_MAX;
double avtVTKFileReader::INVALID_TIME = -DBL_MAX;

// ****************************************************************************
//  Method: avtVTKFileReader constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    February 23, 2001
//
//  Modifications:
//    Hank Childs, Tue May 24 12:05:52 PDT 2005
//    Added arguments.
//
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Initialized matvarname
//
//    Kathleen Bonnell, Thu Sep 22 15:37:13 PDT 2005
//    Save the file extension.
//
//    Kathleen Bonnell, Thu Jun 29 17:30:40 PDT 2006
//    Add vtk_time, to store time from the VTK file if it is available.
//
//    Hank Childs, Mon Jun 11 21:27:04 PDT 2007
//    Do not assume there is an extension.
//
//    Kathleen Bonnell, Wed Jul  9 17:48:21 PDT 2008
//    Add vtk_cycle, to store cycle from the VTK file if it is available.
//
//    Brad Whitlock, Tue May 11 11:13:29 PDT 2010
//    Search for file extension from the back of the filename in case
//    directories contain "." and terminate if we hit a path separator.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Kathleen Biagas, Thu Aug 13 17:29:21 PDT 2015
//    Add support for groups and block names.
//
// ****************************************************************************

avtVTKFileReader::avtVTKFileReader(const char *fname, DBOptionsAttributes *) :
    vtk_meshname()
{
    filename = new char[strlen(fname)+1];
    strcpy(filename, fname);

    nblocks = 1;
    pieceFileNames = NULL;
    pieceDatasets = NULL;
    pieceExtents = NULL;

    readInDataset = false;
    matvarname = NULL;

    // find the file extension
    int i, start = -1;
    int len = strlen(fname);
    for(i = len-1; i >= 0; i--)
    {
        if(fname[i] == '.')
        {
            start = i;
            break;
        }
        else if(fname[i] == '/' || fname[i] == '\\')
        {
            // We hit a path separator. There is no file extension.
            start = -1;
            break;
        }
    }

    if (start != -1)
        fileExtension = string(fname, start+1, len-1);
    else
        fileExtension = "none";

    vtk_time = INVALID_TIME;
    vtk_cycle = INVALID_CYCLE;
}


// ****************************************************************************
//  Method: avtVTKFileReader::FreeUpResources
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
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Freed matvarname
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Mark C. Miller, Wed Jul  2 17:27:35 PDT 2014
//    Delete everything even VTK datasets read.
//
// ****************************************************************************
void
avtVTKFileReader::FreeUpResources(void)
{
    debug4 << "VTK file " << filename << " forced to free up resources." << endl;

    if (matvarname != NULL)
    {
        free(matvarname);
        matvarname = NULL;
    }
    if (pieceFileNames != NULL)
    {
        for (int i = 0; i < nblocks; i++)
            delete [] pieceFileNames[i];
        delete [] pieceFileNames;
        pieceFileNames = 0;
    }
    if (pieceDatasets != NULL)
    {
        for (int i = 0; i < nblocks; i++)
        {
            if (pieceDatasets[i] != NULL)
                pieceDatasets[i]->Delete();
        }
        delete [] pieceDatasets;
        pieceDatasets = 0;
    }
    if (pieceExtents != NULL)
    {
        for (int i = 0; i < nblocks; i++)
        {
            if (pieceExtents[i] != NULL)
                delete [] pieceExtents[i];
        }
        delete [] pieceExtents;
        pieceExtents = 0;
    }
    for(std::map<string, vtkRectilinearGrid *>::iterator pos = vtkCurves.begin();
        pos != vtkCurves.end(); ++pos)
    {
        pos->second->Delete();
    }
    vtkCurves.clear();

    readInDataset = false;
}

// ****************************************************************************
//  Method: avtVTKFileReader destructor
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Freed matvarname
//
//    Brad Whitlock, Wed Oct 26 11:03:14 PDT 2011
//    Delete curves in vtkCurves.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Burlen Loring, Fri Jul 11 11:19:36 PDT 2014
//    fix alloc-dealloc-mismatch (operator new [] vs free)
//
// ****************************************************************************

avtVTKFileReader::~avtVTKFileReader()
{
    FreeUpResources();
    delete [] filename;
}


// ****************************************************************************
// Method: avtVTKFileReader::GetNumberOfDomains
//
// Purpose:
//   Return the number of domains, reading the data file to figure it out.
//
// Returns:    The number of domains.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 22 17:08:06 PDT 2012
//
// Modifications:
//
// ****************************************************************************

int
avtVTKFileReader::GetNumberOfDomains()
{
    if (!readInDataset)
    {
        ReadInFile();
    }

    return nblocks;
}

// ****************************************************************************
//  Method: avtVTKFileReader::ReadInFile
//
//  Purpose:
//      Reads in the file.
//
//  Programmer: Eric Brugger
//  Creation:   June 18, 2012
//
//  Modifications:
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Kathleen Biagas, Thu Aug 13 17:29:21 PDT 2015
//    Add support for groups and block names, as read from 'vtm' file.
//
//    Kathleen Biagas, Thu Sep 21 14:59:31 MST 2017
//    Add support for pvtk files.
//
// ****************************************************************************

void
avtVTKFileReader::ReadInFile(int _domain)
{
    int domain = _domain == -1 ? 0 : _domain;

    if (fileExtension == "pvtu" || fileExtension == "pvts" ||
        fileExtension == "pvtr" || fileExtension == "pvti" ||
        fileExtension == "pvtp")
    {
        vtkVisItXMLPDataReader *xmlpReader = vtkVisItXMLPDataReader::New();
        xmlpReader->SetFileName(filename);
        xmlpReader->ReadXMLInformation();

        ngroups = 1;
        nblocks = xmlpReader->GetNumberOfPieces();
        pieceFileNames = new char*[nblocks];
        for (int i = 0; i < nblocks; i++)
        {
            pieceFileNames[i] = new char[strlen(xmlpReader->GetPieceFileName(i))+1];
            strcpy(pieceFileNames[i], xmlpReader->GetPieceFileName(i));
        }

        pieceExtents = new int*[nblocks];
        for (int i = 0; i < nblocks; i++)
        {
            int *readerExtent = xmlpReader->GetExtent(i);
            if (readerExtent == NULL)
            {
                pieceExtents[i] = NULL;
            }
            else
            {
                pieceExtents[i] = new int[6];
                int *ext = pieceExtents[i];
                ext[0] = readerExtent[0]; ext[1] = readerExtent[1];
                ext[2] = readerExtent[2]; ext[3] = readerExtent[3];
                ext[4] = readerExtent[4]; ext[5] = readerExtent[5];
            }
        }

        xmlpReader->Delete();

        pieceExtension = fileExtension.substr(1,3);
    }
    else if (fileExtension == "pvtk")
    {
        PVTKParser *parser = new PVTKParser();
        parser->SetFileName(filename);
        if (!parser->Parse())
        {
            string em = parser->GetErrorMessage();
            delete parser;
            EXCEPTION2(InvalidFilesException, filename, em);
        }

        ngroups = 1;
        nblocks = parser->GetNumberOfPieces();
        pieceFileNames = new char*[nblocks];
        for (int i = 0; i < nblocks; i++)
        {
            string pfn = parser->GetPieceFileName(i);
            pieceFileNames[i] = new char[pfn.size() +1];
            strcpy(pieceFileNames[i], pfn.c_str());
        }

        if (parser->HasExtents())
        {
            pieceExtents = new int*[nblocks];
            for (int i = 0; i < nblocks; i++)
            {
                vector< int >  &readerExtent = parser->GetPieceExtent(i);
                pieceExtents[i] = new int[6];
                int *ext = pieceExtents[i];
                ext[0] = readerExtent[0]; ext[1] = readerExtent[1];
                ext[2] = readerExtent[2]; ext[3] = readerExtent[3];
                ext[4] = readerExtent[4]; ext[5] = readerExtent[5];
            }
        }

        delete parser;

        pieceExtension = "vtk";
    }
    else if (fileExtension == "vtm")
    {
        VTMParser *parser = new VTMParser;
        parser->SetFileName(filename);
        if (!parser->Parse())
        {
            string em = parser->GetErrorMessage();
            delete parser;
            EXCEPTION2(InvalidFilesException, filename, em);
            return;
        }

        nblocks = parser->GetNumberOfBlocks();
        ngroups = parser->GetNumberOfGroups();
        if (ngroups > 1)
        {
            groupNames = parser->GetGroupNames();
            groupPieceName = parser->GetGroupPieceName();
            groupIds   = parser->GetGroupIds();
        }

        blockNames = parser->GetBlockNames();
        blockPieceName = parser->GetBlockPieceName();

        pieceFileNames = new char*[nblocks];
        for (int i = 0; i < nblocks; i++)
        {
            string bn = parser->GetBlockFileName(i);
            pieceFileNames[i] = new char[bn.size()+1];
            strcpy(pieceFileNames[i], bn.c_str());
        }
        pieceExtension = parser->GetBlockExtension();
        delete parser;
    }
    else
    {
        nblocks = 1;
        ngroups = 1;
        pieceFileNames = new char*[nblocks];
        pieceFileNames[0] = new char[strlen(filename)+1];
        strcpy(pieceFileNames[0], filename);
        pieceExtension = fileExtension;
    }


    pieceDatasets = new vtkDataSet*[nblocks];
    for (int i = 0; i < nblocks; i++)
        pieceDatasets[i] = NULL;

    if (pieceExtents == NULL)
    {
        pieceExtents = new int*[nblocks];
        for (int i = 0; i < nblocks; i++)
            pieceExtents[i] = NULL;
    }

    ReadInDataset(domain);

    readInDataset = true;
}


// ****************************************************************************
//  Method: avtVTKFileReader::ReadInDataset
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
//    Brad Whitlock, Wed Oct 26 11:04:50 PDT 2011
//    Create curves for 1D rectilinear grids.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Kathleen Biagas, Mon Jan 28 11:06:32 PST 2013
//    Remove calls to ds->Update.
//
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Mark C. Miller, Wed Jul  2 17:28:24 PDT 2014
//    Add duplicate node removal (special case). Controlling logic should
//    ensure it is rarely triggered.
//
//    Kathleen Biagas, Mon Dec 22 09:49:22 PST 2014
//    Moved logic for duplicate node removal into avtTransformManager, it
//    is now controlled by setting a global preference.
//
//    Kathleen Biagas, Fri Feb  6 06:00:16 PST 2015
//    Added ability for parsing 'MeshName' field data from vtk file.
//
//    Matt Larsen, Fri Mar 2 09:00:15 PST 2018
//    Getting image data extents correctly from vti files
//
//    Edward Rusu, Mon Oct 1 09:24:24 PST 2018
//    Added support for vtkGhostType.
// ****************************************************************************

void
avtVTKFileReader::ReadInDataset(int domain)
{
    debug4 << "Reading in dataset from VTK file " << filename << endl;

    //
    // This shouldn't ever happen (since we would already have the dataset
    // we are trying to read from the file sitting in memory), but anything
    // to prevent leaks.
    //
    if (pieceDatasets[domain] != NULL)
    {
        pieceDatasets[domain]->Delete();
        pieceDatasets[domain] = NULL;
    }

    vtkDataSet *dataset = NULL;

    if (pieceExtension == "vtk" || pieceExtension == "none")
    {
        if (pieceExtension == "none") {
            debug1 << "No extension given ... assuming legacy VTK format."
                   << endl;
        }

        //
        // Create a file reader and set our dataset to be its output.
        //
        vtkDataSetReader *reader = vtkDataSetReader::New();
        reader->ReadAllScalarsOn();
        reader->ReadAllVectorsOn();
        reader->ReadAllTensorsOn();
        reader->SetFileName(pieceFileNames[domain]);
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, pieceFileNames[domain]);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (pieceExtension == "vti")
    {
        vtkXMLImageDataReader *reader = vtkXMLImageDataReader::New();
        reader->SetFileName(pieceFileNames[domain]);
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, pieceFileNames[domain]);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (pieceExtension == "vtr")
    {
        vtkXMLRectilinearGridReader *reader =
            vtkXMLRectilinearGridReader::New();
        reader->SetFileName(pieceFileNames[domain]);
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, pieceFileNames[domain]);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (pieceExtension == "vts")
    {
        vtkXMLStructuredGridReader *reader =
            vtkXMLStructuredGridReader::New();
        reader->SetFileName(pieceFileNames[domain]);
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, pieceFileNames[domain]);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (pieceExtension == "vtp")
    {
        vtkXMLPolyDataReader *reader = vtkXMLPolyDataReader::New();
        reader->SetFileName(pieceFileNames[domain]);
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, pieceFileNames[domain]);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else if (pieceExtension == "vtu")
    {
        vtkXMLUnstructuredGridReader *reader =
            vtkXMLUnstructuredGridReader::New();
        reader->SetFileName(pieceFileNames[domain]);
        reader->Update();
        dataset = reader->GetOutput();
        if (dataset == NULL)
        {
            EXCEPTION1(InvalidFilesException, pieceFileNames[domain]);
        }
        dataset->Register(NULL);
        reader->Delete();
    }
    else
    {
        EXCEPTION2(InvalidFilesException, pieceFileNames[domain],
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
    vtk_meshname.clear();
    if (dataset->GetFieldData()->GetAbstractArray("MeshName") != 0)
    {
        vtkStringArray *mn = vtkStringArray::SafeDownCast(
            dataset->GetFieldData()->GetAbstractArray("MeshName"));
        if (mn)
            vtk_meshname = mn->GetValue(0);
    }

    if (dataset->GetDataObjectType() == VTK_STRUCTURED_POINTS ||
        dataset->GetDataObjectType() == VTK_IMAGE_DATA)
    {
        //
        // The old dataset passed in will be deleted, a new one will be
        // returned.
        //
        if(pieceExtents[domain] == NULL  &&
           dataset->GetDataObjectType() == VTK_IMAGE_DATA)
        {
          vtkImageData *img = vtkImageData::SafeDownCast(dataset);
          if(img)
          {
            int *ext  = img->GetExtent();
            dataset = ConvertStructuredPointsToRGrid((vtkStructuredPoints*)dataset,
                                                      ext);
          }
        }
        else
        {
            dataset = ConvertStructuredPointsToRGrid((vtkStructuredPoints*)dataset,
                                                     pieceExtents[domain]);
        }
    }

    if(dataset->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(dataset);
        int dims[3];
        rgrid->GetDimensions(dims);
        if(dims[0] > 0 && dims[1] <= 1 && dims[2] <= 1)
        {
            // Make some curves from this dataset.
            CreateCurves(rgrid);
        }
    }

    // Convert vtkGhostType to avtGhostDataType
    // Rename the arrays stored in dataset->GetCellData() and dataset->GetPointData()

    vtkDataArray *zoneArray = dataset->GetCellData()->GetArray("vtkGhostType");
    if (zoneArray)
    {
        zoneArray->SetName("avtGhostZones");
        dataset->GetCellData()->AddArray(zoneArray);
    }

    vtkDataArray *nodeArray = dataset->GetPointData()->GetArray("vtkGhostType");
    if (nodeArray)
    {
        nodeArray->SetName("avtGhostNodes");
        dataset->GetPointData()->AddArray(nodeArray);
    }

    pieceDatasets[domain] = dataset;
}


// ****************************************************************************
// Method: avtVTKFileReader::CreateCurves
//
// Purpose:
//   Create curve datasets based on the input rectilinear grid.
//
// Arguments:
//   rgrid : The rectilinear grid from which to create curves.
//
// Returns:
//
// Note:       vtkCurves gets the new datasets.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 26 11:01:44 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
avtVTKFileReader::CreateCurves(vtkRectilinearGrid *rgrid)
{
    vtkDataArray *xc = rgrid->GetXCoordinates();
    int nPts = xc->GetNumberOfTuples();

    for(int i = 0; i < rgrid->GetPointData()->GetNumberOfArrays(); ++i)
    {
        vtkDataArray *arr = rgrid->GetPointData()->GetArray(i);
        if(arr->GetNumberOfComponents() == 1)
        {
            vtkRectilinearGrid *curve = vtkVisItUtility::Create1DRGrid(nPts,VTK_FLOAT);
            vtkDataArray *curve_xc = curve->GetXCoordinates();
            vtkFloatArray *curve_yc = vtkFloatArray::New();
            curve_yc->SetName(arr->GetName());
            curve_yc->SetNumberOfTuples(nPts);
            for(vtkIdType j = 0; j < nPts; ++j)
            {
                curve_xc->SetTuple1(j, xc->GetTuple1(j));
                curve_yc->SetTuple1(j, arr->GetTuple1(j));
            }
            curve->GetPointData()->SetScalars(curve_yc);
            curve_yc->Delete();

            vtkCurves[string("curve_") + string(arr->GetName())] = curve;
        }
    }

    for(int i = 0; i < rgrid->GetCellData()->GetNumberOfArrays(); ++i)
    {
        vtkDataArray *arr = rgrid->GetCellData()->GetArray(i);
        if(arr->GetNumberOfComponents() == 1)
        {
            vtkRectilinearGrid *curve = vtkVisItUtility::Create1DRGrid(nPts,VTK_FLOAT);
            vtkDataArray *curve_xc = curve->GetXCoordinates();
            for(vtkIdType j = 0; j < nPts; ++j)
                curve_xc->SetTuple1(j, xc->GetTuple1(j));

            int nCells = nPts-1;
            int idx = 0;
            vtkFloatArray *curve_yc = vtkFloatArray::New();
            curve_yc->SetName(arr->GetName());
            curve_yc->SetNumberOfTuples(nPts);
            curve_yc->SetTuple1(idx++, arr->GetTuple1(0));
            for(vtkIdType j = 0; j < nCells-1; ++j)
                curve_yc->SetTuple1(idx++, (arr->GetTuple1(j) + arr->GetTuple1(j+1)) / 2.);
            curve_yc->SetTuple1(idx++, arr->GetTuple1(nCells-1));

            curve->GetPointData()->SetScalars(curve_yc);
            curve_yc->Delete();

            vtkCurves[string("curve_") + string(arr->GetName())] = curve;
        }
    }
}


// ****************************************************************************
//  Method: avtVTKFileReader::GetAuxiliaryData
//
//  Programmer: Mark C. Miller
//  Creation:   September 15, 2005
//
//  Modifications:
//    Hank Childs, Fri Feb 15 11:25:32 PST 2008
//    Fix memory leak.
//
//    Cyrus Harrison, Fri Jan  7 10:17:19 PST 2011
//    Determine the proper material variable name & material metadata
//    if PopulateDatabaseMetaData has not been called. This supports materials
//    in the case we multiple vtk files acting as separate timesteps.
//
//    Cyrus Harrison, Wed Nov 16 13:35:54 PST 2011
//    Use "MaterialIds" field data to help generate avtMaterials result object.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Kathleen Biagas, Mon Nov 20 13:04:51 PST 2017
//    Pass domain to the GetVar call when retrieving materials.
//
// ****************************************************************************

void *
avtVTKFileReader::GetAuxiliaryData(const char *var, int domain,
    const char *type, void *, DestructorFunction &df)
{
    void *rv = NULL;

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        vtkDataSet *dataset = NULL;

        // matvarname is only inited if we call: PopulateDatabaseMetaData().
        // If you have a series of vtk files using this variable will cause
        // a crash any time the time slider is changed (and treat all dbs
        // as time varying is off)

        if(matvarname == NULL)
        {
            if (!readInDataset)
            {
                ReadInFile(domain);
            }

            if (pieceDatasets[domain] == NULL)
            {
                ReadInDataset(domain);
            }
            dataset = pieceDatasets[domain];

            int ncellvars = dataset->GetCellData()->GetNumberOfArrays();
            for(int i=0;( i < ncellvars) && (matvarname == NULL) ;i++)
            {
                // we are looking for either "avtSubsets" or "material"
                if(strcmp(dataset->GetCellData()->GetArrayName(i), "avtSubsets") == 0)
                    matvarname = strdup("avtSubsets");
                else if(strcmp(dataset->GetCellData()->GetArrayName(i), "material") == 0)
                    matvarname = strdup("material");
            }
        }
        else
        {
            dataset = pieceDatasets[domain];
        }

        vtkIntArray *matarr = vtkIntArray::SafeDownCast(GetVar(domain, matvarname));


        // again, if we haven't called PopulateDatabaseMetaData().
        // this data will be bad ...
        if(matnos.size() == 0)
        {
            vtkIntArray  *iarr = NULL;
            // check for field data "MaterialIds" that can directly provide us
            // the proper set of material ids.
            vtkDataArray *mids_arr = dataset->GetFieldData()->GetArray("MaterialIds");
            if( mids_arr != NULL)
                iarr = vtkIntArray::SafeDownCast(mids_arr);
            else
                iarr = vtkIntArray::SafeDownCast(matarr);
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
        }

        int ntuples = matarr->GetNumberOfTuples();
        int *matlist = matarr->GetPointer(0);

        int *matnostmp = new int[matnos.size()];
        char **matnamestmp = new char*[matnames.size()];
        for (size_t i = 0; i < matnos.size(); i++)
        {
            matnostmp[i] = matnos[i];
            matnamestmp[i] = (char*) matnames[i].c_str();
        }

        avtMaterial *mat = new avtMaterial((int)matnos.size(), //silomat->nmat,
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
//  Method: avtVTKFileReader::GetMesh
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
//    Hank Childs, Tue Mar 26 13:33:43 PST 2002
//    Add a reference so that reference counting tricks work.
//
//    Brad Whitlock, Wed Oct 26 11:08:31 PDT 2011
//    Return curves.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Kathleen Biagas, Fri Feb  6 06:06:24 PST 2015
//    Use meshname from file (vtk_meshname), if available.

//    Kathleen Biagas, Thu Apr  2 12:22:55 PDT 2015
//    Return NULL a dataset with 0 points is read in.
//
// ****************************************************************************

vtkDataSet *
avtVTKFileReader::GetMesh(int domain, const char *mesh)
{
    debug5 << "Getting mesh from VTK file \"" << filename << "\", domain = "
           << domain << endl;

    if (!readInDataset)
    {
        ReadInFile(domain);
    }

    if (pieceDatasets[domain] == NULL)
    {
        ReadInDataset(domain);
    }
    vtkDataSet *dataset = pieceDatasets[domain];

    if (dataset->GetNumberOfPoints() == 0)
    {
        return NULL;
    }

    // If the requested mesh is a curve, return it.
    std::map<string, vtkRectilinearGrid *>::iterator pos = vtkCurves.find(mesh);
    if(pos != vtkCurves.end())
    {
        pos->second->Register(NULL);
        return pos->second;
    }

    if(vtk_meshname.empty())
    {
        if (strcmp(mesh, MESHNAME) != 0)
        {
            EXCEPTION1(InvalidVariableException, mesh);
        }
    }
    else
    {
        if (strcmp(mesh, vtk_meshname.c_str()) != 0)
        {
            EXCEPTION1(InvalidVariableException, mesh);
        }
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
//  Method: avtVTKFileReader::GetVar
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
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
// ****************************************************************************

vtkDataArray *
avtVTKFileReader::GetVar(int domain, const char *real_name)
{
    debug5 << "Getting var from VTK file " << filename << endl;

    if (!readInDataset)
    {
        ReadInFile(domain);
    }

    if (pieceDatasets[domain] == NULL)
    {
        ReadInDataset(domain);
    }
    vtkDataSet *dataset = pieceDatasets[domain];

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
//  Method: avtVTKFileReader::GetVectorVar
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
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
// ****************************************************************************

vtkDataArray *
avtVTKFileReader::GetVectorVar(int domain, const char *var)
{
    //
    // There is no difference between vectors and scalars for this class.
    //
    return GetVar(domain, var);
}


// ****************************************************************************
//  Method: avtVTKFileReader::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
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
//    Jeremy Meredith, Mon Nov  9 13:03:18 EST 2009
//    Expand the test for lower topological dimensions to include
//    structured grids.
//
//    Hank Childs, Wed Sep 14 16:29:19 PDT 2011
//    Improve handling of ghost data.
//
//    Brad Whitlock, Wed Oct 26 11:12:17 PDT 2011
//    Add metadata for curves.
//
//    Cyrus Harrison, Wed Nov 16 13:35:54 PST 2011
//    Use "MaterialIds" field data to help generate materials metadata.
//
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Mark C. Miller, Wed Jul  2 17:26:44 PDT 2014
//    FreeUpResources before leaving. This is to ensure mdserver and non-zero
//    mpi-ranks don't hang onto the VTK data read here solely for purposes
//    of populating md.
//
//    Kathleen Biagas, Fri Feb  6 06:00:16 PST 2015
//    Use 'MeshName' from file if provided (stored in vtk_meshname).
//
//    Kathleen Biagas, Thu Aug 13 17:29:21 PDT 2015
//    Add support for groups and block names.
//
//    Kathleen Biagas, Thu Nov 15 09:20:40 PST 2018
//    If unstructured grid has declared no cells (valid in xml verisons),
//    assume it is a point mesh and set topodim to 0.
//
//    Kathleen Biagas, Tue Sep 10 12:11:23 PDT 2019
//    Test UnstructedGrids and vtkPolyData for existence of Points before
//    determining if the topological dimension should be lowered. Lack of
//    points indicates an empty dataset.
//
//    Kathleen Biagas, Thu Oct 31 12:26:22 PDT 2019
//    Set mesh type to POINT_MESH when poly data contains only vertex cells.
//
// ****************************************************************************

void
avtVTKFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!readInDataset)
    {
        ReadInFile();
    }

    vtkDataSet *dataset = pieceDatasets[0];

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
    // Some mesh types can have a lower topological dimension
    //
    if (vtkType == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *) dataset;

        if(ugrid->GetNumberOfPoints() > 0)
        {
            if (ugrid->GetNumberOfCells() == 0)
            {
                // no cells declared, assume  point mesh.
                debug5 << "The VTK file format contains all points -- "
                       << "declaring this a point mesh." << endl;
                type = AVT_POINT_MESH;
                topo = 0;
            }
            else
            {
                vtkUnsignedCharArray *types = vtkUnsignedCharArray::New();
                GetListOfUniqueCellTypes(ugrid, types);

                if (types->GetNumberOfTuples() == 1)
                {
                    int myType = (int) types->GetValue(0);
                    if (myType == VTK_VERTEX)
                    {
                        debug5 << "The VTK file format contains all points -- "
                               << "declaring this a point mesh." << endl;
                        type = AVT_POINT_MESH;
                        topo = 0;
                    }
                    else if(myType == VTK_LINE)
                    {
                        debug5 << "The mesh contains all lines, set topo=1" << endl;
                        topo = 1;
                    }
                }
                types->Delete();
            }
        }
    }
    else if (vtkType == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = (vtkStructuredGrid *) dataset;
        int dims[3];
        sgrid->GetDimensions(dims);
        if ((dims[0] == 1 && dims[1] == 1) ||
            (dims[0] == 1 && dims[2] == 1) ||
            (dims[1] == 1 && dims[2] == 1))
        {
            topo = 1;
        }
        else if (dims[0] == 1 || dims[1] == 1 || dims[2] == 1)
        {
            topo = 2;
        }
    }
    else if (vtkType == VTK_POLY_DATA)
    {
        vtkPolyData *pd = (vtkPolyData *) dataset;
        if (pd->GetNumberOfPoints() > 0)
        {
            if (pd->GetNumberOfPolys() == 0 && pd->GetNumberOfStrips() == 0)
            {
                if (pd->GetNumberOfLines() > 0)
                {
                    topo = 1;
                }
                else
                {
                    debug3 << "The VTK file format contains all points -- "
                           << "declaring this a point mesh." << endl;
                    type = AVT_POINT_MESH;
                    topo = 0;
                }
            }
        }
    }

    avtMeshMetaData *mesh = new avtMeshMetaData;
    if(vtk_meshname.empty())
    {
        mesh->name = MESHNAME;
    }
    else
    {
        mesh->name = vtk_meshname;
    }
    mesh->meshType = type;
    mesh->spatialDimension = spat;
    mesh->topologicalDimension = topo;
    if (ngroups > 1)
    {
        mesh->numGroups = ngroups;
        if (!groupNames.empty())
            mesh->groupNames = groupNames;
        if (!groupPieceName.empty())
        {
            mesh->groupPieceName = groupPieceName;
            mesh->groupTitle = groupPieceName + string("s");
        }
        mesh->groupIds = groupIds;
    }
    mesh->numBlocks = nblocks;
    mesh->blockOrigin = 0;
    if (nblocks == 1)
        mesh->SetExtents(bounds);
    else
    {
        if (!blockPieceName.empty())
        {
            mesh->blockPieceName = blockPieceName;
            mesh->blockTitle = blockPieceName + string("s");
        }
        if (!blockNames.empty() && (int)blockNames.size() == nblocks)
            mesh->blockNames = blockNames;
    }
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
    if (dataset->GetCellData()->GetArray("avtGhostZones"))
    {
        mesh->containsGhostZones = AVT_HAS_GHOSTS;
        int ncells = dataset->GetNumberOfCells();
        vtkUnsignedCharArray *arr = (vtkUnsignedCharArray *) dataset->GetCellData()->GetArray("avtGhostZones");
        unsigned char *ptr = arr->GetPointer(0);
        unsigned char v = '\0';
        avtGhostData::AddGhostZoneType(v, ZONE_EXTERIOR_TO_PROBLEM);
        for (int i = 0 ; i < ncells ; i++)
            if (ptr[i] & v)
            {
                mesh->containsExteriorBoundaryGhosts = true;
                break;
            }
    }
    else
        mesh->containsGhostZones = AVT_NO_GHOSTS;

    md->Add(mesh);

    std::map<string, vtkRectilinearGrid *>::iterator pos;
    for(pos = vtkCurves.begin(); pos != vtkCurves.end(); ++pos)
    {
        avtCurveMetaData *curve = new avtCurveMetaData;
        curve->name = pos->first;
        curve->hasSpatialExtents = false;
        curve->hasDataExtents = false;
        md->Add(curve);
    }

    int nvars = 0;

    for (int i = 0 ; i < dataset->GetPointData()->GetNumberOfArrays() ; i++)
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
            AddScalarVarToMetaData(md, name, mesh->name, AVT_NODECENT, NULL, ascii);
        }
        else if (ncomp <= 4)
        {
            AddVectorVarToMetaData(md, name, mesh->name, AVT_NODECENT, ncomp);
        }
        else if (ncomp == 9)
        {
            AddTensorVarToMetaData(md, name, mesh->name, AVT_NODECENT);
        }
        else
        {
            if(arr->GetDataType() == VTK_UNSIGNED_CHAR ||
               arr->GetDataType() == VTK_CHAR)
            {
                md->Add(new avtLabelMetaData(name, mesh->name, AVT_NODECENT));
            }
            else
            {
                AddArrayVarToMetaData(md, name, ncomp, mesh->name, AVT_NODECENT);
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
    for (int i = 0 ; i < dataset->GetCellData()->GetNumberOfArrays() ; i++)
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
            std::map<int, bool> valMap;
            vtkIntArray  *iarr = NULL;
            // check for field data "MaterialIds" that can directly provide us
            // the proper set of material ids.
            vtkDataArray *mids_arr = dataset->GetFieldData()->GetArray("MaterialIds");
            if( mids_arr != NULL)
                iarr = vtkIntArray::SafeDownCast(mids_arr);
            else
                iarr = vtkIntArray::SafeDownCast(arr);

            int *iptr = iarr->GetPointer(0);
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

            avtMaterialMetaData *mmd =
                new avtMaterialMetaData("materials", mesh->name,
                                        (int)valMap.size(), matnames);
            md->Add(mmd);

            if (strncmp(name, "internal_var_Subsets", strlen("internal_var_Subsets")) == 0)
                matvarname = strdup("internal_var_Subsets");
            else
                matvarname = strdup("material");
        }
        else if (ncomp == 1)
        {
            bool ascii = arr->GetDataType() == VTK_CHAR;
            AddScalarVarToMetaData(md, name, mesh->name, AVT_ZONECENT, NULL, ascii);
        }
        else if (ncomp <= 4)
        {
            AddVectorVarToMetaData(md, name, mesh->name, AVT_ZONECENT, ncomp);
        }
        else if (ncomp == 9)
        {
            AddTensorVarToMetaData(md, name, mesh->name, AVT_ZONECENT);
        }
        else
        {
            if(arr->GetDataType() == VTK_UNSIGNED_CHAR ||
               arr->GetDataType() == VTK_CHAR)
            {
                md->Add(new avtLabelMetaData(name, mesh->name, AVT_ZONECENT));
            }
            else
            {
                AddArrayVarToMetaData(md, name, ncomp, mesh->name, AVT_ZONECENT);
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

    // Don't hang on to all the data we've read. We might not even need it
    // if we're in mdserver or of on non-zero mpi-rank.
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtVTKFileReader::IsEmpty
//
//  Purpose:
//      Returns a flag indicating if the file contains an empty dataset.
//
//  Programmer: Eric Brugger
//  Creation:   June 20, 2017
//
//  Modifications:
//
// ****************************************************************************

bool
avtVTKFileReader::IsEmpty()
{
    if (!readInDataset)
    {
        ReadInFile();
    }

    vtkDataSet *dataset = pieceDatasets[0];

    if (dataset->GetNumberOfCells() == 0 && dataset->GetNumberOfPoints() == 0)
    {
        FreeUpResources();
        return true;
    }

    return false;
}


// ****************************************************************************
//  Function: GetListOfUniqueCellTypes
//
//  Purpose:
//     Gets a list of the unique cell types.
//
//  Notes: This is done externally to the similar method in
//         vtkUnstructuredGrid, since that method is buggy and can get
//         into an infinite loop.
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
//  Notes: The passed in dataset will be deleted.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 9, 2004
//
//  Modifications:
//    Eric Brugger, Tue Jul  9 09:36:44 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Kathleen Biagas, Fri Nov  1 13:27:44 PDT 2013
//    Changed pieceOrigin from int to double to prevent truncating.
//
//    Kathleen Biagas, Thu Sep 14 13:45:00 PDT 2017
//    Take the quick copy-array route when pieceDims == wholeDims.
//
// ****************************************************************************

vtkDataSet *
avtVTKFileReader::ConvertStructuredPointsToRGrid(vtkStructuredPoints *inSP,
    int *extents)
{
    int wholeDims[3];
    double spacing[3];
    double wholeOrigin[3];
    inSP->GetDimensions(wholeDims);
    inSP->GetSpacing(spacing);
    inSP->GetOrigin(wholeOrigin);

    int pieceDims[3];
    double pieceOrigin[3];
    bool pieceEqualWhole = false;
    if (extents == NULL)
    {
        pieceDims[0] = wholeDims[0];
        pieceDims[1] = wholeDims[1];
        pieceDims[2] = wholeDims[2];
        pieceOrigin[0] = wholeOrigin[0];
        pieceOrigin[1] = wholeOrigin[1];
        pieceOrigin[2] = wholeOrigin[2];
        pieceEqualWhole = true;
    }
    else
    {
        pieceDims[0] = extents[1] - extents[0] + 1;
        pieceDims[1] = extents[3] - extents[2] + 1;
        pieceDims[2] = extents[5] - extents[4] + 1;
        pieceOrigin[0] = wholeOrigin[0] + extents[0] * spacing[0];
        pieceOrigin[1] = wholeOrigin[1] + extents[2] * spacing[1];
        pieceOrigin[2] = wholeOrigin[2] + extents[4] * spacing[2];

        pieceEqualWhole = (wholeDims[0] == pieceDims[0]) &&
                          (wholeDims[1] == pieceDims[1]) &&
                          (wholeDims[2] == pieceDims[2]);
    }

    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfComponents(1);
    x->SetNumberOfTuples(pieceDims[0]);
    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfComponents(1);
    y->SetNumberOfTuples(pieceDims[1]);
    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfComponents(1);
    z->SetNumberOfTuples(pieceDims[2]);

    vtkRectilinearGrid *outRG = vtkRectilinearGrid::New();
    outRG->SetDimensions(pieceDims);
    outRG->SetXCoordinates(x);
    outRG->SetYCoordinates(y);
    outRG->SetZCoordinates(z);
    x->Delete();
    y->Delete();
    z->Delete();

    int i;
    float *ptr = x->GetPointer(0);
    for (i = 0; i < pieceDims[0]; i++, ptr++)
        *ptr = pieceOrigin[0] + i * spacing[0];

    ptr = y->GetPointer(0);
    for (i = 0; i < pieceDims[1]; i++, ptr++)
        *ptr = pieceOrigin[1] + i * spacing[1];

    ptr = z->GetPointer(0);
    for (i = 0; i < pieceDims[2]; i++, ptr++)
        *ptr = pieceOrigin[2] + i * spacing[2];

    if (extents == NULL || pieceEqualWhole)
    {
        for (i = 0; i < inSP->GetPointData()->GetNumberOfArrays(); i++)
            outRG->GetPointData()->AddArray(inSP->GetPointData()->GetArray(i));

        for (i = 0; i < inSP->GetCellData()->GetNumberOfArrays(); i++)
            outRG->GetCellData()->AddArray(inSP->GetCellData()->GetArray(i));
    }
    else
    {
        for (i = 0; i < inSP->GetPointData()->GetNumberOfArrays(); i++)
        {
            vtkDataArray *in = inSP->GetPointData()->GetArray(i);

            vtkDataArray *out = vtkDataArray::CreateDataArray(in->GetDataType());
            out->SetName(in->GetName());
            out->SetNumberOfComponents(in->GetNumberOfComponents());

            unsigned long ntuples = pieceDims[0] * pieceDims[1] * pieceDims[2];
            out->SetNumberOfTuples(ntuples);

            vtkIdType outIndex = 0;
            vtkIdType nx  = wholeDims[0];
            vtkIdType nxy = wholeDims[0] * wholeDims[1];
            for (unsigned int iZ = extents[4]; iZ < (unsigned int)extents[5]; iZ++)
            {
                for (unsigned int iY = extents[2]; iY < (unsigned int)extents[3]; iY++)
                {
                    for (unsigned int iX = extents[0]; iX < (unsigned int)extents[1]; iX++)
                    {
                        vtkIdType inIndex = iZ * nxy + iY * nx + iX;
                        out->SetTuple(outIndex, inIndex, in);
                        outIndex++;
                    }
                }
            }
            outRG->GetPointData()->AddArray(out);
        }

        for (i = 0; i < inSP->GetCellData()->GetNumberOfArrays(); i++)
        {
            vtkDataArray *in = inSP->GetCellData()->GetArray(i);

            vtkDataArray *out = vtkDataArray::CreateDataArray(in->GetDataType());
            out->SetName(in->GetName());
            out->SetNumberOfComponents(in->GetNumberOfComponents());

            int pieceZonalDims[3];
            pieceZonalDims[0] = (pieceDims[0] <= 1) ? 1 : (pieceDims[0] - 1);
            pieceZonalDims[1] = (pieceDims[1] <= 1) ? 1 : (pieceDims[1] - 1);
            pieceZonalDims[2] = (pieceDims[2] <= 1) ? 1 : (pieceDims[2] - 1);
            out->SetNumberOfTuples(pieceZonalDims[0] *
                                   pieceZonalDims[1] *
                                   pieceZonalDims[2]);

            int wholeZonalDims[3];
            wholeZonalDims[0] = (wholeDims[0] <= 1) ? 1 : (wholeDims[0] - 1);
            wholeZonalDims[1] = (wholeDims[1] <= 1) ? 1 : (wholeDims[1] - 1);
            wholeZonalDims[2] = (wholeDims[2] <= 1) ? 1 : (wholeDims[2] - 1);
            vtkIdType outIndex = 0;
            vtkIdType nX  = wholeZonalDims[0];
            vtkIdType nXY = wholeZonalDims[0] * wholeZonalDims[1];
            int zoneExtents[6];
            zoneExtents[0] = extents[0];
            zoneExtents[1] = extents[0] + pieceZonalDims[0];
            zoneExtents[2] = extents[2];
            zoneExtents[3] = extents[2] + pieceZonalDims[1];
            zoneExtents[4] = extents[4];
            zoneExtents[5] = extents[4] + pieceZonalDims[2];
            for (int iZ = zoneExtents[4]; iZ < zoneExtents[5]; iZ++)
            {
                for (int iY = zoneExtents[2]; iY < zoneExtents[3]; iY++)
                {
                    for (int iX = zoneExtents[0]; iX < zoneExtents[1]; iX++)
                    {
                        vtkIdType inIndex = iZ * nXY + iY * nX + iX;
                        out->SetTuple(outIndex, inIndex, in);
                        outIndex++;
                    }
                }
            }
            outRG->GetCellData()->AddArray(out);
        }
    }

    inSP->Delete();
    return outRG;
}

// ****************************************************************************
//  Method: avtVTKFileReader::GetTime
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
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
// ****************************************************************************

double
avtVTKFileReader::GetTime()
{
    if (INVALID_TIME == vtk_time && !readInDataset)
        ReadInFile();
    return vtk_time;
}


// ****************************************************************************
//  Method: avtVTKFileReader::GetCycle
//
//  Purpose: Return the cycle associated with this file
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 9, 2008
//
//  Modifications:
//    Eric Brugger, Mon Jun 18 12:28:25 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
// ****************************************************************************

int
avtVTKFileReader::GetCycle()
{
    if (INVALID_CYCLE == vtk_cycle && !readInDataset)
        ReadInFile();
    return vtk_cycle;
}

// ****************************************************************************
//  Method: avtFileFormat::AddScalarVarToMetaData
//
//  Purpose:
//      A convenience routine to add a scalar variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the scalar var to.
//      name      The name of the scalar variable.
//      mesh      The mesh the scalar var is defined on.
//      cent      The centering type - node vs cell.
//      extents   The extents of the scalar var. (optional)
//      treatAsASCII   Whether the var is 'ascii' (optional)
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed Jul 13 18:28:51 PDT 2005
//    Add optional bool 'treatAsASCII' arg.
//
// ****************************************************************************

void
avtVTKFileReader::AddScalarVarToMetaData(avtDatabaseMetaData *md, string name,
                                      string mesh, avtCentering cent,
                                      const double *extents,
                                      const bool treatAsASCII)
{
    avtScalarMetaData *scalar = new avtScalarMetaData();
    scalar->name = name;
    scalar->meshName = mesh;
    scalar->centering = cent;
    if (extents != NULL)
    {
        scalar->hasDataExtents = true;
        scalar->SetExtents(extents);
    }
    else
    {
        scalar->hasDataExtents = false;
    }
    scalar->treatAsASCII = treatAsASCII;

    md->Add(scalar);
}


// ****************************************************************************
//  Method: avtFileFormat::AddVectorVarToMetaData
//
//  Purpose:
//      A convenience routine to add a vector variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the vector var to.
//      name      The name of the vector variable.
//      mesh      The mesh the vector var is defined on.
//      cent      The centering type - node vs cell.
//      dim       The dimension of the vector variable. (optional = 3)
//      extents   The extents of the vector var. (optional)
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
// ****************************************************************************

void
avtVTKFileReader::AddVectorVarToMetaData(avtDatabaseMetaData *md, string name,
                                      string mesh, avtCentering cent,
                                      int dim, const double *extents)
{
    avtVectorMetaData *vector = new avtVectorMetaData();
    vector->name = name;
    vector->meshName = mesh;
    vector->centering = cent;
    vector->varDim = dim;
    if (extents != NULL)
    {
        vector->hasDataExtents = true;
        vector->SetExtents(extents);
    }
    else
    {
        vector->hasDataExtents = false;
    }

    md->Add(vector);
}


// ****************************************************************************
//  Method: avtFileFormat::AddTensorVarToMetaData
//
//  Purpose:
//      A convenience routine to add a tensor variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the tensor variable.
//      mesh      The mesh the tensor var is defined on.
//      cent      The centering type - node vs cell.
//      dim       The dimension of the tensor variable. (optional = 3)
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2003
//
// ****************************************************************************

void
avtVTKFileReader::AddTensorVarToMetaData(avtDatabaseMetaData *md, string name,
                                      string mesh, avtCentering cent, int dim)
{
    avtTensorMetaData *tensor = new avtTensorMetaData();
    tensor->name = name;
    tensor->meshName = mesh;
    tensor->centering = cent;
    tensor->dim = dim;

    md->Add(tensor);
}

// ****************************************************************************
//  Method: avtFileFormat::AddArrayVarToMetaData
//
//  Purpose:
//      A convenience routine to add a array variable to the meta-data.
//
//  Arguments:
//      md        The meta-data object to add the tensor var to.
//      name      The name of the array variable.
//      ncomps    The number of components.
//      mesh      The mesh the array var is defined on.
//      cent      The centering type - node vs cell.
//
//  Programmer: Hank Childs
//  Creation:   July 21, 2005
//
// ****************************************************************************

void
avtVTKFileReader::AddArrayVarToMetaData(avtDatabaseMetaData *md, string name,
                                     int ncomps, string mesh,avtCentering cent)
{
    avtArrayMetaData *st = new avtArrayMetaData();
    st->name = name;
    st->nVars = ncomps;
    st->compNames.resize(ncomps);
    for (int i = 0 ; i < ncomps ; i++)
    {
        char name[16];
        SNPRINTF(name, 16, "comp%02d", i);
        st->compNames[i] = name;
    }
    st->meshName = mesh;
    st->centering = cent;

    md->Add(st);
}
