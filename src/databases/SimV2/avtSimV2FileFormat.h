// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtSimV2FileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SIMV2_FILE_FORMAT_H
#define AVT_SIMV2_FILE_FORMAT_H

#include <database_exports.h>

#include <avtSTMDFileFormat.h>
#include <avtSimulationInformation.h>
#include <avtMaterial.h>

#include <vector>
#include <set>
#include <string>

class avtMixedVariable;
class avtSpecies;

// ****************************************************************************
//  Class: avtSimV2FileFormat
//
//  Purpose:
//      Reads in a .sim2 file for VisIt in the MDServer.
//      Reads in simulation data as input to VisIt in the Engine.
//
//  Notes: This reader is based on SimV1 but is extended to support additional
//         data in the SimV2 structures.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Feb  5 11:59:19 PST 2009
//
//  Modifications:
//    Brad Whitlock, Wed Jul 18 11:53:50 PDT 2012
//    I added ExpandVariable.
//
// ****************************************************************************

class avtSimV2FileFormat : public avtSTMDFileFormat
{
  public:
                       avtSimV2FileFormat(const char *);
    virtual           ~avtSimV2FileFormat() {;};

    virtual const char    *GetType(void)   { return "SimV2"; };
    virtual void           FreeUpResources(void); 
    virtual int            GetCycle() { return -1; }

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);
    virtual avtMaterial   *GetMaterial(int, const char *);
    virtual vtkDataSet    *GetCurve(const char *);

    virtual void          *GetAuxiliaryData(const char *var, int domain,
                                            const char *type, void *,
                                            DestructorFunction &df);

    virtual bool           PopulateIOInformation(const std::string &meshname,
                                                 avtIOInformation& ioInfo);

  protected:
    avtSpecies            *GetSpecies(int, const char *);

    vtkDataArray          *ExpandVariable(vtkDataArray *array, avtMixedVariable **mv, 
                                          int nMixVarComponents, int domain, 
                                          const std::string &varname, 
                                          const intVector &restrictToMats);
    avtCentering           GetCentering(const std::string &varname) const;
    intVector              GetRestrictedMaterialIndices(const std::string &varname) const;

    avtSimulationInformation simInfo;
    std::set<std::string>    curveMeshes;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
