// ************************************************************************* //
//                           avtCurve2DFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_CURVE2D_FILE_FORMAT_H
#define AVT_CURVE2D_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>
#include <fstream.h>


class     vtkPolyData;


// ****************************************************************************
//  Class: avtCurve2DFileFormat
//
//  Purpose:
//      A file format reader for curves.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
// ****************************************************************************

typedef enum
{
    VALID_POINT       = 0,
    HEADER,          /* 1 */
    WHITESPACE,      /* 2 */
    INVALID_POINT    /* 3 */
} CurveToken;


class avtCurve2DFileFormat : public avtSTMDFileFormat
{
  public:
                          avtCurve2DFileFormat(const char *);
    virtual              ~avtCurve2DFileFormat();
    
    virtual const char   *GetType(void) { return "Curve File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

  protected:
    std::string           filename;
    bool                  readFile;

    std::vector<vtkPolyData *> curves;
    std::vector<std::string>   curveNames;

    void                  ReadFile(void);
    CurveToken            GetPoint(ifstream &, float &, float &,
                                   std::string &);
};


#endif


