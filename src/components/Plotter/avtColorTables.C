#include <avtColorTables.h>
#include <ColorTableAttributes.h>
#include <ColorControlPointList.h>
#include <ColorControlPoint.h>
#include <ColorTableManager.h>

//
// Static data that describes the eight default color tables.
//

static const char *predef_ct_names[]  = {"caleblack", "calewhite",
    "contoured", "gray", "hot", "levels", "rainbow", "xray"};
static const int predef_ct_ncolors[]  = {7, 7, 4, 2, 5, 30, 6, 2};
static const int predef_ct_smooth[]   = {1, 1, 0, 1, 1,  0, 1, 1};
static const int predef_ct_equal[]    = {0, 0, 1, 0, 0,  1, 0, 0};
static const int predef_ct_discrete[] = {0, 0, 0, 0, 0,  1, 0, 0};

/* Hot */
static const float ct_hot[] = {
 0.,   0., 0., 1.,
 0.25, 0., 1., 1., 
 0.5,  0., 1., 0., 
 0.75, 1., 1., 0., 
 1.,   1., 0., 0.
};

/* Caleblack */
static const float ct_caleblack[] = {
 0.,   0., 0., 0.,
 0.17, 0., 0., 1., 
 0.34, 0., 1., 1., 
 0.50, 0., 1., 0., 
 0.67, 1., 1., 0., 
 0.84, 1., 0., 0., 
 1.,   1., 0., 1., 
};

/* Calewhite */
static const float ct_calewhite[] = {
 0.,   1., 1., 1., 
 0.17, 0., 0., 1., 
 0.34, 0., 1., 1., 
 0.50, 0., 1., 0., 
 0.67, 1., 1., 0., 
 0.84, 1., 0., 0., 
 1.,   1., 0., 1.,
};

/* Gray */
static const float ct_gray[] = {0., 0., 0., 0., 1., 1., 1., 1.};
/* Xray */
static const float ct_xray[] = {0., 1., 1., 1., 1., 0., 0., 0.};

/* Rainbow */
static const float ct_rainbow[] = {
 0.,  1., 0., 1., 
 0.2, 0., 0., 1., 
 0.4, 0., 1., 1., 
 0.6, 0., 1., 0., 
 0.8, 1., 1., 0., 
 1.0, 1., 0., 0., 
};

/* Contoured */
static const float ct_contoured[] = {
 0.,    0., 0., 1., 
 0.333, 0., 1., 0., 
 0.666, 1., 1., 0., 
 1.0,   1., 0., 0., 
};

/* Levels discrete color table. */
static const float ct_levels[] = {
 0.00f, 1.00f, 0.00f, 0.00f,
 0.03f, 0.00f, 1.00f, 0.00f,
 0.07f, 0.00f, 0.00f, 1.00f,
 0.10f, 0.00f, 1.00f, 1.00f,
 0.14f, 1.00f, 0.00f, 1.00f,
 0.17f, 1.00f, 1.00f, 0.00f,
 0.21f, 1.00f, 0.53f, 0.00f,
 0.24f, 1.00f, 0.00f, 0.53f,
 0.28f, 0.66f, 0.66f, 0.66f,
 0.31f, 1.00f, 0.27f, 0.27f,
 0.34f, 0.39f, 1.00f, 0.39f,
 0.38f, 0.39f, 0.39f, 1.00f,
 0.41f, 0.16f, 0.65f, 0.65f,
 0.45f, 1.00f, 0.39f, 1.00f,
 0.48f, 1.00f, 1.00f, 0.39f,
 0.52f, 1.00f, 0.67f, 0.39f,
 0.55f, 0.67f, 0.31f, 1.00f,
 0.59f, 0.59f, 0.00f, 0.00f,
 0.62f, 0.00f, 0.59f, 0.00f,
 0.66f, 0.00f, 0.00f, 0.59f,
 0.69f, 0.00f, 0.43f, 0.43f,
 0.72f, 0.59f, 0.00f, 0.59f,
 0.76f, 0.59f, 0.59f, 0.00f,
 0.79f, 0.59f, 0.33f, 0.00f,
 0.83f, 0.63f, 0.00f, 0.31f,
 0.86f, 1.00f, 0.41f, 0.11f,
 0.90f, 0.00f, 0.67f, 0.32f,
 0.93f, 0.27f, 1.00f, 0.49f,
 0.97f, 0.00f, 0.51f, 1.00f,
 1.00f, 0.51f, 0.00f, 1.00f,
};

// Static pointer to single instance.
avtColorTables *avtColorTables::instance = NULL;

// ****************************************************************************
// Method: avtColorTables::avtColorTables
//
// Purpose: 
//   Constructor for the avtColorTables class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:04:51 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 13:34:47 PST 2002
//   I rewrote the routine.
//
// ****************************************************************************

avtColorTables::avtColorTables()
{
    ctAtts = new ColorTableAttributes();

    // Set up some pointers.
    const float *predef_ct_colors[8];
    predef_ct_colors[0] = ct_caleblack;
    predef_ct_colors[1] = ct_calewhite;
    predef_ct_colors[2] = ct_contoured;
    predef_ct_colors[3] = ct_gray;
    predef_ct_colors[4] = ct_hot;
    predef_ct_colors[5] = ct_levels;
    predef_ct_colors[6] = ct_rainbow;
    predef_ct_colors[7] = ct_xray;

    // Add each colortable.
    for(int i = 0; i < 8; ++i)
    {
        ColorControlPointList ccpl;

        const float *fptr = predef_ct_colors[i];
        for(int j = 0; j < predef_ct_ncolors[i]; ++j)
        {
            ColorControlPoint p(fptr[0],
                                (unsigned char)(fptr[1]*255),
                                (unsigned char)(fptr[2]*255),
                                (unsigned char)(fptr[3]*255),
                                0);
            ccpl.AddColorControlPoint(p);
            fptr += 4;
        }

        ccpl.SetSmoothingFlag(predef_ct_smooth[i] == 1);
        ccpl.SetEqualSpacingFlag(predef_ct_equal[i] == 1);
        ccpl.SetDiscreteFlag(predef_ct_discrete[i] == 1);
        ctAtts->AddColorTable(predef_ct_names[i], ccpl);
    }

    // Set the active continuous color table to "hot".
    ctAtts->SetActiveContinuous("hot");
    // Set the active discrete color table to "levels".
    ctAtts->SetActiveDiscrete("levels");
}

// ****************************************************************************
// Method: avtColorTables::~avtColorTables
//
// Purpose: 
//   Destructor for the avtColorTables class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:04:51 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 13:35:46 PST 2002
//   I removed some deletion code.
//
// ****************************************************************************

avtColorTables::~avtColorTables()
{
    delete ctAtts;
    ctAtts = 0;
}

// ****************************************************************************
// Method: avtColorTables::Instance
//
// Purpose: 
//   Static function that returns a pointer to the single instance of the
//   class.
//
// Returns:    A pointer to the single instance of avtColorTables.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:05:26 PST 2001
//
// Modifications:
//   
// ****************************************************************************

avtColorTables *
avtColorTables::Instance()
{
    if(instance == NULL)
        instance = new avtColorTables;

    return instance;
}

// ****************************************************************************
// Method: avtColorTables::GetDefaultContinuousColorTable
//
// Purpose: 
//   Returns the name of the default continuous color table.
//
// Returns:    The name of the default continuous color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:06:07 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 13:36:51 PST 2002
//   I renamed the method.
//
// ****************************************************************************

const std::string &
avtColorTables::GetDefaultContinuousColorTable() const
{
    return ctAtts->GetActiveContinuous();
}

// ****************************************************************************
// Method: avtColorTables::GetDefaultDiscreteColorTable
//
// Purpose: 
//   Returns the name of the default discrete color table.
//
// Returns:    The name of the default discrete color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:06:07 PST 2001
//
// Modifications:
//   
// ****************************************************************************

const std::string &
avtColorTables::GetDefaultDiscreteColorTable() const
{
    return ctAtts->GetActiveDiscrete();
}

// ****************************************************************************
// Method: avtColorTables::ColorTableExists
//
// Purpose: 
//   Determines whether or not the specified color table exists.
//
// Arguments:
//   ctName : The name of the color table to look for.
//
// Returns:    True if ctName is in avtColorTables, false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:06:43 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
avtColorTables::ColorTableExists(const std::string &ctName) const
{
    return (ctAtts->GetColorTableIndex(ctName) != -1);
}

// ****************************************************************************
// Method: avtColorTables::IsDiscrete
//
// Purpose: 
//   Returns whether or not the named color table is discrete.
//
// Arguments:
//   ctName : The name of the color table.
//
// Returns:    True if the named color table is discrete.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 14:25:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtColorTables::IsDiscrete(const std::string &ctName) const
{
    bool retval = false;
    int index;
    if((index = ctAtts->GetColorTableIndex(ctName)) != -1)
    {
        const ColorControlPointList &ct = ctAtts->operator[](index);
        retval = ct.GetDiscreteFlag();
    }

    return retval;
}

// ****************************************************************************
// Method: avtColorTables::SetDefaultContinuousColorTable
//
// Purpose: 
//   Sets the default continuous color table.
//
// Arguments:
//   ctName : The name of the new default continuous color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:08:36 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtColorTables::SetDefaultContinuousColorTable(const std::string &ctName)
{
    if(ColorTableExists(ctName))
    {
        ctAtts->SetActiveContinuous(ctName);
    }
}

// ****************************************************************************
// Method: avtColorTables::SetDefaultDiscreteColorTable
//
// Purpose: 
//   Sets the default discrete color table.
//
// Arguments:
//   ctName : The name of the new default discrete color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:08:36 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtColorTables::SetDefaultDiscreteColorTable(const std::string &ctName)
{
    if(ColorTableExists(ctName))
    {
        ctAtts->SetActiveDiscrete(ctName);
    }
}

// ****************************************************************************
// Method: avtColorTables::GetColors
//
// Purpose: 
//   Returns a pointer to the named color table's colors.
//
// Arguments:
//   ctName : The name of the color table for which we want the colors.
//
// Returns:    The color array for the color table or NULL if ctName is not
//             the name of a valid color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:09:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Dec 3 14:24:06 PST 2002
//   I rewrote the method.
//
// ****************************************************************************

const unsigned char *
avtColorTables::GetColors(const std::string &ctName)
{
    const unsigned char *retval = NULL;
    int index;
    if((index = ctAtts->GetColorTableIndex(ctName)) != -1)
    {
        const ColorControlPointList &ct = ctAtts->operator[](index);
        ct.GetColors(tmpColors, GetNumColors());
        retval = tmpColors;
    }

    return retval;
}

// ****************************************************************************
// Method: avtColorTables::GetSampledColors
//
// Purpose: 
//   Returns colors for the specified color table sampled over a certain range.
//
// Arguments:
//   ctName  : The name of the color table.
//   nColors : The number of sample colors.
//   
// Returns:    An unsigned char array that contains the colors.
//
// Note:       The caller is responsible for freeing the returned array.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 14:20:10 PST 2002
//
// Modifications:
//   
// ****************************************************************************

unsigned char *
avtColorTables::GetSampledColors(const std::string &ctName, int nColors) const
{
    unsigned char *retval = NULL;
    int index;
    if((index = ctAtts->GetColorTableIndex(ctName)) != -1)
    {
        const ColorControlPointList &ct = ctAtts->operator[](index);

        int nc = (nColors < 1) ? 1 : nColors;
        retval = new unsigned char[nc * 3];
        ct.GetColors(retval, nc);
    }
  
    return retval;
}

// ****************************************************************************
// Method: avtColorTables::GetControlPointColor
//
// Purpose: 
//   Gets the color of the i'th color control point for the specified
//   color table.
//
// Arguments:
//   ctName : The name of the discrete color table.
//   i      : The index of the color control point. The value is mod'ed so
//            that it always falls within the number of control points for
//            the specified color table.
//   rgb    : The return array for the colors.
//
// Returns:    A boolean value indicating whether or not a color was returned.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 20 13:47:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtColorTables::GetControlPointColor(const std::string &ctName, int i,
    unsigned char *rgb) const
{
    bool retval = false;
    int index;
    if((index = ctAtts->GetColorTableIndex(ctName)) != -1)
    {
        const ColorControlPointList &ct = ctAtts->operator[](index);
        int j = i % ct.GetNumColorControlPoints();

        rgb[0] = ct[j].GetColors()[0];
        rgb[1] = ct[j].GetColors()[1];
        rgb[2] = ct[j].GetColors()[2];
        retval = true;
    }

    return retval;
}

// ****************************************************************************
// Method: avtColorTables::SetColorTables
//
// Purpose: 
//   Copies new color tables into the color table attributes.
//
// Arguments:
//   atts : The new color table list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 3 14:34:48 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtColorTables::SetColorTables(const ColorTableAttributes &atts)
{
    *ctAtts = atts;
}

// ****************************************************************************
// Method: avtColorTables::ExportColorTable
//
// Purpose: 
//   Exports the specified color table to a file that can be shared.
//
// Arguments:
//   ctName : The name of the color table to export.
//
// Returns:    A string indicating what happened.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:28:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
avtColorTables::ExportColorTable(const std::string &ctName)
{
    const ColorControlPointList *ccpl = ctAtts->GetColorControlPoints(ctName);

    if(ccpl != 0)
    {
        ColorTableManager exporter;
        return exporter.Export(ctName, *ccpl);
    }

    return std::string("VisIt could not save the color table\"") + ctName +
           std::string("\" because that color table does not exist.");
}

// ****************************************************************************
// Method: avtColorTables::ImportColorTables
//
// Purpose: 
//   Imports the color tables in the user's home .visit directory and adds
//   them to the list of color tables.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:30:07 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtColorTables::ImportColorTables()
{
    //
    // Create a color table manager to import the color tables and store
    // them in the ctAtts.
    //
    ColorTableManager importer;
    importer.ImportColorTables(ctAtts);
}

