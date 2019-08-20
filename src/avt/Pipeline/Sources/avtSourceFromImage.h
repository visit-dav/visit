// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtSourceFromImage.h                           //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_IMAGE_H
#define AVT_SOURCE_FROM_IMAGE_H
#include <pipeline_exports.h>


#include <avtOriginatingImageSource.h>


class   vtkImageData;


// ****************************************************************************
//  Class: avtSourceFromImage
//
//  Purpose:
//      A source object (pipeline terminator) that is created from a vtk 
//      image data.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 08:09:04 PDT 2001
//    Changed method names to match base class refactor.
//
// ****************************************************************************

class PIPELINE_API avtSourceFromImage : public avtOriginatingImageSource
{
  public:
                           avtSourceFromImage(vtkImageData * = NULL,
                                              float * = NULL);
    virtual               ~avtSourceFromImage();

    void                   SetImage(vtkImageData *, float * = NULL);

  protected:
    vtkImageData          *image;
    float                 *zbuffer;

    virtual bool           FetchImage(avtDataRequest_p,
                                      avtImageRepresentation &);
};


#endif


