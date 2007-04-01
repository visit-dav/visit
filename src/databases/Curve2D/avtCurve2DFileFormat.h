// ************************************************************************* //
//                           avtCurve2DFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_CURVE2D_FILE_FORMAT_H
#define AVT_CURVE2D_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>


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
//  Modifications:
//
//    Hank Childs, Fri Aug  1 21:16:55 PDT 2003
//    Made the format be a STSD.
//
// ****************************************************************************

typedef enum
{
    VALID_POINT       = 0,
    HEADER,          /* 1 */
    WHITESPACE,      /* 2 */
    INVALID_POINT    /* 3 */
} CurveToken;


class avtCurve2DFileFormat : public avtSTSDFileFormat
{
  public:
                          avtCurve2DFileFormat(const char *);
    virtual              ~avtCurve2DFileFormat();
    
    virtual const char   *GetType(void) { return "Curve File Format"; };
    
    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

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


