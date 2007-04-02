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
// ****************************************************************************

class PLOTTER_API avtLookupTable
{
  public:
                                  avtLookupTable();
    virtual                      ~avtLookupTable();

    void                          SetSkewFactor(const double);
    bool                          SetColorTable(const char *ctName, bool);
    void                          SetLUTColors(const unsigned char *, int);
    void                          SetLUTColorsWithOpacity(
                                      const unsigned char *, int);

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
};


#endif


