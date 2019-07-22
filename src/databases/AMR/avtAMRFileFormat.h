// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtAMRFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_AMR_FILE_FORMAT_H
#define AVT_AMR_FILE_FORMAT_H
#include <avtSTMDFileFormat.h>

#include <vector>

class AMRreaderInterface;

// ****************************************************************************
//  Class: avtAMRFileFormat
//
//  Purpose:
//      Reads in AMR files as a plugin to VisIt.
//
//  Programmer: hpan -- generated by xml2avt
//  Creation:   Mon Apr 26 10:26:32 PDT 2010
//
//  Modifications:
//    Brad Whitlock, Wed May 28 14:17:40 PDT 2014
//    Make the reader use AMRreaderInterface to read the AMR data. Also tell
//    VisIt that the metadata and SIL vary over time.
//
// ****************************************************************************

class avtAMRFileFormat : public avtSTMDFileFormat
{
public:
    avtAMRFileFormat(const char *);
    virtual           ~avtAMRFileFormat();

    // Tell VisIt that the SIL and Metadata change over time.
    bool                  HasInvariantMetaData(void) const
    {
        return false;
    };
    bool                  HasInvariantSIL(void) const
    {
        return false;
    };

    //
    // This is used to return unconventional data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void         *GetAuxiliaryData(const char *var, int domain,
                                           const char *type, void *args,
                                           DestructorFunction &);

    //
    // If you know the cycle number, overload this function.
    // Otherwise, VisIt will make up a reasonable one for you.
    virtual int           GetCycle(void);
    virtual double        GetTime(void);

    virtual const char   *GetType(void)
    {
        return "AMR";
    };
    virtual void          FreeUpResources(void);

    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

protected:
    AMRreaderInterface   *GetReader(bool heavyweightInit = true);
    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    static const std::string amr_name;
    static const std::string intf_name;
    static const std::string invalid_name;

    static std::string composeName( const std::string& m, const std::string& v, const char app='/' );
    static void decomposeName( const std::string& s, std::string& m, std::string& v );

private:
    // DATA MEMBERS
    bool                enableAMR;
    AMRreaderInterface *reader_;
};


#endif
