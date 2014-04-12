/*****************************************************************************
* Copyright (c) 2014 Intelligent Light. All rights reserved.
* Work partially supported by DOE Grant SC0007548.
* FieldView XDB Export is provided expressly for use within VisIt.
* All other uses are strictly forbidden.
*****************************************************************************/
#ifndef AVT_XDB_FILE_FORMAT_H
#define AVT_XDB_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>

// ****************************************************************************
//  Class: avtXDBFileFormat
//
//  Purpose:
//      Reads in XDB files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan 14 17:29:14 PST 2014
//
// ****************************************************************************

class avtXDBFileFormat : public avtMTMDFileFormat
{
public:
                       avtXDBFileFormat(const char *);
    virtual           ~avtXDBFileFormat();

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "XDB"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
