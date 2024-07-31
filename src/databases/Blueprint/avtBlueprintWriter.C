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

#ifdef _WIN32
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
#include "conduit_relay_mpi.hpp"
#include "conduit_relay_mpi_io.hpp"
#include "conduit_relay_mpi_io_blueprint.hpp"
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
//    Justin Privitera, Tue Jul  9 10:47:29 PDT 2024
//    Added logic for changing the output type to yaml and json when
//    https://github.com/LLNL/conduit/issues/1291 is addressed.
//    Load special options for partitioning and flattening and regular options
//    for relay::io::blueprint.
//
// ****************************************************************************

avtBlueprintWriter::avtBlueprintWriter(DBOptionsAttributes *options) :m_stem(),
    m_meshName(), m_chunks()
{
    m_op = BP_MESH_OP_NONE;

    m_output_type = "hdf5";

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

        // TODO add in later once https://github.com/LLNL/conduit/issues/1291 is fixed
        // op_val = options->GetEnum("Output type");
        // if(op_val >= 0 && op_val < 3)
        // {
        //     if ((bpOutputType)op_val == JSON)
        //     {
        //         m_output_type = "json";
        //     }
        //     else if ((bpOutputType)op_val == YAML)
        //     {
        //         m_output_type = "yaml";
        //     }
        //     // HDF5 case is default
        // }
        // else
        // {
        //     BP_PLUGIN_EXCEPTION1(InvalidVariableException,
        //         "Invalid value passed for attribute 'Output type'.");
        // }

        if(m_op == BP_MESH_OP_FLATTEN_CSV || m_op == BP_MESH_OP_FLATTEN_HDF5
                || m_op == BP_MESH_OP_PARTITION)
        {
            // Parse JSON/YAML input into m_special_options
            LoadConduitOptions(options->GetMultiLineString("Flatten / Partition extra options"), m_special_options);

            if(m_op == BP_MESH_OP_PARTITION)
            {
                // Only take the target value from the gui if one was not already set
                //   in the JSON
                int target_val = options->GetInt("Partition target number of domains");
                if(!m_special_options.has_child("target") && target_val > 0)
                {
                    m_special_options["target"].set(target_val);
                }
            }
        }
        
        if (m_op == BP_MESH_OP_NONE || m_op == BP_MESH_OP_PARTITION)
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
// 
//  Justin Privitera, Tue Jul  9 10:47:29 PDT 2024
//  Simplified a great deal now that we are using relay::io::blueprint.
// ****************************************************************************

void
avtBlueprintWriter::OpenFile(const string &stemname, int nb)
{
#ifdef PARALLEL
    BP_PLUGIN_INFO("I'm rank " << writeContext.Rank() << " and I called OpenFile().");
#endif
    m_stem = stemname;
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
// 
//  Justin Privitera, Tue Jul  9 10:47:29 PDT 2024
//  Simplfied a great deal now that we use relay::io:blueprint.
// ****************************************************************************
void
avtBlueprintWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
#ifdef PARALLEL
    BP_PLUGIN_INFO("I'm rank " << writeContext.Rank() << " and I called WriteChunk().");
#endif
    Node &mesh = m_chunks.append();
    const int ndims = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    ChunkToBpMesh(ds, chunk, ndims, mesh);
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
//    Justin Privitera, Tue Jul  9 10:47:29 PDT 2024
//    Use index_t_array instead of DataArray<index_t>.
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
        index_t_array orig_vals;
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

    if (m_time != INVALID_TIME)
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
//    Justin Privitera, Tue Jul  9 10:47:29 PDT 2024
//    Use relay::io::blueprint to write out meshes.
//    Lots of simplification.
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
        conduit::relay::mpi::io::blueprint::save_mesh(m_chunks, m_stem, m_output_type, m_options, writeContext.GetCommunicator());
#else
        conduit::relay::io::blueprint::save_mesh(m_chunks, m_stem, m_output_type, m_options);
#endif
        m_chunks.reset();
    }
    else if(m_op == BP_MESH_OP_FLATTEN_CSV || m_op == BP_MESH_OP_FLATTEN_HDF5)
    {
        debug5 << "Flatten options:\n" << m_special_options.to_string() << std::endl;
        conduit::Node table;
        int rank = 0;
        int root = 0;
#ifdef PARALLEL
        rank = writeContext.Rank();
        BP_PLUGIN_INFO("BlueprintMeshWriter: rank " << rank << " flattening.");
        // It's okay to pass empty nodes to this.
        conduit::blueprint::mpi::mesh::flatten(m_chunks, m_special_options, table,
            writeContext.GetCommunicator());
#else
        conduit::blueprint::mesh::flatten(m_chunks, m_special_options, table);
#endif
        // Don't need the mesh anymore.
        m_chunks.reset();

        const std::string filename = m_op == BP_MESH_OP_FLATTEN_CSV ? m_stem + ".csv"
                                                                    : m_stem + ".hdf5";
        if (rank == root)
        {
#ifdef PARALLEL
            BP_PLUGIN_INFO("I'm rank " << rank << " and I'm about to write " << filename << ".");
#endif
            conduit::relay::io::save(table, filename);
        }
    }
    else if(m_op == BP_MESH_OP_PARTITION)
    {
        debug5 << "Partition options:\n" << m_special_options.to_string() << std::endl;
        int rank = 0;
        const int root = 0;
        Node selections, repart_mesh;
        // If the user has not given their own selections then
        //  filter out the ghost nodes/zones
        if(!m_special_options.has_child("selections"))
        {
            BuildSelections(m_chunks, selections);
            if(!selections.dtype().is_empty())
            {
                // Make sure selections stays alive for the partition call!
                m_special_options["selections"].set_external(selections);
            }
        }

#ifdef PARALLEL
        rank = writeContext.Rank();
        BP_PLUGIN_INFO("BlueprintMeshWriter: rank " << rank << " partitioning.");
        conduit::blueprint::mpi::mesh::partition(m_chunks, m_special_options, repart_mesh,
            writeContext.GetCommunicator());
#else
        conduit::blueprint::mesh::partition(m_chunks, m_special_options, repart_mesh);
#endif
        // Don't need the original data anymore
        m_chunks.reset();

        if(!repart_mesh.dtype().is_empty())
        {
            debug5 << "Relay I/O Blueprint options:\n" << m_options.to_string() << std::endl;
#ifdef PARALLEL
            conduit::relay::mpi::io::blueprint::save_mesh(repart_mesh, m_stem, m_output_type, m_options, writeContext.GetCommunicator());
#else
            conduit::relay::io::blueprint::save_mesh(repart_mesh, m_stem, m_output_type, m_options);
#endif
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
// 
//  Justin Privitera, Tue Jul  9 10:47:29 PDT 2024
//  Removed everything from this function.
// ****************************************************************************

void
avtBlueprintWriter::WriteRootFile()
{
    // root file has already been written
}
