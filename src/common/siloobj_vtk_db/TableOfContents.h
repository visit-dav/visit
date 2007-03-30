// ************************************************************************* //
//                           TableOfContents.h                               //
// ************************************************************************* //

#ifndef TABLE_OF_CONTENTS_H
#define TABLE_OF_CONTENTS_H
#include <siloobj_vtk_exports.h>

#include <silo.h>

#include <Field_VTK.h>
#include <IntervalTree_VTK.h>
#include <Value_VTK.h>


// ****************************************************************************
//  Class: TableOfContents
//
//  Purpose:
//      Reads in the meshtvprep SILO format from a set of files.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

class SILOOBJ_VTK_API TableOfContents
{
  public:
                                TableOfContents();
    virtual                    ~TableOfContents();

    virtual Field_VTK          *GetField(const char *) = 0;
    virtual IntervalTree_VTK   *GetIntervalTree(const char *) = 0;
    virtual Value_VTK          *GetValue(const char *) = 0;

    void                        Read(int, const char * const *);
    void                        PrintSelf();

    void                        CloseSILOFile();
    DBfile                     *GetSILOFile(const char *);

  protected:
    int                         nFiles;
    char                      **files;

    DBfile                     *openFile;
    char                       *openFileName;

    // Protected Methods
    virtual void                ReadFile(const char *) = 0;
    virtual void                UpdateReferences(void) = 0;
};


#endif


