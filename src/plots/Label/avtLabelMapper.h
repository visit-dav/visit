// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtLabelMapper.h                                //
// ************************************************************************* //

#ifndef AVT_LABEL_MAPPER_H
#define AVT_LABEL_MAPPER_H

#include <avtMapper2D.h>
#include <vector>
#include <string>

#include <LabelAttributes.h>
#include <vtkType.h>


// ****************************************************************************
//  Class: avtLabelMapper
//
//  Purpose:
//    Label plot specific mapper.
//
//  Programmer: Kathleen Biagas 
//  Creation:   April 13, 2017
//
//  Modifications:
//
// ****************************************************************************

class avtLabelMapper : public avtMapper2D
{
  public:
                               avtLabelMapper();
    virtual                   ~avtLabelMapper();

    virtual void               SetLabels(std::vector<std::string> &L, bool);

    void                       SetAtts(const LabelAttributes *);
    bool                       SetForegroundColor(const double *);
    void                       SetVariable(const std::string &name);
    void                       SetTreatAsASCII(bool);
    void                       Set3D(bool val);
    void                       SetExtents(const double *ext);
    void                       SetUseGlobalLabel(bool val);
    void                       SetCellOrigin(vtkIdType);
    void                       SetNodeOrigin(vtkIdType);

  protected:
    virtual vtkMapper2D       *CreateMapper2D(void);
    virtual void               CustomizeMappers(void);

    std::vector<std::string>   labelNames;

    LabelAttributes            atts; 
    double                     fgColor[4];
    std::string                varname;
    bool                       treatAsASCII;
    bool                       renderLabels3D;
    double                     spatialExtents[6];
    std::string                globalLabel;
    bool                       useGlobalLabel;
    vtkIdType                  cellOrigin;
    vtkIdType                  nodeOrigin;


};

#endif


