/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                          avtOpenGLMoleculeRenderer.C                      //
// ************************************************************************* //

#include "avtOpenGLMoleculeRenderer.h"

#include <InvalidColortableException.h>
#include <LightList.h>
#include <MoleculeAttributes.h>
#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtViewInfo.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkUnsignedIntArray.h>
#include <ImproperUseException.h>

#include "AtomicProperties.h"
#include <LineAttributes.h>

#include <avtLookupTable.h>
#include "avtOpenGLAtomTexturer.h"

#include <DebugStream.h>
#include "matrix.c"


#ifndef VTK_IMPLEMENT_MESA_CXX
  #if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
    #include <OpenGL/gl.h>
  #else
    #if defined(_WIN32)
       #include <windows.h>
    #endif
    #include <GL/gl.h>
  #endif
#else
  #include <GL/gl.h>
#endif

#define SHORTEN_BONDS

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int sphere_quality_levels[4][2] = {
    {6,3},
    {12,6},
    {24,12},
    {48,24}
};

static int cylinder_quality_levels[4] = {
    3,
    5,
    9,
    17
};
 

// ****************************************************************************
//  Constructor: avtOpenGLMoleculeRenderer::avtOpenGLMoleculeRenderer
//
//  Purpose:
//    Initialize the texture memory and the OpenGL texture ID.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  3, 2006
//
//  Modifications:
//    Brad Whitlock, Mon Mar 27 14:51:24 PST 2006
//    Added tex.
//
// ****************************************************************************
avtOpenGLMoleculeRenderer::avtOpenGLMoleculeRenderer()
{
    immediatemode = true;
    displaylistid = 0;

    colortablename="";
    numcolors=0;
    colors=NULL;
    ct_is_discrete=false;
    levelsLUT = 0;

    spheres_calculated = false;
    cylinders_calculated = false;

    tex = (void *)new avtOpenGLAtomTexturer;
}


// ****************************************************************************
//  Destructor: avtOpenGLMoleculeRenderer::~avtOpenGLMoleculeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  3, 2006
//
//  Modifications:
//    Brad Whitlock, Mon Mar 27 14:51:24 PST 2006
//    Added tex.
//
// ****************************************************************************
avtOpenGLMoleculeRenderer::~avtOpenGLMoleculeRenderer()
{
    if (colors)
        delete[] colors;
    colors = NULL;

    if (displaylistid != 0)
    {
        glDeleteLists(displaylistid, 1);
        displaylistid = 0;
    }

    delete ((avtOpenGLAtomTexturer *)tex);
}



// ****************************************************************************
//  Method:  avtOpenGLMoleculeRenderer::InvalidateColors
//
//  Purpose:
//    
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
void
avtOpenGLMoleculeRenderer::InvalidateColors()
{
    numcolors = 0;
    if (colors)
        delete[] colors;
    colors = NULL;
    colortablename = "";

    if (displaylistid != 0)
    {
        glDeleteLists(displaylistid, 1);
        displaylistid = 0;
    }
}

// ****************************************************************************
// Method: avtOpenGLMoleculeRenderer::SetLevelsLUT
//
// Purpose: 
//   Sets a lookup table to be used for resseq coloring.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 29 11:58:51 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLMoleculeRenderer::SetLevelsLUT(avtLookupTable *lut)
{
    levelsLUT = lut;
}

// ****************************************************************************
//  Method:  SetColor3ubv
//
//  Purpose:
//    Okay, so it's a little silly right now, but there was some
//    debate about using glMaterialColor(...), so having things call
//    this instead of the GL call directly made trying it easier.
//    It's staying because I might still change my mind.
//
//  Arguments:
//    c          the pointer to the uchar color
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
static inline void
SetColor3ubv(const unsigned char *c)
{
    glColor3ubv(c);
}

// ****************************************************************************
//  Method: avtOpenGLMoleculeRenderer::DrawAtomsAsSpheres
//
//  Purpose:
//    Draw the atoms as spheres.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  3, 2006
//
//  Modifications:
//    Brad Whitlock, Mon Mar 27 17:39:40 PST 2006
//    Added imposter rendering.
//
//    Jeremy Meredith, Mon Aug 28 18:18:17 EDT 2006
//    Bonds are now line segments cells, and atoms are both points and
//    vertex cells.  This means we cannot look at cell data when looking
//    for atom arrays.  Also, account for model number directory prefix.
//
// ****************************************************************************

void
avtOpenGLMoleculeRenderer::DrawAtomsAsSpheres(vtkPolyData *data,
                                              const MoleculeAttributes &atts)
{
    vtkPoints *points = data->GetPoints();

    vtkDataArray *primary = data->GetPointData()->GetScalars();
    if (!primary)
    {
        // Let's just assume we don't want to plot the spheres for
        // a cell-centered variable
        return;
    }
    if (primary && !primary->IsA("vtkFloatArray"))
    {
        debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
        return;
    }
    float *scalar = (float*)primary->GetVoidPointer(0);

    string primaryname = primary->GetName();
    bool primary_is_element = (primaryname == "element" ||
                    (primaryname.length() > 8 &&
                     primaryname.substr(primaryname.length()-8) == "/element"));
    bool primary_is_resseq = (primaryname == "resseq" ||
                    (primaryname.length() > 7 &&
                     primaryname.substr(primaryname.length()-7) == "/resseq"));
    bool primary_is_restype = (primaryname == "restype" ||
                    (primaryname.length() > 8 &&
                     primaryname.substr(primaryname.length()-8) == "/restype"));

    vtkDataArray *element = primary_is_element ? primary : 
                                   data->GetPointData()->GetArray("element");
    if (element && !element->IsA("vtkFloatArray"))
    {
        debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
        return;
    }
    float *elementnos = element ? (float*)element->GetVoidPointer(0) : NULL;


    bool color_by_levels = (primary_is_element ||
                            primary_is_restype ||
                            primary_is_resseq)     && ct_is_discrete;
    bool color_by_element = color_by_levels && primary_is_element;

    float *radiusvar = NULL;
    bool sbv  = atts.GetScaleRadiusBy() == MoleculeAttributes::Variable;
    bool sbar = atts.GetScaleRadiusBy() == MoleculeAttributes::Atomic;
    bool sbcr = atts.GetScaleRadiusBy() == MoleculeAttributes::Covalent;
    float radiusscale = atts.GetRadiusScaleFactor();

    if (sbv)
    {
        if (atts.GetRadiusVariable() == "default")
            radiusvar = scalar;
        else
        {
            vtkDataArray *radius_array = data->GetPointData()->GetArray(
                                          atts.GetRadiusVariable().c_str());
            if (!radius_array)
            {
                // This shouldn't have gotten this far if it couldn't
                // read the variable like we asked.
                EXCEPTION1(ImproperUseException, "Couldn't read radius variable");
            }
            if (radius_array && !radius_array->IsA("vtkFloatArray"))
            {
                debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
                return;
            }
            radiusvar = (float*)radius_array->GetVoidPointer(0);
        }
    }

    //
    // Do initial setup for imposter rendering.
    //
    float ptOffsets[4][3] = {
        {-1., -1., 0.}, {1., -1., 0.},{1., 1., 0.}, {-1., 1., 0.}
    };
    const float texCoords[4][2] = {{0., 0.}, {1.,0.}, {1.,1.}, {0.,1.}};
    bool imposter = atts.GetDrawAtomsAs() == MoleculeAttributes::ImposterAtoms;
    int vmode = 0;
    if(imposter)
    {
        // If we're doing imposter rendering then set up sphere texturing
        // and disable lighting.
        ((avtOpenGLAtomTexturer *)tex)->BeginSphereTexturing();
        glDisable(GL_LIGHTING);

        // Based on the selected sphere texturing mode, there might be a
        // little more setup required.
        vmode = (((avtOpenGLAtomTexturer *)tex)->GetMode() ==
                avtOpenGLAtomTexturer::TextureMode) ? 1 : 2;
        if(vmode == 1)
        {
            float modelview[4][4], imv[4][4];
            glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)modelview);
            // Zero out the translation.
            modelview[3][0] = 0.;
            modelview[3][1] = 0.;
            modelview[3][2] = 0.;
            // Invert the matrix so we get the inverse rotations needed.
            matrix_invert(modelview, imv);
            // Multiply the imv times the ptOffsets.
            matrix_mul_point(ptOffsets[0], ptOffsets[0], imv);
            matrix_mul_point(ptOffsets[1], ptOffsets[1], imv);
            matrix_mul_point(ptOffsets[2], ptOffsets[2], imv);
            matrix_mul_point(ptOffsets[3], ptOffsets[3], imv);
        }
    }

    glBegin(GL_QUADS);
    for (int i=0; i<data->GetNumberOfPoints(); i++)
    {
        int element_number = 0;
        if (element)
            element_number = int(elementnos[i]-1);

        if (element_number < 0 || element_number >= MAX_ELEMENT_NUMBER)
            element_number = MAX_ELEMENT_NUMBER-1;

        // Determine radius
        float radius = atts.GetRadiusFixed();
        if (element && sbar)
            radius = atomic_radius[element_number] * radiusscale;
        else if (element && sbcr)
            radius = covalent_radius[element_number] * radiusscale;
        else if (radiusvar && sbv)
            radius = radiusvar[i] * radiusscale;

        // Determine color
        if (color_by_element)
        {
            int level = element_number % numcolors;
            SetColor3ubv(&colors[4*level]);
        }
        else if (color_by_levels)
        {
            int level = int(scalar[i]) - (primary_is_resseq ? 1 : 0);
            if(levelsLUT != 0)
            {
                const unsigned char *rgb = 
                    levelsLUT->GetLookupTable()->MapValue(level);
                SetColor3ubv(rgb);
            }
            else
            {
                level = level % numcolors;
                SetColor3ubv(&colors[4*level]);
            }
        }
        else
        {
            float alpha;
            if (varmax == varmin)
                alpha = 0.5;
            else
                alpha = (scalar[i] - varmin) / (varmax - varmin);
            
            int color = int((float(numcolors)-.01) * alpha);
            if (color < 0)
                color = 0;
            if (color > numcolors-1)
                color = numcolors-1;
            SetColor3ubv(&colors[4*color]);
        }

        if(vmode == 2)
        {
            // Plot squares. Use the same point for each of the vertices;
            // a vertex program will figure out the real coordinates.
            glNormal3f(radius, radius, radius);
            for(int j = 0; j < 4; ++j)
            {
                glVertex3dv(points->GetPoint(i));
                glTexCoord2fv(texCoords[j]);
            }
        }
        else if(vmode == 1)
        {
            // Plot squares
            for(int j = 0; j < 4; ++j)
            {
                double *pt = points->GetPoint(i);
                float vert[3];
                vert[0] = pt[0] + ptOffsets[j][0] * radius;
                vert[1] = pt[1] + ptOffsets[j][1] * radius;
                vert[2] = pt[2] + ptOffsets[j][2] * radius;
                glVertex3fv(vert);
                glTexCoord2fv(texCoords[j]);
            }
        }
        else
        {
            // Plot spheres
            double *pt = points->GetPoint(i);
            DrawSphereAsQuads(pt[0],
                              pt[1],
                              pt[2],
                              radius,
                              atts.GetAtomSphereQuality());
        }
    }
    glEnd();

    // If we're imposter rendering then enable lighting once more and turn
    // off sphere texturing.
    if(imposter)
    {
        ((avtOpenGLAtomTexturer *)tex)->EndSphereTexturing();
        glEnable(GL_LIGHTING);
    }
}

// ****************************************************************************
//  Method: avtOpenGLMoleculeRenderer::DrawBonds
//
//  Purpose:
//    Draw the bonds, either as line segments or as cylinders.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  3, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 18:18:17 EDT 2006
//    Bonds are now line segments cells, and atoms are both points and
//    vertex cells.  This means we cannot look at cell data when looking
//    for atom arrays.  Also, account for model number directory prefix.
//
//    Jeremy Meredith, Wed Apr 18 10:23:53 EDT 2007
//    Don't shorten bonds if we're not drawing the atoms.
//
//    Jeremy Meredith, Fri Oct 17 12:20:25 EDT 2008
//    Small cleanups for shortened-bond rendering:
//    - Don't shorten bonds if we're drawing as normal geometry, either;
//      at very small atom sizes (e.g. bond radius==atom radius), it's not
//      what we want, and at larger sizes it shouldn't matter.  It's still
//      necessary for imposter atoms, though.
//    - If the radius of an atom is bigger than half the bond length, don't
//      draw that half of the bond. If we did, then we'd have that section
//      overlap when we're shortening bonds.
//    - Don't shorten the bonds if it's a cell-centered variable, because
//      in that case we're not drawing the atoms at all.
//
// ****************************************************************************

void
avtOpenGLMoleculeRenderer::DrawBonds(vtkPolyData *data,
                                     const MoleculeAttributes &atts)
{
    vtkPoints *points = data->GetPoints();
    int numverts = data->GetNumberOfVerts();
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();

    bool primary_is_cell_centered = false;
    vtkDataArray *primary = data->GetPointData()->GetScalars();
    if (!primary)
    {
        primary = data->GetCellData()->GetScalars();
        primary_is_cell_centered = true;
    }
    if (!primary)
    {
        // eh? no variable at all?  that's a logic error....
        EXCEPTION1(ImproperUseException, "Expected a variable of some sort.");
    }
    if (primary && !primary->IsA("vtkFloatArray"))
    {
        debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
        return;
    }
    float *scalar = (float*)primary->GetVoidPointer(0);

    string primaryname = primary->GetName();
    bool primary_is_element = (primaryname == "element" ||
                    (primaryname.length() > 8 &&
                     primaryname.substr(primaryname.length()-8) == "/element"));
    bool primary_is_resseq = (primaryname == "resseq" ||
                    (primaryname.length() > 7 &&
                     primaryname.substr(primaryname.length()-7) == "/resseq"));
    bool primary_is_restype = (primaryname == "restype" ||
                    (primaryname.length() > 8 &&
                     primaryname.substr(primaryname.length()-8) == "/restype"));

    vtkDataArray *element = primary_is_element ? primary : 
                                   data->GetPointData()->GetArray("element");
    if (element && !element->IsA("vtkFloatArray"))
    {
        debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
        return;
    }
    float *elementnos = element ? (float*)element->GetVoidPointer(0) : NULL;

    bool color_by_levels = (primary_is_element ||
                            primary_is_restype ||
                            primary_is_resseq)     && ct_is_discrete;
    bool color_by_element = color_by_levels && primary_is_element;

    //
    // Get radius variable
    //
#ifdef SHORTEN_BONDS
    float *radiusvar = NULL;
    bool sbv  = atts.GetScaleRadiusBy() == MoleculeAttributes::Variable;
    bool sbar = atts.GetScaleRadiusBy() == MoleculeAttributes::Atomic;
    bool sbcr = atts.GetScaleRadiusBy() == MoleculeAttributes::Covalent;
    float radiusscale = atts.GetRadiusScaleFactor();

    if (sbv)
    {
        if (atts.GetRadiusVariable() == "default")
        {
            if (primary_is_cell_centered)
            {
                EXCEPTION1(ImproperUseException,
                           "Radius variable cannot be cell-centered.");
            }
            radiusvar = scalar;
        }
        else
        {
            vtkDataArray *radius_array = data->GetPointData()->GetArray(
                                          atts.GetRadiusVariable().c_str());
            if (!radius_array)
            {
                // This shouldn't have gotten this far if it couldn't
                // read the variable like we asked.
                EXCEPTION1(ImproperUseException, "Couldn't read radius variable");
            }
            if (radius_array && !radius_array->IsA("vtkFloatArray"))
            {
                debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
                return;
            }
            radiusvar = (float*)radius_array->GetVoidPointer(0);
        }
    }
#endif

    if (atts.GetDrawBondsAs() == MoleculeAttributes::CylinderBonds)
    {
        glBegin(GL_QUADS);
    }
    else //  (atts.GetDrawBondsAs() == MoleculeAttributes::Wireframe)
    {
        glDisable(GL_LIGHTING);
        glLineWidth(LineWidth2Int(Int2LineWidth(atts.GetBondLineWidth())));
        if (atts.GetBondLineStyle() != 0)
        {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, LineStyle2StipplePattern(Int2LineStyle(
                                                  atts.GetBondLineStyle())));
        }
        else
        {
            glDisable(GL_LINE_STIPPLE);
        }
            
        glBegin(GL_LINES);
    }

    int *segptr = segments;
    for (int i=0; i<data->GetNumberOfLines(); i++)
    {
        if (*segptr == 2)
        {
            int v0 = *(segptr+1);
            int v1 = *(segptr+2);

            double pt_0[3];
            double pt_1[3];
            points->GetPoint(v0, pt_0);
            points->GetPoint(v1, pt_1);

            double pt_mid[3] = {(pt_0[0]+pt_1[0])/2.,
                                (pt_0[1]+pt_1[1])/2.,
                                (pt_0[2]+pt_1[2])/2.};

#ifdef SHORTEN_BONDS
            double dpt[3] = {pt_1[0]-pt_0[0], pt_1[1]-pt_0[1], pt_1[2]-pt_0[2]};
            double dptlen = sqrt(dpt[0]*dpt[0]+dpt[1]*dpt[1]+dpt[2]*dpt[2]);
            vtkMath::Normalize(dpt);
#endif

            for (int half=0; half<=1; half++)
            {
                int atom  = (half==0) ? v0 : v1;
                double *pt_a = (half==0) ? pt_0 : pt_mid;
                double *pt_b = (half==0) ? pt_mid : pt_1;

                int element_number = 0;
                if (element)
                {
                    element_number = int(elementnos[atom]-1);
                }

                if (element_number < 0 || element_number >= MAX_ELEMENT_NUMBER)
                    element_number = MAX_ELEMENT_NUMBER-1;

#ifdef SHORTEN_BONDS
                if (!primary_is_cell_centered &&
                    atts.GetDrawAtomsAs() == MoleculeAttributes::ImposterAtoms)
                {
                    // Determine radius
                    float atom_radius = atts.GetRadiusFixed();
                    if (element && sbar)
                        atom_radius = atomic_radius[element_number] * radiusscale;
                    else if (element && sbcr)
                        atom_radius = covalent_radius[element_number] * radiusscale;
                    else if (radiusvar && sbv)
                        atom_radius = radiusvar[i] * radiusscale;

                    if (atom_radius > dptlen/2.)
                        continue;

                    const float fudge = 0.9;

                    if (half == 0)
                    {
                        pt_a[0] += atom_radius * dpt[0] * fudge;
                        pt_a[1] += atom_radius * dpt[1] * fudge;
                        pt_a[2] += atom_radius * dpt[2] * fudge;
                    }
                    else
                    {
                        pt_b[0] -= atom_radius * dpt[0] * fudge;
                        pt_b[1] -= atom_radius * dpt[1] * fudge;
                        pt_b[2] -= atom_radius * dpt[2] * fudge;
                    }
                }
#endif

                float radius = atts.GetBondRadius();

                // Determine color
                if (atts.GetColorBonds() == MoleculeAttributes::SingleColor)
                {
                    SetColor3ubv(atts.GetBondSingleColor().GetColor());
                }
                else // (atts.GetColorBonds() == MoleculeAttributes::ColorByAtom)
                {
                    float scalarval;
                    if (primary_is_cell_centered)
                        scalarval = scalar[i + numverts];
                    else
                        scalarval = scalar[atom];

                    if (color_by_element)
                    {
                        int level = element_number % numcolors;
                        SetColor3ubv(&colors[4*level]);
                    }
                    else if (color_by_levels)
                    {
                        int level = int(scalarval) - (primary_is_resseq ? 1 : 0);
                        if(levelsLUT != 0)
                        {
                            const unsigned char *rgb = 
                            levelsLUT->GetLookupTable()->MapValue(level);
                            SetColor3ubv(rgb);
                        }
                        else
                        {
                            level = level % numcolors;
                            SetColor3ubv(&colors[4*level]);
                        }
                    }
                    else
                    {
                        float alpha;
                        if (varmax == varmin)
                            alpha = 0.5;
                        else
                            alpha = (scalarval - varmin) / (varmax - varmin);
            
                        int color = int((float(numcolors)-.01) * alpha);
                        if (color < 0)
                            color = 0;
                        if (color > numcolors-1)
                            color = numcolors-1;
                        SetColor3ubv(&colors[4*color]);
                    }
                }

                if (atts.GetDrawBondsAs() == MoleculeAttributes::CylinderBonds )
                {
                    DrawCylinderBetweenTwoPoints(pt_a, pt_b, radius,
                                                 atts.GetBondCylinderQuality());
                }
                else // == MoleculeAttributes::Wireframe
                {
                    glVertex3dv(pt_a);
                    glVertex3dv(pt_b);
                }
            }
        }

        segptr += (*segptr) + 1;
    }
    glEnd();
}



// ****************************************************************************
//  Method:  avtOpenGLMoleculeRenderer::Render
//
//  Purpose:
//    Render one image
//
//  Arguments:
//    ds      : the data set to render
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  3, 2006
//
//  Modifications:
//    Brad Whitlock, Mon Mar 27 14:56:06 PST 2006
//    I made sure that we use immediate mode when we need to when we use 
//    imposter rendering,
//
// ****************************************************************************

void
avtOpenGLMoleculeRenderer::Render(vtkPolyData *data,
                                 const MoleculeAttributes &atts,
                                 bool immediateModeRendering,
                                 float _varmin, float _varmax,
                                 float _ambient_coeff,
                                 float _spec_coeff, float _spec_power,
                                 float _spec_r, float _spec_g, float _spec_b,
                                 const int *winsize)
{
    if (!data->GetCellData()->GetScalars() &&
        !data->GetPointData()->GetScalars())
        return;

    // If we're drawing imposter quads then we need to be in immediate mode
    // for certain imposter methods.
    if(atts.GetDrawAtomsAs() == MoleculeAttributes::ImposterAtoms)
    {
        if( ((avtOpenGLAtomTexturer *)tex)->GetMode() ==
            avtOpenGLAtomTexturer::TextureMode)
        {
            immediateModeRendering = true;
        }
    }

    if (immediateModeRendering)
    {
        if (displaylistid != 0)
        {
            glDeleteLists(displaylistid, 1);
            displaylistid = 0;
        }
    }
    else
    {
        // Check to see if we need to regenerate lists
        if (displaylistid != 0 &&
            atts_for_displaylist != atts)
        {
            glDeleteLists(displaylistid, 1);
            displaylistid = 0;
        }

        // If the list we have is valid, just call it
        if (displaylistid != 0)
        {
            glCallList(displaylistid);
            return;
        }

        atts_for_displaylist = atts;
        displaylistid = glGenLists(1);
        glNewList(displaylistid, GL_COMPILE);
    }

    varmin = _varmin;
    varmax = _varmax;

    immediatemode = immediateModeRendering;
    ambient_coeff = _ambient_coeff;
    spec_coeff    = _spec_coeff;
    spec_power    = _spec_power;
    spec_r        = _spec_r;
    spec_g        = _spec_g;
    spec_b        = _spec_b;

    glPushAttrib(GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT |
                 GL_ENABLE_BIT);
    glDisable(GL_BLEND);

    glEnable(GL_LIGHTING);

#if 0
    glDisable(GL_COLOR_MATERIAL);
#else
    float diff[] = {1,1,1,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
    if (ambient_coeff == 0)
    {
        float amb[] = {ambient_coeff, ambient_coeff, ambient_coeff, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    }
    else
    {
        float amb[] = {ambient_coeff, ambient_coeff, ambient_coeff, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    }
    glEnable(GL_COLOR_MATERIAL);
#endif
    float spec[] = {spec_r * spec_coeff,
                    spec_g * spec_coeff,
                    spec_b * spec_coeff,
                    1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &spec_power);
    glShadeModel(GL_SMOOTH);

    TRY
    {
        SetColors(data, atts);
        bool doBonds = atts.GetDrawBondsAs() != MoleculeAttributes::NoBonds;

        if (atts.GetDrawAtomsAs() == MoleculeAttributes::SphereAtoms)
        {
            DrawAtomsAsSpheres(data,atts);
        }
        else if(atts.GetDrawAtomsAs() == MoleculeAttributes::ImposterAtoms)
        {
            // Set some hints in the imposter atom texturer.
            ((avtOpenGLAtomTexturer *)tex)->SetHint(
                 avtOpenGLAtomTexturer::HINT_SET_DEPTH, doBonds ? 0 : 1);
            ((avtOpenGLAtomTexturer *)tex)->SetHint(
                 avtOpenGLAtomTexturer::HINT_SET_SCREEN_WIDTH, winsize[0]);
            ((avtOpenGLAtomTexturer *)tex)->SetHint(
                 avtOpenGLAtomTexturer::HINT_SET_SCREEN_HEIGHT, winsize[1]);

            DrawAtomsAsSpheres(data,atts);
        }

        if (doBonds)
        {
            DrawBonds(data,atts);
        }
    }
    CATCH2(VisItException, e)
    {
        // Note: we should not need to intercept this here, except
        // that it's otherwise uncaught, as of version 1.5.
        // Also note: if we decide to let the exception reach up
        // the call chain, we STILL need at least catch it here,
        // make sure we've cleaned up OpenGL correctly
        // (e.g. popattrib and endlist), and then re-throw.
        avtCallback::IssueWarning(e.Message().c_str());
    }
    ENDTRY

    glPopAttrib();


    if (immediateModeRendering)
    {
    }
    else
    {
        glEndList();
        glCallList(displaylistid);
    }
}


// ****************************************************************************
//  Method:  avtOpenGLMoleculeRenderer::SetColors
//
//  Purpose:
//    Set up the color table.
//
//  Arguments:
//    data    : the data set to render
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 14, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 18:18:17 EDT 2006
//    Bonds are now line segments cells, and atoms are both points and
//    vertex cells.  This means we cannot look at cell data when looking
//    for atom arrays.  Also, account for model number directory prefix.
//
// ****************************************************************************

void
avtOpenGLMoleculeRenderer::SetColors(vtkPolyData *data,
                                     const MoleculeAttributes &atts)
{
    int new_numcolors = -1;

    avtColorTables *ct = avtColorTables::Instance();
    const int opacity = 255;

    //
    // Detect if we're using the default color table or a color table
    // that does not exist anymore.
    //
    string new_colortablename;
    string varName = "";
    if (data->GetPointData()->GetScalars())
    {
        varName = data->GetPointData()->GetScalars()->GetName();
    }
    else if (data->GetCellData()->GetScalars())
    {
        varName = data->GetCellData()->GetScalars()->GetName();
    }

    if (varName == "")
    {
        new_colortablename = "";
        new_numcolors = 0;
    }
    else if (varName == "element" ||
             (varName.length()>8 && varName.substr(varName.length()-8)=="/element"))
    {
        new_colortablename = atts.GetElementColorTable();
        if (new_colortablename == "Default")
            new_colortablename = string(ct->GetDefaultDiscreteColorTable());

        new_numcolors = 110;
    }
    else if (varName == "resseq" ||
             (varName.length()>7 && varName.substr(varName.length()-7)=="/resseq"))
    {
        new_colortablename = atts.GetResidueSequenceColorTable();
        if (new_colortablename == "Default")
            new_colortablename = string(ct->GetDefaultDiscreteColorTable());

        new_numcolors = 256;
    }
    else if (varName == "restype" ||
             (varName.length()>8 && varName.substr(varName.length()-8)=="/restype"))
    {
        new_colortablename = atts.GetResidueTypeColorTable();
        if (new_colortablename == "Default")
            new_colortablename = string(ct->GetDefaultDiscreteColorTable());

        new_numcolors = NumberOfKnownResidues();
    }
    else
    {
        new_colortablename = atts.GetContinuousColorTable();
        if (new_colortablename == "Default")
            new_colortablename = string(ct->GetDefaultContinuousColorTable());

        new_numcolors = 256;
    }

    if (!ct->ColorTableExists(new_colortablename.c_str()))
    {
        EXCEPTION1(InvalidColortableException, new_colortablename);
    }

    //
    // Skip the work if we don't need to do it!
    //
    if (new_numcolors      == numcolors   &&
        new_colortablename == colortablename)
    {
        return;
    } 

    numcolors      = new_numcolors;
    colortablename = new_colortablename;

    if (colors)
        delete[] colors;
    
    colors = new unsigned char[numcolors * 4];
    unsigned char *cptr = colors;

    //
    // Add a color for each subset name.
    //
    if (ct->IsDiscrete(colortablename.c_str()))
    {
        // The CT is discrete, get its color color control points.
        ct_is_discrete = true;
        for (int i = 0; i < numcolors; ++i)
        {
            unsigned char rgb[3] = {0,0,0};
            ct->GetControlPointColor(colortablename.c_str(), i, rgb);
            *cptr++ = rgb[0];
            *cptr++ = rgb[1];
            *cptr++ = rgb[2];
            *cptr++ = opacity;
        }
    }
    else
    {
        // The CT is continuous, sample the CT so we have a unique color
        // for each element.
        ct_is_discrete = false;
        unsigned char *rgb = ct->GetSampledColors(colortablename.c_str(), numcolors);
        if (rgb)
        {
            for (int i = 0; i < numcolors; ++i)
            {
                int j = i * 3;
                *cptr++ = rgb[j];
                *cptr++ = rgb[j+1];
                *cptr++ = rgb[j+2];
                *cptr++ = opacity;
            }
            delete [] rgb;
        }
    }
}

// ****************************************************************************
//  Method:  avtOpenGLMoleculeRenderer::CalculateSpherePts
//
//  Purpose:
//    Precalculate points for sphere geometry.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//
// ****************************************************************************

void avtOpenGLMoleculeRenderer::CalculateSpherePts()
{
    if (spheres_calculated)
        return;

    spheres_calculated = true;

    for (int detail=0; detail<MAX_DETAIL_LEVELS; detail++)
    {
        int qdetail = sphere_quality_levels[detail][0];
        int hdetail = sphere_quality_levels[detail][1];
        sphere_pts[detail] = new float[(qdetail+1)*(hdetail+1)*3];

        for (int a=0; a<=qdetail; a++)
        {
            float theta = 2*M_PI * float(a) / float(qdetail);
            for (int b=0; b<=hdetail; b++)
            {
                float phi = M_PI * float(b) / float(hdetail);

                float dx = sin(phi) * cos(theta);
                float dy = sin(phi) * sin(theta);
                float dz = cos(phi);

                sphere_pts[detail][(a * (hdetail+1) + b)*3 + 0] = dx;
                sphere_pts[detail][(a * (hdetail+1) + b)*3 + 1] = dy;
                sphere_pts[detail][(a * (hdetail+1) + b)*3 + 2] = dz;
            }
        }
    }
}

// ****************************************************************************
//  Method:  avtOpenGLMoleculeRenderer::CalculateCylPts
//
//  Purpose:
//    Precalculate points for cylinder geometry.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//
// ****************************************************************************

void avtOpenGLMoleculeRenderer::CalculateCylPts()
{
    if (cylinders_calculated)
        return;

    cylinders_calculated = true;

    for (int detail=0; detail<MAX_DETAIL_LEVELS; detail++)
    {
        int cdetail = cylinder_quality_levels[detail];
        cyl_pts[detail] = new float[(cdetail+1)*4];

        for (int b=0; b<=cdetail; b++)
        {
            float theta = 2*M_PI * float(b) / float(cdetail);

            float dx = cos(theta);
            float dy = sin(theta);
            float dz = 0;

            cyl_pts[detail][b*4+0] = dx;
            cyl_pts[detail][b*4+1] = dy;
            cyl_pts[detail][b*4+2] = dz;
            cyl_pts[detail][b*4+3] = 0;
        }
    }
}

// ****************************************************************************
//  Method:  avtOpenGLMoleculeRenderer::DrawSphereAsQuads
//
//  Purpose:
//    Make the OpenGL calls to draw a sphere with the
//    given center, radius, and detail level.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//
// ****************************************************************************
void
avtOpenGLMoleculeRenderer::DrawSphereAsQuads(float x0,
                                             float y0,
                                             float z0,
                                             float r,
                                             int detail)
{
    CalculateSpherePts();

    int qdetail = sphere_quality_levels[detail][0];
    int hdetail = sphere_quality_levels[detail][1];

    for (int a=0; a<qdetail; a++)
    {
        int a0 = a;
        int a1 = a+1;
        for (int b=0; b<hdetail; b++)
        {
            int b0 = b;
            int b1 = b+1;

            float *v00, *v01, *v10, *v11;

            v00 = &(sphere_pts[detail][(a0 * (hdetail+1) + b0)*3]);
            v01 = &(sphere_pts[detail][(a0 * (hdetail+1) + b1)*3]);
            v10 = &(sphere_pts[detail][(a1 * (hdetail+1) + b0)*3]);
            v11 = &(sphere_pts[detail][(a1 * (hdetail+1) + b1)*3]);

            glNormal3fv(v00);
            glVertex3f(x0 + r*v00[0], y0 + r*v00[1], z0 + r*v00[2]);

            glNormal3fv(v01);
            glVertex3f(x0 + r*v01[0], y0 + r*v01[1], z0 + r*v01[2]);

            glNormal3fv(v11);
            glVertex3f(x0 + r*v11[0], y0 + r*v11[1], z0 + r*v11[2]);

            glNormal3fv(v10);
            glVertex3f(x0 + r*v10[0], y0 + r*v10[1], z0 + r*v10[2]);
        }
    }
}

// ****************************************************************************
//  Method:  avtOpenGLMoleculeRenderer::DrawCylinderBetweenTwoPoints
//
//  Purpose:
//    Make the OpenGL calls to draw a cylinder with the given begin
//    and end points, radius, and detail level.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 18:25:02 EDT 2006
//    Point locations are now doubles, not floats.
//
// ****************************************************************************
void
avtOpenGLMoleculeRenderer::DrawCylinderBetweenTwoPoints(double *p0,
                                                        double *p1,
                                                        float r,
                                                        int detail)
{
    CalculateCylPts();

    float vc[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
    float va[3];
    float vb[3];

    float vc_len = vtkMath::Normalize(vc);
    if (vc_len == 0)
        return;

    vtkMath::Perpendiculars(vc, va,vb, 0);

    float v0[4];
    float v1[4];
    int cdetail = cylinder_quality_levels[detail];
    for (int b=0; b<cdetail; b++)
    {
        int b0 = b;
        int b1 = b+1;

        float *u0, *u1;
        u0 = &(cyl_pts[detail][b0*4]);
        u1 = &(cyl_pts[detail][b1*4]);

        v0[0] = va[0]*u0[0] + vb[0]*u0[1];
        v0[1] = va[1]*u0[0] + vb[1]*u0[1];
        v0[2] = va[2]*u0[0] + vb[2]*u0[1];

        v1[0] = va[0]*u1[0] + vb[0]*u1[1];
        v1[1] = va[1]*u1[0] + vb[1]*u1[1];
        v1[2] = va[2]*u1[0] + vb[2]*u1[1];

        glNormal3fv(v0);
        glVertex3f(p1[0] + r*v0[0], p1[1] + r*v0[1], p1[2] + r*v0[2]);

        glNormal3fv(v0);
        glVertex3f(p0[0] + r*v0[0], p0[1] + r*v0[1], p0[2] + r*v0[2]);

        glNormal3fv(v1);
        glVertex3f(p0[0] + r*v1[0], p0[1] + r*v1[1], p0[2] + r*v1[2]);

        glNormal3fv(v1);
        glVertex3f(p1[0] + r*v1[0], p1[1] + r*v1[1], p1[2] + r*v1[2]);
    }
}


