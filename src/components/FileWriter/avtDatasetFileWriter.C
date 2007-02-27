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
//                           avtDatasetFileWriter.C                          //
// ************************************************************************* //

#include <avtDatasetFileWriter.h>

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
#include <vtkVisItSTLWriter.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>

#include <avtCommonDataFunctions.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoCurveException.h>
#include <NoInputException.h>


// This array contains strings that correspond to the file types that are 
// enumerated in the DatasetFileFormat enum.
const char *avtDatasetFileWriter::extensions[] = { ".curve", ".obj",
                                                   ".stl", ".vtk", ".ultra" };

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
        sprintf(rootfile, "%s.visit", basename);
        ofstream ofile(rootfile);
        ofile << "!NBLOCKS " << nFilesWritten << endl;
        for (int i = 0 ; i < nFilesWritten ; i++)
        {
            char objname[1024];
            sprintf(objname, "%s.%04d%s", basename, i, extensions[(int)OBJ]);
            ofile << objname << endl;
        }
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
        sprintf(fname, "%s.%04d%s", basename, idx, extensions[(int)OBJ]);
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
        sprintf(rootfile, "%s.visit", basename);
        ofstream ofile(rootfile);
        ofile << "!NBLOCKS " << nFilesWritten << endl;
        for (int i = 0 ; i < nFilesWritten ; i++)
        {
            char vtkname[1024];
            sprintf(vtkname, "%s.%04d%s", basename, i, extensions[(int)VTK]);
            ofile << vtkname << endl;
        }
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
        sprintf(fname, "%s.%04d%s", basename, idx, extensions[(int)VTK]);
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
    int maxnums = 4;
    int dotlen = 1;
    str = new char[len + maxnums + dotlen + extlen + 1];

    if (family)
        sprintf(str, "%s%04d%s", base, nFilesWritten, extensions[(int)format]);
    else
        sprintf(str, "%s%s", base, extensions[(int)format]);

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
            sprintf(tmp, "%s%d", attempt, idx);
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


