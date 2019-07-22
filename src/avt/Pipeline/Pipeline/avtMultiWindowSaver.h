// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_MULTI_WINDOW_SAVER_H
#define AVT_MULTI_WINDOW_SAVER_H

#include <avtImage.h>

#include <SaveSubWindowsAttributes.h>


// ****************************************************************************
// Class: avtMultiWindowSaver
//
// Purpose:
//   Creates a new image out of several avtImage objects and destroys
//   the input images.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   July 16, 2010
//
// Modifications:
//
// ****************************************************************************

class PIPELINE_API avtMultiWindowSaver
{
  public:
    avtMultiWindowSaver(const SaveSubWindowsAttributes &);
    virtual ~avtMultiWindowSaver();

    void AddImage(avtImage_p img, int id);
    void SetImageSize(int w, int h) { width = w; height = h; };

    avtImage_p CreateImage();

  private:
    avtImage_p images[16];
    int        width, height;
    SaveSubWindowsAttributes atts;

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtMultiWindowSaver(const avtMultiWindowSaver &) {;};
    avtMultiWindowSaver       &operator=(const avtMultiWindowSaver &) { return *this; };
};

#endif
