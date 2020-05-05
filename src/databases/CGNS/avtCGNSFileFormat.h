// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCGNSFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_CGNS_FILE_FORMAT_H
#define AVT_CGNS_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <avtMTSDFileFormat.h>
#include <vector>
#include <vectortypes.h>

class avtCGNSFileReader;

// ****************************************************************************
//  Class: avtCGNSFileFormat
//
//  Purpose:
//      Reads in CGNS files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
//  Modifications:
//    Eric Brugger, Fri Feb 28 12:47:02 PST 2020
//    Modified the class to support both MTSD and MTMD variants and moved
//    all the CGNS reading code to avtCGNSFileReader.
//
// ****************************************************************************

class avtCGNS_MTMDFileFormat : public avtMTMDFileFormat
{
public:
                           avtCGNS_MTMDFileFormat(const char *);
    virtual               ~avtCGNS_MTMDFileFormat();

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CGNS"; };
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

protected:
    avtCGNSFileReader     *reader;
};

class avtCGNS_MTSDFileFormat : public avtMTSDFileFormat
{
public:
                           avtCGNS_MTSDFileFormat(const char *);
    virtual               ~avtCGNS_MTSDFileFormat();

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CGNS"; };
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

protected:
    char                  *cgnsFileName;

    avtCGNSFileReader     *reader;
};

#endif

