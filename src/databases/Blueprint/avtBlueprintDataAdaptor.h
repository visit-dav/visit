// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
// avtBlueprintDataAdaptor.h
// ************************************************************************* //

#ifndef AVT_BLUEPRINT_DATA_ADAPTOR_H
#define AVT_BLUEPRINT_DATA_ADAPTOR_H

// NOTE: This is from avtblueprint lib
#include "avtConduitBlueprintDataAdaptor.h"
// NOTE: This is from avtmfem lib
#include "avtMFEMDataAdaptor.h"

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
//      Justin Privitera, Mon May  9 14:35:18 PDT 2022
//      Added LegacyRefineGridFunctionToVTK and LowOrderGridFunctionToVTK to 
//      MFEM class and added "new_refine" as an arg to RefineGridFunctionToVTK.
//
//-----------------------------------------------------------------------------
class avtBlueprintDataAdaptor
{
public:
  /// Helpers for converting Blueprint Material info into avt Objects
  class AVT
  {
    public:
      static avtMaterial *MatsetToMaterial(const conduit::Node &matset);
      // static avtMixedVar *MatsetVarToMixedVar(const conduit::Node &matset_var);
  };
};


#endif
