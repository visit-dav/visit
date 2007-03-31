// ************************************************************************* //
//                                avtTypes.C                                 //
// ************************************************************************* //

#include <avtTypes.h>


const char *AUXILIARY_DATA_DATA_EXTENTS = "DATA_EXTENTS";
const char *AUXILIARY_DATA_EXTERNAL_FACELIST = "EXTERNAL_FACELIST";
const char *AUXILIARY_DATA_MATERIAL = "MATERIAL";
const char *AUXILIARY_DATA_SPECIES = "SPECIES";
const char *AUXILIARY_DATA_SPATIAL_EXTENTS = "SPATIAL_EXTENTS";
const char *AUXILIARY_DATA_MIXED_VARIABLE = "MIXED_VARIABLE";
const char *AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION 
                                               = "DOMAIN_BOUNDARY_INFORMATION";

static const char *avtExtentType_strings[] = { "AVT_ORIGINAL_EXTENTS",
    "AVT_ACTUAL_EXTENTS", "AVT_SPECIFIED_EXTENTS", "AVT_UNKNOWN_EXTENT_TYPE"};

// **************************************************************************** // Function: avtExtentType_ToString
//
// Purpose:
//   Returns a string version of avtExtentType.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 16:44:12 PST 2003
//
// Modifications:
//
// ****************************************************************************
std::string
avtExtentType_ToString(avtExtentType m)
{
    int index = int(m);
    if(m < AVT_ORIGINAL_EXTENTS || m > AVT_UNKNOWN_EXTENT_TYPE) index = 0;
    return avtExtentType_strings[index];
}

std::string
avtExtentType_ToString(int m)
{
    int index = m;
    if(index < 0 || index > 3) index = 0;
    return avtExtentType_strings[index];
}

// ****************************************************************************
// Function: avtExtentType_FromString
//
// Purpose:
//   Returns an avtExtentType associated with a string value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 15 16:44:44 PST 2003
//
// Modifications:
//
// ****************************************************************************

bool
avtExtentType_FromString(const std::string &s, avtExtentType &m)
{
    m = AVT_ORIGINAL_EXTENTS;

    for(int i = 0; i < 3; ++i)
    {
        if(s == avtExtentType_strings[i])
        {
            m = avtExtentType(i);
            return true;
        }
    }

    return false;
}
