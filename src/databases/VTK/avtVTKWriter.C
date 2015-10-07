/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                               avtVTKWriter.C                              //
// ************************************************************************* //

#include <avtVTKWriter.h>

#include <vector>

#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkFieldData.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLRectilinearGridWriter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <avtDatabaseMetaData.h>
#include <avtParallelContext.h>

#include <DBOptionsAttributes.h>

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
// ****************************************************************************

avtVTKWriter::avtVTKWriter(DBOptionsAttributes *atts) :stem(), meshName(), fileNames()
{
    doBinary = atts->GetBool("Binary format");
    doXML = atts->GetBool("XML format");
    nblocks = 0;
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
// ****************************************************************************

void
avtVTKWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;
    if (!fileNames.empty())
        fileNames.clear();
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
// ****************************************************************************

void
avtVTKWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    char chunkname[1024];
    if (nblocks > 1)
        sprintf(chunkname, "%s.%d", stem.c_str(), chunk);
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
                wrtr->SetDataModeToBinary();
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
// ****************************************************************************
//
void
avtVTKWriter::WriteRootFile()
{
#ifdef PARALLEL
    if (nblocks > 1 && doXML)
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
                    char *str = new char[size];
                    MPI_Recv(str, size, MPI_CHAR, stat.MPI_SOURCE, dataTag,
                             writeContext.GetCommunicator(), &stat2);
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
                SNPRINTF(filename, 1024, "%s.%d.vtm", stem.c_str(), writeContext.GroupRank());
            else
                SNPRINTF(filename, 1024, "%s.vtm", stem.c_str());
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
                char chunkname[1024];
                SNPRINTF(chunkname, 1024, "%s.%d.vtk", stem.c_str(), i);
                ofile << chunkname << endl;
            }
        }
    }
}
