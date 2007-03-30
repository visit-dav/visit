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
      case PICK:
        modeS = "Pick";
        break;
      case LINEOUT:
        modeS = "Lineout";
        break;
      default:
        modeS = "Unaccounted For Mode";
        break;
    };

    msg = "Could not handle interactor \"" + modeS + "\"";
}


