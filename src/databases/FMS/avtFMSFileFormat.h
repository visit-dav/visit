// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFMSFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_FMS_FILE_FORMAT_H
#define AVT_FMS_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>
#include <avtDataSelection.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <fms.h>
#include <mfem.hpp>

// ****************************************************************************
// Class: avtFMSFileFormat
//
// Purpose:
//   FMS database reader with LOD / refinement controls.
//
// Notes:    This plugin is derived from the MFEM reader.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 22 17:38:36 PDT 2020
//
// Modifications:
//
// ****************************************************************************

class avtFMSFileFormat : public avtSTMDFileFormat
{
public:
                       avtFMSFileFormat(const char *);
    virtual           ~avtFMSFileFormat();
    
    // VisIt can't cache for us b/c we need to implement LOD support. 
    virtual bool           CanCacheVariable(const char *var) {return false;}
    // Used to enable support for avtResolutionSelection
    virtual void        RegisterDataSelections(
                            const std::vector<avtDataSelection_p> &selList,
                            std::vector<bool> *selectionsApplied);


    virtual const char    *GetType(void)   { return "FMS"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual int            GetCycle();
    virtual double         GetTime();

    virtual void           ActivateTimestep(void);
protected:

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    bool                   HasInvariantMetaData(void) const { return false; };
    bool                   HasInvariantSIL(void) const      { return false; };
  
private:
    class Internals;
    Internals                       *d;
    int                             selectedLOD;
    static const char *MESH_NAME;
    static const char *BDR_MESH_NAME;
    static const char *DOF_MESH_NAME;
    static const char *ELEMENT_ATTRIBUTE_NAME;
    static const char *ELEMENT_COLORING_NAME;

    mfem::Mesh                     *FetchMesh(const std::string &mesh_name,
                                              int domain);
                                              
    vtkDataSet                      *GetRefinedMesh(const std::string &mesh_name,
                                                    int domain,
                                                    int lod);
    vtkDataSet                      *GetRefinedMesh(const std::string &mesh_name,
                                         int domain, int lod,
                                         int (*get_num_elements)(mfem::Mesh *),
                                         int (*get_element_base_geometry)(mfem::Mesh *, int),
                                         mfem::ElementTransformation *(*get_element_transformation)(mfem::Mesh *, int)
                                         );

    vtkDataSet                      *GetRefinedBoundaryMesh(int domain, int lod);
                                                    
    vtkDataArray                    *GetRefinedVar(const std::string &mesh_name,
                                                   int domain,
                                                   int lod);

    vtkDataSet                      *GetDOFMesh(int domain);
    vtkDataArray                    *GetDOFVar(int domain);

    vtkDataArray                    *GetRefinedElementColoring(const std::string &mesh_name, 
                                                                int domain, 
                                                                int lod);
                                                                
    vtkDataArray                    *GetRefinedElementAttribute(const std::string &mesh_name, 
                                                                int domain, 
                                                                int lod);
};

#endif
