/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                             avtLevelsMapper.C                             //
// ************************************************************************* //

#include <avtLevelsMapper.h>

#include <vector>
#include <string>

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

#include <ColorAttribute.h>

#include <avtDatasetExaminer.h>
#include <avtTransparencyActor.h>

#include <vtkVisItDataSetMapper.h>

#include <BadIndexException.h>
#include <DebugStream.h>


using std::string;
using std::vector;

const double INV_255 = 0.0039215686274509803377;



// ****************************************************************************
//  Method: avtLevelsMapper constructor
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Jun 25 12:45:06 PDT 2001
//    Added intialization of stipplePattern.
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk.
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001 
//    Mapper no longer owns the lut, it must be set by the plot using
//    this mapper.  Initialize it to NULL.  Intialize new members
//    useConstantColor and constantColor.
//
//    Kathleen Bonnell, Mon Sep 24 16:02:31 PDT 2001 
//    No longer have lut and constColor members. 
//    
//    Kathleen Bonnell, Wed Sep 11 09:01:37 PDT 2002 
//    Initialize new member useColorMap. 
//    
//    Kathleen Bonnell, Tue Dec  3 16:10:40 PST 2002
//    Re-initialize lineWidth, based on new LineAttributes.
//
// ****************************************************************************

avtLevelsMapper::avtLevelsMapper()
{
    lineWidth = LW_0;
    lineStyle = SOLID; 
    useColorMap = false;
}


// ****************************************************************************
//  Method: avtLevelsMapper destructor
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 10:36:49 PDT 2001
//    Mapper no longer owns the lut, remove it's destruction from here.
//
//    Kathleen Bonnell, Mon Sep 24 17:29:25 PDT 2001 
//    Clear the levels lists. 
//
// ****************************************************************************

avtLevelsMapper::~avtLevelsMapper()
{
    levelNames.clear();
    labelsForColorMapping.clear();
}


// ****************************************************************************
//  Method: avtLevelsMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows the levels mapper to make any
//      calls that were made before the base class set up the vtk mappers
//      take effect.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  9 15:44:57 PST 2001
//    Set the actor's opacity from the lut.
//
//    Kathleen Bonnell, Thu Feb 15 10:06:43 PST 2001 
//    Correct the way opacity is retrieved from lut. 
//
//    Kathleen Bonnell, Wed Apr  4 11:46:13 PDT 2001 
//    Retrieve scalar range from input's cell data in order to determine
//    index into lut for retrieving opacity.  (Previously retrieved 
//    input's scalar range, which includes cell & point data). 
//
//    Hank Childs, Tue Apr 10 10:10:13 PDT 2001
//    Add some error checking for cases where an error happened upstream.
//
//    Kathleen Bonnell, Mon Jun 25 14:31:51 PDT 2001 
//    Set the actors' line stipple pattern. 
//
//    Kathleen Bonnell, Tue Aug 14 15:20:07 PDT 2001 
//    Only set the opacity if we can get the correct range value. 
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001
//    For constant colored plots, turn off mappers scalar visibility,
//    and set actor's color property.  Otherwise turn on scalar visibility
//    for mappers so that lut will be used to color the plot. 
//    
//    Kathleen Bonnell, Mon Sep 24 15:58:35 PDT 2001
//    This mapper now uses labels for determining proper color and 
//    opacity for actor.  Do not use vtkLookupTable.
//   
//    Hank Childs, Mon Jul  8 09:56:41 PDT 2002
//    Add support for transparency.
//
//    Mark C. Miller, Thu Jan 20 22:27:39 PST 2005
//    Passed opacity to InputWasModified
//
//    Hank Childs, Thu Dec  8 18:23:50 PST 2005
//    Reset the specular color when we're done, since the SetColor call
//    clobbers the specular light color, which in turn greatly diminishes
//    the specular effect.  ['5636] ['5580]
//
//    Hank Childs, Fri Dec 29 14:42:42 PST 2006
//    Tell the mapper whether or not we have a 3D scene.
//
//    Hank Childs, Thu Jul 24 09:52:33 PDT 2008
//    Prevent dereference and add exception. ['8690]
//
// ****************************************************************************

void
avtLevelsMapper::CustomizeMappers(void)
{
    double col[4];
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            mappers[i]->ScalarVisibilityOff();

            if (strcmp(mappers[i]->GetClassName(), "vtkVisItDataSetMapper")==0)
            {                 
                vtkVisItDataSetMapper *m = (vtkVisItDataSetMapper *)mappers[i];
                m->SetSceneIs3D(GetInput()->GetInfo().GetAttributes().
                                                    GetSpatialDimension() == 3);
            }

            //
            //  Use labels for mapping to a color.
            //
            if (labelsForColorMapping.size() <= i)
            {
                EXCEPTION1(VisItException, "An internal error occurred.  One common "
                           "way to get to this state is if you are resampling a "
                           "filled boundary plot, which is not allowed.");
            }
            GetLevelColor(labelsForColorMapping[i], col);
            vtkProperty* prop = actors[i]->GetProperty();
            double spec_color[4];
            if (prop->GetSpecular() > 0.)
                prop->GetSpecularColor(spec_color);
            prop->SetColor(col[0], col[1], col[2]);
            if (prop->GetSpecular() > 0.)
                prop->SetSpecularColor(spec_color);
            prop->SetOpacity(col[3]);
            prop->SetLineStipplePattern(LineStyle2StipplePattern(lineStyle));
            prop->SetLineWidth(LineWidth2Int(lineWidth));
 
            if (transparencyActor != NULL)
            {
                transparencyActor->InputWasModified(transparencyIndex, col[3]);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtLevelsMapper::SetLineWidth
//
//  Purpose:
//      Sets the line width of the plot.
//
//  Arguments:
//      w        The new line width
//
//  Programmer:  Hank Childs
//  Creation:    December 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk.
//
//
// ****************************************************************************

void
avtLevelsMapper::SetLineWidth(_LineWidth lw)
{
    lineWidth = lw; 

    if (actors == NULL)
    {
        //
        // This happens when SetLineWidth is called before the input is set.
        //
        return;
    }

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            vtkProperty *prop = actors[i]->GetProperty();
            prop->SetLineWidth(LineWidth2Int(lineWidth));
        }
    }
}


// ****************************************************************************
//  Method: avtLevelsMapper::SetLineStyle
//
//  Purpose:
//      Sets the line style for all the actors of plot.
//
//  Arguments:
//      s        The new line style
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 22, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Jun 21 16:33:54 PDT 2001
//    Enable setting of actor's stipple pattern.
//
//    Kathleen Bonnell, Mon Jun 25 14:31:51 PDT 2001 
//    Made stipplePattern a class member.
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk.
//
// ****************************************************************************

void
avtLevelsMapper::SetLineStyle(_LineStyle ls)
{
    lineStyle = ls; 
    if ( actors == NULL )
    {
        // this occurs when this method called before input is set.
        return;
    }

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->
                SetLineStipplePattern(LineStyle2StipplePattern(lineStyle));
        }
    }
}


// ****************************************************************************
//  Method: avtLevelsMapper::SetPointSize
//
//  Purpose:
//      Sets the point size for all the actors of plot.
//
//  Arguments:
//      s        The new point size
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 22, 2001
//
// ****************************************************************************

void
avtLevelsMapper::SetPointSize(double s)
{
    if ( actors == NULL )
    {
        // this occurs when this method called before input is set.
        return;
    }

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->SetPointSize(s);
        }
    }
}


// ****************************************************************************
//  Method: avtVariableMapper::GetOriginalDataRange
//
//  Purpose:
//      Gets the original range of the input.
//
//  Arguments:
//      rmin          The minimum in the range.
//      rmax          The maximum in the range.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 2, 2001. 
//
//  Modifications:
//
//    Hank Childs, Fri Jun 22 12:24:32 PDT 2001
//    Check for NULL data extents.
//
//    Hank Childs, Tue Sep  4 16:04:18 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Kathleen Bonnell, Wed Dec 22 16:36:29 PST 2004
//    Removed exception for variable dimension != 1, as all var extents now
//    contain two elements, regardless of dimension.
//
// ****************************************************************************

void
avtLevelsMapper::GetOriginalDataRange(double &rmin, double &rmax)
{
    if (mappers == NULL)
    {
        //
        // We have been asked for the range before the input has been set.
        //
        rmin = 0.;
        rmax = 0.;
        return;
    }

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    
    // we want the point data range
    double de[2];
    bool gotExtents = atts.GetDataExtents(de);
    if (gotExtents)
    {
        rmin = de[0];
        rmax = de[1];
    }
    else
    {
        rmin = 0.;
        rmax = 0.;
    }
}


// ****************************************************************************
//  Method: avtLevelsMapper::SetLabels
//
//  Purpose:
//    Sets the labels needed for color mapping. 
//
//  Arguments:
//    labels    The labels. 
//    fromTree  True if labels came from input data tree, false if labels
//              came from data object information. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 24, 2001 
//
// ****************************************************************************

void
avtLevelsMapper::SetLabels(vector<string> &labels, bool fromTree)
{
    if (fromTree)
    {
        //
        // These are the actual levels present in the plot,
        // use them to determine color.  The size of this list
        // should correspond to the number of mappers.
        //
        if (!labelsForColorMapping.empty())
        {
            labelsForColorMapping.clear();
        }
        labelsForColorMapping = labels;
    }
    else 
    {
        //
        // These are all the levels possible.   They correspond to
        // the colors set in method SetColors, and serve as a mapping
        // to retrieve the correct color.  The size of this list
        // should be equivalent to numColors when it comes time to
        // retrieve the colors. 
        //
        if (!levelNames.empty())
        {
            levelNames.clear();
        }
        levelNames = labels; 
    }
}


// ****************************************************************************
// Method: avtLevelsMapper::SetColors
//
// Purpose: 
//   Set the color attribute list used for colormapping levels. 
//
// Arguments:
//   c         The new color attribute list. 
//
// Programmer: Kathleen Bonnell 
// Creation:   September 24, 2001 
//
// Modifications:
//
//    Hank Childs, Tue Jul  9 08:44:00 PDT 2002
//    Add support for transparency.
//
//    Mark C. Miller, Thu Jan 20 22:27:39 PST 2005
//    Passed opacity to InputWasModified
//
//    Hank Childs, Thu Dec  8 18:23:50 PST 2005
//    Reset the specular color when we're done, since the SetColor call
//    clobbers the specular light color, which in turn greatly diminishes
//    the specular effect.  ['5636] ['5580]
//
//    Hank Childs, Thu Jul 24 09:52:33 PDT 2008
//    Prevent dereference and add exception. ['8690]
//
// ****************************************************************************

void
avtLevelsMapper::SetColors(const ColorAttributeList &c)
{
    cal = c;
    double col[4];
    for (int i = 0; i < nMappers; i++)
    {
        if (mappers[i] != NULL)
        {
            if (labelsForColorMapping.size() <= i)
            {
                EXCEPTION1(VisItException, "An internal error occurred.  One common "
                           "way to get to this state is if you are resampling a "
                           "filled boundary plot, which is not allowed.");
            }
            GetLevelColor(labelsForColorMapping[i], col);
            vtkProperty* prop = actors[i]->GetProperty();
            double spec_color[4];
            if (prop->GetSpecular() > 0.)
                prop->GetSpecularColor(spec_color);
            prop->SetColor(col[0], col[1], col[2]);
            if (prop->GetSpecular() > 0.)
                prop->SetSpecularColor(spec_color);
            prop->SetOpacity(col[3]);

            if (transparencyActor != NULL)
            {
                transparencyActor->InputWasModified(transparencyIndex, col[3]);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtLevelsMapper::GetLevelColor
//
//  Purpose: 
//    Retrieves the color associated with name. 
//
//  Arguments:
//    name      The level name. 
//    col       A place to store the color. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 24, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 11 09:01:37 PDT 2002   
//    Add code to use a label-to-color-index map if specified.  It allows more
//    consistent mapping for labels to their corresponding colors.
//
//    Brad Whitlock, Fri Nov 15 10:08:24 PDT 2002
//    Changed the map type.
//
//    Brad Whitlock, Tue Mar 13 11:19:16 PDT 2007
//    Changed due to code generation.
//
// ****************************************************************************

void
avtLevelsMapper::GetLevelColor(const string &name, double col[4])
{
    int nc = cal.GetNumColors();
    if (nc == 1)  // constant color for all levels
    {
        col[0] = cal[0].Red()   * INV_255;
        col[1] = cal[0].Green() * INV_255;
        col[2] = cal[0].Blue()  * INV_255;
        col[3] = cal[0].Alpha() * INV_255;
        return;
    }

    int index = -1;

    if (useColorMap)
    {
        //
        //  Find the correct index into the ColorAttributeList by finding
        //  the label in the label-to-color map.
        //
        LevelColorMap::iterator it = labelColorMap.find(name);
        if (it != labelColorMap.end())
        {
            index = it->second;
        }
    }
    else
    {
        //
        //  Find the correct index into the ColorAttributeList by comparing
        //  name against the levelNames.
        //
        for (int i = 0; i < levelNames.size(); i++)
        {
            if (name == levelNames[i])
            {
                index = i;
                break;
            }
        }
    }

    if (index < 0 || index >= nc)
    {
        EXCEPTION2(BadIndexException, index, nc); 
    }

    col[0] = cal[index].Red()   * INV_255;
    col[1] = cal[index].Green() * INV_255;
    col[2] = cal[index].Blue()  * INV_255;
    col[3] = cal[index].Alpha() * INV_255;
}



// ****************************************************************************
// Method: avtLevelsColorMap::GetLevelColor
//
// Purpose: 
//   Retrieves the color associated with levelNum. 
//
// Arguments:
//   levelNum  The level number. 
//   col       A place to store the color. 
//
// Programmer: Kathleen Bonnell 
// Creation:   September 24, 2001 
//
// Modifications:
//   Brad Whitlock, Tue Mar 13 11:19:47 PDT 2007
//   Changed due to code generation.
//
// ****************************************************************************

void
avtLevelsMapper::GetLevelColor(const int levelNum, double col[4])
{
    int nc = cal.GetNumColors();
    if (nc == 1)  // constant color for all levels
    {
        col[0] = cal[0].Red()   * INV_255;
        col[1] = cal[0].Green() * INV_255;
        col[2] = cal[0].Blue()  * INV_255;
        col[3] = cal[0].Alpha() * INV_255;
        return;
    }

    if (levelNum < 0 || levelNum >= nc)
    {
        EXCEPTION2(BadIndexException, levelNum, nc); 
    }
 
    col[0] = cal[levelNum].Red()   * INV_255;
    col[1] = cal[levelNum].Green() * INV_255;
    col[2] = cal[levelNum].Blue()  * INV_255;
    col[3] = cal[levelNum].Alpha() * INV_255;
}


// ****************************************************************************
//  Method: avtLevelsMapper::SetLabelColorMap
//
//  Purpose: 
//    Set the map for mapping level to color index. 
//
//  Arguments:
//    cmap      The new color map. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 11, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Nov 15 10:08:54 PDT 2002
//    I changed the argument type.
//
// ****************************************************************************

void 
avtLevelsMapper::SetLabelColorMap(const LevelColorMap &cmap)
{
    useColorMap = true;
    labelColorMap = cmap;
}

