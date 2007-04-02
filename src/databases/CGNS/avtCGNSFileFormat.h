// ************************************************************************* //
//                            avtCGNSFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_CGNS_FILE_FORMAT_H
#define AVT_CGNS_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <vectortypes.h>
#include <map>

// ****************************************************************************
//  Class: avtCGNSFileFormat
//
//  Purpose:
//      Reads in CGNS files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
// ****************************************************************************

class avtCGNSFileFormat : public avtMTMDFileFormat
{
  public:
                       avtCGNSFileFormat(const char *);
    virtual           ~avtCGNSFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                     int timestep, int domain,void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    // virtual void        GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);
    
    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CGNS"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    int                    GetFileHandle();
    void                   ReadTimes();
    bool                   GetCoords(int base, int zone, const int *zsize,
                                     bool structured, float **coords,
                                     int *ncoords);

    vtkDataSet *           GetCurvilinearMesh(int, int, const char *,
                                              const int *);
    vtkDataSet *           GetUnstructuredMesh(int, int, const char *,
                                               const int *);

    int                              fn;
    bool                             timesRead;
    doubleVector                     times;
    std::map<std::string, intVector> MeshDomainMapping;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
