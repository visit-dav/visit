// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtVTKWriter.C                              //
// ************************************************************************* //

#include <avtVTKWriter.h>

#include <vector>

#include <vtkDataSet.h>
#include <vtkDataSetTriangleFilter.h>
#include <vtkDataSetWriter.h>
#include <vtkFieldData.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkStringArray.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLRectilinearGridWriter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <DebugStream.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <avtDatabaseMetaData.h>
#include <avtParallelContext.h>
#include <FileFunctions.h>

#include <DBOptionsAttributes.h>
#ifdef WIN32
#include <direct.h>
#endif

using     std::string;
using     std::vector;

int    avtVTKWriter::INVALID_CYCLE = -INT_MAX;
double avtVTKWriter::INVALID_TIME = -DBL_MAX;

// ****************************************************************************
//  Method: avtVTKWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2005
//
//  Modifications:
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Initialize doMultiBlock.
//
//    Jeremy Meredith, Tue Mar 27 15:10:21 EDT 2007
//    Added nblocks so we don't have to trust the meta data.
//
//    Kathleen Biagas, Thu Dec 18 14:10:36 PST 2014
//    Add doXML.
//
//    Kathleen Biagas, Wed Feb 25 13:25:07 PST 2015
//    Added meshName.
//
//    Kathleen Biagas, Tue Sep  1 11:27:23 PDT 2015
//    Added fileNames.
//
//    Kathleen Biagas, Fri Feb 17 15:41:33 PST 2017
//    Handle new Write options.
//
//    Mark C. Miller, Tue Apr  9 18:46:02 PDT 2019
//    Add tetrahedralize option.
// ****************************************************************************

avtVTKWriter::avtVTKWriter(const DBOptionsAttributes *atts) :stem(), meshName(), fileNames()
{
    doBinary = false;
    doXML = false;
    nblocks = 0;

    switch(atts->GetEnum("FileFormat"))
    {
        case 0: doBinary = false; doXML = false; break;
        case 1: doBinary = true;  doXML = false; break;
        case 2: doBinary = false; doXML = true;  break;
        case 3: doBinary = true;  doXML = true;  break;
    }
    tetrahedralize = atts->GetBool("Tetrahedralize");
}


// ****************************************************************************
//  Method: avtVTKWriter::OpenFile
//
//  Purpose:
//      Does no actual work.  Just records the stem name for the files.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 27 15:10:21 EDT 2007
//    Added nblocks to this function and save it so we don't have to
//    trust the meta data.
//
//    Kathleen Biagas, Tue Sep  1 11:27:23 PDT 2015
//    Clear fileNames if necessary.
//
//    Kathleen Biagas, Fri Feb 17 15:43:28 PST 2017
//    Multi-block files now write individual files to subdir of the same name.
//
// ****************************************************************************

void
avtVTKWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;
    if (!fileNames.empty())
        fileNames.clear();

    if(nb > 1)
    {
       // we want the basename without the extension to use as a sub-dir name
       mbDirName = FileFunctions::Basename(stem);
#ifdef WIN32
       _mkdir(stem.c_str());
#else
       mkdir(stem.c_str(), 0777);
#endif
    }
}


// ****************************************************************************
//  Method: avtVTKWriter::WriteHeaders
//
//  Purpose:
//      Writes out a VisIt file to tie the VTK files together.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Add support for curves.
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Initialize doMultiBlock.
//
//    Jeremy Meredith, Tue Mar 27 11:36:57 EDT 2007
//    Use the saved nblocks because we can't trust the meta data.
//
//    Hank Childs, Thu Oct 29 17:21:14 PDT 2009
//    Only have processor 0 write out the header file.
//
//    Kathleen Biagas, Wed Feb 25 13:25:07 PST 2015
//    Retrieve meshName.
//
//    Mark C. Miller, Mon Mar  9 19:51:45 PDT 2020
//    Capture a copy of exprList.
// ****************************************************************************

void
avtVTKWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           const vector<string> &scalars,
                           const vector<string> &vectors,
                           const vector<string> &materials)
{
    meshName = GetMeshName(md);
    time     = GetTime();
    cycle    = GetCycle();
    exprList = md->GetExprList();
}


// ****************************************************************************
//  Method: avtVTKWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//
//    Hank Childs, Thu May 26 17:23:39 PDT 2005
//    Add support for binary writes through DB options.
//
//    Hank Childs, Thu Mar 30 12:20:24 PST 2006
//    Change name based on whether or not we are doing multi-block.
//
//    Kathleen Biagas, Thu Dec 18 14:10:06 PST 2014
//    Add support for XML format through DB options.
//
//    Kathleen Biagas, Wed Feb 25 13:25:07 PST 2015
//    Use meshName if not empty.
//
//    Kathleen Biagas, Tue Sep  1 11:28:50 PDT 2015
//    For multi-block xml, save chunk name in fileNames for later processing.
//
//    Kathleen Biagas, Fri Feb 17 15:43:28 PST 2017
//    Multi-block files now write individual files (chunks) to subdir.
//
//    Kathleen Biagas, Mon Nov 20 13:06:01 PST 2017
//    When writing ascii xml files, set DataMode to Ascii to prevent data
//    being appended, which is always binary.
//
//    Mark C. Miller, Tue Apr  9 18:45:38 PDT 2019
//    Add tetrahedralize option (works in 2D and 3D).
//
//    Mark C. Miller, Mon Mar  9 19:50:57 PDT 2020
//    Add output of expressions
//
//    Kathleen Biagas, Fri Mar 12, 2021
//    Remove deletion of ds when tetrahedralizing, as it causes engine to
//    crash when deleting plots after an export.
//
//    Justin Privitera, Mon Apr 25 15:57:29 PDT 2022
//    Removed the expression output.
//
// ****************************************************************************

void
avtVTKWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    char chunkname[1024];
    if (nblocks > 1)
        sprintf(chunkname, "%s/%s.%d", stem.c_str(), mbDirName.c_str(), chunk);
    else
        sprintf(chunkname, "%s", stem.c_str());

    if (!meshName.empty())
    {
        vtkStringArray *mn = vtkStringArray::New();
        mn->SetNumberOfValues(1);
        mn->SetValue(0, meshName);
        mn->SetName("MeshName");
        ds->GetFieldData()->AddArray(mn);
        mn->Delete();
    }

    if (cycle != INVALID_CYCLE)
    {
        vtkIntArray *mn = vtkIntArray::New();
        mn->SetNumberOfValues(1);
        mn->SetValue(0, cycle);
        mn->SetName("CYCLE");
        ds->GetFieldData()->AddArray(mn);
        mn->Delete();
    }

    if (time != INVALID_TIME )
    {
        vtkDoubleArray *mn = vtkDoubleArray::New();
        mn->SetNumberOfValues(1);
        mn->SetValue(0, time);
        mn->SetName("TIME");
        ds->GetFieldData()->AddArray(mn);
        mn->Delete();
    }

    if (tetrahedralize)
    {
        if (ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
        {
            debug1 << "Converting mesh to tets/tris..." << endl;
            debug1 << "    " << ds->GetNumberOfPoints() << " points, " << ds->GetNumberOfCells() << " cells ==> ";
            vtkDataSetTriangleFilter *tf = vtkDataSetTriangleFilter::New();
            tf->SetInputData(ds);
            tf->Update();
            vtkDataSet *_ds = (vtkDataSet*) tf->GetOutput();
            _ds->Register(NULL);
            tf->Delete();
            ds = _ds;
            debug1 << ds->GetNumberOfPoints() << " points, " << ds->GetNumberOfCells() << " cells." << endl;;
        }
        else
        {
            debug1 << "Request to tetrahedralize structured grid not supported...ignored." << endl;
        }
    }

    if (!doXML)
    {
        sprintf(chunkname, "%s.vtk", chunkname);
        vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
        if (doBinary)
            wrtr->SetFileTypeToBinary();
        wrtr->SetInputData(ds);
        wrtr->SetFileName(chunkname);
        wrtr->Write();

        wrtr->Delete();
        if (nblocks > 1)
            fileNames.push_back(chunkname);
    }
    else
    {
        vtkXMLWriter *wrtr = NULL;
        if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        {
           sprintf(chunkname, "%s.vtr", chunkname);
           wrtr = vtkXMLRectilinearGridWriter::New();
        }
        else if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
        {
           sprintf(chunkname, "%s.vts", chunkname);
           wrtr = vtkXMLStructuredGridWriter::New();
        }
        else if (ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
        {
           sprintf(chunkname, "%s.vtu", chunkname);
           wrtr = vtkXMLUnstructuredGridWriter::New();
        }
        else if (ds->GetDataObjectType() == VTK_POLY_DATA)
        {
           sprintf(chunkname, "%s.vtp", chunkname);
           wrtr = vtkXMLPolyDataWriter::New();
        }

        if (wrtr)
        {
            if (nblocks > 1)
                fileNames.push_back(chunkname);
            if(doBinary)
            {
                wrtr->SetDataModeToBinary();
            }
            else
            {
                wrtr->SetDataModeToAscii();
                wrtr->SetCompressorTypeToNone();
            }
            wrtr->SetInputData(ds);
            wrtr->SetFileName(chunkname);
            wrtr->Write();

            wrtr->Delete();
        }
    }
}


// ****************************************************************************
//  Method: avtVTKWriter::CloseFile
//
//  Purpose:
//      Closes the file.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//    Kathleen Biagas, Tue Sep  1 08:58:23 PDT 2015
//    Create 'vtm' file for multi-block XML.
//
//  Modifications:
//
// ****************************************************************************

void
avtVTKWriter::CloseFile(void)
{
}

// ****************************************************************************
//  Method: avtVTKWriter::WriteRootFile
//
//  Purpose:
//      Writes a root file.
//
//  Programmer: Brad Whitlock
//  Creation:
//
//  Modifications:
//    Kathleen Biagas, Tue Sep  1 08:58:23 PDT 2015
//    Create 'vtm' file for multi-block XML.
//
//    Kathleen Biagas, Wed Oct  7 08:32:53 PDT 2015
//    Collect fileNames from all processors to proc 0 before writing .vtm file.
//
//    Kathleen Biagas, Fri Feb 17 15:45:30 PST 2017
//    Use short filenames instead of full-path for writing names to root.
//    .vtm expects relative not full paths.
//
// ****************************************************************************

void
avtVTKWriter::WriteRootFile()
{
    if (nblocks > 1)
    {
        // shorten our full-path-filenames to just the filename
        for (size_t i = 0; i < fileNames.size(); ++i)
        {
            fileNames[i] = mbDirName + string("/") + FileFunctions::Basename(fileNames[i]);
        }
    }
#ifdef PARALLEL
    if (nblocks > 1)
    {
        int tags[3];
        writeContext.GetUniqueMessageTags(tags, 3);
        int nFNTag  = tags[0];
        int sizeTag = tags[1];
        int dataTag = tags[2];


        if (writeContext.Rank() == 0)
        {
            for (int i = 1; i < writeContext.Size(); ++i)
            {
                MPI_Status stat;
                MPI_Status stat2;
                int nfn = 0, size = 0;
                MPI_Recv(&nfn, 1, MPI_INT, MPI_ANY_SOURCE, nFNTag,
                         writeContext.GetCommunicator(), &stat);
                for (int j = 0; j < nfn; ++j)
                {
                    MPI_Recv(&size, 1, MPI_INT, stat.MPI_SOURCE, sizeTag,
                             writeContext.GetCommunicator(), &stat2);
                    char *str = new char[size+1];
                    MPI_Recv(str, size, MPI_CHAR, stat.MPI_SOURCE, dataTag,
                             writeContext.GetCommunicator(), &stat2);
                    str[size] = '\0';
                    fileNames.push_back(str);
                    delete [] str;
                }
            }
        }
        else
        {
            int nfn = (int)fileNames.size();
            MPI_Send(&nfn, 1, MPI_INT, 0, nFNTag, writeContext.GetCommunicator());
            for (int i = 0; i < nfn; ++i)
            {
                int len = (int)fileNames[i].length();
                MPI_Send(&len, 1, MPI_INT, 0, sizeTag, writeContext.GetCommunicator());
                char *str = const_cast<char*>(fileNames[i].c_str());
                MPI_Send(str, len, MPI_CHAR, 0, dataTag, writeContext.GetCommunicator());
            }
        }
    }
#endif
    if (nblocks > 1 && writeContext.Rank() == 0)
    {
        char filename[1024];
        if(doXML)
        {
            if(writeContext.GroupSize() > 1)
                snprintf(filename, 1024, "%s.%d.vtm", stem.c_str(), writeContext.GroupRank());
            else
                snprintf(filename, 1024, "%s.vtm", stem.c_str());
            ofstream ofile(filename);
            ofile << "<?xml version=\"1.0\"?>" << endl;
            ofile << "<VTKFile type=\"vtkMultiBlockDataSet\" version=\"1.0\">" << endl;
            ofile << "  <vtkMultiBlockDataSet>"<< endl;
            ofile << "    <Block index =\"0\">" << endl;
            for (int i = 0 ; i < nblocks ; i++)
            {
                ofile << "      <DataSet index=\"" << i << "\" file=\""
                      << fileNames[i] << "\"/>" << endl;
            }
            ofile << "    </Block>" << endl;
            ofile << "  </vtkMultiBlockDataSet>"<< endl;
            ofile << "</VTKFile>" << endl;
        }
        else
        {
            sprintf(filename, "%s.visit", stem.c_str());
            ofstream ofile(filename);
            ofile << "!NBLOCKS " << nblocks << endl;
            for (int i = 0 ; i < nblocks ; i++)
            {
                ofile << fileNames[i] << endl;
            }
        }
    }
}
