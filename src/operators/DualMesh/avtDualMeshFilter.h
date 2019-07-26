// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtDualMeshFilter.h
// ************************************************************************* //

#ifndef AVT_DUALMESH_FILTER_H
#define AVT_DUALMESH_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <DualMeshAttributes.h>

class vtkDataArray;


// ****************************************************************************
//  Class: avtDualMeshFilter
//
//  Purpose:
//      Converts rectilinear mesh data between dual representations.
//
//        Nodes to Zones: Creates output zones centered at input nodes and 
//        converts point data to cell data.
//        
//        Zones to Nodes: Creates output nodes centered at input zone centers 
//        and converts cell data to point data.
//        
//        Auto: Looks at the primary varaible to determine conversion mode.
//        If there is no primary var (which is a valid case for a mesh plot)
//        will default to "Nodes to Zones". 
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed May 7 15:59:34 PST 2008
//
//  Modifications:
//    Brad Whitlock, Wed Aug 15 12:21:41 PDT 2012
//    Override ModifyContract.
//
//    Eric Brugger, Wed Jul 23 12:06:25 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtDualMeshFilter : public avtPluginDataTreeIterator
{
  public:
                         avtDualMeshFilter();
    virtual             ~avtDualMeshFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtDualMeshFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Dual Mesh"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    DualMeshAttributes   atts;
    std::string          actualVar;
    int                  actualMode;

    virtual void         UpdateDataObjectInfo(void);
    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void         ExamineContract(avtContract_p contract);
    virtual avtContract_p ModifyContract(avtContract_p);

    vtkDataArray        *ExpandDual(vtkDataArray *coords);
    vtkDataArray        *ContractDual(vtkDataArray *coords);
    
};


#endif
