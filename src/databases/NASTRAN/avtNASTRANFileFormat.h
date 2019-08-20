// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtNASTRANFileFormat.h                         //
// ************************************************************************* //

#ifndef AVT_NASTRAN_FILE_FORMAT_H
#define AVT_NASTRAN_FILE_FORMAT_H

#include <vector>
#include <map>
#include <fstream>
#include <vtkUnstructuredGrid.h>

#include <avtSTSDFileFormat.h>

class DBOptionsAttributes;
class avtMaterial;


// ****************************************************************************
//  Class: avtNASTRANFileFormat
//
//  Purpose:
//      Reads in NASTRAN files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 5 17:24:35 PST 2005
//
//  Modifications:
//
// ****************************************************************************

class avtNASTRANFileFormat : public avtSTSDFileFormat
{
public:
                       avtNASTRANFileFormat(const char *filename, DBOptionsAttributes*);
    virtual           ~avtNASTRANFileFormat();

    virtual void         *GetAuxiliaryData(const char *var,
                                           const char *type, void *args,
                                           DestructorFunction &);

    virtual const char    *GetType(void)   { return "NASTRAN bulk data"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    avtMaterial           *GetMaterial(const char *);
    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    bool ReadFile(const char *, int nLines);

    // DATA MEMBERS
    vtkDataSet            *meshDS;
    std::string            title;
    int                    matCountOpt;
    std::vector<int>       matList;
    std::map<int,int>      uniqMatIds;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    
private:
    bool ReadLine(ifstream&, char *);
    void ParseLine(vtkIdType *, char *, int, int);
    int CheckBuildable(const vtkIdType *, int);
    int CheckBuildable(const vtkIdType *, int, int);
    int ParseField(char *, int);

};


#endif
