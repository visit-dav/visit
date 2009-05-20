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
//                           avtDatasetFileWriter.C                          //
// ************************************************************************* //

#include <avtDatasetFileWriter.h>

#include <snprintf.h>
#include <visitstream.h>

#include <vtkAppendFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataSetWriter.h>
#include <vtkInformation.h>
#include <vtkFloatArray.h>
#include <vtkOBJWriter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItSTLWriter.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkGeometryFilter.h>
#include <avtColorTables.h>
#include <ColorTableAttributes.h>
#include <ColorControlPointList.h>
#include <ColorControlPoint.h>
#include <AtomicProperties.h>
#include <StringHelpers.h>

#include <avtCommonDataFunctions.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoCurveException.h>
#include <NoInputException.h>

#include <float.h>

// This array contains strings that correspond to the file types that are 
// enumerated in the DatasetFileFormat enum.
const char *avtDatasetFileWriter::extensions[] = { ".curve", ".obj",
                                                   ".stl", ".vtk", ".ultra",
                                                   ".pov"};

static void SortLineSegments(vtkPolyData *, std::vector< std::vector<int> > &);
static void TakeOffPolyLine(int *, int, std::vector< std::vector<int> > &);


using   std::string;
using   std::vector;


// ****************************************************************************
//  Method: avtDatasetFileWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Mar 6 17:36:50 PST 2006
//    Added oldFileBase.
//
// ****************************************************************************

avtDatasetFileWriter::avtDatasetFileWriter()
{
    nFilesWritten = 0;
    oldFileBase = 0;
}


// ****************************************************************************
//  Method: avtDatasetFileWriter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//    Brad Whitlock, Mon Mar 6 17:37:26 PST 2006
//    Added oldFileBase.
//
// ****************************************************************************

avtDatasetFileWriter::~avtDatasetFileWriter()
{
    if(oldFileBase != 0)
        delete [] oldFileBase;
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::Write
//
//  Purpose:
//      Writes out the dataset to a file.
//
//  Arguments:
//      format   The format to write the file in.
//      base     The basename of the file.
//      binary   True if we should write out a binary file, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    May 24, 2002
//
//  Modifications:
//
//    Hank Childs, Sun May 26 19:32:08 PDT 2002
//    Re-wrote routine to reflect Curve, OBJ, and VTK files.
//
//    Jeremy Meredith, Sat Apr 12 14:37:40 PDT 2003
//    Added the ULTRA file format.
//
//    Jeremy Meredith, Tue Dec 30 09:13:08 PST 2003
//    Removed the Curve format and had the curve format write ULTRA files
//    instead.  Renamed ULTRA to curve internally.
//
//    Jeremy Meredith, Thu Apr  5 17:23:37 EDT 2007
//    Added POVRay file type.
//
// ****************************************************************************

void
avtDatasetFileWriter::Write(DatasetFileFormat format, const char *filename,
                            bool binary)
{
    switch (format)
    {
      case CURVE:
        WriteCurveFile(filename);
        break;
      case OBJ:
        WriteOBJFamily(filename);
        break;
      case POVRAY:
        WritePOVRayFamily(filename);
        break;
      case STL:
        WriteSTLFile(filename, binary);
        break;
      case ULTRA:
        WriteCurveFile(filename);
        break;
      case VTK:
        WriteVTKFamily(filename, binary);
        break;

      default:
        // Don't know what type to write out.
        debug1 << "Cannot handle format of type " << format << endl;
        EXCEPTION0(ImproperUseException);
    }
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WriteOBJFamily
//
//  Purpose:
//      Writes out the input as a wavefront obj file.  This will remap the 
//      data to a texture coordinate between 0 and 1.
//
//  Programmer: Hank Childs
//  Creation:   May 27, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 10 14:03:12 PDT 2002
//    Do not put 'FORMAT' in the .visit file, since it has been antiquated.
//
//    Hank Childs, Fri Feb 15 16:25:30 PST 2008
//    Use SNPRINTF.  Also fix memory leak.
//
// ****************************************************************************

void
avtDatasetFileWriter::WriteOBJFamily(const char *filename)
{
    avtDataTree_p dt = GetInputDataTree();
    if (dt->GetNumberOfLeaves() == 1)
    {
        vtkDataSet *leaf = dt->GetSingleLeaf();
        WriteOBJFile(leaf, filename, NULL);
    }
    else
    {
        //
        // Re-construct the basename.
        //
        char *basename = new char[strlen(filename)+1];
        int badExt = strlen(extensions[(int) OBJ]);
        strncpy(basename, filename, strlen(filename)-badExt);
        basename[strlen(filename)-badExt] = '\0';

        //
        // Write out the VTK datasets as their own files.
        //
        int nFilesWritten = WriteOBJTree(dt, 0, basename);

        //
        // Now make a .visit file to bind them together.
        //
        char rootfile[1024];
        SNPRINTF(rootfile, 1024, "%s.visit", basename);
        ofstream ofile(rootfile);
        ofile << "!NBLOCKS " << nFilesWritten << endl;
        for (int i = 0 ; i < nFilesWritten ; i++)
        {
            char objname[1024];
            SNPRINTF(objname, 1024, "%s.%04d%s", basename, i, extensions[(int)OBJ]);
            ofile << objname << endl;
        }

        delete [] basename;
    }
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WriteOBJTree
//
//  Purpose:
//      Writes out an avt data tree.
//
//  Arguments:
//      dt        The data tree to write.
//      idx       The current index of the file to write.
//      basename  The file name to use.
//
//  Returns:    The number of files written from dt.
//
//  Programmer: Hank Childs
//  Creation:   May 27, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 16:25:45 PST 2008
//    Use SNPRINTF.
//
// ****************************************************************************

int
avtDatasetFileWriter::WriteOBJTree(avtDataTree_p dt, int idx,
                                   const char *basename)
{
    if (*dt == NULL)
    {
        return 0;
    }

    int totalWritten = 0;
    if (dt->HasData())
    {
        avtDataRepresentation &rep = dt->GetDataRepresentation();
        vtkDataSet *ds = rep.GetDataVTK();
        char fname[1024];
        SNPRINTF(fname, 1024, "%s.%04d%s", basename, idx, extensions[(int)OBJ]);
        WriteOBJFile(ds, fname, rep.GetLabel().c_str());
        totalWritten = 1;
    }
    else
    {
        int workingIndex = idx;
        for (int i = 0 ; i < dt->GetNChildren() ; i++)
        {
            if (dt->ChildIsPresent(i))
            {
                int numWritten = WriteOBJTree(dt->GetChild(i), workingIndex,
                                              basename);
                workingIndex += numWritten;
                totalWritten += numWritten;
            }
        }
    }

    return totalWritten;
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WriteOBJFile
//
//  Purpose:
//      Writes the OBJ file associated with a single vtkDataSet.
//
//  Arguments:
//      ds     The vtkDataSet to write.
//      fname  The filename to use.
//      label  A description describing this file.
//
//  Programmer: Hank Childs
//  Creation:   May 27, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  2 15:16:50 PST 2003 
//    Replace MakeObject() with NewInstance() to match new vtk api.
// ****************************************************************************

void
avtDatasetFileWriter::WriteOBJFile(vtkDataSet *ds, const char *fname,
                                   const char *label)
{
    if (ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION0(NoInputException);
    }

    vtkDataSet *activeDS = ds;
    vtkCellDataToPointData *cd2pd = NULL;

    //
    // The OBJ file is going to expect the dataset as having node-centered
    // data.  
    //
    if (activeDS->GetCellData()->GetScalars() != NULL)
    {
        cd2pd = vtkCellDataToPointData::New();
        cd2pd->SetInput(activeDS);
        activeDS = cd2pd->GetOutput();
        activeDS->Update();
    }

    vtkDataSet *toBeWritten = (vtkDataSet *) activeDS->NewInstance();
    toBeWritten->ShallowCopy(activeDS);

    //
    // We are going to stuff the data into the texture coordinates, since that
    // is what is transferable between Maya, the TSB, and VisIt.
    //
    // I am converting the variable to the first component of a texture
    // coordinates.  The second is all 0 until I can think of something
    // better to do with it.
    //
    vtkDataArray *scalars = activeDS->GetPointData()->GetScalars();
    if (scalars != NULL)
    {
        //
        // Get some information for normalizing the variable.
        //
        double range[2];
        activeDS->GetScalarRange(range);
        double gap = (range[1] != range[0] ? range[1] - range[0] : 1.);

        //
        // Create the actual texture coordinate.
        //
        vtkFloatArray *tcoords = vtkFloatArray::New();
        tcoords->SetNumberOfComponents(2);
        tcoords->SetNumberOfTuples(scalars->GetNumberOfTuples());
        for (int i = 0 ; i < scalars->GetNumberOfTuples() ; i++)
        {
            double *p = scalars->GetTuple(i);
            double s[2];
            s[0] = (*p - range[0]) / gap;
            s[1] = 0.;
            tcoords->SetTuple(i, s);
        }
        toBeWritten->GetPointData()->SetTCoords(tcoords);
        tcoords->Delete();
    }

    vtkOBJWriter *writer = vtkOBJWriter::New();
    if (label != NULL && strlen(label) > 0)
    {
        writer->SetLabel(label);
    }
    writer->SetInput((vtkPolyData *) toBeWritten);
    writer->SetFileName(fname);
    writer->Write();
    writer->Delete();

    toBeWritten->Delete();
    if (cd2pd != NULL)
    {
        cd2pd->Delete();
    }
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WriteVTKFamily
//
//  Purpose:
//      Writes out the input as a VTK family. 
//
//  Programmer: Hank Childs
//  Creation:   May 27, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct 10 14:03:12 PDT 2002
//    Do not put 'FORMAT' in the .visit file, since it has been antiquated.
//
//    Hank Childs, Fri Feb 15 16:26:09 PST 2008
//    Use SNPRINTF.  Also fix memory leak.
//
// ****************************************************************************

void
avtDatasetFileWriter::WriteVTKFamily(const char *filename, bool binary)
{
    avtDataTree_p dt = GetInputDataTree();
    if (dt->GetNumberOfLeaves() == 1)
    {
        vtkDataSet *leaf = dt->GetSingleLeaf();
        WriteVTKFile(leaf, filename, binary);
    }
    else
    {
        //
        // Re-construct the basename.
        //
        char *basename = new char[strlen(filename)+1];
        int badExt = strlen(extensions[(int) VTK]);
        strncpy(basename, filename, strlen(filename)-badExt);
        basename[strlen(filename)-badExt] = '\0';

        //
        // Write out the VTK datasets as their own files.
        //
        int nFilesWritten = WriteVTKTree(dt, 0, basename, binary);

        //
        // Now make a .visit file to bind them together.
        //
        char rootfile[1024];
        SNPRINTF(rootfile, 1024, "%s.visit", basename);
        ofstream ofile(rootfile);
        ofile << "!NBLOCKS " << nFilesWritten << endl;
        for (int i = 0 ; i < nFilesWritten ; i++)
        {
            char vtkname[1024];
            SNPRINTF(vtkname, 1024, "%s.%04d%s", basename, i, extensions[(int)VTK]);
            ofile << vtkname << endl;
        }

        delete [] basename;
    }
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WriteVTKFile
//
//  Purpose:
//      Writes the VTK file associated with a single vtkDataSet.
//
//  Arguments:
//      ds     The vtkDataSet to write.
//      fname  The filename to use.
//      bin    True if we should write a binary file, false for ASCII.
//
//  Programmer: Hank Childs
//  Creation:   May 27, 2002
//
// ****************************************************************************

void
avtDatasetFileWriter::WriteVTKFile(vtkDataSet *ds, const char *fname, bool bin)
{
    vtkDataSetWriter *writer = vtkDataSetWriter::New();
    writer->SetInput(ds);
    if (bin)
    {
        writer->SetFileTypeToBinary();
    }
    else
    {
        writer->SetFileTypeToASCII();
    }
    writer->SetFileName(fname);
    writer->Write();
    writer->Delete();
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WriteVTKTree
//
//  Purpose:
//      Writes out an avt data tree.
//
//  Arguments:
//      dt        The data tree to write.
//      idx       The current index of the file to write.
//      basename  The file name to use.
//      bin       True if we should write out a binary file, false for ASCII.
//
//  Returns:    The number of files written from dt.
//
//  Programmer: Hank Childs
//  Creation:   May 27, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 16:26:25 PST 2008
//    Use SNPRINTF.
//
// ****************************************************************************

int
avtDatasetFileWriter::WriteVTKTree(avtDataTree_p dt, int idx,
                                   const char *basename, bool bin)
{
    if (*dt == NULL)
    {
        return 0;
    }

    int totalWritten = 0;
    if (dt->HasData())
    {
        avtDataRepresentation &rep = dt->GetDataRepresentation();
        vtkDataSet *ds = rep.GetDataVTK();
        char fname[1024];
        SNPRINTF(fname, 1024, "%s.%04d%s", basename, idx, extensions[(int)VTK]);
        WriteVTKFile(ds, fname, bin);
        totalWritten = 1;
    }
    else
    {
        int workingIndex = idx;
        for (int i = 0 ; i < dt->GetNChildren() ; i++)
        {
            if (dt->ChildIsPresent(i))
            {
                int numWritten = WriteVTKTree(dt->GetChild(i), workingIndex,
                                              basename, bin);
                workingIndex += numWritten;
                totalWritten += numWritten;
            }
        }
    }

    return totalWritten;
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WriteSTLFile
//
//  Purpose:
//      Writes out the input as a STL file.  This throws out all the
//      information except for the geometry.
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jul 31 17:14:02 PDT 2002
//    Made use of our own copy of the STL writer which has some fixed bugs.
//
// ****************************************************************************

void
avtDatasetFileWriter::WriteSTLFile(const char *filename, bool binary)
{
    vtkDataSet *ds = GetSingleDataset();

    if (ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION0(NoInputException);
    }

    //
    // STL can only handle triangles, so turn our quads, strips, etc, into
    // pure triangles.
    //
    vtkTriangleFilter *tris = vtkTriangleFilter::New();
    tris->SetPassLines(false);
    tris->SetPassVerts(false);
    tris->SetInput((vtkPolyData *) ds);

    vtkVisItSTLWriter *writer = vtkVisItSTLWriter::New();
    if (binary)
    {
        writer->SetFileTypeToBinary();
    }
    else
    {
        writer->SetFileTypeToASCII();
    }
    writer->SetFileName(filename);
    writer->SetInput(tris->GetOutput());
    writer->Write();
    writer->Delete();
    ds->Delete();
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WriteCurveFile
//
//  Purpose:
//      Writes out the input as an ULTRA style curve file.  This throws out
//      all the information except for the line segments.
//
//  Note:  Much of this was stolen from the now non-existent WriteCurveFile.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 12, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Dec 30 09:14:08 PST 2003
//    Removed the Curve writer and renamed this one to Curve.
//
// ****************************************************************************

void
avtDatasetFileWriter::WriteCurveFile(const char *filename)
{
    // We want it all in a single output file
    vtkDataSet *ds = GetSingleDataset();

    // We need polydata
    if (ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        EXCEPTION0(NoInputException);
    }

    vtkPolyData *pd = (vtkPolyData *) ds;

    // Get only the lines
    vtkCellArray *lines = pd->GetLines();
    int numCells = lines->GetNumberOfCells();

    if (numCells == 0 && pd->GetPolys()->GetNumberOfCells() > 0)
    {
        EXCEPTION0(NoCurveException);
    }

    // Get the line segments as continuous lines
    std::vector< std::vector<int> >  line_segments;
    SortLineSegments(pd, line_segments);

    ofstream ofile(filename, ios::out);
    vtkPoints *pts = pd->GetPoints();
    for (int i = 0 ; i < line_segments.size() ; i++)
    {
        if (line_segments.size() <= 1)
            ofile << "# curve" << endl;
        else
            ofile << "# curve" << i << endl;

        for (int j = 0 ; j < line_segments[i].size() ; j++)
        {
            double pt[3]; 
            pts->GetPoint(line_segments[i][j], pt);
            ofile << pt[0] << " " << pt[1] << endl;
        }
    }

    // Free memory
    ds->Delete();
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::CreateFilename
//
//  Purpose: 
//      Creates a filename with the appropriate file extension and returns it 
//      as a dynamically allocated array of characters.
//
//  Arguments:
//      base     The base filename.
//      family   Whether or not there should be a family in the filename.
//      format   The desired dataset format.
//
//  Returns:    A dynamically allocated array of characters that contains the
//              filename.
//
//  Notes:      The calling procedure must free the memory.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
//  Modifications:
//   Hank Childs, Mon Feb 24 18:22:04 PST 2003
//   Allow for non-familied filenames.
//
//   Brad Whitlock, Mon Mar 6 17:39:39 PST 2006
//   Added code to reset nFilesWritten if the file base changes.
//
//   Hank Childs, Fri Feb 15 16:23:31 PST 2008
//   Use SNPRINTF.
//
// ****************************************************************************

char *
avtDatasetFileWriter::CreateFilename(const char *base, bool family,
                                     DatasetFileFormat format)
{
    char *str = NULL;
    int len = strlen(base);

    // Reset the nFilesWritten count if the file base changes.
    if(family)
    {
        if(oldFileBase == NULL)
        {
            oldFileBase = new char[len+1];
            strcpy(oldFileBase, base);
        }
        else if(strcmp(oldFileBase, base) != 0)
        {
            delete [] oldFileBase;
            oldFileBase = new char[len+1];
            strcpy(oldFileBase, base);

            nFilesWritten = 0;
        }
    }

    //
    // Get memory for the filename.
    //
    int extlen = strlen(extensions[(int)format]);
    int maxnums = 10; // 4 is the minimum, not the maximum.
    int dotlen = 1;
    int total_len = len + maxnums + dotlen + extlen + 2;
    str = new char[total_len];

    if (family)
        SNPRINTF(str, total_len-1, "%s%04d%s", base, nFilesWritten, 
                                               extensions[(int)format]);
    else
        SNPRINTF(str, total_len-1, "%s%s", base, extensions[(int)format]);

    //
    // Increment the number of files written.
    //
    ++nFilesWritten;

    return str;
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::GetSingleDataset
//
//  Purpose:
//      Gets the input as a single dataset.
//
//  Returns:    A single dataset.  The calling function must free this.
//
//  Programmer: Hank Childs
//  Creation:   May 26, 2002
//
//  Modificatons:
//    Kathleen Bonnell, Wed May 17 14:44:08 PDT 2006
//    Changed GetNumberOfInputs to GetTotalNumberOfInputConnections.  Can
//    no longer call 'GetInputs' on vtkAppendFillter, must get individual port
//    info, then the dataset from the info. 
//
// ****************************************************************************

vtkDataSet *
avtDatasetFileWriter::GetSingleDataset(void)
{
    avtDataTree_p dt = GetInputDataTree();

    struct map
    {
        vtkAppendFilter *af;
        vtkAppendPolyData *pf;
    } pmap;

    pmap.af = vtkAppendFilter::New(); // Just in case...
    pmap.pf = vtkAppendPolyData::New();

    if (*dt != NULL)
    {
        bool dummy;
        dt->Traverse(CAddInputToAppendFilter, &pmap, dummy);
    }

    if (pmap.af->GetTotalNumberOfInputConnections() > 1 && 
        pmap.pf->GetTotalNumberOfInputConnections() > 1)
    {
        //
        // We have inputs that are unstructured and poly data.  Put them all
        // into the unstructured grid.
        //
        int numInputs = pmap.pf->GetTotalNumberOfInputConnections();
        vtkInformation *inInfo;
        for (int i = 0 ; i < numInputs ; i++)
        {
            inInfo = pmap.pf->GetInputPortInformation(i);
            pmap.af->AddInput(vtkPolyData::SafeDownCast(
                              inInfo->Get(vtkDataObject::DATA_OBJECT()))); 
        }
        pmap.pf->RemoveAllInputs();
    }

    //
    // We can now assume that we have either data in the normal append filter
    // or in the poly data append filter (from logic above).
    //
    vtkDataSet *rv = NULL;
    if (pmap.af->GetTotalNumberOfInputConnections() > 1)
    {
        if (pmap.af->GetTotalNumberOfInputConnections() == 1)
        {
            rv = pmap.af->GetInput();
        }
        else
        {
            rv = pmap.af->GetOutput();
        }
    }
    else
    {
        if (pmap.pf->GetTotalNumberOfInputConnections() == 1)
        {
            rv = pmap.pf->GetInput();
        }
        else
        {
            rv = pmap.pf->GetOutput();
        }
    }
    rv->Register(NULL);
    rv->Update();
    pmap.af->Delete();
    pmap.pf->Delete();
    return rv;
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::GenerateName
//
//  Purpose:
//      Generates a unique name.
//
//  Arguments:
//      label       A hint at what a good name would be.  May be NULL.
//      desc        A description of what we are writing.  This cannot be NULL.
//      namesUsed   A list of names already used.
//
//  Returns:        A unique name.
//
//  Notes:          The calling function must free the returned memory.
//
//  Programmer:     Hank Childs
//  Creation:       May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 16:26:25 PST 2008
//    Use SNPRINTF.
//
// ****************************************************************************

char *
avtDatasetFileWriter::GenerateName(const char *label, const char *desc,
                                   vector<string> &namesUsed)
{
    const char *attempt = (label != NULL ? label : desc);
    char tmp[1024];
    bool foundCandidate = false;
    bool triedNormal = false;
    int  idx = 2;
    while (!foundCandidate)
    {
        foundCandidate = true;
        if (!triedNormal)
        {
            strcpy(tmp, attempt);
            triedNormal = true;
        }
        else
        {
            SNPRINTF(tmp, 1024, "%s%d", attempt, idx);
            idx++;
        }
        for (int i = 0 ; i < namesUsed.size() ; i++)
        {
            if (namesUsed[i] == tmp)
            {
                foundCandidate = false;
                break;
            }
        }
    }

    char *rv = new char[strlen(tmp)+1];
    strcpy(rv, tmp);
    return rv;
}


// ****************************************************************************
//  Function: AddSegment
//
//  Purpose:
//      Adds a segment to the segment list.  This isn't hard, but it takes
//      more code than can be compactly added to SortLineSegments.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2003
//
// ****************************************************************************

static void
AddSegment(int *seg_list, int id1, int id2)
{
    if (seg_list[2*id1] != -1)
    {
        if (seg_list[2*id1+1] != -1)
        {
            // Can't handle 3 lines intersecting at one point.
            EXCEPTION0(NoCurveException); 
        }
        else
        {
            seg_list[2*id1+1] = id2;
        }
    }
    else
    {
        seg_list[2*id1] = id2;
    }
}


// ****************************************************************************
//  Function: SortLineSegments
//
//  Purpose:
//      Sorts the line segments associated with a curve.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 16:29:37 PST 2008
//    Fix memory leak.
//
// ****************************************************************************

static void
SortLineSegments(vtkPolyData *pd, std::vector< std::vector<int> > &ls)
{
    int   i;

    ls.clear();
    
    int ntotalpts = pd->GetNumberOfPoints();

    int *seg_list = new int[2*ntotalpts];
    int tmp = 2*ntotalpts;
    for (i = 0 ; i < tmp ; i++)
    {
        seg_list[i] = -1;
    }

    vtkCellArray *lines = pd->GetLines();
    vtkIdType npts, *ids;
    for (lines->InitTraversal() ; lines->GetNextCell(npts, ids) ; )
    {
        if (npts == 2)
        {
            AddSegment(seg_list, ids[0], ids[1]);
            AddSegment(seg_list, ids[1], ids[0]);
        }
        else   // Plausible case for polylines.
        {
            for (i = 0 ; i < npts ; i++)
            {
                AddSegment(seg_list, ids[i], ids[i%npts]);
                AddSegment(seg_list, ids[i%npts], ids[i]);
            }
        }
    }

    //
    // This loop will try to pick off polylines where there is a start and an
    // end.
    //
    bool isEndpoint = true;
    bool segmentsLeft = true;
    while (isEndpoint && segmentsLeft)
    {
        isEndpoint = false;
        segmentsLeft = false;
        for (i = 0 ; i < ntotalpts ; i++)
        {
            if (seg_list[2*i] != -1)
            {
                segmentsLeft = true;
                if (seg_list[2*i+1] == -1)
                {
                    TakeOffPolyLine(seg_list, i, ls);
                    isEndpoint = true;
                }
            }
        }
    }

    //
    // We now only have loops left.
    //
    while (segmentsLeft)
    {
        segmentsLeft = false;
        for (i = 0 ; i < ntotalpts ; i++)
        {
            if (seg_list[2*i] != -1 && seg_list[2*i+1] != -1)
            {
                segmentsLeft = true;
                TakeOffPolyLine(seg_list, i, ls);
            }
        }
    }

    delete [] seg_list;
}


// ****************************************************************************
//  Function: TakeOffPolyLine
//
//  Purpose:
//      Takes out a polyline, given a start vertex.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2003
//
// ****************************************************************************

static void
TakeOffPolyLine(int *seg_list,int start_pt,std::vector< std::vector<int> > &ls)
{
    std::vector<int> pl;

    pl.push_back(start_pt);
    int currentPt = start_pt;
    while (currentPt != -1)
    {
        int nextPt = seg_list[2*currentPt];
        if (nextPt == -1)
            break;
        seg_list[2*currentPt] = -1;
        if (seg_list[2*nextPt+1] == currentPt)
        {
            seg_list[2*nextPt+1] = -1;
        }
        else if (seg_list[2*nextPt] == currentPt)
        {
            seg_list[2*nextPt] = seg_list[2*nextPt+1];
            seg_list[2*nextPt+1] = -1;
        } 
        else
        {
            // This should never happen.  Internal error.
            EXCEPTION0(NoCurveException);
        }
        currentPt = nextPt;
        pl.push_back(currentPt);
    }

    ls.push_back(pl);
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WritePOVRayFamily
//
//  Purpose:
//      Writes out the input as a POV-Ray family of files.
//
//  Programmer: Jeremy Meredith
//  Creation:   April  5, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu May 31 15:31:31 EDT 2007
//    Added suppport for volume rendering rectilinear grids
//
//    Jeremy Meredith, Wed Jan  9 13:12:22 EST 2008
//    Added support for a user-defined function to scale vector and
//    vertex glyphs.  Added new atomic properties include file.
//
//    Jeremy Meredith, Wed Feb  6 10:53:16 EST 2008
//    Fixed coordinate handedness mismatch correctly.
//
//    Hank Childs, Fri Feb 15 16:28:29 PST 2008
//    Fix memory leak.
//
//    Jeremy Meredith, Wed May 20 11:49:18 EDT 2009
//    MAX_ELEMENT_NUMBER now means the actual max element number, not the
//    total number of known elements in visit.  Added a fake "0" element
//    which means "unknown", and hydrogen now starts at 1.  This
//    also means we don't have to correct for 1-origin atomic numbers.
//
//    Jeremy Meredith, Wed May 20 13:48:34 EDT 2009
//    Write atomicproperties and colortables .inc files to same directory
//    as the pov files.
//
// ****************************************************************************

void
avtDatasetFileWriter::WritePOVRayFamily(const char *filename)
{
    avtDataTree_p dt = GetInputDataTree();

    //
    // Re-construct the base file name without the extension.
    //
    char *basename = new char[strlen(filename)+1];
    int badExt = strlen(extensions[(int) POVRAY]);
    strncpy(basename, filename, strlen(filename)-badExt);
    basename[strlen(filename)-badExt] = '\0';

    //
    // Get the dir name in case user's not saving in current directory
    //
    string dirname(StringHelpers::Dirname(basename));

    //
    // It's easiest and safest to collect data and spatial extents
    // manually, especially for spatial since POVRay and VisIt are
    // of opposite handedness.
    //
    double spatialExtents[6] = {DBL_MAX,-DBL_MAX,
                                DBL_MAX,-DBL_MAX,
                                DBL_MAX,-DBL_MAX};
    double dataExtents[2] = {DBL_MAX, -DBL_MAX};

    //
    // Write out the VTK datasets as their own files.
    //
    int nChunks = WritePOVRayTree(dt, 0, basename, spatialExtents,dataExtents);

    //
    // Fix up data extents; if we don't have any then
    // we need fake values.  Similarly, we need to avoid
    // max being equal to min.
    //
    if (dataExtents[0] == DBL_MAX && dataExtents[1] == -DBL_MAX)
    {
        dataExtents[0] = dataExtents[1] = 0;
    }

    const float epsilon = 1e-6;
    if (fabs(dataExtents[0] - dataExtents[1]) < epsilon)
    {
        dataExtents[1] = dataExtents[0] + 1;
    }

    //
    // And make a .inc file with the current visit colortables
    //
    string ct_fname = dirname + VISIT_SLASH_CHAR + "colortables.inc";
    ofstream ctfile(ct_fname.c_str());
    const ColorTableAttributes *colortables =
        avtColorTables::Instance()->GetColorTables();
    int num = colortables->GetNumColorTables();
    for (int i=0; i<num; i++)
    {
        char num[100];
        ctfile << "#declare "
               << "ct_" << colortables->GetNames()[i]
               << " = color_map {" << endl;
        const ColorControlPointList &ct = colortables->GetColorTables(i);
        for (int j=0; j<ct.GetNumControlPoints(); j++)
        {
            const ColorControlPoint &pt = ct.GetControlPoints(j);
            sprintf(num, "%0.3f", pt.GetPosition());
            ctfile << "  [" << num << " color rgb<";
            sprintf(num, "%0.4f", float(pt.GetColors()[0])/255.);
            ctfile << num << " ";
            sprintf(num, "%0.4f", float(pt.GetColors()[1])/255.);
            ctfile << num << " ";
            sprintf(num, "%0.4f", float(pt.GetColors()[2])/255.);
            ctfile << num << ">]" << endl;
        }
        ctfile << "};" << endl;
        ctfile << endl;
    }
    ctfile.close();

    //
    // And make a .inc file with the current atomic properties
    //
    string atom_fname = dirname + VISIT_SLASH_CHAR + "atomicproperties.inc";
    ofstream atomfile(atom_fname.c_str());
    atomfile << "#declare atomic_radius = array["<<MAX_ELEMENT_NUMBER+1<<"]\n";
    atomfile << "{" << endl;
    for (int i=0; i<=MAX_ELEMENT_NUMBER; i++)
    {
        atomfile << "  " << atomic_radius[i];
        if (i<MAX_ELEMENT_NUMBER)
            atomfile << ",";
        atomfile << endl;
    }
    atomfile << "};" << endl;
    atomfile << endl;
    atomfile << "#declare covalent_radius = array["<<MAX_ELEMENT_NUMBER+1<<"]\n";
    atomfile << "{" << endl;
    for (int i=0; i<=MAX_ELEMENT_NUMBER; i++)
    {
        atomfile << "  " << covalent_radius[i];
        if (i<MAX_ELEMENT_NUMBER)
            atomfile << ",";
        atomfile << endl;
    }
    atomfile << "};" << endl;
    atomfile.close();

    //
    // And make a .pov file with initial values for the user to play with
    //
    char masterfilename[1024];
    sprintf(masterfilename, "%s.pov", basename);
    ofstream masterfile(masterfilename);
    masterfile << "#include \"colortables.inc\"" << endl;
    masterfile << "#include \"atomicproperties.inc\"" << endl;
    masterfile << "#include \"math.inc\"" << endl;
    masterfile << "#include \"transforms.inc\"" << endl;
    masterfile << endl;
    masterfile << "// Keep it fast for now" << endl;
    masterfile << "global_settings {" << endl;
    masterfile << "    max_trace_level 10" << endl;
    masterfile << "    adc_bailout 1/128." << endl;
    masterfile << "}" << endl;
    masterfile << endl;
    masterfile << "// Sizes for glyph objects" << endl;
    masterfile << "#declare VertFixedSize = 0.3; // absolute scale" << endl;
    masterfile << "#declare VertScaleSize = 0.0; // scale by value" << endl;
    masterfile << "#declare LineWidth     = 0.1; // absolute scale only" << endl;
    masterfile << "#declare VecFixedSize  = 1.0; // absolute scale" << endl;
    masterfile << "#declare VecScaleSize  = 0.0; // scale by value" << endl;
    masterfile << endl;
    masterfile << "// Default scale function for vertex glyph objects with a scalar value" << endl;
    masterfile << "#macro VertScaleFunction(value)" << endl;
    masterfile << "  VertFixedSize + value*VertScaleSize" << endl;
    masterfile << "#end" << endl;
    masterfile << endl;
    masterfile << "// Default scale function for vector glyph objects" << endl;
    masterfile << "#macro VecScaleFunction(vec)" << endl;
    masterfile << "  VecFixedSize + vlength(vec)*VecScaleSize" << endl;
    masterfile << "#end" << endl;
    masterfile << endl;
    masterfile << "// Glyph objects (normalized to unit-1 size)" << endl;
    masterfile << "#declare VertGlyph = object { sphere {<0,0,0>,1} };" << endl;
    masterfile << "#declare VecGlyph = object { union { object { cylinder {<0,0,0>,<0,.5,0>,.1} } object { cone {<0,.5,0>,.3,<0,1,0>,0} } } };" << endl;
    masterfile << endl;
    masterfile << "// Default pigments for geometries without scalar values" << endl;
    masterfile << "#declare VertPigment = pigment { rgb<.6,0,0> };" << endl;
    masterfile << "#declare LinePigment = pigment { rgb<.6,.6,.6> };" << endl;
    masterfile << "#declare PolyPigment = pigment { rgb<0,.4,.8> };" << endl;
    masterfile << "#declare VecPigment  = pigment { rgb<.4,.3,.8> };" << endl;
    masterfile << endl;
    masterfile << "// To force single pigment for the geometries, set these to 1" << endl;
    masterfile << "#declare ConstantVertColor = 0;" << endl;
    masterfile << "#declare ConstantLineColor = 0;" << endl;
    masterfile << "#declare ConstantPolyColor = 0;" << endl;
    masterfile << "#declare ConstantVecColor  = 0;" << endl;
    masterfile << endl;
    masterfile << "// Finishes for each type of geometry" << endl;
    masterfile << "#declare VertFinish = finish {ambient .3 reflection .0 diffuse .8 specular 0.2 roughness 0.1};" << endl;
    masterfile << "#declare LineFinish = finish {ambient .3 reflection .0 diffuse .8 specular 0.2 roughness 0.1};" << endl;
    masterfile << "#declare PolyFinish = finish {ambient .3 reflection .0 diffuse .8 specular 0.2 roughness 0.1};" << endl;
    masterfile << "#declare VecFinish  = finish {ambient .3 reflection .0 diffuse .8 specular 0.2 roughness 0.1};" << endl;
    masterfile << endl;
    masterfile << "// Clamp values for scalar quantities, and a default linear map to [0,1]" << endl;
    masterfile << "#macro ScalarNormalize(value)" << endl;
    masterfile << "  (clip(value,min_scalar_value,max_scalar_value)-min_scalar_value)/(max_scalar_value-min_scalar_value)" << endl;
    masterfile << "#end" << endl;
    masterfile << "#declare min_scalar_value = " << dataExtents[0] << ";" << endl;
    masterfile << "#declare max_scalar_value = " << dataExtents[1] << ";" << endl;
    masterfile << "//note: min/max = 1/108 for elements and 0/108 for residues\n";
    masterfile << endl;
    masterfile << "// Extents and sizes of spatial geometries" << endl;
    masterfile << "#declare xmin = " << spatialExtents[0] << ";" << endl;
    masterfile << "#declare xmax = " << spatialExtents[1] << ";" << endl;
    masterfile << "#declare ymin = " << spatialExtents[2] << ";" << endl;
    masterfile << "#declare ymax = " << spatialExtents[3] << ";" << endl;
    masterfile << "#declare zmin = " << spatialExtents[4] << ";" << endl;
    masterfile << "#declare zmax = " << spatialExtents[5] << ";" << endl;
    masterfile << "#declare xsize = (xmax - xmin);" << endl;
    masterfile << "#declare ysize = (ymax - ymin);" << endl;
    masterfile << "#declare zsize = (zmax - zmin);" << endl;
    masterfile << "#declare xcenter = (xmin + xmax) / 2.;" << endl;
    masterfile << "#declare ycenter = (ymin + ymax) / 2.;" << endl;
    masterfile << "#declare zcenter = (zmin + zmax) / 2.;" << endl;
    masterfile << "#declare ds_size = sqrt(xsize*xsize + ysize*ysize + zsize*zsize);" << endl;
    masterfile << endl;
    masterfile << "// Set up the color table default" << endl;
    masterfile << "#declare colortable = ct_hot;" << endl;
    masterfile << endl;
    masterfile << "// Set up the color table and opacity for volume renderings " << endl;
    masterfile << "#declare volren_colortable = ct_hot;" << endl;
    masterfile << "#declare volren_opacity = color_map {" << endl;
    masterfile << "   [0.00  color  0.0]" << endl;
    masterfile << "   [1.00  color  1.0]" << endl;
    masterfile << "};" << endl;
    masterfile << endl;
    masterfile << "// The emission for volume plots typically needs to scale with data set size" << endl;
    masterfile << "#declare volren_attenuation = 2.0 / ds_size;" << endl;
    masterfile << endl;
    masterfile << "// Set some variables to make camera/light source positioning easier" << endl;
    masterfile << "#declare aspect       = 4/3;" << endl;
    masterfile << "#declare scene_origin = <xcenter,ycenter,zcenter>;" << endl;
    masterfile << "#declare camera_at    = scene_origin;" << endl;
    masterfile << "#declare camera_pos   = scene_origin + z*ds_size*1.5;" << endl;
    masterfile << "#declare camera_up    = y;" << endl;
    masterfile << endl;
    masterfile << "// Set the camera/aspect from the given parameters" << endl;
    masterfile << "camera {" << endl;
    masterfile << "    location camera_pos" << endl;
    masterfile << "    right    -x*aspect" << endl;
    masterfile << "    look_at  camera_at" << endl;
    masterfile << "    sky      camera_up" << endl;
    masterfile << "    angle    40" << endl;
    masterfile << "}" << endl;
    masterfile << "" << endl;
    masterfile << "// Set the light source near the camera" << endl;
    masterfile << "light_source {" << endl;
    masterfile << "    camera_pos color 1" << endl;
    masterfile << "}" << endl;
    masterfile << endl;
    masterfile << "// Include the declarations for all chunk geometries" << endl;
    masterfile << endl;
    masterfile << "//" << endl;
    masterfile << "// NOTE: To change the colortable, scalar clamps, pigments, " << endl;
    masterfile << "//       finishes, etc. for different chunks, just redeclare" << endl;
    masterfile << "//       them in between the appropriate include files here." << endl;
    masterfile << "//" << endl;
    masterfile << endl;
    for (int i = 0 ; i < nChunks ; i++)
    {
        char name[1024];
        sprintf(name, "%s.%04d.inc", basename, i);
        masterfile << "#include \"" << name << "\"" << endl;
    }
    masterfile << endl;
    masterfile << "// Instantiate all chunk geometries" << endl;
    for (int i = 0 ; i < nChunks ; i++)
    {
        char idxstr[1024];
        sprintf(idxstr, "%04d", i);
        masterfile << "#if (nverts"<<idxstr<<" > 0) object { vertex_geometry"<<idxstr<<" }  #end" << endl;
        masterfile << "#if (nvecs"<<idxstr<<"  > 0) object { vector_geometry"<<idxstr<<" }  #end" << endl;
        masterfile << "#if (nlines"<<idxstr<<" > 0) object { line_geometry"<<idxstr<<"   }  #end" << endl;
        masterfile << "#if (ntris"<<idxstr<<"  > 0) object { poly_geometry"<<idxstr<<"   }  #end" << endl;
        masterfile << "#if (nvols"<<idxstr<<"  > 0) object { volume_geometry"<<idxstr<<" }  #end" << endl;

        masterfile << endl;
    }
    masterfile.close();
    delete [] basename;
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WritePOVRayTree
//
//  Purpose:
//      Writes out an avt data tree as POVRay files.
//
//  Arguments:
//      dt               The data tree to write.
//      idx              The current index of the file to write.
//      basename         The file name to use.
//      spatialextents   The currently accumulated spatial extents
//      dataextents      The currently accumulated data extents
//
//  Returns:    The number of files written from dt.
//
//  Programmer: Jeremy Meredith
//  Creation:   April  5, 2007
//
// ****************************************************************************

int
avtDatasetFileWriter::WritePOVRayTree(avtDataTree_p dt, int idx,
                                      const char *basename,
                                      double *spatialextents,
                                      double *dataextents)
{
    if (*dt == NULL)
    {
        return 0;
    }

    int totalWritten = 0;
    if (dt->HasData())
    {
        avtDataRepresentation &rep = dt->GetDataRepresentation();
        vtkDataSet *ds = rep.GetDataVTK();
        char filename[1024];
        sprintf(filename, "%s.%04d.inc", basename, idx);
        WritePOVRayFile(ds, filename, idx, spatialextents, dataextents);
        totalWritten = 1;
    }
    else
    {
        int workingIndex = idx;
        for (int i = 0 ; i < dt->GetNChildren() ; i++)
        {
            if (dt->ChildIsPresent(i))
            {
                int numWritten = WritePOVRayTree(dt->GetChild(i), workingIndex,
                                                 basename, spatialextents,
                                                 dataextents);
                workingIndex += numWritten;
                totalWritten += numWritten;
            }
        }
    }

    return totalWritten;
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WritePOVRayFile
//
//  Purpose:
//      Writes a single vtkDataSet to a POVRay file.
//
//  Arguments:
//      ds               The vtkDataSet to write.
//      filename         The filename to use.
//      idx              The current index of the file to write.
//      spatialextents   The currently accumulated spatial extents
//      dataextents      The currently accumulated data extents
//
//  Returns:    nothing
//
//  Programmer: Jeremy Meredith
//  Creation:   April  5, 2007
//
//  Modifications:
//    Jeremy Meredith, Thu May 31 15:34:42 EDT 2007
//    Added volume rendering support through dense-media in a bounding box,
//    using a 32-bit density file, and with transfer functions implemented
//    using independent density maps for color and opacity.
//
//    Jeremy Meredith, Wed Jan  9 13:12:22 EST 2008
//    Added support for a user-defined function to scale vector and
//    vertex glyphs.
//
//    Jeremy Meredith, Thu Jan 10 11:25:35 EST 2008
//    Write out all the cell and point scalar arrays, not just the
//    active one.
//
//    Jeremy Meredith, Wed Feb  6 10:53:16 EST 2008
//    Fixed coordinate handedness mismatch correctly.
//
// ****************************************************************************

void
avtDatasetFileWriter::WritePOVRayFile(vtkDataSet *ds,
                                      const char *filename,
                                      int idx,
                                      double *spatialextents,
                                      double *dataextents)
{
    ofstream out(filename, ios::out);
    vtkGeometryFilter *geom = vtkGeometryFilter::New();
    vtkTriangleFilter *tris = vtkTriangleFilter::New();
    tris->SetPassLines(true);
    tris->SetPassVerts(true);

    //
    // Write out a density file for rectilinear grids
    //
    bool densityWritten = false;
    double densityExtents[6];
    char df3name[256];
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        sprintf(df3name, "%s.df3", filename);
        densityWritten = WritePOVRayDF3File((vtkRectilinearGrid*)ds,
                                            df3name, densityExtents);
    }

    //
    // We need triangle-only polydata
    //
    if (ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        geom->SetInput(ds);
        tris->SetInput(geom->GetOutput());
    }
    else
    {
        tris->SetInput((vtkPolyData*)ds);
    }

    //
    // Get a bunch of info from the dataset
    //
    vtkPolyData *pd = (vtkPolyData *) tris->GetOutput();
    pd->Update();

    vtkDataArray *ptscalars = pd->GetPointData()->GetScalars();
    vtkDataArray *ptvectors = pd->GetPointData()->GetVectors();
    vtkDataArray *ptnormals = pd->GetPointData()->GetNormals();
    vtkDataArray *cellscalars = pd->GetCellData()->GetScalars();
    vtkDataArray *cellvectors = pd->GetCellData()->GetVectors();

    vtkCellArray *lines = pd->GetLines();
    vtkCellArray *verts = pd->GetVerts();
    vtkCellArray *polys = pd->GetPolys();

    int numPoints = pd->GetNumberOfPoints();
    int numCells = pd->GetNumberOfCells();
    int numLines = lines->GetNumberOfCells();
    int numVerts = verts->GetNumberOfCells();
    int numPolys = polys->GetNumberOfCells();
    int numVecs = (ptvectors || cellvectors) ? numVerts : 0;
    int numVols = densityWritten ? 1 : 0;

    vtkIdType npts, *ids;
    vtkIdList *idlist = vtkIdList::New();
    
    //
    // We need to accumulate the data and spatial extents to
    // set a good max/min and camera default for the user
    //
    for (int i=0; i<numPoints; i++)
    {
        double pt[3];
        pd->GetPoint(i, pt);
        if (spatialextents[0] > pt[0])
            spatialextents[0] = pt[0];
        if (spatialextents[1] < pt[0])
            spatialextents[1] = pt[0];
        if (spatialextents[2] > pt[1])
            spatialextents[2] = pt[1];
        if (spatialextents[3] < pt[1])
            spatialextents[3] = pt[1];
        if (spatialextents[4] > pt[2])
            spatialextents[4] = pt[2];
        if (spatialextents[5] < pt[2])
            spatialextents[5] = pt[2];
    }
    if (cellscalars)
    {
        for (int i=0; i<cellscalars->GetNumberOfTuples(); i++)
        {
            double val = cellscalars->GetComponent(i, 0);
            if (dataextents[0] > val)
                dataextents[0] = val;
            if (dataextents[1] < val)
                dataextents[1] = val;
        }
    }
    if (ptscalars)
    {
        for (int i=0; i<ptscalars->GetNumberOfTuples(); i++)
        {
            double val = ptscalars->GetComponent(i, 0);
            if (dataextents[0] > val)
                dataextents[0] = val;
            if (dataextents[1] < val)
                dataextents[1] = val;
        }
    }

    //
    // Write the array sizes.
    //
    char idxstr[20];
    sprintf(idxstr, "%04d", idx);
    out << "#declare npts"<<idxstr<<"   = "<<numPoints<<";"<<endl;
    out << "#declare ncells"<<idxstr<<" = "<<numCells<<";"<<endl;
    out << "#declare nverts"<<idxstr<<" = "<<numVerts<<";"<<endl;
    out << "#declare nlines"<<idxstr<<" = "<<numLines<<";"<<endl;
    out << "#declare ntris"<<idxstr<<"  = "<<numPolys<<";"<<endl;
    out << "#declare nvecs"<<idxstr<<"  = "<<numVecs<<";"<<endl;
    out << "#declare nvols"<<idxstr<<"  = "<<numVols<<";"<<endl;
    out << endl;

    //
    // Write the coordinates
    //
    if (numPoints > 0)
    {
        out << "#declare pts"<<idxstr<<" = array[npts"<<idxstr<<"]" << endl;
        out << "{"<<endl;
        for (int i=0; i<numPoints; i++)
        {
            double pt[3];
            pd->GetPoint(i, pt);
            out << "  <"<<pt[0]<<","<<pt[1]<<","<<pt[2]<<">";
            if (i < numPoints-1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;
        out << endl;
    }

    //
    // Write the point scalars
    //
    if (ptscalars)
    {
        out << "#declare ptscalars"<<idxstr<<" = array[npts"<<idxstr<<"]\n" << endl;
        out << "{"<<endl;
        for (int i=0; i<numPoints; i++)
        {
            double val = ptscalars->GetComponent(i, 0);
            out << "  " << val;
            if (i < numPoints-1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;
        out << endl;
    }

    //
    // Write the cell scalars
    //
    if (cellscalars)
    {
        out << "#declare cellscalars"<<idxstr<<" = array[ncells"<<idxstr<<"]" << endl;
        out << "{"<<endl;
        for (int i=0; i<numCells; i++)
        {
            double val = cellscalars->GetComponent(i, 0);
            out << "  " << val;
            if (i < numCells-1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;
        out << endl;
    }

    //
    // Write the point vectors
    //
    if (ptvectors)
    {
        out << "#declare ptvectors"<<idxstr<<" = array[npts"<<idxstr<<"]" << endl;
        out << "{"<<endl;
        for (int i=0; i<numPoints; i++)
        {
            double vec[3] = {ptvectors->GetComponent(i, 0),
                             ptvectors->GetComponent(i, 1),
                             ptvectors->GetComponent(i, 2)};
            out << "  <"<<vec[0]<<","<<vec[1]<<","<<vec[2]<<">";
            if (i < numPoints-1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;
        out << endl;
    }

    //
    // Write the cell vectors
    //
    if (cellvectors)
    {
        out << "#declare cellvectors"<<idxstr<<" = array[ncels"<<idxstr<<"]" << endl;
        out << "{"<<endl;
        for (int i=0; i<numCells; i++)
        {
            double vec[3] = {cellscalars->GetComponent(i, 0),
                             cellscalars->GetComponent(i, 1),
                             cellscalars->GetComponent(i, 2)};
            out << "  <"<<vec[0]<<","<<vec[1]<<","<<vec[2]<<">";
            if (i < numCells-1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;
        out << endl;
    }

    //
    // Write the point normals (cell normals are useless)
    //
    if (ptnormals)
    {
        out << "#declare ptnorms"<<idxstr<<" = array[npts"<<idxstr<<"]"<<endl;
        out << " {"<<endl;
        for (int i=0; i<numPoints; i++)
        {
            double norm[3];
            ptnormals->GetTuple(i, norm);
            out << "    <"<<norm[0]<<","<<norm[1]<<","<<norm[2]<<">";
            if (i < numPoints-1)
                out << ",";
            out << endl;
        }
        out << "};"<<endl;
        out << endl;
    }

    //
    // Write out the all of the point scalars
    //
    for (int i=0; i<pd->GetPointData()->GetNumberOfArrays(); i++)
    {
        vtkDataArray *array = pd->GetPointData()->GetArray(i);
        out << "#declare ptscalars_"<<array->GetName()<<idxstr<<" = array[npts"<<idxstr<<"]" << endl;
        out << "{"<<endl;
        for (int i=0; i<numPoints; i++)
        {
            double val = array->GetComponent(i, 0);
            out << "  " << val;
            if (i < numPoints-1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;
        out << endl;
    }

    //
    // Write out the all of the cell scalars
    //
    for (int i=0; i<pd->GetCellData()->GetNumberOfArrays(); i++)
    {
        vtkDataArray *array = pd->GetCellData()->GetArray(i);
        out << "#declare cellscalars_"<<array->GetName()<<idxstr<<" = array[ncells"<<idxstr<<"]" << endl;
        out << "{"<<endl;
        for (int i=0; i<numCells; i++)
        {
            double val = array->GetComponent(i, 0);
            out << "  " << val;
            if (i < numCells-1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;
        out << endl;
    }

    //
    // Write the connectivity for 0-dimensional data as a
    // (1+1)-component array (the 0th entry is the cell ID).
    //
    if (numVerts > 0)
    {
        out << "#declare verts"<<idxstr<<" = array[nverts"<<idxstr<<"][2]"<<endl;
        out << "{"<<endl;
        bool first = true;
        for (int c=0; c<numCells; c++)
        {
            int type = pd->GetCellType(c);
            pd->GetCellPoints(c, idlist);
            npts = idlist->GetNumberOfIds();
            ids = idlist->GetPointer(0);
            if (type != VTK_VERTEX || npts != 1)
                continue;
            if (!first)
                out << "," << endl;
            first = false;
            out << "  {"<<c<<",  "<<ids[0]<<"}";
        }
        out << endl << "};" << endl;
        out << endl;
    }

    //
    // Write the connectivity for 1-dimensional data as a
    // (2+1)-component array (the 0th entry is the cell ID).
    //
    if (numLines > 0)
    {
        out << "#declare lines"<<idxstr<<" = array[nlines"<<idxstr<<"][3]"<<endl;
        out << "{"<<endl;
        bool first = true;
        for (int c=0; c<numCells; c++)
        {
            int type = pd->GetCellType(c);
            pd->GetCellPoints(c, idlist);
            npts = idlist->GetNumberOfIds();
            ids = idlist->GetPointer(0);
            if (type != VTK_LINE || npts != 2)
                continue;
            if (!first)
                out << "," << endl;
            first = false;
            out << "  {"<<c<<",  "<<ids[0]<<","<<ids[1]<<"}";
        }
        out << endl  << "};" << endl;
        out << endl;
    }

    //
    // Write the connectivity for 2-dimensional data as a
    // (3+1)-component array (the 0th entry is the cell ID).
    //
    if (numPolys > 0)
    {
        out << "#declare tris"<<idxstr<<" = array[ntris"<<idxstr<<"][4]"<<endl;
        out << "{"<<endl;
        bool first = true;
        for (int c=0; c<numCells; c++)
        {
            int type = pd->GetCellType(c);
            pd->GetCellPoints(c, idlist);
            npts = idlist->GetNumberOfIds();
            ids = idlist->GetPointer(0);
            if (type != VTK_TRIANGLE || npts != 3)
                continue;
            if (!first)
                out << "," << endl;
            first = false;
            out << "  {"<<c<<",  "<<ids[0]<<","<<ids[1]<<","<<ids[2]<<"}";
        }
        out << endl  << "};" << endl;
        out << endl;
    }

    //
    // Write the geometry for 0-dimensional vertex cells.
    //
    if (numVerts)
    {
        out << "#declare vertex_geometry"<<idxstr<<" = union"<<endl;
        out << "{"<<endl;
        out << "#declare vertctr=0;" <<endl;
        out << "#while (vertctr < nverts"<<idxstr<<")"<<endl;
        if (ptscalars)
            out << "    #declare vertvalue=ptscalars"<<idxstr<<"[verts"<<idxstr<<"[vertctr][1]];"<<endl;
        else if (cellscalars)
            out << "    #declare vertvalue=cellscalars"<<idxstr<<"[verts"<<idxstr<<"[vertctr][0]];"<<endl;
        out << "    object {"<<endl;
        out << "        VertGlyph"<<endl;
        if (ptscalars || cellscalars)
            out << "        scale VertScaleFunction(vertvalue)"<<endl;
        else
            out << "        scale VertFixedSize"<<endl;
        out << "        translate pts"<<idxstr<<"[verts"<<idxstr<<"[vertctr][1]]"<<endl;
        out << "        #if (ConstantVertColor)" << endl;
        out << "        pigment { VertPigment }" << endl;
        out << "        #else" << endl;
        if (ptscalars)
        {
            out << "        pigment {"<<endl;
            out << "            pigment_pattern {" << endl;
            out << "                color ";
            out << "ScalarNormalize(ptscalars"<<idxstr<<"[verts"<<idxstr<<"[vertctr][1]])"<<endl;
            out << "            }"<<endl;
            out << "            color_map {colortable}"<<endl;
            out << "        }" << endl;
        }
        else if (cellscalars)
        {
            out << "        pigment {"<<endl;
            out << "            pigment_pattern {" << endl;
            out << "                color ";
            out << "ScalarNormalize(cellscalars"<<idxstr<<"[verts"<<idxstr<<"[vertctr][0]])"<<endl;
            out << "            }"<<endl;
            out << "            color_map {colortable}"<<endl;
            out << "        }" << endl;
        }
        else
        {
            out << "        pigment {VertPigment}" << endl;
        }
        out << "        #end" << endl;
        out << endl;
        out << "        finish {VertFinish}" << endl;
        out << "    }" << endl;
            
        out << "#declare vertctr = vertctr+1;" << endl;
        out << "#end"<<endl;
        out << "};" << endl;
    }
    out << endl;

    //
    // Write the geometry for 0-dimensional-origin glyphed 3D vectors.
    //
    if (numVecs)
    {
        out << "#declare vector_geometry"<<idxstr<<" = union"<<endl;
        out << "{"<<endl;
        out << "#declare vecctr=0;" <<endl;
        out << "#while (vecctr < nvecs"<<idxstr<<")"<<endl;
        if (ptvectors)
            out << "    #declare vec=ptvectors"<<idxstr<<"[verts"<<idxstr<<"[vecctr][1]];"<<endl;
        else // cellvectors
            out << "    #declare vec=cellvectors"<<idxstr<<"[verts"<<idxstr<<"[vecctr][0]];"<<endl;
        out << "    object {"<<endl;
        out << "        VecGlyph" << endl;
        out << "        Point_At_Trans(vec)"<<endl;
        out << "        scale VecScaleFunction(vec)"<<endl;
        out << "        translate pts"<<idxstr<<"[verts"<<idxstr<<"[vecctr][1]]"<<endl;
        out << "        #if (ConstantVecColor)" << endl;
        out << "        pigment { VecPigment }" << endl;
        out << "        #else" << endl;
        if (ptscalars)
        {
            out << "        pigment {"<<endl;
            out << "            pigment_pattern {" << endl;
            out << "                color ";
            out << "ScalarNormalize(ptscalars"<<idxstr<<"[verts"<<idxstr<<"[vecctr][1]])"<<endl;
            out << "            }"<<endl;
            out << "            color_map {colortable}"<<endl;
            out << "        }" << endl;
        }
        else if (cellscalars)
        {
            out << "        pigment {"<<endl;
            out << "            pigment_pattern {" << endl;
            out << "                color ";
            out << "ScalarNormalize(cellscalars"<<idxstr<<"[verts"<<idxstr<<"[vecctr][0]])"<<endl;
            out << "            }"<<endl;
            out << "            color_map {colortable}"<<endl;
            out << "        }" << endl;
        }
        else
        {
            out << "        pigment {VecPigment}";
        }
        out << "        #end" << endl;
        out << "        finish {VecFinish}" << endl;;
        out << "    }" << endl;            
        out << "#declare vecctr = vecctr+1;" << endl;
        out << "#end"<<endl;
        out << "};" << endl;
    }
    out << endl;

    //
    // Write the geometry for 1-dimensional line segment cells.
    //
    if (numLines)
    {
        out << "#declare line_geometry"<<idxstr<<" = union"<<endl;
        out << "{"<<endl;
        out << "#declare linectr=0;" <<endl;
        out << "#while (linectr < nlines"<<idxstr<<")"<<endl;
        out << "    #declare cellid=lines"<<idxstr<<"[linectr][0];"<<endl;
        out << "    #declare id0=lines"<<idxstr<<"[linectr][1];"<<endl;
        out << "    #declare id1=lines"<<idxstr<<"[linectr][2];"<<endl;
        out << "    #declare pt_0=pts"<<idxstr<<"[id0];"<<endl;
        out << "    #declare pt_1=pts"<<idxstr<<"[id1];"<<endl;
        out << "    #declare pt_mid=(pt_0 + pt_1)/2.;"<<endl;
        if (ptscalars)
        {
            out << "    object {"<<endl;
            out << "        cylinder {pt_0,pt_mid,LineWidth}"<<endl;
            out << "        #if (ConstantLineColor)" << endl;
            out << "        pigment { LinePigment }" << endl;
            out << "        #else" << endl;
            out << "        pigment {"<<endl;
            out << "            pigment_pattern {" << endl;
            out << "                color ";
            out << "ScalarNormalize(ptscalars"<<idxstr<<"[id0])"<<endl;
            out << "            }"<<endl;
            out << "            color_map {colortable}"<<endl;
            out << "        }"<<endl;
            out << "        #end" << endl;
            out << "        finish {LineFinish}" << endl;;
            out << "    }"<<endl;
            out << "    object {"<<endl;
            out << "        cylinder {pt_mid,pt_1,LineWidth}"<<endl;
            out << "        #if (ConstantLineColor)" << endl;
            out << "        pigment { LinePigment }" << endl;
            out << "        #else" << endl;
            out << "        pigment {"<<endl;
            out << "            pigment_pattern {" << endl;
            out << "                color ";
            out << "ScalarNormalize(ptscalars"<<idxstr<<"[id1])"<<endl;
            out << "            }"<<endl;
            out << "            color_map {colortable}"<<endl;
            out << "        }"<<endl;
            out << "        #end" << endl;
            out << "        finish {LineFinish}" << endl;;
            out << "    }"<<endl;
        }
        else if (cellscalars)
        {
            out << "    object {"<<endl;
            out << "        cylinder {pt_0,pt_1,LineWidth}"<<endl;
            out << "        #if (ConstantLineColor)" << endl;
            out << "        pigment { LinePigment }" << endl;
            out << "        #else" << endl;
            out << "        pigment {"<<endl;
            out << "            pigment_pattern {" << endl;
            out << "                color ";
            out << "ScalarNormalize(cellscalars"<<idxstr<<"[cellid])"<<endl;
            out << "            }"<<endl;
            out << "            color_map {colortable}"<<endl;
            out << "        }"<<endl;
            out << "        #end" << endl;
            out << "        finish {LineFinish}" << endl;;
            out << "    }"<<endl;
        }
        else
        {
            out << "    object {"<<endl;
            out << "        cylinder {pt_0,pt_1,LineWidth}"<<endl;
            out << "        pigment { LinePigment }"<<endl;
            out << "        finish {LineFinish}" << endl;;
            out << "    }"<<endl;
        }
        out << "#declare linectr = linectr+1;" << endl;
        out << "#end"<<endl;
        out << "};"<<endl;
    }
    out << endl;

    //
    // Write the geometry for 2-dimensional triangle cells.
    //
    if (numPolys > 0)
    {
        out << "#declare poly_geometry"<<idxstr<<" = mesh2"<<endl;
        out << "{"<<endl;
        out << "    vertex_vectors {"<<endl;
        out << "        npts"<<idxstr<<","<<endl;
        out << "        #declare ptctr=0;"<<endl;
        out << "        #while (ptctr < npts"<<idxstr<<")"<<endl;
        out << "            pts"<<idxstr<<"[ptctr]," << endl;
        out << "            #declare ptctr = ptctr+1;" << endl;
        out << "        #end"<<endl;
        out << "    }"<<endl;

        if (ptnormals)
        {
            out << "    normal_vectors {"<<endl;
            out << "        npts"<<idxstr<<","<<endl;
            out << "        #declare normctr=0;"<<endl;
            out << "        #while (normctr < npts"<<idxstr<<")"<<endl;
            out << "            ptnorms"<<idxstr<<"[normctr]," << endl;
            out << "            #declare normctr = normctr+1;" << endl;
            out << "        #end"<<endl;
            out << "    }"<<endl;
        }

        if (ptscalars || cellscalars)
        {
            out << "    #if (!ConstantPolyColor)" << endl;
            out << "    texture_list {"<<endl;
            out << "        256,"<<endl;
            out << "        #declare texctr=0;"<<endl;
            out << "        #while (texctr < 256)"<<endl;
            out << "            texture {"<<endl;
            out << "                pigment {"<<endl;
            out << "                    pigment_pattern {color texctr/255.}"<<endl;
            out << "                    color_map {colortable}"<<endl;
            out << "                }"<<endl;
            out << "                finish { PolyFinish }"<<endl;
            out << "            },"<<endl;
            out << "            #declare texctr = texctr+1;" << endl;
            out << "        #end"<<endl;
            out << "    }"<<endl;
            out << "    #end"<<endl;
        }

        out << "    face_indices {"<<endl;
        out << "        ntris"<<idxstr<<","<<endl;
        out << "        #declare facectr=0;"<<endl;
        out << "        #while (facectr < ntris"<<idxstr<<")"<<endl;
        out << "            <tris"<<idxstr<<"[facectr][1],"<<endl;
        out << "             tris"<<idxstr<<"[facectr][2],"<<endl;
        out << "             tris"<<idxstr<<"[facectr][3]>"<<endl;
        if (ptscalars)
        {
            out << "            #if (!ConstantPolyColor)" << endl;
            out << "            ,255*ScalarNormalize(ptscalars"<<idxstr<<"[tris"<<idxstr<<"[facectr][1]])"<<endl;
            out << "            ,255*ScalarNormalize(ptscalars"<<idxstr<<"[tris"<<idxstr<<"[facectr][2]])"<<endl;
            out << "            ,255*ScalarNormalize(ptscalars"<<idxstr<<"[tris"<<idxstr<<"[facectr][3]])"<<endl;
            out << "            #end" << endl;
        }
        else if (cellscalars)
        {
            out << "            #if (!ConstantPolyColor)" << endl;
            out << "            ,255*ScalarNormalize(cellscalars"<<idxstr<<"[tris"<<idxstr<<"[facectr][0]])"<<endl;
            out << "            #end" << endl;
        }
        out << "            #declare facectr = facectr+1;" << endl;
        out << "        #end"<<endl;
        out << "    }"<<endl;

        if (ptnormals)
        {
            out << "    normal_indices {"<<endl;
            out << "        ntris"<<idxstr<<","<<endl;
            out << "        #declare facectr=0;"<<endl;
            out << "        #while (facectr < ntris"<<idxstr<<")"<<endl;
            out << "            <tris"<<idxstr<<"[facectr][1],"<<endl;
            out << "             tris"<<idxstr<<"[facectr][2],"<<endl;
            out << "             tris"<<idxstr<<"[facectr][3]>"<<endl;
            out << "            #declare facectr = facectr+1;" << endl;
            out << "        #end"<<endl;
            out << "    }"<<endl;
        }

        if (cellscalars || ptscalars)
            out << "    #if (ConstantPolyColor)" << endl;
        out << "    pigment {PolyPigment} finish {PolyFinish}" << endl;
        if (cellscalars || ptscalars)
            out << "    #end" << endl;
        out << "};"<<endl;
    }
    out << endl;

    //
    // Write the geometry for 3-dimensional grids as a density
    // map of emissive media in a bounding box.
    //
    if (numVols > 0)
    {
        double dx = densityExtents[1]-densityExtents[0];
        double dy = densityExtents[3]-densityExtents[2];
        double dz = densityExtents[5]-densityExtents[4];
        out << "#declare volume_geometry"<<idxstr<<" = box" << endl;
        out << "{" << endl;
        out << "   <0,0,0> <1,1,1>" << endl;
        out << "   texture { pigment { color rgbf 1 } }" << endl;
        out << "   interior {" << endl;
        out << "       media {" << endl;
        out << "           intervals 100" << endl;
        out << "           samples 1,10" << endl;
        out << "           emission <1,1,1> * volren_attenuation" << endl;
        out << "           absorption <0,0,0>" << endl;
        out << "           scattering { 1,<0,0,0> }" << endl;
        out << "           confidence 0.99" << endl;
        out << "           variance 1/256" << endl;
        out << "           density {" << endl;
        out << "               density_file df3 \""<<df3name<<"\"" << endl;
        out << "               interpolate 1" << endl;
        out << "               color_map { volren_colortable }" << endl;
        out << "           }" << endl;
        out << "           density {" << endl;
        out << "               density_file df3 \""<<df3name<<"\"" << endl;
        out << "               interpolate 1" << endl;
        out << "               color_map { volren_opacity }" << endl;
        out << "           }" << endl;
        out << "       }" << endl;
        out << "   }" << endl;
        out << "   scale <"
            << dx << ","
            << dy << ","
            << dz << ">" << endl;
        out << "   translate <"
            <<densityExtents[0]<<","
            <<densityExtents[2]<<","
            <<densityExtents[4]<<">" << endl;
        out << "   hollow" << endl;
        out << "};" << endl;
    }

    //
    // Close the file, free memory
    //
    out.close();
    tris->Delete();
    geom->Delete();
}


// ****************************************************************************
//  Method: avtDatasetFileWriter::WritePOVRayDF3File
//
//  Purpose:
//      Writes a rectilinear grid to a DF3 file for POVRay.
//
//  Arguments:
//      rgrid            The vtkRectilinearGrid to write.
//      filename         The filename to use.
//      extents          The actual spatial extents of this grid.
//
//  Returns:    true if file was written
//
//  Programmer: Jeremy Meredith
//  Creation:   May 31, 2007
//
// ****************************************************************************

bool
avtDatasetFileWriter::WritePOVRayDF3File(vtkRectilinearGrid *rgrid,
                                         const char *df3name,
                                         double extents[6])
{
    vtkDataArray *ptscalars = rgrid->GetPointData()->GetScalars();
    vtkDataArray *cellscalars = rgrid->GetCellData()->GetScalars();

    // If there is no data, this is pointless
    if (!ptscalars && !cellscalars)
        return false;

    // Get the dimensions
    int dims[3];
    rgrid->GetDimensions(dims);

    // Get the  extents
    rgrid->ComputeBounds();
    double *bounds = rgrid->GetBounds();
    for (int i=0; i<6; i++)
        extents[i] = bounds[i];

    // Get the scalar array and adjust dims to match
    vtkDataArray *scalars = ptscalars;
    if (!ptscalars)
    {
        scalars = cellscalars;
        dims[0]--;
        dims[1]--;
        dims[2]--;
    }
    int nvals = dims[0]*dims[1]*dims[2];

    // These had better match....
    if (nvals != scalars->GetNumberOfTuples())
        return false;

    // Get the min/max value for scaling
    double minval = +FLT_MAX;
    double maxval = -FLT_MAX;
    for (int i=0; i<nvals; i++)
    {
        double val = scalars->GetComponent(i, 0);
        if (minval > val)
            minval = val;
        if (maxval < val)
            maxval = val;
    }
    if (maxval==minval)
        maxval = minval+1;

    // Open the file and write the dims as big-endian 2-byte ints
    ofstream out(df3name, ios::out);
    out.put((dims[0] >> 8) & 0xff);
    out.put(dims[0] & 0xff);
    out.put((dims[1] >> 8) & 0xff);
    out.put(dims[1] & 0xff);
    out.put((dims[2] >> 8) & 0xff);
    out.put(dims[2] & 0xff);

    // Write the data as 4-byte big-endian ints
    for (int i=0; i<nvals; i++)
    {
        double value      = scalars->GetComponent(i, 0);
        double normalized = (value-minval)/(maxval-minval);
        if (normalized<0)
            normalized=0;
        if (normalized>1)
            normalized=1;
        unsigned int scaled =
            (unsigned int)((unsigned int)(0xffffffff) * normalized);
        out.put((scaled >> 24) & 0xff);
        out.put((scaled >> 16) & 0xff);
        out.put((scaled >> 8)  & 0xff);
        out.put(scaled & 0xff);
    }

    out.close();
    return true;
}


