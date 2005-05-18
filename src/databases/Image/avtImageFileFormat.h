
// ************************************************************************* //
//                            avtImageFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Image_FILE_FORMAT_H
#define AVT_Image_FILE_FORMAT_H

#include <string>
#include <vector>
#include <database_exports.h>
#include <avtDataSelection.h>
#include <avtSTSDFileFormat.h>
#include <vtkImageData.h>


// ****************************************************************************
//  Class: avtImageFileFormat
//
//  Purpose:
//      Reads in Image files as a plugin to VisIt.
// 
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//    Mark C. Miller, Thu Nov  4 17:00:40 PST 2004
//    Added support for data selections. Eliminated xdim/ydim data members
//    since they are known from vtkImageData object
//
//    Mark C. Miller, Tue Nov  9 13:41:33 PST 2004
//    Removed unnused pointvarnames, pointvars. Added fext and CanCacheVariable
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Deleted a bunch of commented-out code that was hold-over from the
//    plugin generation processes.
//
// ****************************************************************************

class avtImageFileFormat : public avtSTSDFileFormat
{
  public:
                       avtImageFileFormat(const char *filename);
    virtual           ~avtImageFileFormat();

    virtual const char    *GetType(void)   { return "Image"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    virtual bool           CanCacheVariable(const char *);

    virtual void           RegisterDataSelections(
                               const std::vector<avtDataSelection_p> &selList,
                               std::vector<bool> *selectionsApplied);

  protected:

    std::string                          fname;
    std::string                          fext;
    std::vector<std::vector<float> >     cellvars; 
    std::vector<std::string>             cellvarnames;
    vtkImageData                         *image;
    std::vector<avtDataSelection_p>      selList;
    std::vector<bool>                    *selsApplied;
    bool                                 haveReadWholeImage;
    void                                 ReadInImage(void);
    bool                                 ProcessDataSelections(
                                             int *xmin, int *xmax,
                                             int *ymin, int *ymax);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
