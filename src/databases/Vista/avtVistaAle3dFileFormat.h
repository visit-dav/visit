// ************************************************************************* //
//                            avtVistaAle3dFileFormat.h                      //
// ************************************************************************* //

#ifndef AVT_VistaAle3d_FILE_FORMAT_H
#define AVT_VistaAle3d_FILE_FORMAT_H

#include <database_exports.h>

#include <avtMaterial.h>
#include <avtVistaFileFormat.h>

#include <string>
#include <vector>

class vtkFloatArray;

using std::string;
using std::vector;

// ****************************************************************************
//  Class: avtVistaAle3dFileFormat
//
//  Purpose:
//      Reads in Vista files as a plugin to VisIt. 
//
//      Note that a Vista file can be written by either Silo or HDF5 natively.
//      We support both here. Where there is a choice in API data-types, we
//      favor use of HDF5's data-types.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 17, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Thu Apr 29 12:14:37 PDT 2004
//    Added data members to remember material names/numbers
//    Added GetMaterial method
//    Added GetAuxiliaryData method
//    Added GetFileNameForRead method
//
//    Eric Brugger, Wed May 12 13:42:01 PDT 2004
//    Prefixed some uses of "vector" with "std::".
//
//    Mark C. Miller, Wed May 19 10:56:11 PDT 2004
//    Added spatialDim data member
//
// ****************************************************************************

class avtVistaAle3dFileFormat : public avtVistaFileFormat
{

  public:
    static avtFileFormatInterface *CreateInterface(avtVistaFileFormat *vff,
                                       const char *const *filenames, int nList);

                               avtVistaAle3dFileFormat(const char *);
                               avtVistaAle3dFileFormat(const char *,
                                                       avtVistaFileFormat *vff);
    virtual                   ~avtVistaAle3dFileFormat();

    virtual void               FreeUpResources(void); 

    virtual void              *GetAuxiliaryData(const char *var, int,
                                                const char *type, void *args,
                                                DestructorFunction &);

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    virtual vtkDataArray      *GetVectorVar(int, const char *);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

  private:

    vtkFloatArray       *ReadVar(int domain, const char *visitName);

    avtMaterial         *GetMaterial(int, const char *);

    int                  numPieces;
    Node               **pieceNodes;

    int                  spatialDim;

    int                  numMaterials;
    std::vector<int>     materialNumbers;
    std::vector<string>  materialNames;
    int                 *materialNumbersArray;
    const char         **materialNamesArray;

};

#endif
