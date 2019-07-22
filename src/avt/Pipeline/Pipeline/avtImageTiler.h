// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_IMAGE_TILER_H
#define AVT_IMAGE_TILER_H

#include <avtImage.h>

// ****************************************************************************
// Class: avtImageTiler
//
// Purpose:
//   Creates a new tiled image out of several avtImage objects and destroys
//   the input images.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 15 15:18:17 PST 2004
//
// Modifications:
//   
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtImageTiler
{
  public:
    avtImageTiler(int nImg);
    virtual ~avtImageTiler();

    void AddImage(avtImage_p img);
    int GetNumberOfColumnsForNTiles(int nTiles) const;
    int GetNumberOfRowsForNTiles(int nTiles) const;

    avtImage_p CreateTiledImage();

  private:
    avtImage_p *images;
    int        nImages;
    int        maxImages;

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtImageTiler(const avtImageTiler &) {;};
    avtImageTiler       &operator=(const avtImageTiler &) { return *this; };
};

#endif
