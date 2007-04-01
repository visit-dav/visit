// ************************************************************************* //
//                            avtVistaDiabloFileFormat.h                     //
// ************************************************************************* //

#ifndef AVT_VistaDiablo_FILE_FORMAT_H
#define AVT_VistaDiablo_FILE_FORMAT_H

#include <database_exports.h>

#include <avtVistaFileFormat.h>

#include <string>
#include <vector>

class vtkDataArray;
class vtkDataSet;
class vtkFloatArray;

using std::string;
using std::vector;

// ****************************************************************************
//  Class: avtVistaDiabloFileFormat
//
//  Purpose:
//      Reads Vista files written by the Diablo code 
//
//  Programmer: Mark C. Miller 
//  Creation:   July 17, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct 27 16:02:46 PDT 2004
//    Added GetFileNameForRead
//
// ****************************************************************************

class avtVistaDiabloFileFormat : public avtVistaFileFormat
{

  public:
    static avtFileFormatInterface *CreateInterface(avtVistaFileFormat *vff,
                                       const char *const *filenames, int nList);

                               avtVistaDiabloFileFormat(const char *);
                               avtVistaDiabloFileFormat(const char *,
                                                       avtVistaFileFormat *vff);
    virtual                   ~avtVistaDiabloFileFormat();

    virtual void               FreeUpResources(void); 

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    virtual vtkDataArray      *GetVectorVar(int, const char *);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

  private:

    typedef struct _fieldInfo
    {
        string         visitVarName;
        string         meshName;
        avtVarType     varType;
        avtCentering   centering;
        vector<string> compNames;
    } FieldInfo_t;

    vtkFloatArray       *ReadVar(int domain, const char *visitName);

    void                 GetFileNameForRead(int dom, char *fileName, int size);

    int                  numPieces;
    Node               **pieceNodes;
    vector<vector<FieldInfo_t> > fieldInfos;

    int                  spatialDim;
};


#endif
