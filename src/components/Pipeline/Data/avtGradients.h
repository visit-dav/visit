// ************************************************************************* //
//                               avtGradients.h                              //
// ************************************************************************* //

#ifndef AVT_GRADIENTS_H
#define AVT_GRADIENTS_H
#include <pipeline_exports.h>


// ****************************************************************************
//  Class: avtGradients
//
//  Purpose:
//      Holds a vector of gradients.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 13 15:52:21 PST 2002
//    Added new routines for more efficient calculation of Sobel gradients.
//
// ****************************************************************************

class PIPELINE_API avtGradients
{
    friend class            avtVolume;

  public:
                            avtGradients(int);
     virtual               ~avtGradients();

     void                   Initialize(void);
     void                   Normalize(void);

     void                   SetGradient(int, const double[3]);
     void                   GetGradient(int, double[3]) const;

     void                   PartialAddGradient(int, int, double);

  protected:
     int                    numGradients;
     double                *gradients;
};


#endif


