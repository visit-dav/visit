// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtANSYSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_ANSYS_FILE_FORMAT_H
#define AVT_ANSYS_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


// ****************************************************************************
//  Class: avtANSYSFileFormat
//
//  Purpose:
//      Reads in ANSYS files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
//  Modifications:
//
//    Mark C. Miller, 12Apr17
//    Adjust interface to Interpret methods to support more flexibility in
//    format string.
// ****************************************************************************

class avtANSYSFileFormat : public avtSTSDFileFormat
{
public:
                       avtANSYSFileFormat(const char *filename);
    virtual           ~avtANSYSFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                  void *args, DestructorFunction &);
    //

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    // virtual int       GetCycle(void);
    // virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "ANSYS"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    bool ReadFile(const char *, int nLines);
    void InterpretFormatString(char *line, int numFields, int &firstFieldWidth,
             int &fieldStart, int &fieldWidth, int &expectedLineLength) const;
    void Interpret(const char *fmt, bool isstd, int &numFields,
             int &fieldWidth, int &linelen) const;

    // DATA MEMBERS
    vtkDataSet            *meshDS;
    std::string            title;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif

