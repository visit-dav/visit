// ************************************************************************* //
//                            avtSimV1FileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SIMV1_FILE_FORMAT_H
#define AVT_SIMV1_FILE_FORMAT_H

#include <database_exports.h>

#include <avtSTMDFileFormat.h>
#include <VisItDataInterface_V1.h>
#include <avtSimulationInformation.h>
#include <avtMaterial.h>

#include <vector>
#include <set>
#include <string>

// ****************************************************************************
//  Class: avtSimV1FileFormat
//
//  Purpose:
//      Reads in a .sim1 file for VisIt in the MDServer.
//      Reads in simulation data as input to VisIt in the Engine.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 10, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Apr 14 16:47:07 PDT 2005
//    Added Curve and Material support.
//
// ****************************************************************************

class avtSimV1FileFormat : public avtSTMDFileFormat
{
  public:
                       avtSimV1FileFormat(const char *);
    virtual           ~avtSimV1FileFormat() {;};

    virtual const char    *GetType(void)   { return "SimV1"; };
    virtual void           FreeUpResources(void); 
    virtual int            GetCycle() { return -1; }

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);
    virtual avtMaterial   *GetMaterial(int, const char *);
    virtual vtkDataSet    *GetCurve(const char *);

    virtual void          *GetAuxiliaryData(const char *var, int domain,
                                            const char *type, void *,
                                            DestructorFunction &df);

  protected:
    avtSimulationInformation simInfo;
    VisIt_SimulationCallback cb;
    std::set<std::string>    curveMeshes;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
