// ************************************************************************* //
//                            avtVistaFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Vista_FILE_FORMAT_H
#define AVT_Vista_FILE_FORMAT_H

#include <database_exports.h>

#include <avtMaterial.h>
#include <avtSTMDFileFormat.h>

#include <string>

#include <silo.h>
#include <hdf5.h>

// imported definitions from Jeff Keasler's code
struct Node;
void  VisitParseInternal(char *, Node **);
void  ExtractAttr(Node *);
void  VisitFreeVistaInfo(Node *);
void  VisitDumpTree(Node *);
const Node *VisitGetNodeFromPath(const Node *, const char *);
char *VisitGetPathFromNode(const Node *);
void  VisitFindNodes(const Node *, const char *, Node ***, int *);

// used in STL maps where we need to control initialization of
// value upon first reference into the map
template<class T, T ival=0>
class IMVal {
  public:
    T val;
    IMVal() : val(ival) {};
};

class vtkFloatArray;

using std::string;

// ****************************************************************************
//  Class: avtVistaFileFormat
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
// ****************************************************************************

class avtVistaFileFormat : public avtSTMDFileFormat
{

  public:
                               avtVistaFileFormat(const char *);
    virtual                   ~avtVistaFileFormat();

    virtual const char        *GetType(void)   { return "Vista"; };
    virtual void               FreeUpResources(void); 

    virtual void              *GetAuxiliaryData(const char *var, int,
                                                const char *type, void *args,
                                                DestructorFunction &);

    virtual vtkDataSet        *GetMesh(int, const char *);
    virtual vtkDataArray      *GetVar(int, const char *);
    virtual vtkDataArray      *GetVectorVar(int, const char *);

    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

  private:

    class VistaTreeParser
    {

      public:

                         VistaTreeParser(const char *buf, size_t size)
                         {
                             theVistaString = new char[size];
                             memcpy(theVistaString, buf, size);
                             VisitParseInternal(theVistaString, &top);
                             ExtractAttr(top);
                         };

                        ~VistaTreeParser()
                         {
                             VisitFreeVistaInfo(top);
                             delete [] theVistaString;
                         };

          const void     DumpTree() { VisitDumpTree(top); };
          const Node    *GetNodeFromPath(const Node *root, const char *path)
                             { return VisitGetNodeFromPath(root, path); };
          char          *GetPathFromNode(const Node *root) const
                             { return VisitGetPathFromNode(root); };
          const void     FindNodes(const Node *root, const char *path_re,
                             Node ***results, int *nmatches) const
                             { VisitFindNodes(root, path_re, results, nmatches); };
          const Node    *GetTop() { return top; };

      private:
          Node          *top;
          char          *theVistaString;

    };

    // low-level I/O methods
    void                *OpenFile(const char *fileName);
    void                *OpenFile(int fid);
    void                 CloseFile(int fid);
    bool                 ReadDataset(const char *fileName, const char *dsPath,
                             hid_t *dataType, hsize_t *size, void **buf);
    vtkFloatArray       *ReadVar(int domain, const char *visitName);

    avtMaterial         *GetMaterial(int, const char *);

    void                 GetFileNameForRead(int domain, char *fileName, int size);

    static const int     MASTER_FILE_INDEX;
    string               masterFileName;
    string               masterDirName;

    int                 *domToFileMap;

    int                  numPieces;
    Node               **pieceNodes;

    int                  numMaterials;
    vector<int>          materialNumbers;
    vector<string>       materialNames;
    int                 *materialNumbersArray;
    const char         **materialNamesArray;

    VistaTreeParser     *vTree;

    void                **fileHandles;

    static int           objcnt;
    bool                 isSilo;


};


#endif
