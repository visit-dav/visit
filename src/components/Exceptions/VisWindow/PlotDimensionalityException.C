// ************************************************************************* //
//                       PlotDimensionalityException.C                       //
// ************************************************************************* //

#include <stdio.h>  // for sprintf
#include <PlotDimensionalityException.h>


// ****************************************************************************
//  Method: PlotDimensionalityException constructor
//
//  Programmer: Hank Childs
//  Creation:   May 16, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue May  7 08:33:54 PDT 2002
//    Added WINMODE_CURVE.  
//
// ****************************************************************************

PlotDimensionalityException::PlotDimensionalityException(WINDOW_MODE oldMode,
                               WINDOW_MODE newMode, int numPlots)
{
    std::string  oldModeS;
    switch (oldMode)
    {
      case WINMODE_2D:
        oldModeS = "2D";
        break;
      case WINMODE_3D:
        oldModeS = "3D";
        break;
      case WINMODE_CURVE:
        oldModeS = "Curve";
        break;
      case WINMODE_NONE:
        oldModeS = "None";
        break;
      default:
        oldModeS = "Unaccounted For Mode";
        break;
    };

    std::string  newModeS;
    switch (newMode)
    {
      case WINMODE_2D:
        newModeS = "2D";
        break;
      case WINMODE_3D:
        newModeS = "3D";
        break;
      case WINMODE_CURVE:
        newModeS = "Curve";
        break;
      case WINMODE_NONE:
        newModeS = "None";
        break;
      default:
        newModeS = "Unaccounted For Mode";
        break;
    };

    char  numPlotsString[30];
    sprintf(numPlotsString, "%d", numPlots);
    msg = "Tried to enter mode " + newModeS + " when there were " 
          + numPlotsString + " plots of dimension " + oldModeS;
}


