// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//   avtSubsetMapper.h
// ****************************************************************************

#ifndef AVT_SUBSETMAPPER_H
#define AVT_SUBSETMAPPER_H

#include <avtLevelsMapper.h>
#include <GlyphTypes.h>

#include <string>
#include <vector>


// ****************************************************************************
//  Class:  avtSubsetMapper
//
//  Purpose:
//      Subset plot specific mapper.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 5, 2020
//
//  Modifications:
//
// ****************************************************************************

class avtSubsetMapper : public avtLevelsMapper
{
  public:
                avtSubsetMapper();
    virtual    ~avtSubsetMapper();


    void        ScaleByVar(const std::string &);
    void        DataScalingOn(const std::string &, int = 1);
    void        DataScalingOff(void);

    void        SetScale(double);
    void        SetGlyphType(GlyphType);
    void        SetPointSize(double s);

    bool        SetFullFrameScaling(bool, const double *) override;

  protected:
    // these are called from avtMapper
    void        CreateActorMapperPairs(vtkDataSet **children) override;
    void        CustomizeMappers(void) override;
    void        SetLabels(std::vector<std::string> &, bool) override;

  private:

    int         spatialDim;
    double      scale;
    std::string scalingVarName;
    GlyphType   glyphType;
    double      pointSize;
    bool        dataScaling;

    std::vector<std::string> labels;

    void        CustomizeMappersInternal(bool invalidateTransparency=false);
};


#endif


