// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtPDFFilter.h
// ************************************************************************* //

#ifndef AVT_PDF_FILTER_H
#define AVT_PDF_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <PDFAttributes.h>


// ****************************************************************************
//  Class: avtPDFFilter
//
//  Purpose:
//      A plugin operator for PDF.
//
//  Programmer: childs -- generated by xml2avt
//  Creation:   Sun Nov 20 12:22:59 PDT 2005
//
//  Modifications:
//    Eric Brugger, Thu Jul 31 12:08:35 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtPDFFilter : public avtPluginDataTreeIterator
{
  public:
                         avtPDFFilter();
    virtual             ~avtPDFFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtPDFFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Probability Density Function"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

    virtual void         PreExecute(void);
    virtual void         PostExecute(void);
    virtual void         UpdateDataObjectInfo(void);
    virtual avtContract_p
                         ModifyContract(avtContract_p);

  protected:
    PDFAttributes   atts;
    int            *density;
    float           var1Min;
    float           var1Max;
    float           var2Min;
    float           var2Max;
    float           var3Min;
    float           var3Max;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
};


#endif
