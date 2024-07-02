// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include "conduit_relay_io_hdf5.hpp"
#include "conduit_blueprint.hpp"

#ifdef PARALLEL
#include "conduit_blueprint_mpi.hpp"
#endif

#include "avtBlueprintLogging.h"

using     std::string;
using     std::vector;

using namespace conduit;

int    avtBlueprintWriter::INVALID_CYCLE = -INT_MAX;
double avtBlueprintWriter::INVALID_TIME = -DBL_MAX;

// ****************************************************************************
//  Method: LoadConduitOptions
//
//  Purpose:
//    Helper function to load Conduit options from a string
//
//  Programmer: Chris Laganella
//  Creation:   Wed Feb  9 11:42:53 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static void
LoadConduitOptions(const std::string &optString, conduit::Node &out)
{
    out.reset();
    // Just return an empty node if there's an empty string, no issue.
    if(optString.empty())
    {
        return;
    }

    bool ok = false;
    TRY
    {
        out.parse(optString, "yaml");
        ok = true;
    }
    CATCHALL
    {
        ok = false;
        out.reset();
    }
    ENDTRY

    if(!ok)
    {
        TRY
        {
            out.parse(optString, "json");
            ok = true;
        }
        CATCHALL
        {
            ok = false;
            out.reset();
        }
        ENDTRY
    }

    if(!ok)
    {
        out.reset();
        BP_PLUGIN_EXCEPTION1(VisItException, "Could not parse"
            " 'Flatten / Partition extra options' as either JSON or Yaml.");
    }
}

//-----------------------------------------------------------------------------
// These methods are used to re-wire conduit's default error handling
//-----------------------------------------------------------------------------
void
blueprint_writer_plugin_print_msg(const std::string &msg,
                           const std::string &file,
                           int line)
{
    // Uncomment for very verbose traces:
    //
    // debug5 << "File:"    << file << std::endl
    //        << "Line:"    << line << std::endl
    //        << "Message:" << msg  << std::endl;
    debug5 << msg  << std::endl;
}

//-----------------------------------------------------------------------------
void
blueprint_writer_plugin_info_handler(const std::string &msg,
                              const std::string &file,
                              int line)
{
    blueprint_writer_plugin_print_msg(msg,file,line);
}


//-----------------------------------------------------------------------------
void
blueprint_writer_plugin_warning_handler(const std::string &msg,
                                 const std::string &file,
                                 int line)
{
    blueprint_writer_plugin_print_msg(msg,file,line);
}

//-----------------------------------------------------------------------------
void
blueprint_writer_plugin_error_handler(const std::string &msg,
                               const std::string &file,
                               int line)
{
    std::ostringstream bp_err_oss;
    bp_err_oss << msg << std::endl << "  from " << file << ":" << line;
    // Make a copy of the stream output so it is not empty the second time we
    // need to use it.
    std::string tmp(bp_err_oss.str());
    debug1 << tmp;

    BP_PLUGIN_EXCEPTION1(VisItException, tmp);
}

// ****************************************************************************
//  Method: avtBlueprintWriter constructor
//
//  Programmer: Matt Larsen
//  Creation:   Feb 1, 2019
//
//  Modifications:
//
//  Chris Laganella Thu Nov  4 15:15:06 EDT 2021
//  Added support for blueprint mesh operations
//
//  Chris Laganella Wed Dec 15 17:57:09 EST 2021
//  Add conditional compilation based on flatten/partition support
// 
//    Justin Privitera, Tue Aug 23 14:40:24 PDT 2022
//    Removed `CONDUIT_HAVE_PARTITION_FLATTEN` check.
//
// ****************************************************************************

avtBlueprintWriter::avtBlueprintWriter(DBOptionsAttributes *options) :m_stem(),
    m_meshName(), m_chunks()
{
    m_nblocks = 0;

    m_op = BP_MESH_OP_NONE;

    if(options)
    {
        int op_val = options->GetEnum("Operation");
        if(op_val >= 0 && op_val < 4)
        {
            m_op = (bpMeshOp)op_val;
        }
        else
        {
            BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                "Invalid value passed for attribute 'Operation'.");
        }

        if(m_op == BP_MESH_OP_FLATTEN_CSV || m_op == BP_MESH_OP_FLATTEN_HDF5
                || m_op == BP_MESH_OP_PARTITION)
        {
            // Parse JSON/YAML input into m_options
            LoadConduitOptions(options->GetMultiLineString("Flatten / Partition extra options"), m_options);

            if(m_op == BP_MESH_OP_PARTITION)
            {
                // Only take the target value from the gui if one was not already set
                //   in the JSON
                int target_val = options->GetInt("Partition target number of domains");
                if(!m_options.has_child("target") && target_val > 0)
                {
                    m_options["target"].set(target_val);
                }
            }
        }
        else
        {
            // Parse JSON/YAML input into m_options
            LoadConduitOptions(options->GetMultiLineString("Blueprint Relay I/O extra options"), m_options);
        }
    }

    conduit::utils::set_info_handler(blueprint_writer_plugin_info_handler);
    conduit::utils::set_warning_handler(blueprint_writer_plugin_warning_handler);
    // this catches any uncaught conduit errors, logs them to debug 1
    // and  converts them into a VisIt Exception
    conduit::utils::set_error_handler(blueprint_writer_plugin_error_handler);
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
//  Chris Laganella Thu Nov  4 15:22:37 EDT 2021
//  I moved the subdirectory creation code out into its own function (CreateOutputDir),
//  and only create the directory if we are not performing an operation.
//
//  Chris Laganella Thu Nov  4 18:53:09 EDT 2021
//  Updated m_nblocks to match the number of target domains if we are partitioning
// ****************************************************************************

void
avtBlueprintWriter::OpenFile(const string &stemname, int nb)
{
#ifdef PARALLEL
    BP_PLUGIN_INFO("I'm rank " << writeContext.Rank() << " and I called OpenFile().");
#endif
    m_stem = stemname;
    m_nblocks = nb;
    if(m_op == BP_MESH_OP_PARTITION)
    {
        m_genRoot = true;
        n_root_file.reset();
        CreateOutputDir();
    }
    if(m_op == BP_MESH_OP_NONE)
    {
        const int c = [&]() -> int
        {
            const int cycle = GetCycle();
            return (cycle == INVALID_CYCLE ? 0 : cycle);
        }();
        // we want the basename without the extension to use as a sub-dir name
        m_mbDirName = FileFunctions::Basename(m_stem);
        std::ostringstream oss;
        oss << m_stem << ".cycle_" << std::setfill('0') << std::setw(6) << c;
        m_output_dir = oss.str();
    }
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
#ifdef PARALLEL
    BP_PLUGIN_INFO("I'm rank " << writeContext.Rank() << " and I called WriteHeaders().");
#endif
    m_meshName = GetMeshName(md);
    m_time     = GetTime();
    m_cycle    = GetCycle();
    exprList   = md->GetExprList();
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
//  Chris Laganella Thu Nov  4 16:07:56 EDT 2021
//  I moved the vtkDataSet -> blueprint mesh code into ChunkToBpMesh() and
//  the relay::io::save code into WriteMeshDomain()
// ****************************************************************************
void
avtBlueprintWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
#ifdef PARALLEL
    BP_PLUGIN_INFO("I'm rank " << writeContext.Rank() << " and I called WriteChunk().");
#endif
    char chunkname[1024];
    if (m_nblocks > 1)
        sprintf(chunkname, "%s/%s.%d", m_stem.c_str(), m_mbDirName.c_str(), chunk);
    else
        sprintf(chunkname, "%s", m_stem.c_str());

    BP_PLUGIN_INFO("BlueprintMeshWriter: " << chunkname
                    << " [domain " << chunk<< "]");

    Node &mesh = m_chunks.append();
    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    ChunkToBpMesh(ds, chunk, ndims, mesh);

    // TODO do equivalent at hte end
    // if(m_op == BP_MESH_OP_NONE)
    // {
    //     // If we aren't partitioning/flattening the mesh
    //     // then we can write it out like normal and clear m_chunks.
    //     WriteMeshDomain(mesh, chunk);

    //     if(m_genRoot)
    //     {
    //         BP_PLUGIN_INFO("BlueprintMeshWriter: generating root");
    //         GenRootNode(mesh, m_output_dir, ndims);
    //         m_genRoot = false;
    //     }
    //     m_chunks.reset();
    // }
    // // Need to defer all mesh operations to CloseFile()
}


// ****************************************************************************
//  Method: BuildSelections
//
//  Purpose:
//      Reads the avtGhostZones field and builds a "selections" node so that
//      ghost cells do not contribute to the partition operation.
//
//      This function will remove the avtGhostZones field from the input domains.
//
//  Programmer: Chris Laganella
//  Creation:   Mon Nov  8 15:26:05 EST 2021
//
//  Modifications:
//
//  Chris Laganella Wed Jan 12 12:52:01 EST 2022
//  I converted this from a static method of the avtBlueprintWriter class
//  to a static function local to this file. I now conditionally compile
//  the function based off partition/flatten support since it is only used
//  by the partition operation.
//
//  Brad Whitlock, Fri Apr  1 13:41:32 PDT 2022
//  Removed /c0 since Conduit scalars are no longer mcarrays.
// 
//    Justin Privitera, Tue Aug 23 14:40:24 PDT 2022
//    Removed `CONDUIT_HAVE_PARTITION_FLATTEN` check.
//
// ****************************************************************************
static void
BuildSelections(Node &domains, Node &selections)
{
    selections.reset();
    const std::vector<Node*> n_domains =
        conduit::blueprint::mesh::domains(domains);

    for(Node *m : n_domains)
    {
        if(!m->has_path("fields/avtGhostZones"))
        {
            continue;
        }
        const Node &avtGhostZones = m->fetch("fields/avtGhostZones");

        // This path should always be correct, the field is created by VisIt.
        const Node &values = avtGhostZones.fetch_existing("values");
        const DataType dt = DataType::index_t(values.dtype().number_of_elements());

        // Cast the ghost info to the correct type if necessary
        Node n_tmp;
        DataArray<index_t> orig_vals;
        if(values.dtype().is_index_t())
        {
            orig_vals = values.value();
        }
        else
        {
            values.to_data_type(dt.id(), n_tmp);
            orig_vals = n_tmp.value();
        }

        // Build selection element ids
        std::vector<index_t> sel;
        sel.reserve(dt.number_of_elements());
        for(index_t i = 0; i < orig_vals.number_of_elements(); i++)
        {
            if(orig_vals[i] == 0)
            {
                sel.push_back(i);
            }
        }

        // Create a selection for this domain
        Node &selection = selections.append();
        selection["type"].set("explicit");
        selection["topology"].set(avtGhostZones.child("topology"));
        selection["domain_id"].set(m->fetch_existing("state/domain_id"));
        selection["elements"].set(sel);

        // No longer need the avtGhostZones field
        m->child("fields").remove_child("avtGhostZones");
    }
    BP_PLUGIN_INFO("Done building selections." << selections.schema().to_json());
}

// ****************************************************************************
//  Method: avtBlueprintWriter::ChunkToBpMesh
//
//  Purpose:
//      Calls VTKToBlueprint on the given vtkDataSet. Uses chunk for
//      mesh["state/domain_id"] and stores the output in the mesh parameter.
//
//  Programmer: Chris Laganella
//  Creation:   Thu Nov  4 16:00:44 EDT 2021
//  This code originated in OpenFile()
//
//  Modifications:
//     Justin Privitera, Wed Aug 24 11:08:51 PDT 2022
//     Call vtk to bp from the avt conduit bp data adaptor.
//
// ****************************************************************************
void
avtBlueprintWriter::ChunkToBpMesh(vtkDataSet *ds, int chunk, int ndims,
                                  Node &mesh)
{
    mesh.reset();
    mesh["state/domain_id"] = chunk;
    // std::string topo_name = "topo";

    if (!m_meshName.empty())
    {
        // topo_name = m_meshName;
    }

    if (m_cycle != INVALID_CYCLE)
    {
        mesh["state/cycle"] = m_cycle;
    }

    if (m_time != INVALID_TIME )
    {
        mesh["state/time"] = m_time;
    }

    avtConduitBlueprintDataAdaptor::VTKToBlueprint::VTKToBlueprintMesh(mesh, ds, ndims);

    Node verify_info;
    if(!blueprint::mesh::verify(mesh,verify_info))
    {
        BP_PLUGIN_EXCEPTION1(InvalidVariableException,
                             "VTK to Blueprint conversion failed " << verify_info.to_json());
        return;
    }
}

// ****************************************************************************
//  Method: avtBlueprintWriter::CreateOutputDir
//
//  Purpose:
//      Creates a subdirectory based off m_stem and the current cycle.
//
//  Programmer: Chris Laganella
//  Creation:   Thu Nov  4 15:22:37 EDT 2021
//  This code originated in OpenFile()
//
//  Modifications:
//
// ****************************************************************************
void
avtBlueprintWriter::CreateOutputDir()
{
    const int c = [&]() -> int
    {
        const int cycle = GetCycle();
        return (cycle == INVALID_CYCLE ? 0 : cycle);
    }();
    // we want the basename without the extension to use as a sub-dir name
    m_mbDirName = FileFunctions::Basename(m_stem);
    std::ostringstream oss;
    oss << m_stem << ".cycle_" << std::setfill('0') << std::setw(6) << c;
    m_output_dir = oss.str();

#ifdef WIN32
    _mkdir(m_output_dir.c_str());
#else
    mkdir(m_output_dir.c_str(), 0777);
#endif
    BP_PLUGIN_INFO("BlueprintMeshWriter: create output dir "<<m_output_dir);
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
//  Mark C. Miller, Thu May  7 15:04:11 PDT 2020
//  Add expressions output
// 
//  Justin Privitera, Wed Apr 27 22:56:31 PDT 2022
//  Removed expressions output.
// 
// ****************************************************************************
void
avtBlueprintWriter::GenRootNode(conduit::Node &mesh,
                                const std::string output_dir,
                                const int ndims)
{
#ifdef PARALLEL
    BP_PLUGIN_INFO("I'm rank " << writeContext.Rank() << " and I called GenRootNode().");
#endif
    const int c = [&]() -> int
    {
        const int cycle = GetCycle();
        return (cycle == INVALID_CYCLE ? 0 : cycle);
    }();

    std::stringstream oss;
    std::string root_dir = FileFunctions::Dirname(output_dir);
    oss << m_mbDirName << ".cycle_" << std::setfill('0') << std::setw(6) << c << ".root";
    m_root_file = oss.str();

    // TODO I think this is bugged
    m_root_file = utils::join_file_path(root_dir,
                                        m_root_file);

    std::string output_file_pattern;
    output_file_pattern = utils::join_file_path(output_dir,
                                                "domain_%06d.hdf5");
    // TODO can this all be deleted?
    // n_root_file.reset();
    // Node &bp_idx = n_root_file["blueprint_index"];
    // blueprint::mesh::generate_index(mesh,
    //                                 "",
    //                                 m_nblocks,
    //                                 bp_idx["mesh"]);

    // // work around conduit bug
    // if(mesh.has_path("state/cycle"))
    // {
    //   bp_idx["mesh/state/cycle"] = mesh["state/cycle"].to_int32();
    // }

    // if(mesh.has_path("state/time"))
    // {
    //   bp_idx["mesh/state/time"] = mesh["state/time"].to_double();
    // }

    // n_root_file["protocol/name"]    =  "hdf5";
    // n_root_file["protocol/version"] = "0.4.0";

    // n_root_file["number_of_files"]  = m_nblocks;
    // // for now we will save one file per domain, so trees == files
    // n_root_file["number_of_trees"]  = m_nblocks;
    // n_root_file["file_pattern"]     = output_file_pattern;
    // n_root_file["tree_pattern"]     = "/";

    // //n_root_file.print();
}

// ****************************************************************************
//  Method: avtBlueprintWriter::WriteMeshDomain
//
//  Purpose:
//      Writes the given blueprint mesh domain to a file based off the given
//      domain_id.
//
//  Programmer: Chris Laganella
//  Creation:   Thu Nov  4 16:00:44 EDT 2021
//  This code orginated in WriteChunk
//
//  Modifications:
//
// ****************************************************************************
void
avtBlueprintWriter::WriteMeshDomain(Node &mesh, int domain_id)
{
#ifdef PARALLEL
    BP_PLUGIN_INFO("I'm rank " << writeContext.Rank() << " and I called WriteMeshDomain().");
#endif
    std::stringstream oss;
    oss << "domain_" << std::setfill('0') << std::setw(6) << domain_id << "." << "hdf5";
    string output_file  = conduit::utils::join_file_path(m_output_dir,oss.str());
    // TODO does this stay? does this whole function die?
    relay::io::save(mesh, output_file);
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
//  Chris Laganella Thu Nov  4 16:47:28 EDT 2021
//  Added support for flatten to CSV or HDF5
//
//  Chris Laganella Thu Nov  4 18:54:15 EDT 2021
//  Added support for partition to a target number of domains
//
//  Chris Laganella Mon Nov  8 16:58:22 EST 2021
//  Added BuildSelections call and surrounding logic
//
//  Chris Laganella Wed Dec 15 18:01:21 EST 2021
// 
//    Justin Privitera, Tue Aug 23 14:40:24 PDT 2022
//    Removed `CONDUIT_HAVE_PARTITION_FLATTEN` check.
//
// ****************************************************************************
void
avtBlueprintWriter::CloseFile(void)
{
#ifdef PARALLEL
    BP_PLUGIN_INFO("I'm rank " << writeContext.Rank() << " and I called CloseFile().");
#endif

    if (m_op == BP_MESH_OP_NONE)
    {
        debug5 << "Relay I/O Blueprint options:\n" << m_options.to_string() << std::endl;
        int rank = 0;
        const int root = 0;
#ifdef PARALLEL
        rank = writeContext.Rank();
        BP_PLUGIN_INFO("BlueprintMeshWriter: rank " << rank << " relay io blueprint save_mesh.");
#endif
        if(rank == root)
        {
            conduit::relay::io::blueprint::save_mesh(m_chunks, m_mbDirName, "hdf5", m_options);
        }
    }
    else if(m_op == BP_MESH_OP_FLATTEN_CSV || m_op == BP_MESH_OP_FLATTEN_HDF5)
    {
        debug5 << "Flatten options:\n" << m_options.to_string() << std::endl;
        conduit::Node table;
        int rank = 0;
        int root = 0;
#ifdef PARALLEL
        rank = writeContext.Rank();
        BP_PLUGIN_INFO("BlueprintMeshWriter: rank " << rank << " flattening.");
        // It's okay to pass empty nodes to this.
        conduit::blueprint::mpi::mesh::flatten(m_chunks, m_options, table,
            writeContext.GetCommunicator());
#else
        conduit::blueprint::mesh::flatten(m_chunks, m_options, table);
#endif
        // Don't need the mesh anymore.
        m_chunks.reset();

        const std::string filename = m_op == BP_MESH_OP_FLATTEN_CSV ? m_stem + ".csv"
                                                                    : m_stem + ".hdf5";
        if(rank == root)
        {
#ifdef PARALLEL
        BP_PLUGIN_INFO("I'm rank " << rank << " and I'm about to write " << filename << ".");
#endif
            conduit::relay::io::save(table, filename);
        }
    }
    else if(m_op == BP_MESH_OP_PARTITION)
    {
        debug5 << "Partition options:\n" << m_options.to_string() << std::endl;
        int rank = 0;
        Node repart_mesh;
        {
            Node selections;
            // If the user has not given their own selections then
            //  filter out the ghost nodes/zones
            if(!m_options.has_child("selections"))
            {
                BuildSelections(m_chunks, selections);
                if(!selections.dtype().is_empty())
                {
                    // Make sure selections stays alive for the partition call!
                    m_options["selections"].set_external(selections);
                }
            }

#ifdef PARALLEL
            rank = writeContext.Rank();
            BP_PLUGIN_INFO("BlueprintMeshWriter: rank " << rank << " partitioning.");
            conduit::blueprint::mpi::mesh::partition(m_chunks, m_options, repart_mesh,
                writeContext.GetCommunicator());
            m_nblocks = conduit::blueprint::mpi::mesh::number_of_domains(repart_mesh,
                writeContext.GetCommunicator());
#else
            conduit::blueprint::mesh::partition(m_chunks, m_options, repart_mesh);
            if(!repart_mesh.dtype().is_empty())
            {
                m_nblocks = conduit::blueprint::mesh::number_of_domains(repart_mesh);
            }
            else
            {
                m_nblocks = 0;
            }
#endif
        }
        // Don't need the original data anymore
        m_chunks.reset();

        if(!repart_mesh.dtype().is_empty())
        {
            const std::vector<Node*> n_domains =
                conduit::blueprint::mesh::domains(repart_mesh);

            for(Node *m : n_domains)
            {
                BP_PLUGIN_INFO("Rank " << rank << " domain:" << m->schema().to_json());
                int dom_id = m->fetch("state/domain_id").to_int();
                WriteMeshDomain(*m, dom_id);

                if(m_genRoot)
                {
                    BP_PLUGIN_INFO("BlueprintMeshWriter: generating root");
                    int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
                    GenRootNode(*m, m_output_dir, ndims);
                    m_genRoot = false;
                }
            }
        }
    }
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
//  Chris Laganella Thu Nov  4 18:55:00 EDT 2021
//  Writes a root file as long as we aren't flattening
// ****************************************************************************

void
avtBlueprintWriter::WriteRootFile()
{
    if (m_op == BP_MESH_OP_PARTITION)
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
            BP_PLUGIN_INFO("BlueprintMeshWriter: writing root "<<m_root_file);
            relay::io::save(n_root_file, m_root_file,"hdf5");
        }
    }
}
