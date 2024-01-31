// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  avtVTKFileFormat.h
// ************************************************************************* //

#ifndef AVT_VTK_FILE_FORMAT_H
#define AVT_VTK_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <avtVTKFileReader.h>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtVTK_STSDFileFormat
//
//  Purpose:
//      Reads in VTK_STSD files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 22 16:11:00 PST 2012
//
//  Modifications:
//    Eric Brugger, Tue Jun 20 13:43:04 PDT 2017
//    Modified the STSD databases so that they get the database metadata
//    from the first non empty database for a collection of STSD databases
//    that have been grouped into a multi data version using a visit file.
//
// ****************************************************************************

class avtVTK_STSDFileFormat : public avtSTSDFileFormat
{
public:
                       avtVTK_STSDFileFormat(const char *filename,
                                             const DBOptionsAttributes *);
                       avtVTK_STSDFileFormat(const char *filename,
                                             const DBOptionsAttributes *,
                                             avtVTKFileReader *r);
    virtual           ~avtVTK_STSDFileFormat();

    virtual const char    *GetType(void);
    virtual void           FreeUpResources(void);

    virtual int            GetCycleFromFilename(const char *f) const;
    virtual int            GetCycle(void);
    virtual double         GetTime(void);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    virtual void          *GetAuxiliaryData(const char *var,
                              const char *type, void *, DestructorFunction &df);

    virtual bool           IsEmpty();

protected:
    avtVTKFileReader *reader;

    mutable int cycleFromFilename;
};

#include <avtSTMDFileFormat.h>

// ****************************************************************************
//  Class: avtVTK_STMDFileFormat
//
//  Purpose:
//      Reads in VTK_STMD files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 22 16:11:00 PST 2012
//
// ****************************************************************************

class avtVTK_STMDFileFormat : public avtSTMDFileFormat
{
public:
                       avtVTK_STMDFileFormat(const char *filename,
                                             const DBOptionsAttributes *);
                       avtVTK_STMDFileFormat(const char *filename,
                                             const DBOptionsAttributes *,
                                             avtVTKFileReader *r);
    virtual           ~avtVTK_STMDFileFormat();

    virtual const char    *GetType(void);
    virtual void           FreeUpResources(void);

    virtual int            GetCycleFromFilename(const char *f) const;
    virtual int            GetCycle(void);
    virtual double         GetTime(void);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual vtkDataSet    *GetMesh(int domain, const char *);
    virtual vtkDataArray  *GetVar(int domain, const char *);
    virtual vtkDataArray  *GetVectorVar(int domain, const char *);

    virtual void          *GetAuxiliaryData(const char *var, int domain,
                              const char *type, void *, DestructorFunction &df);

protected:
    avtVTKFileReader *reader;

    mutable int cycleFromFilename;
};


class avtPVDFileReader;
#include <avtMTMDFileFormat.h>

// ****************************************************************************
//  Class: avtPVD_MTMDFileFormat
//
//  Purpose:
//      Reads in PVD files as a plugin to VisIt.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 3, 2021
//
// ****************************************************************************


class avtPVD_MTMDFileFormat : public avtMTMDFileFormat
{
  public:
                       avtPVD_MTMDFileFormat(const char *filename,
                                             const DBOptionsAttributes *);
    virtual           ~avtPVD_MTMDFileFormat();

    int                GetNTimesteps(void) override;
    void               GetTimes(std::vector<double> &) override;

    vtkDataSet        *GetMesh(int, int, const char *) override;
    vtkDataArray      *GetVar(int, int, const char *) override;
    vtkDataArray      *GetVectorVar(int, int, const char *) override;

    void *             GetAuxiliaryData(const char *, int, const char *,
                                        void *, DestructorFunction &);

    const char        *GetType(void)  override;
    void               FreeUpResources(void) override;

    void               ActivateTimestep(int ts) override;

  protected:
    void               PopulateDatabaseMetaData(avtDatabaseMetaData *, int) override;

    avtPVDFileReader *reader;
};


#endif
