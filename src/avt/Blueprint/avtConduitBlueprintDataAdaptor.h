// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_CONDUIT_BLUEPRINT_DATA_ADAPTOR_H
#define AVT_CONDUIT_BLUEPRINT_DATA_ADAPTOR_H
#include <avtblueprint_exports.h>
#include <conduit.hpp>

//-----------------------------------------------------------------------------
// mfem includes
//-----------------------------------------------------------------------------
#include <mfem.hpp>

//-----------------------------------------------------------------------------
// vtk forward decls
//-----------------------------------------------------------------------------
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtConduitBlueprintDataAdaptor
//
//  Purpose:
//    Helper methods that convert between Conduit Blueprint and VTK meshes.
//
//  Programmer:  Cyrus Harrison
//  Creation:    May 31, 2022
//
//  Modifications to Blueprint Data Adaptor (now deprecated and moved here):
//      Matt Larsen, Feb 15, 2019 -- adding conversions from vtk to bp
//
//      Justin Privitera, Wed Mar 23 12:26:31 PDT 2022
//      Added "domain" as first arg of MeshToVTK.
// 
//  Modifications:
//      Justin Privitera, Mon Aug 22 17:15:06 PDT 2022
//      Added all functions, mostly taken from the blueprint plugin data 
//      adaptor.
//
//      Cyrus Harrison, Thu Dec 15 14:43:47 PST 2022
//      Added topo arg to FieldToVTK.
//
// ****************************************************************************

class AVTBLUEPRINT_API avtConduitBlueprintDataAdaptor
{
public:
    static void Initialize();

    // set warning and info handlers to redirect conduit warnings and info
    static void SetInfoWarningHandlers();

    static vtkDataArray * ConduitArrayToVTKDataArray(const conduit::Node &n,
                                                     int src_idxs_length = 0,
                                                     int *src_idxs = NULL);

    static vtkDataArray * ConduitArrayToStairStepVTKDataArray(const conduit::Node &n,
                                                              bool doing_x);

    class AVTBLUEPRINT_API BlueprintToVTK
    {
    public:
        /// Helpers for converting Mesh and Field Blueprint conforming data
        /// to vtk instances.
        static vtkDataSet*    MeshToVTK(int domain, 
                                        const conduit::Node &mesh);

        /// for structured grids, we may have fields that provide
        /// optional offsets and strides relative to the topology,
        /// so topology rides along to help with those cases
        static vtkDataArray*  FieldToVTK(const conduit::Node &topo,
                                         const conduit::Node &field);
    };

    class AVTBLUEPRINT_API VTKToBlueprint
    {
    public:
        /// Helpers for converting vtk datasets to Mesh and Field Blueprint
        /// conforming data
        static void VTKFieldNameToBlueprint(const std::string &vtk_name,
                                            const std::string &topo_name,
                                            std::string &bp_name);

        static void VTKFieldsToBlueprint(conduit::Node &node,
                                         const std::string topo_name,
                                         vtkDataSet* dataset);

        static void VTKToBlueprintMesh(conduit::Node &mesh,
                                       vtkDataSet* dataset,
                                       const int ndims);
    };

    class AVTBLUEPRINT_API BlueprintToMFEM
    {
    public:
        static mfem::Mesh         *MeshToMFEM(const conduit::Node &mesh,
                                              const std::string &topo_name = "");

        static mfem::GridFunction *FieldToMFEM(mfem::Mesh *mesh,
                                               const conduit::Node &field);
    };
};

#endif
