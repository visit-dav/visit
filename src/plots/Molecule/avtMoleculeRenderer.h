/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtMoleculeRenderer.h                          //
// ************************************************************************* //

#ifndef AVT_MOLECULE_RENDERER_H
#define AVT_MOLECULE_RENDERER_H

#include <avtCustomRenderer.h>
#include <MoleculeAttributes.h>

class vtkDataArray;
class avtLookupTable;
class avtMoleculeRendererImplementation;

// ****************************************************************************
//  Class: avtMoleculeRenderer
//
//  Purpose:
//      An implementation of an avtCustomRenderer for a molecule plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  3, 2006
//
//  Modifications:
//    Jeremy Meredith, Tue Aug 29 13:20:08 EDT 2006
//    Changed spec coeffs to doubles.
//
// ****************************************************************************

class avtMoleculeRenderer : public avtCustomRenderer
{
  public:
                            avtMoleculeRenderer();
    virtual                ~avtMoleculeRenderer();
    static avtMoleculeRenderer *New(void);

    void                    SetAtts(const AttributeGroup*);

    void                    InvalidateColors();

    virtual void            ReleaseGraphicsResources();
    virtual void            Render(vtkDataSet *);

    std::string GetPrimaryVariable() { return primaryVariable; }
    std::string primaryVariable;

    void SetLevelsLUT(avtLookupTable *);

  protected:
    avtMoleculeRendererImplementation *rendererImplementation;
    bool                               currentRendererIsValid;

    MoleculeAttributes        atts;

    void                    Initialize(vtkDataSet*);
    bool                    initialized;


    // lighting props
    virtual void GlobalLightingOn();
    virtual void GlobalLightingOff();
    virtual void GlobalSetAmbientCoefficient(const float);
    virtual void SetSpecularProperties(bool,double,double,
                                       const ColorAttribute&);

    float ambient_coeff;
    float spec_coeff;
    float spec_power;
    float spec_r, spec_g, spec_b;
    avtLookupTable *levelsLUT;
};


typedef ref_ptr<avtMoleculeRenderer> avtMoleculeRenderer_p;


#endif


