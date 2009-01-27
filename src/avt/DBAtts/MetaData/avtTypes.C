/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                avtTypes.C                                 //
// ************************************************************************* //

#include <avtTypes.h>


const char *AUXILIARY_DATA_DATA_EXTENTS = "DATA_EXTENTS";
const char *AUXILIARY_DATA_EXTERNAL_FACELIST = "EXTERNAL_FACELIST";
const char *AUXILIARY_DATA_HISTOGRAM = "HISTOGRAM";
const char *AUXILIARY_DATA_IDENTIFIERS = "IDENTIFIERS";
const char *AUXILIARY_DATA_MATERIAL = "MATERIAL";
const char *AUXILIARY_DATA_POST_GHOST_MATERIAL = "POST_GHOST_MATERIAL";
const char *AUXILIARY_DATA_SPECIES = "SPECIES";
const char *AUXILIARY_DATA_SPATIAL_EXTENTS = "SPATIAL_EXTENTS";
const char *AUXILIARY_DATA_MIXED_VARIABLE = "MIXED_VARIABLE";
const char *AUXILIARY_DATA_POST_GHOST_MIXED_VARIABLE = "POST_GHOST_MIXED_VARIABLE";
const char *AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION 
                                               = "DOMAIN_BOUNDARY_INFORMATION";
const char *AUXILIARY_DATA_STREAMING_GHOST_GENERATION 
                                               =  "STREAMING_GHOST_GENERATION";
const char *AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION 
                                               = "DOMAIN_NESTING_INFORMATION";
const char * AUXILIARY_DATA_GLOBAL_NODE_IDS = "GLOBAL_NODE_IDS";
const char * AUXILIARY_DATA_GLOBAL_ZONE_IDS = "GLOBAL_ZONE_IDS";

static const char *avtExtentType_strings[] = { "AVT_ORIGINAL_EXTENTS",
    "AVT_ACTUAL_EXTENTS", "AVT_SPECIFIED_EXTENTS", "AVT_UNKNOWN_EXTENT_TYPE"};

// ****************************************************************************
// Function: avtExtentType_ToString
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


// ****************************************************************************
//  Function: avtVarTypeToString
//
//  Purpose:
//      Creates a string from an avtVarType.
//
//  Programmer: Hank Childs
//  Creation:   August 16, 2005
//
// ****************************************************************************

std::string
avtVarTypeToString(avtVarType v)
{
    switch (v)
    {
      case AVT_MESH:
        return "mesh";
      case AVT_SCALAR_VAR:
        return "scalar";
      case AVT_VECTOR_VAR:
        return "vector";
      case AVT_TENSOR_VAR:
        return "tensor";
      case AVT_SYMMETRIC_TENSOR_VAR:
        return "symmetric tensor";
      case AVT_ARRAY_VAR:
        return "array";
      case AVT_LABEL_VAR:
        return "label";
      case AVT_MATERIAL:
        return "material";
      case AVT_MATSPECIES:
        return "species";
      case AVT_CURVE:
        return "curve";
      default:
        break;
    }
    return "unknown";
}


// ****************************************************************************
// Function: GuessVarTypeFromNumDimsAndComps
//
// Purpose:
//   Guesses the avt variable type from spatial dimensions and component count 
//
// Programmer: Mark C. Miller 
// Creation:  Tue May 18 15:31:37 PDT 2004 
//
// ****************************************************************************
avtVarType
GuessVarTypeFromNumDimsAndComps(int numSpatialDims, int componentCount)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    if (componentCount == 1)
        retval = AVT_SCALAR_VAR;
    else if (numSpatialDims == 2)
    {
        if (componentCount == 2)
            retval = AVT_VECTOR_VAR;
        else if (componentCount == 3)
            retval = AVT_SYMMETRIC_TENSOR_VAR;
        else if (componentCount == 4)
            retval = AVT_TENSOR_VAR;
    }
    else if (numSpatialDims == 3)
    {
        if (componentCount == 3)
            retval = AVT_VECTOR_VAR;
        else if (componentCount == 6)
            retval = AVT_SYMMETRIC_TENSOR_VAR;
        else if (componentCount == 9)
            retval = AVT_TENSOR_VAR;
    }

    return retval;
}

std::string
LoadBalanceSchemeToString(LoadBalanceScheme s)
{
    switch (s)
    {
        case LOAD_BALANCE_CONTIGUOUS_BLOCKS_TOGETHER:
            return "contiguous blocks together";
        case LOAD_BALANCE_STRIDE_ACROSS_BLOCKS:
            return "stride across blocks";
        case LOAD_BALANCE_RANDOM_ASSIGNMENT:
            return "random assignment";
        case LOAD_BALANCE_DBPLUGIN_DYNAMIC:
            return "dbplugin dynamic";
        case LOAD_BALANCE_RESTRICTED:
            return "restricted";
        case LOAD_BALANCE_ABSOLUTE:
            return "absolute";
        case LOAD_BALANCE_UNKNOWN:
        default:
            return "unknown";
    }
}
