// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtLookupTable.h                                 //
// ************************************************************************* //

#ifndef AVT_LOOKUP_TABLE_H
#define AVT_LOOKUP_TABLE_H
#include <plotter_exports.h>

class vtkLookupTable;
class vtkLogLookupTable;
class vtkSkewLookupTable;

// ****************************************************************************
//  Class: avtLookupTable
//
//  Purpose:  
//    Serves as a wrapper for the various types of vtkLookupTable.
//    Provides consistent and central place for vtkLookupTable manipulation.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 27, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 20 11:18:31 EST 2009
//    Allowed SetColorTable to optionally also use the color table's opacity.
//    Added new SetLUTColorsAndOpacity method to actually perform the work.
//
//    Kathleen Bonnell, Mon Jan 17 17:36:52 MST 2011
//    Added invert argument to SetColorTable.
//
// ****************************************************************************

class PLOTTER_API avtLookupTable
{
  public:
                                  avtLookupTable();
    virtual                      ~avtLookupTable();

    void                          SetSkewFactor(const double);
    void                          SetRampOpacity(const double);
    bool                          SetColorTable(const char *ctName, bool,
                                                bool useOpacities = false,
                                                bool invert = false,
                                                double rampOpacity = -1);
    void                          SetLUTColors(const unsigned char *, int);
    void                          SetLUTColorsWithOpacity(
                                      const unsigned char *, int);
    void                          SetLUTColorsAndOpacity(
                                      const unsigned char *,
                                      const unsigned char *,
                                      int);

    int                           GetNumberOfColors(void);

    vtkLookupTable               *GetLookupTable(void) 
                                      { return stdLUT; };
    vtkLookupTable               *GetLogLookupTable(void)
                                      { return (vtkLookupTable*) logLUT; };
    vtkLookupTable               *GetSkewLookupTable(void)
                                      { return (vtkLookupTable*) skewLUT; };

  protected:
    vtkLookupTable               *stdLUT;
    vtkLogLookupTable            *logLUT;
    vtkSkewLookupTable           *skewLUT;
    
    double                        rampOpacity;
};
#endif
