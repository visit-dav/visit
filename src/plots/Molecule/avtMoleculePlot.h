// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtMoleculePlot.h                               //
// ************************************************************************* //

#ifndef AVT_MOLECULE_PLOT_H
#define AVT_MOLECULE_PLOT_H

#include <avtPlot.h>
#include <MoleculeAttributes.h>
#include <avtLevelsLegend.h>

class avtVariableLegend;
class avtLevelsLegend;
class avtLookupTable;
class avtMoleculeFilter;
class avtMoleculeMapper;
class avtExtractMolInfoFilter;

// ****************************************************************************
//  Method: avtMoleculePlot
//
//  Purpose:
//      A concrete type of avtPlot for molecules.
//
//  Programmer: Jeremy Meredith
//  Creation:   Februray 14, 2006
//
//  Modifications:
//    Jeremy Meredith, Tue Aug 29 13:19:09 EDT 2006
//    Changed to point data plot.
//
// ****************************************************************************

class
avtMoleculePlot : public avtPointDataPlot
{
  public:
    avtMoleculePlot();
    virtual        ~avtMoleculePlot();

    static avtPlot *Create();

    virtual const char *GetName(void) { return "MoleculePlot"; };

    virtual void    SetAtts(const AttributeGroup*);
    virtual void    GetDataExtents(std::vector<double> &);
    virtual void    ReleaseData(void);
    virtual bool    SetColorTable(const char *ctName);

    void            SetLegend(bool);
    void            SetLegendRange(void);

    avtContract_p EnhanceSpecification(avtContract_p);
  protected:
    MoleculeAttributes       atts;

    avtMoleculeMapper       *mapper;

    avtMoleculeFilter       *moleculeFilter;
    avtExtractMolInfoFilter *extractMolInfoFilter;

    avtLevelsLegend         *levelsLegend;
    avtLegend_p              levelsLegendRefPtr;

    avtVariableLegend       *variableLegend;
    avtLegend_p              variableLegendRefPtr;

    avtLookupTable          *levelsLUT;
    avtLookupTable          *variableLUT;

    LevelColorMap            elementColorMap;
    LevelColorMap            residueColorMap;
    LevelColorMap            blankColorMap;

    virtual avtMapperBase   *GetMapper(void);
    virtual avtDataObject_p  ApplyOperators(avtDataObject_p);
    virtual avtDataObject_p  ApplyRenderingTransformation(avtDataObject_p);
    virtual void             CustomizeBehavior(void);
    virtual void             CustomizeMapper(avtDataObjectInformation &);
    virtual avtLegend_p      GetLegend(void);
    void                     CreateLegendColorMaps();
};


#endif


