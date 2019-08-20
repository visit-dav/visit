// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         BadWindowModeException.C                          //
// ************************************************************************* //

#include <BadWindowModeException.h>


// ****************************************************************************
//  Method: BadWindowModeException constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue May  7 08:21:01 PDT 2002
//    Add  WINMODE_CURVE.
//
//    Jeremy Meredith, Mon Jan 28 17:40:55 EST 2008
//    Added WINMODE_AXISARRAY.
//
// ****************************************************************************

BadWindowModeException::BadWindowModeException(WINDOW_MODE mode)
{
    std::string  modeS;
    switch (mode)
    {
      case WINMODE_2D:
        modeS = "2D";
        break;
      case WINMODE_3D:
        modeS = "3D";
        break;
      case WINMODE_CURVE:
        modeS = "Curve";
        break;
      case WINMODE_AXISARRAY:
        modeS = "AxisArray";
        break;
      case WINMODE_NONE:
        modeS = "None";
        break;
      default:
        modeS = "Unaccounted For Mode";
        break;
    };

    msg = "Encountered unexpected window mode " + modeS;
}


