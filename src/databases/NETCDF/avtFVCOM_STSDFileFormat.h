// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFVCOM_STSDFileFormat.h                      //
// ************************************************************************* //

#ifndef AVT_FVCOM_STSD_FILE_FORMAT_H
#define AVT_FVCOM_STSD_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <avtSTSDFileFormatInterface.h>

#include <vectortypes.h>

class NETCDFFileObject;
class avtFVCOMReader;

// ****************************************************************************
//  Class: avtFVCOM_STSDFileFormat
//
//  Purpose:
//      Reads in FVCOM_STSD files as a plugin to VisIt.
//
//  Programmer: David Stuebe
//  Creation:   Wed May 31 15:50:45 PST 2006
//
// ****************************************************************************

class avtFVCOM_STSDFileFormat : public avtSTSDFileFormat
{
public:
    static bool        Identify(NETCDFFileObject *);
    static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f,
                         const char *const *list, int nList, int nBlock);

                       avtFVCOM_STSDFileFormat(const char *, NETCDFFileObject *);
                       avtFVCOM_STSDFileFormat(const char *);
    virtual           ~avtFVCOM_STSDFileFormat();

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    virtual int          GetCycle(void);
    virtual double       GetTime(void);
    //

    virtual const char    *GetType(void)   { return "FVCOM_STSD"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    virtual void          *GetAuxiliaryData(const char *var,
                                            const char *type,
                                            void *args,
                                            DestructorFunction &);

protected:
    // DATA MEMBERS

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    avtFVCOMReader        *reader;
};


#endif
