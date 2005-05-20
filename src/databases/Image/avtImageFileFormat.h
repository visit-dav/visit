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
//    Hank Childs, Fri Mar 18 11:41:04 PST 2005
//    Added support for image volumes.
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
    virtual void           ActivateTimestep(void);

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
    std::string                          image_fext;
    std::vector<std::vector<float> >     cellvars; 
    std::vector<std::string>             cellvarnames;
    vtkImageData                         *image;
    std::vector<avtDataSelection_p>      selList;
    std::vector<bool>                    *selsApplied;
    bool                                 haveInitialized;

    bool                                 haveImageVolume;
    std::vector<std::string>             subImages;
    float                                zStart;
    bool                                 specifiedZStart;
    float                                zStep;
    bool                                 specifiedZStep;

    bool                                 haveReadWholeImage;
    int                                  indexOfImageAlreadyRead;
    int                                  indexOfImageToRead;

    float                                xStart, yStart;
    float                                xStep, yStep;

    void                                 ReadInImage(void);
    void                                 ReadImageVolumeHeader(void);
    void                                 Initialize(void);
    bool                                 ProcessDataSelections(
                                             int *xmin, int *xmax,
                                             int *ymin, int *ymax);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    vtkDataSet                          *GetImageVolumeMesh(const char *);
    vtkDataSet                          *GetOneMesh(const char *);
    vtkDataArray                        *GetImageVolumeVar(const char *);
    vtkDataArray                        *GetOneVar(const char *);
};


#endif
