/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            avtMoleculeMapper.C                           //
// ************************************************************************* //

#include <avtMoleculeMapper.h>

#include <avtColorTables.h>
#include <InvalidColortableException.h>
#include <LineAttributes.h>


#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <vtkVisItMoleculeMapper.h>


// ****************************************************************************
//  Method: avtMoleculeMapper constructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 22, 2016 
//
//  Modifications:
//
// ****************************************************************************

avtMoleculeMapper::avtMoleculeMapper() 
{
    lut = NULL;
    levelsLUT = NULL;
    vmin = vmax = 0.;
    is2D = false;
}


// ****************************************************************************
//  Method: avtMoleculeMapper destructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 22, 2016 
//
//  Modifications:
//
// ****************************************************************************

avtMoleculeMapper::~avtMoleculeMapper()
{
}


// ****************************************************************************
//  Method: avtMoleculeMapper::SetSurfaceRepresentation
//
//  Purpose:
//      Sets the drawable's surface representation.
//
//  Arguments:
//      rep : The new surface representation.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 22, 2016
//
// ****************************************************************************

void
avtMoleculeMapper::SetSurfaceRepresentation(int globalRep)
{
  // This is global rep, don't allow it to set anything for now.
}


// ****************************************************************************
//  Method: avtMoleculeMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows the surface and wireframe
//      mapper to make any calls take effect that were made before the base
//      class set up the vtk mappers.
//
//  Programmer: Kathleen Biagas
//  Creation:   July 22, 2016 
//
//  Modifications:
//
// ****************************************************************************

void
avtMoleculeMapper::CustomizeMappers()
{
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] == NULL)
            continue;
        vtkVisItMoleculeMapper *mm = (vtkVisItMoleculeMapper*)mappers[i];
        mm->SetRange(vmin, vmax);
        mm->SetIs2D(is2D);
        if (lut != NULL)
            mm->SetLookupTable(lut);
        if (levelsLUT != NULL)
            mm->SetLevelsLUT(levelsLUT); 

        // Set up Atoms
        if (atts.GetDrawAtomsAs() == MoleculeAttributes::SphereAtoms)
        {
            mm->RenderAtomsOn();
            mm->DrawAtomsAsSpheres();
            mm->SetSphereQuality((int)atts.GetAtomSphereQuality());
        }
        else if (atts.GetDrawAtomsAs() == MoleculeAttributes::ImposterAtoms)
        {
            mm->RenderAtomsOn();
            mm->DrawAtomsAsImposters();
        }
        else 
        {
            mm->RenderAtomsOff();
        }
        mm->SetRadiusType((int)atts.GetScaleRadiusBy());
        if (atts.GetScaleRadiusBy() == MoleculeAttributes::Variable)
        {
            mm->SetRadiusVariable(atts.GetRadiusVariable().c_str());
        }

        mm->SetRadiusScaleFactor(atts.GetRadiusScaleFactor());
        mm->SetRadiusFixed(atts.GetRadiusFixed());
           

        if (atts.GetDrawBondsAs() == MoleculeAttributes::LineBonds)
        {
            mm->RenderBondsOn();
            mm->DrawBondsAsLines();
        }
        else if (atts.GetDrawBondsAs() == MoleculeAttributes::CylinderBonds)
        {
            mm->RenderBondsOn();
            mm->DrawBondsAsCylinders();
        }
        else
        {
            mm->RenderBondsOff();
        }

        mm->SetCylinderQuality(atts.GetBondCylinderQuality());
        mm->SetBondColorMode(atts.GetColorBonds());
        mm->SetBondColor(atts.GetBondSingleColor().GetColor());
        mm->SetBondRadius(atts.GetBondRadius());

        vtkProperty *prop = actors[i]->GetProperty();
        prop->SetLineWidth(
            LineWidth2Int(Int2LineWidth(atts.GetBondLineWidth())));

        // Color table stuff
        bool invalid = 
          mm->GetElementCTName()    != atts.GetElementColorTable() ||
          mm->GetResSeqCTName()     != atts.GetResidueSequenceColorTable() ||
          mm->GetResTypeCTName()    != atts.GetResidueTypeColorTable() ||
          mm->GetContinuousCTName() != atts.GetContinuousColorTable();

        if (invalid)        
        {
            mm->SetElementCTName(atts.GetElementColorTable());
            mm->SetResSeqCTName(atts.GetResidueSequenceColorTable());
            mm->SetResTypeCTName(atts.GetResidueTypeColorTable());
            mm->SetContinuousCTName(atts.GetContinuousColorTable());
            mm->InvalidateColors();
        }
    }
}


// ****************************************************************************
//  Method: avtMoleculeMapper::CreateMapper
//
//  Purpose:
//     Creates a vtkMoleculeMapper
//
//  Programmer: Kathleen Biagas
//  Creation:   July 22, 2016
//
//  Modifications:
//
// ****************************************************************************

vtkDataSetMapper *
avtMoleculeMapper::CreateMapper()
{
    return (vtkDataSetMapper*)(vtkVisItMoleculeMapper::New());
}


void
avtMoleculeMapper::SetAtts(const AttributeGroup *a)
{
    // do we want to check if certain things have changed  rather than
    // always updating the mapper with all the settings via CustomizeMappers?


    atts = *(const MoleculeAttributes*)a;

    CustomizeMappers();
}

void
avtMoleculeMapper::InvalidateColors()
{
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] == NULL)
            continue;

        vtkVisItMoleculeMapper *m = (vtkVisItMoleculeMapper *)mappers[i];
        m->InvalidateColors();
    }
}


void
avtMoleculeMapper::SetLookupTable(vtkLookupTable *LUT)
{
    if (lut == LUT)
        return;
    lut = LUT;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
            mappers[i]->SetLookupTable(lut); 

    }
}

void
avtMoleculeMapper::SetLevelsLUT(vtkLookupTable *LUT)
{
    if (levelsLUT == LUT)
        return;
    levelsLUT = LUT;
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] == NULL)
            continue;

        vtkVisItMoleculeMapper *m = (vtkVisItMoleculeMapper *)mappers[i];
        m->SetLevelsLUT(levelsLUT); 

    }
}

// ****************************************************************************
//  Method: avtMoleculeMapper::GetCurrentDataRange
//
//  Arguments:
//    rmin      The minimum in the range.
//    rmax      The maximum in the range.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 25, 2016 
//
//  Modifications:
//
// ****************************************************************************

bool
avtMoleculeMapper::GetCurrentDataRange(double &rmin, double &rmax)
{
    if (mappers == NULL)
    {
        //
        // We have been asked for the range before the input has been set.
        //
        rmin = 0.;
        rmax = 1.;
        return false;
    }

    avtMapper::GetCurrentRange(rmin, rmax);
    return true;
}


void
avtMoleculeMapper::SetRange(double rmin, double rmax)
{
    if (vmin != rmin || vmax != rmax)
    {
        vmin = rmin;
        vmax = rmax;

        for (int i = 0 ; i < nMappers ; i++)
        {
            if (mappers[i] != NULL)
                ((vtkVisItMoleculeMapper *)mappers[i])->SetRange(vmin, vmax);
        }
    }
}

void
avtMoleculeMapper::SetIs2D(bool val)
{
    if (is2D != val) 
    {
        is2D = val;
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (mappers[i] != NULL)
                ((vtkVisItMoleculeMapper *)mappers[i])->SetIs2D(is2D);
        }
    }
}

