// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtMoleculeMapper.h                            //
// ************************************************************************* //

#ifndef AVT_MOLECULEMAPPER_H
#define AVT_MOLECULEMAPPER_H

#include <avtMapper.h>
#include <MoleculeAttributes.h>


class vtkLookupTable;

// ****************************************************************************
//  Class:  avtMoleculeMapper
//
//  Purpose:
//      Utilizes vtkVisItMoleculeMapper instead of vtkDataSetMapper.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 22, 2016 
//
//  Modifications:
//
// ****************************************************************************

class avtMoleculeMapper : public avtMapper
{
  public:
                               avtMoleculeMapper();
    virtual                   ~avtMoleculeMapper();


    // From avtMapper
    void                       SetSurfaceRepresentation(int);

    // this class, called from avtPlot
    void                       SetRange(double, double);
    void                       SetAtts(const AttributeGroup *);
    void                       SetLookupTable(vtkLookupTable *);
    void                       SetLevelsLUT(vtkLookupTable *);
    void                       SetIs2D(bool val);

    bool                       GetCurrentDataRange(double &, double &);
    void                       InvalidateColors();

  protected:
    MoleculeAttributes         atts;
    vtkLookupTable            *lut;
    vtkLookupTable            *levelsLUT;
    double                     vmin;
    double                     vmax;
    bool                       is2D;

    virtual void               CustomizeMappers(void);
    virtual vtkDataSetMapper  *CreateMapper(void);
};


#endif


