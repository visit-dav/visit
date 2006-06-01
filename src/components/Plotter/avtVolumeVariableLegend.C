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
//                             avtVolumeVariableLegend.C                           //
// ************************************************************************* //

#include <vtkVerticalScalarBarWithOpacityActor.h>
#include <avtVolumeVariableLegend.h>
#include <vtkLookupTable.h>

// ****************************************************************************
// Method: avtVolumeVariableLegend::avtVolumeVariableLegend
//
// Purpose: 
//   Constructor for the avtVolumeVariableLegend class.
//
// Notes:
//   This constructor does not use the default constructor of avtVariableLegend.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 15:37:06 PST 2001
//
// Modifications:
//   
//   Hank Childs, Thu Mar 14 15:57:09 PST 2002
//   Make legend a bit smaller.
//
//   Eric Brugger, Mon Jul 14 15:54:59 PDT 2003
//   Remove initialization of scalar bar title.
//
//   Eric Brugger, Wed Jul 16 16:04:18 PDT 2003
//   I added barVisibility and rangeVisibility.  I changed the default
//   size and position of the legend.
//
// ****************************************************************************

avtVolumeVariableLegend::avtVolumeVariableLegend() : avtVariableLegend(1)
{
    sBar = vtkVerticalScalarBarWithOpacityActor::New();
    sBar->SetShadow(0);
    sBar->SetLookupTable(lut);

    size[0] = 0.08;
    size[1] = 0.26;
    sBar->SetPosition2(size[0], size[1]);

    SetLegendPosition(0.05, 0.72);

    barVisibility = 1;
    rangeVisibility = 1;

    //
    // Set the legend to also point to sBar, so the base methods will work
    // without us re-defining them.
    //
    legend = sBar;
    legend->Register(NULL);
}

// ****************************************************************************
// Method: avtVolumeVariableLegend::~avtVolumeVariableLegend
//
// Purpose: 
//   Destructor for the avtVolumeVariableLegend class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 15:37:49 PST 2001
//
// Modifications:
//   
// ****************************************************************************

avtVolumeVariableLegend::~avtVolumeVariableLegend()
{
    // nothing here
}

// ****************************************************************************
// Method: avtVolumeVariableLegend::SetLegendOpacities
//
// Purpose: 
//   Sets the opacity values that the legend needs.
//
// Arguments:
//   opacity : An array of 256 opacity values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 20 12:20:48 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtVolumeVariableLegend::SetLegendOpacities(const unsigned char *opacity)
{
    ((vtkVerticalScalarBarWithOpacityActor *)sBar)->SetLegendOpacities(opacity);
}
