// ************************************************************************* //
//                            avtVistaFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Vista_FILE_FORMAT_H
#define AVT_Vista_FILE_FORMAT_H

#include <string>

#include <VisitALE.h>
#include <avtSTMDFileFormat.h>
#include <avtFileFormatInterface.h>

using std::string;

// used in STL maps where we need to control initialization of
// value upon first reference into the map
template<class T, T ival=0>
class IMVal {
  public:
    T val;
    IMVal() : val(ival) {};
};

// ****************************************************************************
//  Class: avtVistaFileFormat
//
//  Purpose:
//      Base class for all Vista file formats. Includes functionality that is
//      NOT specific to any particular Vista file format
//
//      Note that a Vista file can be written by either Silo or HDF5 natively.
//      We support both here. Where there is a choice in API data-types, we
//      favor use of HDF5's data-types.
//
//  Programmer: Mark C. Miller 
//  Creation:   July 14, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Jul 21 11:19:15 PDT 2004
//    Totally re-organized to support multiple different Vista file formats
//    Moved most of Vista/Ale3d specific stuff to its respectife file format
//    file.
//
// ****************************************************************************

class avtVistaFileFormat : public avtSTMDFileFormat
{
    typedef enum
    {
        FTYPE_ALE3D,
        FTYPE_DIABLO,
        FTYPE_UNKNOWN
    } VistaFormatType;

    typedef enum
    {
        DTYPE_CHAR,
        DTYPE_INT,
        DTYPE_FLOAT,
        DTYPE_DOUBLE,
        DTYPE_UNKNOWN
    } VistaDataType;

  public:
    static avtFileFormatInterface *
                               CreateFileFormatInterface(
                                   const char * const *, int);

                               avtVistaFileFormat(const char *);
    virtual                   ~avtVistaFileFormat();

    virtual const char        *GetType(void)   { return "Vista"; };

    VistaFormatType            GetFormatType(void) const
                                   { return formatType; };
    const char *               GetWriterName(void) const
                                   { return writerName; };

    virtual void               FreeUpResources(void); 

    // satisfy avtSTMDFileFormat interface
    virtual vtkDataSet        *GetMesh(int, const char *) { return 0; };
    virtual vtkDataArray      *GetVar(int, const char *) { return 0; };
    virtual void               PopulateDatabaseMetaData(avtDatabaseMetaData *) {};

  protected:
                               avtVistaFileFormat(const char *,
                                                  avtVistaFileFormat *morphFrom);
    bool                 ReadDataset(const char *fileName, const char *dsPath,
                             VistaDataType *dataType, size_t *size, void **buf);
    void                 GetFileNameForRead(int domain, char *fileName, int size);


    class VistaTree
    {

      public:
                         VistaTree(const char *buf, size_t size);
                        ~VistaTree();
          const void     DumpTree() const;
          const Node    *GetNodeFromPath(const Node *root, const char *path) const;
          char          *GetPathFromNode(const Node *root, const Node *node) const;
          void           FindNodes(const Node *root, const char *re, Node ***results,
                                   int *nmatches, RecurseMode rmode) const;
          const Node    *GetTop() const;

      private:
          Node          *top;
          char          *theVistaString;
    };

    VistaTree     *vTree;

  private:

    // low-level I/O methods
    void                *OpenFile(const char *fileName);
    void                *OpenFile(int fid);
    void                 CloseFile(int fid);

    char                *writerName;
    VistaFormatType      formatType;

    static const int     MASTER_FILE_INDEX;
    string               masterFileName;
    string               masterDirName;

    int                  numChunks;
    int                 *chunkToFileMap;

    // we use void * here so we can use either HDF5 or Silo
    void                **fileHandles;

    static int           objcnt;
    bool                 isSilo;

    bool                 wasMorphed;

};


#endif
