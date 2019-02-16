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

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkDataSetWriter.h>
#include <vtkFieldData.h>
#include <vtkStringArray.h>
#include <vtkIntArray.h>
#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLRectilinearGridWriter.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <avtDatabaseMetaData.h>
#include <avtParallelContext.h>
#include <FileFunctions.h>

#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

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

#include "avtBlueprintDataAdaptor.h"
#include "avtBlueprintLogging.h"

using     std::string;
using     std::vector;

using namespace conduit;

int    avtBlueprintWriter::INVALID_CYCLE = -INT_MAX;
double avtBlueprintWriter::INVALID_TIME = -DBL_MAX;

// ****************************************************************************
//  Method: avtBlueprintWriter constructor
//
//  Programmer: Matt Larsen
//  Creation:   Feb 1, 2019
//
//  Modifications:
//
// ****************************************************************************

avtBlueprintWriter::avtBlueprintWriter() :stem(), meshName()
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
//  Programmer: Matt Larsen
//  Creation:   Feb 1, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtBlueprintWriter::OpenFile(const string &stemname, int nb)
{
    stem = stemname;
    nblocks = nb;

    int c = GetCycle();
    if (c != INVALID_CYCLE)
    {
        c = 0;
    }
    char fmt_buff[64];
    snprintf(fmt_buff, sizeof(fmt_buff), "%06d",c);
    // we want the basename without the extension to use as a sub-dir name
    mbDirName = FileFunctions::Basename(stem);
    std::ostringstream oss;
    oss << stem << ".cycle_" << fmt_buff;
    output_dir  =  oss.str();

#ifdef WIN32
    _mkdir(output_dir.c_str());
#else
    mkdir(output_dir.c_str(), 0777);
#endif
    genRoot = true;
    n_root_file.reset();
}


// ****************************************************************************
//  Method: avtBlueprintWriter::WriteHeader
//
//  Purpose:
//      Get any info from the metadata that we need to wrtie data
//
//  Programmer: Matt Larsen
//  Creation:   Feb 1, 2019
//
//  Modifications:
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
//  Programmer: Matt Larsen
//  Creation:  Feb 1, 2019
//
//  Modifications:
//
//
// ****************************************************************************
void
avtBlueprintWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    char chunkname[1024];
    if (nblocks > 1)
        sprintf(chunkname, "%s/%s.%d", stem.c_str(), mbDirName.c_str(), chunk);
    else
        sprintf(chunkname, "%s", stem.c_str());

    Node mesh;
    mesh["state/domain_id"] = chunk;
    //std::string topo_name = "topo";

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

    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    avtBlueprintDataAdaptor::BP::VTKToBlueprint(mesh, ds, ndims);

    Node verify_info;
    if(!blueprint::mesh::verify(mesh,verify_info))
    {
         BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                              "VTK to Blueprint convertion failed " << verify_info.to_json());
        return;
    }

    char fmt_buff[64];
    snprintf(fmt_buff, sizeof(fmt_buff), "%06llu",chunk);
    std::stringstream oss;
    oss << "domain_" << fmt_buff << "." << "hdf5";
    string output_file  = conduit::utils::join_file_path(output_dir,oss.str());
    relay::io::save(mesh, output_file);

    if(genRoot)
    {
        GenRootNode(mesh, output_dir);
        genRoot = false;
    }
}


// ****************************************************************************
//  Method: avtBlueprintWriter::GenRootNode
//
//  Purpose:
//      Generates the conduit node that contains the root file info
//
//  Programmer: Matt Larsen
//  Creation:   Feb 1, 2019
//
//  Modifications:
//
// ****************************************************************************
void
avtBlueprintWriter::GenRootNode(conduit::Node &mesh,
                                const std::string output_dir)
{

    int c = GetCycle();
    if (c != INVALID_CYCLE)
    {
        c = 0;
    }

    char fmt_buff[64];
    snprintf(fmt_buff, sizeof(fmt_buff), "%06llu",c);

    std::stringstream oss;
    std::string root_dir = FileFunctions::Dirname(output_dir);
    std::cout<<"root dir "<<root_dir<<"\n";
    oss << mbDirName << ".cycle_" << fmt_buff << ".root";
    root_file = oss.str();

    root_file = utils::join_file_path(root_dir,
                                      root_file);

    std::cout<<"Full root "<<root_file<<"\n";
    std::string output_file_pattern;
    output_file_pattern = utils::join_file_path(output_dir,
                                                "domain_%06d.hdf5");
    n_root_file.reset();
    Node &bp_idx = n_root_file["blueprint_index"];
    blueprint::mesh::generate_index(mesh,
                                    "",
                                    nblocks,
                                    bp_idx["mesh"]);
    // work around conduit bug
    if(mesh.has_path("state/cycle"))
    {
      bp_idx["mesh/state/cycle"] = mesh["state/cycle"].to_int32();
    }

    if(mesh.has_path("state/time"))
    {
      bp_idx["mesh/state/time"] = mesh["state/time"].to_double();
    }

    n_root_file["protocol/name"]    =  "hdf5";
    n_root_file["protocol/version"] = "0.4.0";

    n_root_file["number_of_files"]  = nblocks;
    // for now we will save one file per domain, so trees == files
    n_root_file["number_of_trees"]  = nblocks;
    n_root_file["file_pattern"]     = output_file_pattern;
    n_root_file["tree_pattern"]     = "/";

    //n_root_file.print();
}

// ****************************************************************************
//  Method: avtBlueprintWriter::CloseFile
//
//  Purpose:
//      Closes the file.
//
//  Programmer: Matt Larsen
//  Creation:   Feb 1, 2019
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
//  Programmer: Matt Larsen
//  Creation:   Feb 1, 2019
//
//  Modifications:
//
// ****************************************************************************

void
avtBlueprintWriter::WriteRootFile()
{
    int root_writer = 0;
    int rank = 0;
#ifdef PARALLEL
    // assume nothing about what rank was given a chunk;
    rank = writeContext.Rank();
    bool has_root_file = n_root_file.has_path("blueprint_index");
    int i_has_root = has_root_file ? 1 : 0;
    int *roots = new int[writeContext.Size()];

    MPI_Allgather(&i_has_root, 1, MPI_INT, roots, 1, MPI_INT,
                  writeContext.GetCommunicator());

    for(int i = 0; i < writeContext.Size(); ++i)
    {
        if(roots[i] == 1)
        {
            root_writer = i;
            break;
        }
    }

    delete[] roots;
#endif
    if(rank == root_writer)
    {
        relay::io::save(n_root_file, root_file,"hdf5");
    }
}
