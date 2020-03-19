// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          BadInteractorException.C                         //
// ************************************************************************* //

#include <BadInteractorException.h>


using namespace std;


// ****************************************************************************
//  Method: BadInteractorException constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr 26 11:17:45 PDT 2002
//    Added LINEOUT.
//
//    Kathleen Bonnell, Fri Jun 27 16:43:23 PDT 2003 
//    Pick renamed ZonePick, added NodePick. 
//
//    Gunther H. Weber, Wed Mar 19 16:10:11 PDT 2008
//    Added SPREADSHEET_PICK 
//
//   Jonathan Byrd (Allinea Software), Sun Dec 18, 2011
//   Added the DDT_PICK mode
//
// ****************************************************************************

BadInteractorException::BadInteractorException(INTERACTION_MODE mode)
{
    string  modeS;
    switch (mode)
    {
      case NAVIGATE:
        modeS = "Navigate";
        break;
      case ZOOM:
        modeS = "Zoom";
        break;
      case ZONE_PICK:
        modeS = "ZonePick";
        break;
      case NODE_PICK:
        modeS = "NodePick";
        break;
      case LINEOUT:
        modeS = "Lineout";
        break;
      case SPREADSHEET_PICK:
        modeS = "SpreadsheetPick";
        break;
      case DDT_PICK:
        modeS = "DDTPick";
        break;
      default:
        modeS = "Unaccounted For Mode";
        break;
    };

    msg = "Could not handle interactor \"" + modeS + "\"";
}


