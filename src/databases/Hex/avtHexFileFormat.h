// ************************************************************************* //
//                             avtHexFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_FOO_FILE_FORMAT_H
#define AVT_FOO_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>
#include <fstream.h>


class     vtkUnstructuredGrid;


// ****************************************************************************
//  Class: avtHexFileFormat
//
//  Purpose:
//      A file format reader for foo files.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 20, 2002
//
// ****************************************************************************

class avtHexFileFormat : public avtSTMDFileFormat
{
  public:
                          avtHexFileFormat(const char *);
    virtual              ~avtHexFileFormat();
    
    virtual const char   *GetType(void) { return "Hex File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    std::string                         filename;

    std::vector<float>                  points;
    std::vector<int>                    hexnodes;
    std::vector<std::vector<float> >    cellvars;
    std::vector<std::string>            cellvarnames;
    std::vector<std::vector<float> >    pointvars;
    std::vector<std::string>            pointvarnames;

    void                  ReadFile(ifstream &);
};


#endif
