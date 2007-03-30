#include <ViewerPalette.h>
#include <ColorAttribute.h>

// Static members.
ViewerPalette *ViewerPalette::instance = 0;

// Default palette of 30 colors stored in ABGR format.
unsigned char ViewerPalette::defaultPalette[] = {
    0x00, 0x00, 0xff,
    0x00, 0xff, 0x00,
    0xff, 0x00, 0x00,
    0xff, 0xff, 0x00,
    0xff, 0x00, 0xff,
    0x00, 0xff, 0xff,
    0x00, 0x88, 0xff,
    0x88, 0x00, 0xff,
    0xa9, 0xa9, 0xa9,
    0x46, 0x46, 0xff,
    0x64, 0xff, 0x64,
    0xff, 0x64, 0x64,
    0xa5, 0xa5, 0x28,
    0xff, 0x64, 0xff,
    0x64, 0xff, 0xff,
    0x64, 0xaa, 0xff,
    0xff, 0x50, 0xaa,
    0x00, 0x00, 0x96,
    0x00, 0x96, 0x00,
    0x96, 0x00, 0x00,
    0x6d, 0x6e, 0x00,
    0x96, 0x00, 0x96,
    0x00, 0x96, 0x96,
    0x00, 0x55, 0x96,
    0x50, 0x00, 0xa0,
    0x1c, 0x68, 0xff,
    0x51, 0xac, 0x00,
    0x7d, 0xff, 0x44,
    0xff, 0x82, 0x00,
    0xff, 0x00, 0x82
};

// ****************************************************************************
// Method: ViewerPalette::ViewerPalette
//
// Purpose: 
//   Constructor for the ViewerPalette class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 13:08:06 PST 2000
//
// Modifications:
//    Jeremy Meredith, Mon Feb 26 15:49:02 PST 2001
//    Changed it to use an array of unsigned chars for platform independence.
//   
// ****************************************************************************

ViewerPalette::ViewerPalette()
{
    // Allocate ColorAttribute objects to hold the default palette.
    ncolors = 30;
    palette = new ColorAttribute[30];

    // Set all the colors using the default palette.
    unsigned char *cPalette = defaultPalette;
    for(int i = 0; i < ncolors; ++i)
    {
        palette[i].SetRgba(cPalette[2], cPalette[1], cPalette[0], 255);
        cPalette += 3;
    }
}

// ****************************************************************************
// Method: ViewerPalette::ViewerPalette(const ViewerPalette &)
//
// Purpose: 
//   Copy constructor for the ViewerPalette class.
//
// Note:       This method is disabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 13:08:27 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ViewerPalette::ViewerPalette(const ViewerPalette &)
{
    // empty copy constructor.
}

// ****************************************************************************
// Method: ViewerPalette::~ViewerPalette
//
// Purpose: 
//   Destructor for the ViewerPalette class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 13:09:11 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ViewerPalette::~ViewerPalette()
{
    delete [] palette;
    palette = 0;
}

// ****************************************************************************
// Method: ViewerPalette::Instance
//
// Purpose: 
//   Returns a pointer to the one and only instance of this class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 13:09:33 PST 2000
//
// Modifications:
//   
// ****************************************************************************

ViewerPalette *
ViewerPalette::Instance()
{
    if(instance == 0)
        instance = new ViewerPalette;

    return instance;
}

// ****************************************************************************
// Method: ViewerPalette::Color
//
// Purpose: 
//   Returns the color at the specified index.
//
// Arguments:
//   i : The index of the color to return.
//
// Returns:    The color at index i.
//
// Note:       If the index is mod'd by ncolors to give an index in the range
//             0..ncolors-1.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 13:09:59 PST 2000
//
// Modifications:
//   
// ****************************************************************************

const ColorAttribute &
ViewerPalette::Color(int i) const
{
    // Clamp the index to 0..ncolors-1
    int index = i % ncolors;

    return palette[index];
}

// ****************************************************************************
// Method: ViewerPalette::SetColor
//
// Purpose: 
//   Sets the color at the specified index.
//
// Arguments:
//   i     : The index of the color to set.
//   color : The new color value.
//
// Note:       If the index is mod'd by ncolors to give an index in the range
//             0..ncolors-1.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 13:09:59 PST 2000
//
// Modifications:
//   
// ****************************************************************************
void
ViewerPalette::SetColor(int i, const ColorAttribute &color)
{
    // Clamp the index to 0..ncolors-1
    int index = i % ncolors;

    palette[index] = color;    
}

// ****************************************************************************
// Method: ViewerPalette::NumColors
//
// Purpose: 
//   Returns the number of colors in the palette.
//
// Returns:    The number of colors in the palette.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 11 13:12:02 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPalette::NumColors() const
{
    return ncolors;
}
