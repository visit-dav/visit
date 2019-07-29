// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtTecplotBinaryFileFormat.h                   //
// ************************************************************************* //

#ifndef AVT_TECPLOTBINARY_FILE_FORMAT_H
#define AVT_TECPLOTBINARY_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>
#include <DBOptionsAttributes.h>

#include <string>
#include <vector>
#include <map>

class TecplotFile;
class vtkPoints;

// ****************************************************************************
//  Class: avtTecplotBinaryFileFormat
//
//  Purpose:
//      Reads in TecplotBinary files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Oct 25 12:37:42 EDT 2011
//    Allow user manual override of coordinate axis variables (via options).
//
// ****************************************************************************

class avtTecplotBinaryFileFormat : public avtSTMDFileFormat
{
public:
    avtTecplotBinaryFileFormat(const char *, DBOptionsAttributes *);
    virtual           ~avtTecplotBinaryFileFormat() {;};

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int domain,
    //                                     const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the cycle number, overload this function.
    // Otherwise, VisIt will make up a reasonable one for you.
    //
    // virtual int         GetCycle(void);
    virtual double         GetTime();

    virtual const char    *GetType(void)   { return "TecplotBinary"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    static bool MatchFormat(const char *filename);

protected:
    std::map<std::string, std::vector<int> > zoneNameToZoneList;
    std::map<std::string, std::string>       scalarToZone;
    void Initialize(TecplotFile *f,
                    avtDatabaseMetaData *md,
                    std::map<std::string, std::vector<int> > &zoneToZoneIds,
                    std::map<std::string, std::string> &s2z);

    TecplotFile *File();
    vtkPoints   *GetPoints(int zoneId, int ndims = 3);
    vtkDataSet  *GetCurvilinearMesh(int zoneId);
    vtkDataSet  *GetUnstructuredMesh(int zoneId);
    vtkDataSet  *GetPolyMesh(int zoneId);
    vtkDataSet  *GetCurve(int zoneId);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
private:
    TecplotFile *tec;

    bool userSpecifiedAxisVars;
    int userSpecifiedX;
    int userSpecifiedY;
    int userSpecifiedZ;

};


#endif
