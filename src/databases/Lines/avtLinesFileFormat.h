// ************************************************************************* //
//                            avtLinesFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_LINES_FILE_FORMAT_H
#define AVT_LINES_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>


class     vtkPolyData;


// ****************************************************************************
//  Class: avtLinesFileFormat
//
//  Purpose:
//      A file format reader for lines.
//
//  Programmer: Hank Childs
//  Creation:   August 22, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

class avtLinesFileFormat : public avtSTMDFileFormat
{
  public:
                          avtLinesFileFormat(const char *);
    virtual              ~avtLinesFileFormat();
    
    virtual const char   *GetType(void) { return "Lines File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    std::string           filename;
    bool                  readInFile;

    std::vector<vtkPolyData *> lines;
    std::vector<std::string>   lineNames;

    void                  ReadFile(void);
    bool                  GetPoint(ifstream &, float &, float &, float &,
                                   std::string &);
};


#endif


