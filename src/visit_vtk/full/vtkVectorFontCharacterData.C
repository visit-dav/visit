#include <vtkVectorFontCharacterData.h>
#include <vtkTextProperty.h>

// Include the font data
#include "visit_arial.h"
#include "visit_arial_italic.h"
#include "visit_arial_bold.h"
#include "visit_arial_bold_italic.h"

#include "visit_courier.h"
#include "visit_courier_italic.h"
#include "visit_courier_bold.h"
#include "visit_courier_bold_italic.h"

#include "visit_times.h"
#include "visit_times_italic.h"
#include "visit_times_bold.h"
#include "visit_times_bold_italic.h"

// and so on...

#define FONT_NORMAL      0
#define FONT_ITALIC      1
#define FONT_BOLD        2
#define FONT_BOLD_ITALIC 3

// ****************************************************************************
// Method: vtkVectorFontCharacterData::GetCharacterData
//
// Purpose: 
//   Returns the vtkVectorFontCharacterData record for the specified character
//   with the given font properties.
//
// Arguments:
//   charIndex : The index of the character to return [0,255].
//   family    : VTK_ARIAL, VTK_COURIER, VTK_TIMES
//   bold      : True for a bold font; false otherwise
//   italic    : True for an italic font; false otherwise
//
// Returns:    A vtkVectorFontCharacterData record or 0 if none was found.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 16:00:17 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

const vtkVectorFontCharacterData *
vtkVectorFontCharacterData::GetCharacterData(int charIndex, int family, bool bold,
    bool italic)
{
    const vtkVectorFontCharacterData *retval = 0;
    // Make sure that we're requesting a character that we support.
    if(charIndex >= 33 && charIndex <= 131)
    {       
        int index = charIndex - 33;
        int font = (bold?2:0) | (italic?1:0);
        if(family == VTK_ARIAL)
        {
            switch(font)
            {
            case FONT_NORMAL:
                retval = &visit_arial_font_data[index];
                break;
            case FONT_ITALIC:
                retval = &visit_arial_italic_font_data[index];
                break;
            case FONT_BOLD:
                retval = &visit_arial_bold_font_data[index];
                break;
            case FONT_BOLD_ITALIC:
                retval = &visit_arial_bold_italic_font_data[index];
                break;
            }
        }
        else if(family == VTK_COURIER)
        {
            switch(font)
            {
            case FONT_NORMAL:
                retval = &visit_courier_font_data[index];
                break;
            case FONT_ITALIC:
                retval = &visit_courier_italic_font_data[index];
                break;
            case FONT_BOLD:
                retval = &visit_courier_bold_font_data[index];
                break;
            case FONT_BOLD_ITALIC:
                retval = &visit_courier_bold_italic_font_data[index];
                break;
            }
        }
        else if(family == VTK_TIMES)
        {
            switch(font)
            {
            case FONT_NORMAL:
                retval = &visit_times_font_data[index];
                break;
            case FONT_ITALIC:
                retval = &visit_times_italic_font_data[index];
                break;
            case FONT_BOLD:
                retval = &visit_times_bold_font_data[index];
                break;
            case FONT_BOLD_ITALIC:
                retval = &visit_times_bold_italic_font_data[index];
                break;
            }
        }
    }

    return retval;
}
