// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_MFEM_DATA_ADAPTOR_H
#define AVT_MFEM_DATA_ADAPTOR_H
#include <avtmfem_exports.h>
#include <mfem.hpp>

//-----------------------------------------------------------------------------
// vtk forward decls
//-----------------------------------------------------------------------------
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtMFEMDataAdaptor
//
//  Purpose:
//    Helper methods that refine MFEM meshes and grid functions 
//    to low-order VTK objects.
//
//  Programmer:  Cyrus Harrison
//  Creation:    May 31, 2022
//
//  Modifications to Blueprint Data Adaptor (now deprecated and moved here):
//      Justin Privitera, Mon Apr 11 18:20:19 PDT 2022
//      Added "new_refine" as an arg to RefineMeshToVTK.
// 
//      Justin Privitera, Wed Apr 13 13:49:43 PDT 2022
//      Added LegacyRefineMeshToVTK and LowOrderMeshToVTK to MFEM class.
// 
//      Justin Privitera, Mon May  9 14:35:18 PDT 2022
//      Added LegacyRefineGridFunctionToVTK and LowOrderGridFunctionToVTK to 
//      MFEM class and added "new_refine" as an arg to RefineGridFunctionToVTK.
// 
//  Modifications:
//    Justin Privitera, Tue Aug 23 14:52:34 PDT 2022
//    Added all functions, mostly taken from the blueprint plugin data adaptor.
//
//    Cyrus Harrison, Thu Mar  2 09:36:49 PST 2023
//    Added boundary mesh relater helpers
//
// ****************************************************************************

class AVTMFEM_API avtMFEMDataAdaptor
{
public:
      static vtkDataSet   *LegacyRefineMeshToVTK(mfem::Mesh *mesh,
                                                 int domain,
                                                 int lod);

      static vtkDataSet   *LowOrderMeshToVTK(mfem::Mesh *mesh);

      static vtkDataSet   *RefineMeshToVTK(mfem::Mesh *mesh,
                                           int domain,
                                           int lod,
                                           bool new_refine);

      static vtkDataSet   *BoundaryMeshToVTK(mfem::Mesh *mesh);

      static vtkDataArray *LegacyRefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                         mfem::GridFunction *gf,
                                                         int lod,
                                                         bool var_is_nodal);

      static vtkDataArray *LowOrderGridFunctionToVTK(mfem::GridFunction *gf);

      static vtkDataArray *RefineGridFunctionToVTK(mfem::Mesh *mesh,
                                                   mfem::GridFunction *gf,
                                                   int lod,
                                                   bool new_refine,
                                                   bool var_is_nodal = true);

      static vtkDataArray *RefineElementColoringToVTK(mfem::Mesh *mesh,
                                                      int domain_id,
                                                      int lod);

      static vtkDataArray *RefineElementAttributeToVTK(mfem::Mesh *mesh,
                                                       int lod);

      static vtkDataArray *BoundaryAttributeToVTK(mfem::Mesh *mesh);

};

#endif
