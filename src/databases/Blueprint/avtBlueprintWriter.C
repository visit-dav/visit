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
//                               avtBlueprintWriter.C                        //
// ************************************************************************* //

#include <avtBlueprintWriter.h>

#include <vector>

#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkFieldData.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLRectilinearGridWriter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <avtDatabaseMetaData.h>
#include <avtParallelContext.h>
#include <FileFunctions.h>

#include <DBOptionsAttributes.h>
#ifdef WIN32
#include <direct.h>
#endif

//-----------------------------------------------------------------------------
// conduit includes
//-----------------------------------------------------------------------------
#include "conduit.hpp"
#include "conduit_relay.hpp"
#include "conduit_relay_hdf5.hpp"
#include "conduit_blueprint.hpp"

using     std::string;
using     std::vector;

using namespace conduit;

int    avtBlueprintWriter::INVALID_CYCLE = -INT_MAX;
double avtBlueprintWriter::INVALID_TIME = -DBL_MAX;

// ****************************************************************************
//  Method: avtBlueprintWriter constructor
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
// ****************************************************************************

avtBlueprintWriter::avtBlueprintWriter() :stem(), meshName(), fileNames()
{
    doBinary = false;
    doXML = false;
    nblocks = 0;
}


// ****************************************************************************
//  Method: avtBlueprintWriter::OpenFile
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
avtBlueprintWriter::OpenFile(const string &stemname, int nb)
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
//  Method: avtBlueprintWriter::WriteHeaders
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
avtBlueprintWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           const vector<string> &scalars,
                           const vector<string> &vectors,
                           const vector<string> &materials)
{
    meshName = GetMeshName(md);
    time     = GetTime();
    cycle    = GetCycle();
}


// ****************************************************************************
//  Method: avtBlueprintWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2003
//
//  Modifications:
//
//
// ****************************************************************************
void
CopyTuple1(Node &node, vtkDataArray *da)
{

    int nvals= da->GetNumberOfTuples();
    node.set(DataType::float64(nvals));
    conduit::float64 *vals = node.value();
    for(int i = 0; i < nvals; ++i)
    {
      vals[i] = da->GetTuple1(i);
    }
}

void
CopyComponent64(Node &node, vtkDataArray *da, int component)
{

    int nvals= da->GetNumberOfTuples();
    node.set(DataType::float64(nvals));
    conduit::float64 *vals = node.value();
    for(int i = 0; i < nvals; ++i)
    {
      vals[i] = da->GetComponent(i, component);
    }
}

void
CopyComponent32(Node &node, vtkDataArray *da, int component)
{

    int nvals= da->GetNumberOfTuples();
    node.set(DataType::float32(nvals));
    conduit::float32 *vals = node.value();
    for(int i = 0; i < nvals; ++i)
    {
      vals[i] = (float)da->GetComponent(i, component);
    }
}

void VTKDataArrayToNode(Node &node, vtkDataArray *arr)
{
    // copy up to 3 components
    int ncomps = std::min(3,arr->GetNumberOfComponents());
    std::vector<std::string> cnames;
    cnames.push_back("/x");
    cnames.push_back("/y");
    cnames.push_back("/z");

    int nTuples = arr->GetNumberOfTuples();
    if (ncomps == 1)
    {
        CopyTuple1(node, arr);
    }
    else if(arr->GetDataType() == VTK_DOUBLE)
    {
        for(int comp = 0; comp < ncomps; ++comp)
        {
          CopyComponent64(node[cnames[comp]], arr, comp);
        }
    }
    else
    {
        for(int comp = 0; comp < ncomps; ++comp)
        {
          CopyComponent32(node[cnames[comp]], arr, comp);
        }
    }

}

void WriteVars(Node &node,
               const string topo_name,
               vtkPointData *pd,
               vtkCellData *cd)
{

    for (size_t i = 0 ; i < (size_t)pd->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = pd->GetArray(i);
         // skip special variables
         if (strstr(arr->GetName(), "vtk") != NULL)
             continue;
         if (strstr(arr->GetName(), "avt") != NULL)
             continue;

         std::string fname = arr->GetName();
         std::string field_path = "fields/" + fname;
         node[field_path + "/association"] = "vertex";
         node[field_path + "/topology"] = topo_name;

         VTKDataArrayToNode(node[field_path + "/values"], arr);
    }

    for (size_t i = 0 ; i < (size_t)cd->GetNumberOfArrays() ; i++)
    {
         vtkDataArray *arr = pd->GetArray(i);
         // skip special variables
         if (strstr(arr->GetName(), "vtk") != NULL)
             continue;
         if (strstr(arr->GetName(), "avt") != NULL)
             continue;

         std::string fname = arr->GetName();
         std::string field_path = "fields/" + fname;
         node[field_path + "/association"] = "element";
         node[field_path + "/topology"] = topo_name;

         VTKDataArrayToNode(node[field_path + "/values"], arr);
    }
}

void
avtBlueprintWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    char chunkname[1024];
    if (nblocks > 1)
        sprintf(chunkname, "%s/%s.%d", stem.c_str(), mbDirName.c_str(), chunk);
    else
        sprintf(chunkname, "%s", stem.c_str());

    Node mesh;
    std::string topo_name = "topo";
    if (!meshName.empty())
    {
       // topo_name = meshName;
    }

    if (cycle != INVALID_CYCLE)
    {
        mesh["state/cycle"] = cycle;
    }

    if (time != INVALID_TIME )
    {
        mesh["state/time"] = time;
    }

    std::string topo_path = "topologies/" + topo_name;
    std::string coord_path = "coordsets/coords";
    mesh[topo_path + "/coordset"] = "coords";

    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    std::cout<<"DIMS "<<ndims<<"\n";

    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
       mesh[coord_path+ "/type"] = "rectilinear";
       mesh[topo_path + "/type"] = "structured";
       vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;
       if(ndims > 0)
       {
          int dimx = rgrid->GetXCoordinates()->GetNumberOfTuples();
          CopyTuple1(mesh[coord_path+ "/values/x"], rgrid->GetXCoordinates());
          mesh[topo_path+ "/elements/dims/i"] = dimx;
       }
       if(ndims > 1)
       {
          int dimy = rgrid->GetYCoordinates()->GetNumberOfTuples();
          CopyTuple1(mesh[coord_path + "/values/y"], rgrid->GetYCoordinates());
          mesh[topo_path+ "/elements/dims/j"] = dimy;
       }
       if(ndims > 2)
       {
          int dimz = rgrid->GetZCoordinates()->GetNumberOfTuples();
          CopyTuple1(mesh[coord_path + "/values/z"], rgrid->GetZCoordinates());
          mesh[topo_path+ "/elements/dims/k"] = dimz;
       }
       WriteVars(mesh, topo_name, rgrid->GetPointData(), rgrid->GetCellData());
    }
    else if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
       mesh[coord_path + "/type"] = "explicit";
       mesh[topo_path + "/type"] = "structured";
    }
    else if (ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
       mesh[topo_path + "/type"] = "explicit";
       mesh[coord_path + "/type"] = "explicit";
       //sprintf(chunkname, "%s.vtu", chunkname);
       //wrtr = vtkXMLUnstructuredGridWriter::New();
    }

    Node verify_info;
    if(!blueprint::mesh::verify(mesh,verify_info))
    {
        //BP_PLUGIN_INFO("Skipping mesh named \"" << mesh_name << "\"" << endl
        //               << "blueprint::mesh::index::verify failed " << endl
        //               << verify_info.to_json());
        std::cout<<verify_info.to_json()<<"\n";
        mesh.print();
        return;
    }
    else std::cout<<"MESH smells good\n";
    //if (wrtr)
    //{
    //    if (nblocks > 1)
    //        fileNames.push_back(chunkname);
    //    if(doBinary)
    //    {
    //        wrtr->SetDataModeToBinary();
    //    }
    //    else
    //    {
    //        wrtr->SetDataModeToAscii();
    //        wrtr->SetCompressorTypeToNone();
    //    }
    //    wrtr->SetInputData(ds);
    //    wrtr->SetFileName(chunkname);
    //    wrtr->Write();

    //    wrtr->Delete();
    //}
}


// ****************************************************************************
//  Method: avtBlueprintWriter::CloseFile
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
avtBlueprintWriter::CloseFile(void)
{
}

// ****************************************************************************
//  Method: avtBlueprintWriter::WriteRootFile
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
avtBlueprintWriter::WriteRootFile()
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
                ofile << fileNames[i] << endl;
            }
        }
    }
}
