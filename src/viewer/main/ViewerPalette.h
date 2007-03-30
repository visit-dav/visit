#ifndef VIEWER_PALETTE_H
#define VIEWER_PALETTE_H
#include <viewer_exports.h>

class ColorAttribute;

// ****************************************************************************
// Class: ViewerPalette
//
// Purpose:
//   Maintains a global list of colors that the viewer can use for plots, etc.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 12:50:07 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Mon Feb 26 15:49:31 PST 2001
//    Changed the default palette to an unsigned char array.
//   
// ****************************************************************************

class VIEWER_API ViewerPalette
{
public:
    ~ViewerPalette();
    static ViewerPalette *Instance();

    const ColorAttribute &Color(int i) const;
    void SetColor(int i, const ColorAttribute &color);
    int NumColors() const;
protected:
    ViewerPalette();
    ViewerPalette(const ViewerPalette &);
private:
    static ViewerPalette *instance;
    static unsigned char  defaultPalette[];

    ColorAttribute       *palette;
    int                  ncolors;
};

#endif
