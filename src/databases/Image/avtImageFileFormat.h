
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
// ****************************************************************************

class avtImageFileFormat : public avtSTSDFileFormat
{
  public:
                       avtImageFileFormat(const char *filename);
    virtual           ~avtImageFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      &GetAuxiliaryData(const char *var, const char *type,
    //                                  void *args, DestructorFunction &);
    //

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    // virtual bool      ReturnsValidCycle() const { return true; };
    // virtual int       GetCycle(void);
    // virtual bool      ReturnsValidTime() const { return true; };
    // virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "Image"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    virtual void           RegisterDataSelections(
                               const std::vector<avtDataSelection_p> &selList,
                               std::vector<bool> *selectionsApplied);

  protected:
    // DATA MEMBERS
    std::string                          fname;
    std::vector<std::vector<float> >     cellvars; 
    std::vector<std::string>             cellvarnames;
    std::vector<std::vector<float> >     pointvars;
    std::vector<std::string>             pointvarnames;
    vtkImageData                         *image;
    std::vector<avtDataSelection_p>      selList;
    std::vector<bool>                    *selsApplied;
    bool                                 readInImage;
    void                                 ReadInImage(void);
    bool                                 ProcessDataSelections(
                                             int *xmin, int *xmax,
                                             int *ymin, int *ymax);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
