// ************************************************************************* //
//                            avtVistaFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Vista_FILE_FORMAT_H
#define AVT_Vista_FILE_FORMAT_H

#include <database_exports.h>

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
Node *VisitGetNode(Node *, const char *);
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
// ****************************************************************************

class avtVistaFileFormat : public avtSTMDFileFormat
{

  public:
                         avtVistaFileFormat(const char *);
                        ~avtVistaFileFormat();

      const char        *GetType(void)   { return "Vista"; };
      void               FreeUpResources(void); 

      vtkDataSet        *GetMesh(int, const char *);
      vtkDataArray      *GetVar(int, const char *);
      vtkDataArray      *GetVectorVar(int, const char *);

      void               PopulateDatabaseMetaData(avtDatabaseMetaData *);

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
          const Node    *GetNode(Node *root, const char *path)
                             { return VisitGetNode(root, path); };
          const void     FindNodes(Node *root, const char *path_re,
                             Node ***results, int *nmatches)
                             { VisitFindNodes(root, path_re, results, nmatches); };
          const Node    *GetTop() { return top; };

      private:
          Node          *top;
          char          *theVistaString;

    };

    vtkFloatArray       *ReadVar(int domain, const char *visitName);
    void                *OpenFile(const char *fileName);
    void                *OpenFile(int fid);
    void                 CloseFile(int fid);
    bool                 ReadDataset(const char *fileName, const char *dsPath,
                             hid_t *dataType, hsize_t *size, void **buf);

    static const int     MASTER_FILE_INDEX;
    string               masterFileName;
    string               masterDirName;

    int *domToFileMap;

    int numPieces;
    Node **pieceNodes;

    VistaTreeParser     *vTree;

    void                **fileHandles;

    static int           objcnt;
    bool                 isSilo;


};


#endif
