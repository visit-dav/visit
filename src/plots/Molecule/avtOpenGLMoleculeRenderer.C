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
// ****************************************************************************

void
avtOpenGLMoleculeRenderer::DrawAtomsAsSpheres(vtkPolyData *data,
                                              const MoleculeAttributes &atts)
{
    vtkPoints *points = data->GetPoints();
    const float *vertices = points->GetPoint(0);

    vtkDataArray *element = data->GetCellData()->GetArray("element");
    float *elementnos = element ? element->GetTuple(0) : NULL;

    vtkDataArray *primary = data->GetCellData()->GetScalars();
    if (!primary)
        primary = data->GetPointData()->GetScalars();
    float *scalar = primary->GetTuple(0);

    bool color_by_element = primary && ct_is_discrete &&
        (string(primary->GetName()) == "element");
    bool is_resseq = string(primary->GetName()) == "resseq";
    bool color_by_levels = primary && ct_is_discrete &&
        (is_resseq ||
         string(primary->GetName()) == "restype");

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
            vtkDataArray *radius_array = data->GetCellData()->GetArray(
                                          atts.GetRadiusVariable().c_str());
            if (!radius_array)
                radius_array = data->GetPointData()->GetArray(
                                          atts.GetRadiusVariable().c_str());
            if (!radius_array)
            {
                // This shouldn't have gotten this far if it couldn't
                // read the variable like we asked.
                EXCEPTION1(ImproperUseException, "Couldn't read radius variable");
            }
            radiusvar = radius_array->GetTuple(0);
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
    for (int i=0; i<data->GetNumberOfCells(); i++)
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
            int level = int(scalar[i]) - (is_resseq ? 1 : 0);
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
                glVertex3fv(&vertices[3*i]);
                glTexCoord2fv(texCoords[j]);
            }
        }
        else if(vmode == 1)
        {
            // Plot squares
            for(int j = 0; j < 4; ++j)
            {
                float vert[3];
                vert[0] = vertices[3*i+0] + ptOffsets[j][0] * radius;
                vert[1] = vertices[3*i+1] + ptOffsets[j][1] * radius;
                vert[2] = vertices[3*i+2] + ptOffsets[j][2] * radius;
                glVertex3fv(vert);
                glTexCoord2fv(texCoords[j]);
            }
        }
        else
        {
            // Plot spheres
            DrawSphereAsQuads(vertices[3*i+0],
                              vertices[3*i+1],
                              vertices[3*i+2],
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
//
// ****************************************************************************
void
avtOpenGLMoleculeRenderer::DrawBonds(vtkPolyData *data,
                                     const MoleculeAttributes &atts)
{

    vtkPoints *points = data->GetPoints();
    float *vertices = points->GetPoint(0);
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();

    vtkDataArray *element = data->GetCellData()->GetArray("element");
    float *elementnos = element ? element->GetTuple(0) : NULL;

    vtkDataArray *primary = data->GetCellData()->GetScalars();
    if (!primary)
        primary = data->GetPointData()->GetScalars();
    float *scalar = primary->GetTuple(0);

    bool color_by_element = primary && ct_is_discrete &&
        (string(primary->GetName()) == "element");
    bool is_resseq = string(primary->GetName()) == "resseq";
    bool color_by_levels = primary && ct_is_discrete &&
        (is_resseq  ||
         string(primary->GetName()) == "restype");

    vtkUnsignedIntArray *origcell_array = vtkUnsignedIntArray::SafeDownCast(
                      data->GetCellData()->GetArray("avtOriginalCellNumbers"));
    //vtkDataArray *origcell_array = data->GetCellData()->GetArray("avtOriginalCellNumbers");
    if (!origcell_array)
    {
        EXCEPTION1(ImproperUseException, "Couldn't read original cell number array.");
    }
    //cerr << "cell number array type = "<<origcell_array->GetDataType()<<" "<<vtkImageScalarTypeNameMacro(origcell_array->GetDataType())<<endl;
    //int *orig_index = origcell_array ? (int*)(origcell_array->GetTuple(0)) : NULL;
    unsigned int *orig_index = origcell_array ? origcell_array->GetPointer(0) : NULL;

    vtkDataArray *bond_array = data->GetCellData()->GetArray("bonds");
    if (!bond_array)
    {
        EXCEPTION1(ImproperUseException, "Couldn't read bonds array.");
    }
    float *bonds = bond_array ? bond_array->GetTuple(0) : NULL;

    //
    // Create a map of atom indices for bond indexing
    //
    int max_index = 0;
    for (int i=0; i<data->GetNumberOfCells(); i++)
    {
        if (orig_index[2*i+1] > max_index)
            max_index = orig_index[2*i+1];
    }
    int *atom_index_map = new int[max_index+1];
    for (int i=0; i<=max_index; i++)
    {
        atom_index_map[i] = -1;
    }
    for (int i=0; i<data->GetNumberOfCells(); i++)
    {
        int ix = orig_index[2*i+1];
        atom_index_map[ix] = i;
    }

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

    for (int i=0; i<data->GetNumberOfCells(); i++)
    {
        for (int b=0; b<4; b++)
        {
            int v0_orig = orig_index[2*i+1];
            int v1_orig = int(bonds[4*i+b]);

            // Skip bonds that didn't exist in the first place
            if (v1_orig == -1)
                continue;

            // Skip bonds to atoms outside the valid original cells numbers
            if (v1_orig > max_index)
                continue;

            // By symmetry, don't plot bonds twice
            if (v0_orig > v1_orig)
                continue;

            int v0 = i;
            int v1 = atom_index_map[v1_orig];

            // Skip bonds to atoms that were removed
            if (v1 == -1)
                continue;

            float *pt_0 = &vertices[3*v0];
            float *pt_1 = &vertices[3*v1];
            float pt_mid[3] = {(pt_0[0]+pt_1[0])/2.,
                               (pt_0[1]+pt_1[1])/2.,
                               (pt_0[2]+pt_1[2])/2.};

            for (int half=0; half<=1; half++)
            {
                int atom  = (half==0) ? v0 : v1;
                float *pt_a = (half==0) ? pt_0 : pt_mid;
                float *pt_b = (half==0) ? pt_mid : pt_1;

                int element_number = 0;
                if (element)
                {
                    element_number = int(elementnos[atom]-1);
                }

                if (element_number < 0 || element_number >= MAX_ELEMENT_NUMBER)
                    element_number = MAX_ELEMENT_NUMBER-1;

                float radius = atts.GetBondRadius();

                // Determine color
                if (atts.GetColorBonds() == MoleculeAttributes::SingleColor)
                {
                    SetColor3ubv(atts.GetBondSingleColor().GetColor());
                }
                else // (atts.GetColorBonds() == MoleculeAttributes::ColorByAtom)
                {
                    if (color_by_element)
                    {
                        int level = element_number % numcolors;
                        SetColor3ubv(&colors[4*level]);
                    }
                    else if (color_by_levels)
                    {
                        int level = int(scalar[i]) - (is_resseq ? 1 : 0);
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
                            alpha = (scalar[atom] - varmin) / (varmax - varmin);
            
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
                    glVertex3fv(pt_a);
                    glVertex3fv(pt_b);
                }
            }
        }
    }
    glEnd();

    delete[] atom_index_map;
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
                                 float _spec_r, float _spec_g, float _spec_b)
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

    vtkPoints *points = data->GetPoints();
    const float *vertices = points->GetPoint(0);
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();

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

        if (atts.GetDrawAtomsAs() == MoleculeAttributes::SphereAtoms ||
            atts.GetDrawAtomsAs() == MoleculeAttributes::ImposterAtoms)
        {
            DrawAtomsAsSpheres(data,atts);
        }

        if (atts.GetDrawBondsAs() != MoleculeAttributes::NoBonds)
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
    if (data->GetCellData()->GetScalars())
    {
        varName = data->GetCellData()->GetScalars()->GetName();
    }
    else if (data->GetPointData()->GetScalars())
    {
        varName = data->GetPointData()->GetScalars()->GetName();
    }

    if (varName == "")
    {
        new_colortablename = "";
        new_numcolors = 0;
    }
    else if (varName == "element")
    {
        new_colortablename = atts.GetElementColorTable();
        if (new_colortablename == "Default")
            new_colortablename = string(ct->GetDefaultDiscreteColorTable());

        new_numcolors = 110;
    }
    else if (varName == "resseq")
    {
        new_colortablename = atts.GetResidueSequenceColorTable();
        if (new_colortablename == "Default")
            new_colortablename = string(ct->GetDefaultDiscreteColorTable());

        new_numcolors = 256;
    }
    else if (varName == "restype")
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
//
// ****************************************************************************
void
avtOpenGLMoleculeRenderer::DrawCylinderBetweenTwoPoints(float *p0,
                                                        float *p1,
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


