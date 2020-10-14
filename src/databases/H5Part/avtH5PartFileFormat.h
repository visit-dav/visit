// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtH5PartFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_H5PART_FILE_FORMAT_H
#define AVT_H5PART_FILE_FORMAT_H

#include <visit-config.h>

#include <avtMTSDFileFormat.h>

// H5Part
#include <H5Part.h>
#include <H5Block.h>
#include <H5BlockTypes.h>

// STL
#include <vector>
#include <map>
#include <string>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtH5PartFileFormat
//
//  Purpose:
//      Reads in H5Part files as a plugin to VisIt.
//
//  Programmer: ghweber -- generated by xml2avt
//  Creation:   Tue Feb 9 13:44:50 PST 2010
//
//  Modifications:
//    Kurt Stockinger, Tue Aug 28 17:35:50 PDT 2007
//    Added support for field data
//
//    Gunther H. Weber, Fri Apr 17 13:03:47 PDT 2009
//    Added option to reject file if FastBit index is present.
//
//    Gunther H. Weber, Tue Nov 10 19:48:28 PST 2009
//    Removed unused data members.
//
//    Gunther H. Weber, Tue Feb  9 17:16:20 PST 2010
//    Complete restructuring. Recreated plugin as MTSD that supports
//    domain decomposition and rewrote major portions using original
//    pieces. Merged with HDF_UC plugin.
//
//    Eddie Rusu, Wed Oct 14 15:52:13 PDT 2020
//    Removed FastBit and FastQuery
//
// ****************************************************************************

class avtH5PartFileFormat : public avtMTSDFileFormat
{
  public:
                       avtH5PartFileFormat(const char *, DBOptionsAttributes *);
    virtual           ~avtH5PartFileFormat();

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    // virtual void        GetCycles(std::vector<int> &);
    // virtual void        GetTimes(std::vector<double> &);
    //

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void) { return "H5Part"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual void           ActivateTimestep(int ts);

  protected:
    // DATA MEMBERS
    // ... Constants
    static const int       maxVarNameLen = 256; // Maximum variable name length used in H5Part calls

    // ... Reader options
    bool                   enableDomainDecomposition;

    std::string            variablePathPrefix;
    
// ... File information
    H5PartFile            *file;
    enum { cartesianCoordSystem, cylindricalCoordSystem, sphericalCoordSystem }
                           coordType;
    int                    particleNSpatialDims;
    typedef std::map<std::string, h5part_int64_t>
                           VarNameToInt64Map_t;
    VarNameToInt64Map_t    particleVarNameToTypeMap;
    VarNameToInt64Map_t    fieldScalarVarNameToTypeMap;
    VarNameToInt64Map_t    fieldVectorVarNameToTypeMap;
    VarNameToInt64Map_t    fieldVectorVarNameToFieldRankMap;
    h5part_int64_t         numTimestepsInFile;
    h5part_int64_t         activeTimeStep;

    // The name of the current variable which contains the particle id
    std::string            idVariableName;
    // The name of the default variable which contains the particle id
    std::string            defaultIdVariableName;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    virtual void           GetCycles(std::vector<int> &c);
    virtual void           GetTimes(std::vector<double> &t);

  private:
    void                   SelectParticlesToRead( const char * = 0 );
    vtkDataSet            *GetParticleMesh(int);
    vtkDataSet            *GetFieldMesh(int, const char *);
    vtkDataArray          *GetFieldVar(int, const char*);
    void                   GetSubBlock(h5part_int64_t gridDims[3], h5part_int64_t subBlockDims[6]);
    std::string            DoubleToString(double x);

    std::vector<int>       cycles;
    std::vector<double>    times;
  
    void GetDecomp( h5part_int64_t nObjects,
                    h5part_int64_t &firstIndex,
                    h5part_int64_t &lastIndex,
                    h5part_int64_t &total );

    const std::string getVariablePathPrefix( int timestep );
};

#endif
