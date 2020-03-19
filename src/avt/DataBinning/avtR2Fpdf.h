// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                  avtR2Fpdf.h                              //
// ************************************************************************* //

#ifndef AVT_R2F_PDF_H
#define AVT_R2F_PDF_H

#include <dbin_exports.h>

#include <avtR2Foperator.h>

#include <vector>
#include <string>


// ****************************************************************************
//  Class: avtR2Fpdf
//
//  Purpose:
//      Calculates the PDF.
//
//  Programmer: Hank Childs
//  Creation:   August 20, 2010
//
// ****************************************************************************

class AVTDBIN_API avtR2Fpdf : public avtR2Foperator
{
  public:
                           avtR2Fpdf(int);
    virtual               ~avtR2Fpdf();

    virtual float         *FinalizePass(int);
    virtual void           AddData(int, float);

  protected:
    float                 *pdf;
};


#endif


