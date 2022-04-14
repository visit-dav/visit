// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
// avtBlueprintDataAdaptor.h
// ************************************************************************* //

#ifndef AVT_BLUEPRINT_DATA_ADAPTOR_H
#define AVT_BLUEPRINT_DATA_ADAPTOR_H

//-----------------------------------------------------------------------------
// conduit includes
//-----------------------------------------------------------------------------
#include "conduit.hpp"

//-----------------------------------------------------------------------------
// mfem includes
//-----------------------------------------------------------------------------
#include "mfem.hpp"

//-----------------------------------------------------------------------------
// vtk forward decls
//-----------------------------------------------------------------------------
class vtkDataSet;
class vtkDataArray;

//-----------------------------------------------------------------------------
// avt forward decls
//-----------------------------------------------------------------------------
class avtMaterial;

//-----------------------------------------------------------------------------
//  Class: avtBlueprintDataAdaptor
//
//  Purpose:
//
//  Programmer: Cyrus Harrison
//  Creation:   Tue Dec 27 14:14:40 PST 2016
//  Modifications:
//      Matt Larsen, Feb 15, 2019 -- adding conversions from vtk to bp
//
//      Justin Privitera, Wed Mar 23 12:26:31 PDT 2022
//      Added "domain" as first arg of MeshToVTK.
// 
//      Justin Privitera, Mon Apr 11 18:20:19 PDT 2022
//      Added "new_refine" as an arg to RefineMeshToVTK.
// 
//      Justin Privitera, Wed Apr 13 13:49:43 PDT 2022
//      Added LegacyRefineMeshToVTK and LowOrderMeshToVTK to MFEM class.
//
//-----------------------------------------------------------------------------
class avtBlueprintDataAdaptor
{
public:

  /// Helpers for converting Mesh and Field Blueprint conforming data
  /// to vtk instances.
  class VTK
  {
    public:
      static vtkDataSet*    MeshToVTK(int domain, 
                                      const conduit::Node &mesh);
      static vtkDataArray*  FieldToVTK(const conduit::Node &field);

  };

  /// Helpers for converting Blueprint Material info into avt Objects
  class AVT
  {
    public:
      static avtMaterial *MatsetToMaterial(const conduit::Node &matset);
      // static avtMixedVar *MatsetVarToMixedVar(const conduit::Node &matset_var);
  };

  /// Helpers for converting vtk datasets to Mesh and Field Blueprint
  /// conforming data
  class BP
  {
    public:
      static void VTKFieldNameToBlueprint(const std::string &vtk_name,
                                          const std::string &topo_name,
                                          std::string &bp_name);

      static void VTKFieldsToBlueprint(conduit::Node &node,
                                       const std::string topo_name,
                                       vtkDataSet* dataset);

      static void VTKToBlueprint(conduit::Node &mesh,
                                 vtkDataSet* dataset,
                                 const int ndims);
  };
  /// Helpers for converting Mesh and Field Blueprint conforming data
  /// to mfem instances + helpers for refining mfem data to vtk.
  class MFEM
  {
    public:
    //-------------------------------------------------------------------------
    // blueprint to mfem
    //-------------------------------------------------------------------------
    static mfem::Mesh         *MeshToMFEM(const conduit::Node &mesh,
                                          const std::string &topo_name = "");

    static mfem::GridFunction *FieldToMFEM(mfem::Mesh *mesh,
                                           const conduit::Node &field);
    //-------------------------------------------------------------------------
    // mfem to vtk
    //-------------------------------------------------------------------------
    static vtkDataSet   *LegacyRefineMeshToVTK(mfem::Mesh *mesh,
                                               int lod);

    static vtkDataSet   *LowOrderMeshToVTK(mfem::Mesh *mesh);

    static vtkDataSet   *RefineMeshToVTK(mfem::Mesh *mesh,
                                         int lod,
                                         bool new_refine);

    static vtkDataArray *RefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                     mfem::GridFunction *gf,
                                                     int lod);

    static vtkDataArray *RefineElementColoringToVTK(mfem::Mesh *mesh,
                                                    int domain_id,
                                                    int lod);

    static vtkDataArray *RefineElementAttributeToVTK(mfem::Mesh *mesh,
                                                     int lod);
  };
};


#endif
