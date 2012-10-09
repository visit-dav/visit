/*****************************************************************************
 *
 * Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                            avtunvFileFormat.C
// A reader for SDRC I-Deas Master Series unv files:
//   Show the mesh, free faces and normals.
// Makes prints go to terminal or to vlog files as usual
#define INTERACTIVEPLOT 0
#define INTERACTIVEREAD 0
// Validate the following if gzip is installed on your system.
// VisIt usually assumes it is installed.
#define GZSTUFF 1
// ************************************************************************* //

#include <avtunvFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkCellType.h>
#include <vtkTetra.h>
#include <vtkHexahedron.h>

#include <avtIntervalTree.h>
#include <avtMaterial.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>

#include <TimingsManager.h>

using     std::string;
using namespace std;

// ****************************************************************************
//  Method: avtunvFileFormat auxiliary functions and data
//
//  Programmer: cessenat
//  Creation:   Thu Sep 27 17:38:20 PST 2012
//
// ****************************************************************************
// Type, Face, Nodes => Node position in reference element for Type
// This is supposed to be SDRC I-Deas Master Series convention.
static int nodefac2[2][4][2] =
{
    {
        {1, 2},         // 1 TRI
        {2, 3},         // 2
        {3, 1},         // 3
        {0, 0}          // 3
    },
    {
        {1, 2},         // 1 QUAD
        {2, 3},         // 2
        {3, 4},         // 2
        {4, 1}          // 3
    }
};
static int nodefac[4][6][4] =
{
    {
        {1, 3, 2, 0},           // 1 TET
        {1, 2, 4, 0},           // 2
        {2, 3, 4, 0},           // 3
        {1, 4, 3, 0},         // 4
        {0, 0, 0, 0},         // 5
        {0, 0, 0, 0}         // 6
    },
    {
        {1, 4, 3, 2},           // 1 HEX
        {5, 6, 7, 8},           // 2
        {1, 2, 6, 5},           // 3
        {2, 3, 7, 6},         // 4
        {3, 4, 8, 7},         // 5
        {1, 5, 8, 4}         // 6
    },
    {
        {1, 3, 2, 0},           // 1 WEDGE
        {4, 5, 6, 0},           // 2
        {1, 2, 5, 4},           // 3
        {2, 3, 6, 5},         // 4
        {1, 4, 6, 3},         // 5
        {0, 0, 0, 0}         // 6
    },
    {
        {1, 4, 3, 2},           // 1 PYRAMID
        {1, 2, 5, 0},           // 2
        {2, 3, 5, 0},           // 3
        {3, 4, 5, 0},         // 4
        {4, 1, 5, 0},         // 5
        {0, 0, 0, 0}           // 6
    }
};

// Provides position in the nodefac array for the element, face to nodes convention
int avtunvFileFormat::is3DKnownElt (int typelt)
{
    int n;
    switch (typelt)
    {
    case 111:
        n = 0;
        break;
    case 115:
        n = 1;
        break;
    case 112:
        n = 2;
        break;
    case 312:
        n = 3;
        break;
    default:
        n = -1;
        break;
    }
    return(n);
}
int avtunvFileFormat::is2DKnownElt (int typelt)
{
    int n;
    switch (typelt)
    {
    case 91:
        n = 0;
        break;
    case 94:
        n = 1;
        break;
    default:
        n = -1;
        break;
    }
    return(n);
}
int avtunvFileFormat::is1DKnownElt (int typelt)
{
    int n = -1;
    if (typelt == 21)
    {
        n = 0;
    }
    return(n);
}
int avtunvFileFormat::getEltDim (int typelt)
{
    int n;
    switch (typelt)
    {
    case 111:
        n = 3;
        break;
    case 115:
        n = 3;
        break;
    case 112:
        n = 3;
        break;
    case 312:
        n = 3;
        break;
    case 91:
        n = 2;
        break;
    case 94:
        n = 2;
        break;
    case 21:
        n = 1;
        break;
    default:
        n = -1;
        break;
    }
    return(n);
}

// Provides position in a private ranking array of elements
int avtunvFileFormat::isKnownElt (int typelt)
{
    int n;
    switch (typelt)
    {
    case 111:
        n = 0;
        break;
    case 115:
        n = 1;
        break;
    case 112:
        n = 2;
        break;
    case 312:
        n = 3;
        break;
    case 91:
        n = 4;
        break;
    case 94:
        n = 5;
        break;
    case 21:
        n = 6;
        break;
    default:
        n = -1;
        break;
    }
    return(n);
}

// Provides the number of nodes per element type
int avtunvFileFormat::getNbnodes (int typelt)
{
    int n;
    switch (typelt)
    {
    case 111:
        n = 4;
        break;
    case 115:
        n = 8;
        break;
    case 112:
        n = 6;
        break;
    case 312:
        n = 5;
        break;
    case 91:
        n = 3;
        break;
    case 94:
        n = 4;
        break;
    case 21:
        n = 2;
        break;
    default:
        n = -1;
        break;
    }
    return(n);
}

// Provides the number of faces per element type
int avtunvFileFormat::getNbfaces (int typelt)
{
    int n;
    switch (typelt)
    {
    case 111:
        n = 4;
        break;
    case 115:
        n = 6;
        break;
    case 112:
        n = 5;
        break;
    case 312:
        n = 5;
        break;
    case 91:
        n = 3;
        break;
    case 94:
        n = 4;
        break;
    case 21:
        n = 2;
        break;
    default:
        n = -1;
        break;
    }
    return(n);
}

// Provides the number of nodes for 3D elements and for the Face Presure Load-Set
void
avtunvFileFormat::getNormal3D (float *one_entry,
                               set<UnvElement, UnvElement::compare_UnvElement>::iterator itre,
                               int iflo, int facloc)
{
    int indicnbnodperfac = nodefac[iflo][facloc-1][3];
    double x1,x2,y1,y2,z1,z2;
    UnvNode anUnvNode;
    set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
    if (indicnbnodperfac == 0)
    {
#if 1
        int in ;
        in = nodefac[iflo][facloc-1][0];
        anUnvNode.label = itre->nodes[in-1];
        itrg = meshUnvNodes.find(anUnvNode);
        x1 = -itrg->x;
        y1 = -itrg->y;
        z1 = -itrg->z;
        x2 = -itrg->x;
        y2 = -itrg->y;
        z2 = -itrg->z;
        in = nodefac[iflo][facloc-1][1];
        anUnvNode.label = itre->nodes[in-1];
        itrg = meshUnvNodes.find(anUnvNode);
        x1 += itrg->x;
        y1 += itrg->y;
        z1 += itrg->z;
        in = nodefac[iflo][facloc-1][2];
        anUnvNode.label = itre->nodes[in-1];
        itrg = meshUnvNodes.find(anUnvNode);
        x2 += itrg->x;
        y2 += itrg->y;
        z2 += itrg->z;
#else
        for (int ln=0; ln<3; ln++)
        {
            int in = nodefac[iflo][facloc-1][ln];
            if (in > 0)
            {
                anUnvNode.label = itre->nodes[in-1];
                itrg = meshUnvNodes.find(anUnvNode);
                if (ln == 0)
                {
                    x1 = -itrg->x;
                    y1 = -itrg->y;
                    z1 = -itrg->z;
                    x2 = -itrg->x;
                    y2 = -itrg->y;
                    z2 = -itrg->z;
                }
                else if (ln == 1)
                {
                    x1 += itrg->x;
                    y1 += itrg->y;
                    z1 += itrg->z;
                }
                else if (ln == 2)
                {
                    x2 += itrg->x;
                    y2 += itrg->y;
                    z2 += itrg->z;
                }
            }
        }
#endif
    }
    else
    {
#if 1
        int in ;
        in = nodefac[iflo][facloc-1][0];
        anUnvNode.label = itre->nodes[in-1];
        itrg = meshUnvNodes.find(anUnvNode);
        x2 = -itrg->x;
        y2 = -itrg->y;
        z2 = -itrg->z;
        in = nodefac[iflo][facloc-1][1];
        anUnvNode.label = itre->nodes[in-1];
        itrg = meshUnvNodes.find(anUnvNode);
        x1 = itrg->x;
        y1 = itrg->y;
        z1 = itrg->z;
        in = nodefac[iflo][facloc-1][2];
        anUnvNode.label = itre->nodes[in-1];
        itrg = meshUnvNodes.find(anUnvNode);
        x2 += itrg->x;
        y2 += itrg->y;
        z2 += itrg->z;
        in = nodefac[iflo][facloc-1][3];
        anUnvNode.label = itre->nodes[in-1];
        itrg = meshUnvNodes.find(anUnvNode);
        x1 -= itrg->x;
        y1 -= itrg->y;
        z1 -= itrg->z;
#else
        for (int ln=0; ln<4; ln++)
        {
            int in = nodefac[iflo][facloc-1][ln];
            if (in > 0)
            {
                anUnvNode.label = itre->nodes[in-1];
                itrg = meshUnvNodes.find(anUnvNode);
                if (ln == 0)
                {
                    x2 = -itrg->x;
                    y2 = -itrg->y;
                    z2 = -itrg->z;
                }
                else if (ln == 1)
                {
                    x1 = itrg->x;
                    y1 = itrg->y;
                    z1 = itrg->z;
                }
                else if (ln == 2)
                {
                    x2 += itrg->x;
                    y2 += itrg->y;
                    z2 += itrg->z;
                }
                else if (ln == 3)
                {
                    x1 -= itrg->x;
                    y1 -= itrg->y;
                    z1 -= itrg->z;
                }
            }
        }
#endif
    }
    one_entry[0] = 0.5*(y1*z2-y2*z1);
    one_entry[1] = 0.5*(z1*x2-z2*x1);
    one_entry[2] = 0.5*(x1*y2-x2*y1);
}
// Provides the number of nodes for 2D element:
void
avtunvFileFormat::getNormal2D (float *one_entry,
                               set<UnvElement, UnvElement::compare_UnvElement>::iterator itre,
                               int facloc)
{
    double x1,x2,y1,y2,z1,z2;
    UnvNode anUnvNode;
    set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
    // First compute the normal to the element:
    int nbnos = avtunvFileFormat::getNbnodes(itre->typelt);
    if (nbnos == 3)
    {
        for (int ln=0; ln<nbnos; ln++)
        {
            anUnvNode.label = itre->nodes[ln];
            itrg = meshUnvNodes.find(anUnvNode);
            if (ln == 0)
            {
                x1 = -itrg->x;
                y1 = -itrg->y;
                z1 = -itrg->z;
                x2 = -itrg->x;
                y2 = -itrg->y;
                z2 = -itrg->z;
            }
            else if (ln == 1)
            {
                x1 += itrg->x;
                y1 += itrg->y;
                z1 += itrg->z;
            }
            else if (ln == 2)
            {
                x2 += itrg->x;
                y2 += itrg->y;
                z2 += itrg->z;
            }
        }
    }
    else
    {
        for (int ln=0; ln<4; ln++)
        {
            anUnvNode.label = itre->nodes[ln];
            itrg = meshUnvNodes.find(anUnvNode);
            if (ln == 0)
            {
                x2 = -itrg->x;
                y2 = -itrg->y;
                z2 = -itrg->z;
            }
            else if (ln == 1)
            {
                x1 = itrg->x;
                y1 = itrg->y;
                z1 = itrg->z;
            }
            else if (ln == 2)
            {
                x2 += itrg->x;
                y2 += itrg->y;
                z2 += itrg->z;
            }
            else if (ln == 3)
            {
                x1 -= itrg->x;
                y1 -= itrg->y;
                z1 -= itrg->z;
            }
        }
    }
    one_entry[0] = (y1*z2-y2*z1);
    one_entry[1] = (z1*x2-z2*x1);
    one_entry[2] = (x1*y2-x2*y1);
    double fac=1./sqrt(one_entry[0]*one_entry[0]+one_entry[1]*one_entry[1]+one_entry[2]*one_entry[2]);
    x2 = fac * one_entry[0];
    y2 = fac * one_entry[1];
    z2 = fac * one_entry[2];
#if INTERACTIVEREAD
    if (debuglevel >= 4) fprintf(stdout,"\t* 2D Element Normal=(%lf,%lf,%lf)\n",x2,y2,z2);
#endif
    int iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
    for (int ln=0; ln<2; ln++)
    {
        int in = nodefac2[iflo][facloc-1][ln];
        if (in > 0)
        {
            anUnvNode.label = itre->nodes[in-1];
            itrg = meshUnvNodes.find(anUnvNode);
#if INTERACTIVEREAD
            if (debuglevel >= 4) fprintf(stdout,"\t\t Node=(%lf,%lf,%lf)\n",itrg->x,itrg->y,itrg->z);
#endif
            if (ln == 0)
            {
                x1 = -itrg->x;
                y1 = -itrg->y;
                z1 = -itrg->z;
            }
            else
            {
                x1 += itrg->x;
                y1 += itrg->y;
                z1 += itrg->z;
            }
        }
    }
    // Then compute the normal with tangent vector:
#if INTERACTIVEREAD
    if (debuglevel >= 4) fprintf(stdout,"\tTangent=(%lf,%lf,%lf)\n",x1,y1,z1);
#endif
    one_entry[0] = (y1*z2-y2*z1);
    one_entry[1] = (z1*x2-z2*x1);
    one_entry[2] = (x1*y2-x2*y1);
#if INTERACTIVEREAD
    if (debuglevel >= 4) fprintf(stdout,"\tResult=(%lf,%lf,%lf)\n",one_entry[0],one_entry[1],one_entry[2]);
#endif
}

// Provides the number of nodes for the Face Presure Load-Set
int avtunvFileFormat::getNbnolsv ()
{
    if (nbfalsv > 0)
    {
        if (nbnolsv == 0)
        {
            // Compute the number of nodes:
            set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
            set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
            set<UnvNode, UnvNode::compare_UnvNode>::iterator itrl; // Sub-list node iterator
            UnvElement anUnvElement; // an element
            UnvNode anotherUnvNode; // a node
            UnvNode anUnvNode;
            set<UnvNode, UnvNode::compare_UnvNode> maliste;  // Sub list
            anotherUnvNode.number=0;
            for (int i=0; i<meshUnvFacePressures.size(); i++)
                for (int j=0; j<meshUnvFacePressures[i].faces.size(); j++)
                {
                    int iel = meshUnvFacePressures[i].faces[j].element;
                    int facloc = meshUnvFacePressures[i].faces[j].facloc;
                    // Fetch the element:
                    anUnvElement.label = iel;
                    itre = meshUnvElements.find(anUnvElement);
                    int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                    if (iflo >= 0)
                        for (int ln=0; ln<4; ln++)
                        {
                            int in = nodefac[iflo][facloc-1][ln];
                            if (in > 0)
                            {
                                anUnvNode.label = itre->nodes[in-1];
                                itrg = meshUnvNodes.find(anUnvNode);
                                itrl = maliste.find(anUnvNode);
                                if (itrl == maliste.end())
                                {
                                    // Node not known yet:
                                    anotherUnvNode.label = anUnvNode.label;
                                    maliste.insert(anotherUnvNode);
                                    anotherUnvNode.number++; // This is the actual number of nodes
                                }
                            }
                        }
                }

            nbnolsv = anotherUnvNode.number;
            maliste.clear();
#if INTERACTIVEPLOT
            if (debuglevel >= 3) fprintf(stdout,"Number of nodes for load-sets boundary conditions = %d\n",nbnolsv);
#else
            debug3 << "Number of nodes for load-sets boundary conditions = " << nbnolsv << endl;
#endif
        }
        return(nbnolsv);
    }
    else
    {
        return(0);
    }
}

// Gets the number of unique nodes on all free faces, nbnff
int avtunvFileFormat::getNbnodesFreeFaces ()
{
    if (nbfaextv == 0)
    {
#if INTERACTIVEPLOT
        if (debuglevel >= 3) fprintf(stdout,"nbfaextv is zero\n");
#else
        debug3 << "nbfaextv is zero" << endl;
#endif
        return(0);
    }
    else if (nbnff >= 0)
    {
#if INTERACTIVEPLOT
        if (debuglevel >= 3) fprintf(stdout,"nbnff exists=%d\n",nbnff);
#else
        debug3 << "nbnff exists=" << nbnff << endl;
#endif
        return(nbnff);
    }
    else
    {
        // Compute the number of nodes:
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrl; // Sub-list node iterator
        UnvElement anUnvElement; // an element
        UnvNode anotherUnvNode; // a node
        UnvNode anUnvNode;
        set<UnvNode, UnvNode::compare_UnvNode> maliste;  // Sub list
        anotherUnvNode.number=0;
        for (int j=0; j<freeUnvFaces.size(); j++)
        {
            int iel = freeUnvFaces[j].element;
            int facloc = freeUnvFaces[j].facloc;
            // Fetch the element:
            anUnvElement.label = iel;
            itre = meshUnvElements.find(anUnvElement);
            int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
            if (iflo >= 0)
            {
                for (int ln=0; ln<4; ln++)
                {
                    int in = nodefac[iflo][facloc-1][ln];
                    if (in > 0)
                    {
                        anUnvNode.label = itre->nodes[in-1];
                        itrg = meshUnvNodes.find(anUnvNode);
                        itrl = maliste.find(anUnvNode);
                        if (itrl == maliste.end())
                        {
                            anotherUnvNode.label = anUnvNode.label;
                            maliste.insert(anotherUnvNode);
                            anotherUnvNode.number++;
                        }
                    }
                }
            }
            else
            {
                iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
                if (iflo >= 0)
                {
                    for (int ln=0; ln<2; ln++)
                    {
                        int in = nodefac2[iflo][facloc-1][ln];
                        if (in > 0)
                        {
                            anUnvNode.label = itre->nodes[in-1];
                            itrg = meshUnvNodes.find(anUnvNode);
                            itrl = maliste.find(anUnvNode);
                            if (itrl == maliste.end())
                            {
                                anotherUnvNode.label = anUnvNode.label;
                                maliste.insert(anotherUnvNode);
                                anotherUnvNode.number++;
                            }
                        }
                    }
                }
                else
                {
                    int in = facloc;
                    anUnvNode.label = itre->nodes[in-1];
                    itrg = meshUnvNodes.find(anUnvNode);
                    itrl = maliste.find(anUnvNode);
                    if (itrl == maliste.end())
                    {
                        anotherUnvNode.label = anUnvNode.label;
                        maliste.insert(anotherUnvNode);
                        anotherUnvNode.number++; // This is the actual number of nodes
                    }
                }
            }
        }
        nbnff = anotherUnvNode.number;
        maliste.clear();
#if INTERACTIVEPLOT
        if (debuglevel >= 3) fprintf(stdout,"Number of nodes for free faces = %d\n",nbnff);
#else
        debug3 << "Number of nodes for free faces = " << nbnff << endl;
#endif
        return(nbnff);
    }
}

// A 3D neighbour function
int avtunvFileFormat::getNeighbour3D (int iel, int l1, int l2, int l3)
{
    set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg1, itrg2, itrg3; // Global node iterator
    UnvNode anUnvNode;
    anUnvNode.label = l1; // itre->nodes[in1-1];
    itrg1 = meshUnvNodes.find(anUnvNode);
    anUnvNode.label = l2; // itre->nodes[in2-1];
    itrg2 = meshUnvNodes.find(anUnvNode);
    anUnvNode.label = l3; // itre->nodes[in3-1];
    itrg3 = meshUnvNodes.find(anUnvNode);
    int nbdav1, nbdav2, nbdav3;
    nbdav1 = itrg1->nod2elts.size();
    nbdav2 = itrg2->nod2elts.size();
    nbdav3 = itrg3->nod2elts.size();
    for (int i1=0; i1 < nbdav1; i1++ )
    {
        int jel = itrg1->nod2elts[i1];
        if (jel != iel)
        {
            for (int i2=0; i2 < nbdav2; i2++ )
            {
                int iel2 = itrg2->nod2elts[i2];
                if (iel2 == jel)
                {
                    for (int i3=0; i3 < nbdav3; i3++ )
                    {
                        int iel3 = itrg3->nod2elts[i3];
                        if (iel3 == jel)
                        {
                            // Check the dimension
                            UnvElement anUnvElement; // an element
                            anUnvElement.label = jel;
                            if (avtunvFileFormat::getEltDim(meshUnvElements.find(anUnvElement)->typelt) == cdim)
                                return(jel);

                        }
                    }
                }
            }
        }
    }
    return(0);
}

// A 2D neighbour function
int avtunvFileFormat::getNeighbour2D (int iel, int l1, int l2)
{
    set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg1, itrg2; // Global node iterator
    UnvNode anUnvNode;
    anUnvNode.label = l1; // itre->nodes[in1-1];
    itrg1 = meshUnvNodes.find(anUnvNode);
    anUnvNode.label = l2; // itre->nodes[in2-1];
    itrg2 = meshUnvNodes.find(anUnvNode);
    int nbdav1, nbdav2;
    nbdav1 = itrg1->nod2elts.size();
    nbdav2 = itrg2->nod2elts.size();
    for (int i1=0; i1 < nbdav1; i1++ )
    {
        int jel = itrg1->nod2elts[i1];
        if (jel != iel)
        {
            for (int i2=0; i2 < nbdav2; i2++ )
            {
                int iel2 = itrg2->nod2elts[i2];
                if (iel2 == jel)
                {
                    // Check the dimension
                    UnvElement anUnvElement; // an element
                    anUnvElement.label = jel;
                    if (avtunvFileFormat::getEltDim(meshUnvElements.find(anUnvElement)->typelt) == cdim)
                        return(jel);

                }
            }
        }
    }
    return(0);
}

// A 3D neighbour function
int
avtunvFileFormat::getfastNeighbour3D (int iel, int l1, int l2, int l3,
                                      set<UnvNode, UnvNode::compare_UnvNode>::iterator * itrgs)
{
    int nbdav1, nbdav2, nbdav3;
    nbdav1 = itrgs[l1]->nod2elts.size();
    nbdav2 = itrgs[l2]->nod2elts.size();
    nbdav3 = itrgs[l3]->nod2elts.size();
    for (int i1=0; i1 < nbdav1; i1++ )
    {
        int jel = itrgs[l1]->nod2elts[i1];
        if (jel != iel)
        {
            for (int i2=0; i2 < nbdav2; i2++ )
            {
                int iel2 = itrgs[l2]->nod2elts[i2];
                if (iel2 == jel)
                {
                    for (int i3=0; i3 < nbdav3; i3++ )
                    {
                        int iel3 = itrgs[l3]->nod2elts[i3];
                        if (iel3 == jel)
                        {
                            // Check the dimension
                            UnvElement anUnvElement; // an element
                            anUnvElement.label = jel;
                            if (avtunvFileFormat::getEltDim(meshUnvElements.find(anUnvElement)->typelt) == cdim)
                                return(jel);

                        }
                    }
                }
            }
        }
    }
    return(0);
}

// A 2D neighbour function
int
avtunvFileFormat::getfastNeighbour2D (int iel, int l1, int l2, set<UnvNode,
                                      UnvNode::compare_UnvNode>::iterator * itrgs)
{
    int nbdav1, nbdav2;
    nbdav1 = itrgs[l1]->nod2elts.size();
    nbdav2 = itrgs[l2]->nod2elts.size();
    for (int i1=0; i1 < nbdav1; i1++ )
    {
        int jel = itrgs[l1]->nod2elts[i1];
        if (jel != iel)
        {
            for (int i2=0; i2 < nbdav2; i2++ )
            {
                int iel2 = itrgs[l2]->nod2elts[i2];
                if (iel2 == jel)
                {
                    // Check the dimension
                    UnvElement anUnvElement; // an element
                    anUnvElement.label = jel;
                    if (avtunvFileFormat::getEltDim(meshUnvElements.find(anUnvElement)->typelt) == cdim)
                        return(jel);

                }
            }
        }
    }
    return(0);
}

#if 1
// A 3D neighbour function
int avtunvFileFormat::getarrayNeighbour3D (int iel, int l1, int l2, int l3, int *itab, int *jtab, int *ctab)
{
    set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg1, itrg2, itrg3; // Global node iterator
    int nbdav1, nbdav2, nbdav3;
    nbdav1 = itab[ ctab[l1]+1 ] - itab[ ctab[l1] ]; // itrg1->nod2elts.size();
    nbdav2 = itab[ ctab[l2]+1 ] - itab[ ctab[l2] ]; // itrg2->nod2elts.size();
    nbdav3 = itab[ ctab[l3]+1 ] - itab[ ctab[l3] ]; // itrg3->nod2elts.size();
    for (int i1=0; i1 < nbdav1; i1++ )
    {
        int jel = jtab[ itab[ ctab[l1] ] + i1 ]; // itrg1->nod2elts[i1];
        if (jel != iel)
        {
            for (int i2=0; i2 < nbdav2; i2++ )
            {
                int iel2 = jtab[ itab[ ctab[l2] ] + i2 ]; // itrg2->nod2elts[i2];
                if (iel2 == jel)
                {
                    for (int i3=0; i3 < nbdav3; i3++ )
                    {
                        int iel3 = jtab[ itab[ ctab[l3] ] + i3 ]; // itrg3->nod2elts[i3];
                        if (iel3 == jel)
                        {
                            // On verifie la dimension:
                            UnvElement anUnvElement; // an element
                            anUnvElement.label = jel;
                            if (avtunvFileFormat::getEltDim(meshUnvElements.find(anUnvElement)->typelt) == cdim)
                                return(jel);

                        }
                    }
                }
            }
        }
    }
    return(0);
}

// Gets the number of free faces, nbfaextv, this is the routine that sets the reverse connectivity
int avtunvFileFormat::attemptgetNbfaextv ()
{
    if (nbfaextv >= 0)
    {
        return(nbfaextv);
    }
    else
    {
        // Build the free faces mesh data structure:
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: computing the free faces for %d-D mesh\n",cdim);
#else
        debug3 << "getNbfaextv: computing the free faces for " << cdim << "D mesh" << endl;
#endif
        // Sets the nodes to elements list first:
        freeUnvFaces.clear();
        nbfaextv = 0;
        // - loop on elements:
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
        UnvElement anUnvElement; // an element
        UnvNode anUnvNode;
        UnvNode anotherUnvNode;
        // Gets the max nod number
        int maxnod = 0;
        for (itrg = meshUnvNodes.begin(); itrg != meshUnvNodes.end(); itrg++)
        {
            if (itrg->label > maxnod)
                maxnod = itrg->label;

        }
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: maxnod=%d\n",maxnod);
#else
        debug3 << "getNbfaextv: maxnod=" << maxnod << endl;
#endif
        // Direct access to nodes
        int * ctab = new int[maxnod];
        for (itrg = meshUnvNodes.begin(); itrg != meshUnvNodes.end(); itrg++)
            ctab[itrg->label] = itrg->number;

#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: direct access nodes=%d\n",nbnodes);
#else
        debug3 << "getNbfaextv: direct access nodes=" << nbnodes << endl;
#endif
        // Pointer array to nodes in jtab
        //int * itab = new int[nbnodes+1];
        int * itab = (int *)malloc((nbnodes+1)*sizeof(int));
        for (int i=0; i <= nbnodes; i++)
            itab[i] = 0;

#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: fill itab\n");
#else
        debug3 << "getNbfaextv: fill itab" << endl;
#endif
        for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        {
            int nn = avtunvFileFormat::getNbnodes(itre->typelt);
            for (int i=0; i < nn; i++)
            {
                anUnvNode.label = itre->nodes[i];
                itrg = meshUnvNodes.find(anUnvNode);
                int j = itrg->number;
                itab[j] = itab[j] + 1;
            }
        }
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: make pointer\n");
#else
        debug3 << "getNbfaextv: make pointer" << endl;
#endif
        for (int i=0; i < nbnodes; i++)
            itab[i+1] += itab[i];

        int kt = itab[nbnodes+1];
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: kt=%d\n",kt);
#else
        debug3 << "getNbfaextv: kt=" << kt << endl;
#endif
        int * jtab = new int[kt];
        for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        {
            int nn = avtunvFileFormat::getNbnodes(itre->typelt);
            for (int i=0; i < nn; i++)
            {
                anUnvNode.label = itre->nodes[i];
                itrg = meshUnvNodes.find(anUnvNode);
                itab[itrg->number]--;
                int l = itab[itrg->number];
                jtab[l] = itre->label;
            }
        }
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: build\n");
#else
        debug3 << "getNbfaextv: build" << endl;
#endif
        int iflo3 = -1, iflo2 = -1, iflo1 = -1;
        // Now loop on elements and on their faces and look for their neighbours:
        for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        {
            iflo3 = avtunvFileFormat::is3DKnownElt(itre->typelt);
            int nf = avtunvFileFormat::getNbfaces(itre->typelt);
            if (iflo3 >= 0) // 3D element
            {
                for (int facloc=0; facloc < nf; facloc++)
                {
                    // Array technique:
                    int in1, in2, in3;
                    if (iflo3 >= 0 && 1 == 1)
                    {
                        in1 = nodefac[iflo3][facloc][0];
                        in2 = nodefac[iflo3][facloc][1];
                        in3 = nodefac[iflo3][facloc][2];
                        int jel=getarrayNeighbour3D(itre->label,itre->nodes[in1-1],itre->nodes[in2-1],itre->nodes[in3-1],itab,jtab,ctab);
                        if (jel <= 0)
                        {
                            UnvFace anUnvFace; // Elementary face
                            anUnvFace.number = nbfaextv;
                            anUnvFace.element = itre->label;
                            anUnvFace.facloc = facloc + 1; // Set local face number to Ideas convention (starting at 1)
                            freeUnvFaces.push_back(anUnvFace); // Add this face to the list of free faces
                            nbfaextv++;
#if INTERACTIVEREAD
                            if (debuglevel >= 5) fprintf(stdout,"Element %d, face %d has a no neighbour %d\n",itre->label,facloc+1,nbfaextv);
#endif
                        }
                    }
                }
            }
        }
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: done\n");
#else
        debug3 << "getNbfaextv: done" << endl;
#endif
        delete [] jtab;
        free(itab);
        //delete [] itab;
        delete [] ctab;
        return(nbfaextv);
    }
}
#endif

// Gets the number of free faces, nbfaextv, this is the routine that sets the reverse connectivity
int avtunvFileFormat::getNbfaextv ()
{
    if (nbfaextv >= 0)
    {
        return(nbfaextv);
    }
    else
    {
        // Validate following if you know what you are doing...
        // return(avtunvFileFormat::attemptgetNbfaextv());
        // Build the free faces mesh data structure:
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: computing\n");
#else
        debug3 << "getNbfaextv: computing" << endl;
#endif
        int t1 = visitTimer->StartTimer();
        // Sets the nodes to elements list first:
        freeUnvFaces.clear();
        nbfaextv = 0;
        // - loop on elements:
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
        UnvElement anUnvElement; // an element
        UnvNode anUnvNode;
        UnvNode anotherUnvNode;
        revconnect = 1;
        for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        {
            int nn = avtunvFileFormat::getNbnodes(itre->typelt);
            for (int i=0; i < nn; i++)
            {
                anUnvNode.label = itre->nodes[i];
                itrg = meshUnvNodes.find(anUnvNode);
                itrg->nod2elts.push_back(itre->label);
#if INTERACTIVEREAD
                if (debuglevel >= 5) fprintf(stdout,"getNbfaextv: Adding to node %d element %d\n",itrg->label,itre->label);
#endif
            }
        }
        visitTimer->StopTimer(t1, "Building reverse connectivity");
        int t2 = visitTimer->StartTimer();
#if INTERACTIVEREAD
        if (debuglevel >= 5)
        {
            int nnodes[64];
            for (int i=0; i < 64; i++)
                nnodes[i] = 0;
            for (itrg = meshUnvNodes.begin(); itrg != meshUnvNodes.end(); itrg++)
            {
                fprintf(stdout,"getNbfaextvNode =%d has #elts=%d :",itrg->label,itrg->nod2elts.size());
                for (int i=0; i < itrg->nod2elts.size(); i++)
                    fprintf(stdout," %d",itrg->nod2elts[i]);
                fprintf(stdout,"\n");
                nnodes[itrg->nod2elts.size()]++;
            }
            for (int i=0; i < 64; i++)
                if (nnodes[i] > 0)
                    fprintf(stdout,"#elts=%d, #nodes=%d\n",i,nnodes[i]);
        }
        else if (debuglevel >= 3)
        {
            fprintf(stdout,"\tgetNbfaextv: reverse mesh has been built\n");
        }
#else
        debug3 << "getNbfaextv: reverse mesh has been built" << endl;
#endif
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre2;
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre3;
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre4;
        int iflo3 = -1, iflo2 = -1, iflo1 = -1;
        // Now loop on elements and on their faces and look for their neighbours:
        for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        {
            if (avtunvFileFormat::getEltDim(itre->typelt) == cdim) // Element of the right dimension
            {
                set<UnvNode, UnvNode::compare_UnvNode>::iterator itrgs[8]; // Global node iterator
                int nn = avtunvFileFormat::getNbnodes(itre->typelt);
                for (int i=0; i < nn; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrgs[i] = meshUnvNodes.find(anUnvNode);
                }
                int nf = avtunvFileFormat::getNbfaces(itre->typelt);
                if (cdim == 3)
                    iflo3 = avtunvFileFormat::is3DKnownElt(itre->typelt);
                else if (cdim == 2)
                    iflo2 = avtunvFileFormat::is2DKnownElt(itre->typelt);

                for (int facloc=0; facloc < nf; facloc++)
                {
                    // Array technique:
                    int in1, in2, in3;
                    if (cdim == 3)
                    {
                        in1 = nodefac[iflo3][facloc][0];
                        in2 = nodefac[iflo3][facloc][1];
                        in3 = nodefac[iflo3][facloc][2];
                        //int jel = getNeighbour3D(itre->label, itre->nodes[in1-1],itre->nodes[in2-1],itre->nodes[in3-1]);
                        int jel = getfastNeighbour3D(itre->label, in1-1, in2-1, in3-1, itrgs);
                        if (jel <= 0)
                        {
                            UnvFace anUnvFace; // Elementary face
                            anUnvFace.number = nbfaextv;
                            anUnvFace.element = itre->label;
                            anUnvFace.facloc = facloc + 1; // Set local face number to Ideas convention (starting at 1)
                            freeUnvFaces.push_back(anUnvFace); // Add this face to the list of free faces
                            nbfaextv++;
#if INTERACTIVEREAD
                            if (debuglevel >= 5) fprintf(stdout,"Element %d, face %d has a no neighbour %d\n",itre->label,facloc+1,nbfaextv);
#endif
                        }
                    }
                    else if (cdim == 2)
                    {
                        in1 = nodefac2[iflo2][facloc][0];
                        in2 = nodefac2[iflo2][facloc][1];
                        //int jel = getNeighbour2D(itre->label, itre->nodes[in1-1],itre->nodes[in2-1]);
                        int jel = getfastNeighbour2D(itre->label, in1-1, in2-1, itrgs);
                        if (jel <= 0)
                        {
                            UnvFace anUnvFace; // Elementary face
                            anUnvFace.number = nbfaextv;
                            anUnvFace.element = itre->label;
                            anUnvFace.facloc = facloc + 1; // Set local face number to Ideas convention (starting at 1)
                            freeUnvFaces.push_back(anUnvFace); // Add this face to the list of free faces
                            nbfaextv++;
#if INTERACTIVEREAD
                            if (debuglevel >= 5) fprintf(stdout,"Element %d, face %d has a no neighbour %d\n",itre->label,facloc+1,nbfaextv);
#endif
                        }
                    }
                    else
                    {
                        // STL technique:
                        int nf = avtunvFileFormat::getNbfaces(itre->typelt);
                        int nodes[4]; // Maximum 4 nodes per face
                        iflo3 = avtunvFileFormat::is3DKnownElt(itre->typelt);
                        if (iflo3 < 0)
                        {
                            iflo2 = avtunvFileFormat::is2DKnownElt(itre->typelt);
                            if (iflo2 < 0)
                            {
                                iflo1 = avtunvFileFormat::is1DKnownElt(itre->typelt);
                            }
                        }
                        std::set<UnvElement, UnvElement::compare_UnvElement> maliste[3];  // Temporary element list:
                        int ns;
                        int nbp;
                        if (iflo3 >= 0)
                        {
                            ns = nodefac[iflo3][facloc][3];
                            if (ns == 0)
                            {
                                nbp = 3;
                            }
                            else
                            {
                                nbp = 4;
                            }
                        }
                        else if (iflo2 >= 0)
                        {
                            nbp = 2;
                        }
                        else
                        {
                            nbp = 1;
                        }
                        for (int ln=0; ln<nbp; ln++)
                        {
                            int in;
                            if (iflo3 >= 0)
                                in = nodefac[iflo3][facloc][ln];
                            else if (iflo2 >= 0)
                                in = nodefac2[iflo2][facloc][ln];
                            else
                                in = facloc;

                            if (in > 0)
                            {
                                anUnvNode.label = itre->nodes[in-1];
                                nodes[ln] = anUnvNode.label;
                                itrg = meshUnvNodes.find(anUnvNode);
                                // Loop on elements that possess this node and check for candidates:
                                for (int iel=0; iel < itrg->nod2elts.size(); iel++ )
                                {
                                    anUnvElement.label = itrg->nod2elts[iel];
                                    if (anUnvElement.label != itre->label)
                                    {
                                        itre2 = maliste[0].find(anUnvElement);
                                        if (itre2 == maliste[0].end())
                                        {
                                            if (avtunvFileFormat::getEltDim(meshUnvElements.find(anUnvElement)->typelt) == cdim)
                                                maliste[0].insert(anUnvElement);

                                        }
                                        else if (cdim > 1)
                                        {
                                            itre3 = maliste[1].find(anUnvElement);
                                            if (itre3 == maliste[1].end())
                                            {
                                                maliste[1].insert(anUnvElement);
                                                if (cdim == 2)
                                                {
                                                    break; // Found a neighbour for 2D stuff
                                                }
                                            }
                                            else if (cdim > 2)
                                            {
                                                itre4 = maliste[2].find(anUnvElement);
                                                if (itre4 == maliste[2].end())
                                                {
                                                    maliste[2].insert(anUnvElement);
                                                    break; // Found a neighbour !
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
#if INTERACTIVEREAD
                        // Debugging:
                        if (debuglevel >= 5)
                        {
                            fprintf(stdout,"Element %d, face %d has %d neighbours :",itre->label,facloc+1,maliste[cdim-1].size());
                            for (itre2 = maliste[cdim-1].begin(); itre2 != maliste[cdim-1].end(); itre2++)
                                fprintf(stdout," %d",itre2->label);
                            fprintf(stdout,"\n");
                        }
#endif
                        // Does any neighbour share the same nodes ?
                        int iok = 0;
                        for (itre2 = maliste[cdim-1].begin(); itre2 != maliste[cdim-1].end(); itre2++)
                        {
                            anUnvElement.label = itre2->label;
                            itre3 = meshUnvElements.find(anUnvElement);
                            int nbno = avtunvFileFormat::getNbnodes(itre3->typelt);
                            iok = 0;
                            for (int jn=0; jn<nbno; jn++)
                            {
                                // gets the node label:
                                for (int ln=0; ln<nbp; ln++)
                                    if (nodes[ln] == itre3->nodes[jn])
                                        iok++;
                            }
                            if (iok == nbp)
                            {
                                // Yes, itre2->label is the neighbour !
#if INTERACTIVEREAD
                                if (debuglevel >= 5) fprintf(stdout,"Element %d, face %d has a neighbour %d\n",itre->label,facloc+1,itre2->label);
#endif
                                break;
                            }
                        }
                        if (iok != nbp)
                        {
                            // Neighbour has not been found : increase the number of faces into freefaces
                            UnvFace anUnvFace; // Elementary face
                            anUnvFace.number = nbfaextv;
                            anUnvFace.element = itre->label;
                            anUnvFace.facloc = facloc + 1; // Set local face number to Ideas convention (starting at 1)
                            freeUnvFaces.push_back(anUnvFace); // Add this face to the list of free faces
                            nbfaextv++;
#if INTERACTIVEREAD
                            if (debuglevel >= 5) fprintf(stdout,"Element %d, face %d has a no neighbour %d\n",itre->label,facloc+1,nbfaextv);
#endif
                        }
                        for (int ln=0; ln<cdim; ln++)
                            maliste[ln].clear();
                    }
                }
            }
        }
        visitTimer->StopTimer(t2, "Building free faces");
        // Release current memory:
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: release memory\n");
#else
        debug3 << "getNbfaextv: release memory" << endl;
#endif
        if (revconnect > 0)
        {
            set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg;
            for (itrg = meshUnvNodes.begin(); itrg != meshUnvNodes.end(); itrg++)
                itrg->nod2elts.clear();

            revconnect = 0;
        }
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"getNbfaextv: nbfaextv=%d\n",nbfaextv);
#else
        debug3 << "nbfaextv=" << nbfaextv << endl;
#endif
        return(nbfaextv);
    }
}

// Gets the number of different free faces as a connected boundary set,
// checking which is the outer one.
int avtunvFileFormat::getNbfreeSets ()
{
    if (nbfreesets > 0)
        return(nbfreesets);
    else
    {
        // Make sure the free faces have already been built:
        if (nbfaextv < 0)
            avtunvFileFormat::getNbfaextv();

        nbfreesets = 1;
        // Identify the faces to sub-elements and make freeelts, on the fly make the freenoeuds
        // list of free nodes and builds the reverse connectivity.
        std::set<UnvElement, UnvElement::compare_UnvElement> freeelts;  // Full mesh all types of elements
        std::set<UnvNode, UnvNode::compare_UnvNode> freenoeuds; // Full mesh nodes
        UnvElement anUnvElement; // an element
        UnvNode anUnvNode; // a node
        int nbnn = 0;
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrl; // Sub-list node iterator
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Primary elements iterator
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"Building elements from free faces %d\n",freeUnvFaces.size());
#else
        debug3 << "Building elements from free faces" << freeUnvFaces.size() << endl;
#endif
        for (int j=0; j<freeUnvFaces.size(); j++)
        {
            int iel = freeUnvFaces[j].element;
            int facloc = freeUnvFaces[j].facloc;
            // Fetch the element:
            anUnvElement.label = iel;
            itre = meshUnvElements.find(anUnvElement);
            int iflo3 = -1, iflo2 = -1, iflo1 = -1;
            iflo3 = avtunvFileFormat::is3DKnownElt(itre->typelt);
            if (iflo3 < 0)
            {
                iflo2 = avtunvFileFormat::is2DKnownElt(itre->typelt);
                if (iflo2 < 0)
                {
                    iflo1 = avtunvFileFormat::is1DKnownElt(itre->typelt);
                }
            }
            if (iflo3 >= 0 || iflo2 >= 0 || iflo1 >= 0)// 3D element
            {
                UnvElement anUnvElement; // an element
                int nbnel;
                if (iflo3 >= 0)
                {
                    if (nodefac[iflo3][facloc-1][3] > 0)
                    {
                        nbnel = 4;
                        anUnvElement.typelt = 94;
                    }
                    else
                    {
                        nbnel = 3;
                        anUnvElement.typelt = 91;
                    }
                }
                else if (iflo2 >= 0)
                {
                    nbnel = 2;
                    anUnvElement.typelt = 21;
                }
                else
                {
                    nbnel = 1;
                }
                anUnvElement.nodes = new int[nbnel];
                anUnvElement.number = j;
                anUnvElement.label = j;
                // Adds the nodes build:
                for (int ln=0; ln<nbnel; ln++)
                {
                    int in;
                    if (iflo3 >= 0)
                    {
                        in = nodefac[iflo3][facloc-1][ln]; // local node number
                    }
                    else if (iflo2 >= 0)
                    {
                        in = nodefac2[iflo2][facloc-1][ln]; // local node number
                    }
                    else
                    {
                        in = facloc;
                    }
                    UnvNode anotherUnvNode; // a node
                    anotherUnvNode.label = itre->nodes[in-1]; // Global node label
                    anUnvElement.nodes[ln] = anotherUnvNode.label;
                    itrl = freenoeuds.find(anotherUnvNode); // Find the global node id in current free list
                    if (itrl == freenoeuds.end())
                    {
                        anotherUnvNode.nod2elts.push_back(j); // Add the current "element" number or label
                        anotherUnvNode.number = nbnn;
                        // Debugging purpose below:
                        itrg = meshUnvNodes.find(anotherUnvNode);
                        anotherUnvNode.x = itrg->x;
                        anotherUnvNode.y = itrg->y;
                        anotherUnvNode.z = itrg->z;
                        //
                        freenoeuds.insert(anotherUnvNode);
                        nbnn++; // Increase the nodes counter
                    }
                    else
                    {
                        itrl->nod2elts.push_back(j); // Add the current "element" number or label
                    }
                }
                anUnvElement.matid = 0; // Initialize the matid !
                freeelts.insert(anUnvElement);
            }
        }
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"Built %d nodes from freefaces\n",nbnn);
#else
        debug3 << "Built " << nbnn << " nodes from freefaces" << endl;
#endif
#if INTERACTIVEREAD
        // Print out the reverse mesh topology for debugging purpose:
        if (debuglevel >= 5)
        {
            int nnodes[64];
            for (int i=0; i < 64; i++)
                nnodes[i] = 0;
            for (itrl = freenoeuds.begin(); itrl != freenoeuds.end(); itrl++)
            {
                fprintf(stdout,"Node=(n=%d,l=%d pos=(%lf,%lf,%lf)) has #elts=%d : ",
                        itrl->number,itrl->label,itrl->x,itrl->y,itrl->z,itrl->nod2elts.size());
                for (int i=0; i < itrl->nod2elts.size(); i++)
                    fprintf(stdout," %d",itrl->nod2elts[i]);
                fprintf(stdout,"\n");
                nnodes[itrl->nod2elts.size()]++;
            }
            for (int i=0; i < 64; i++)
                if (nnodes[i] > 0)
                    fprintf(stdout,"#elts=%d, #nodes=%d\n",i,nnodes[i]);
            // Debugging the standard elemenst to nodes:
            for (itre = freeelts.begin(); itre != freeelts.end(); itre++)
            {
                int nbnel = avtunvFileFormat::getNbnodes(itre->typelt);
                fprintf(stdout,"Element %d has #nodes=%d : ",itre->label,nbnel);
                for (int i=0; i < nbnel; i++)
                {
                    anUnvNode.label = itre->nodes[i]; // Gets a global node label, suitable for free array as well
                    itrl = freenoeuds.find(anUnvNode);
                    fprintf(stdout," %d(%d)",itre->nodes[i],itrl->nod2elts.size());
                }
                fprintf(stdout,"\n");
            }
        }
#endif
        // Able to loop on the sets of elements:two and one
        std::set<UnvElement, UnvElement::compare_UnvElement> oldelts;
        std::set<UnvElement, UnvElement::compare_UnvElement> newelts;
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre2;
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre3;
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre4;
        vector<UnvRange> vranges;
        UnvRange arange;
        double myrange[6];
        anUnvElement.label = 0;
        oldelts.insert(anUnvElement);
        myrange[0]=1.e30;
        myrange[1]=-1.e30;
        myrange[2]=1.e30;
        myrange[3]=-1.e30;
        myrange[4]=1.e30;
        myrange[5]=-1.e30;
        // myrange = { 1.e30, -1.e30, 1.e30, -1.e30, 1.e30, -1.e30 };
        while(oldelts.size() > 0)
        {
#if INTERACTIVEREAD
            if (debuglevel >= 4) fprintf(stdout,"\tColorizing %d elements to color %d\n",oldelts.size(), nbfreesets);
#else
            debug4 << "\tColorizing " << oldelts.size() << " elements to color " << nbfreesets << endl;
#endif
            for (itre = oldelts.begin(); itre != oldelts.end(); itre++)
            {
                anUnvElement.label = itre->label;
                itre2 = freeelts.find(anUnvElement);
                itre2->matid = nbfreesets;
#if INTERACTIVEREAD
                if (debuglevel >= 5) fprintf(stdout,"\t\tPainting element %d\n",itre2->label);
#else
                debug5 << "\t\tPainting element " << itre2->label << endl;
#endif
            }
            // Loop on oldelts to find new list:
            for (itre = oldelts.begin(); itre != oldelts.end(); itre++)
            {
                anUnvElement.label = itre->label;
                itre2 = freeelts.find(anUnvElement);
                // Look for it's neighbours using nodes:
                int nbnel = avtunvFileFormat::getNbnodes(itre2->typelt);
#if INTERACTIVEREAD
                if (debuglevel >= 4) fprintf(stdout,"\t Elt %d has %d nodes\n",itre2->label,nbnel);
#endif
                for (int in=0; in<nbnel; in++)
                {
                    anUnvNode.label = itre2->nodes[in]; // Gets a global node label, suitable for free array as well
                    itrl = freenoeuds.find(anUnvNode);
#if INTERACTIVEREAD
                    if (debuglevel >= 5) fprintf(stdout,"\t\t Elt %d has node (n=%d,l=%d, pos=(%lf,%lf,%lf)) belonging to %d elts",
                                                     itre2->label,itrl->number,itrl->label,itrl->x,itrl->y,itrl->z,itrl->nod2elts.size());
#endif
                    myrange[0] = min(myrange[0],itrl->x);
                    myrange[1] = max(myrange[1],itrl->x);
                    myrange[2] = min(myrange[2],itrl->y);
                    myrange[3] = max(myrange[3],itrl->y);
                    myrange[4] = min(myrange[4],itrl->z);
                    myrange[5] = max(myrange[5],itrl->z);
                    for (int iel=0; iel < itrl->nod2elts.size(); iel++ )
                    {
                        UnvElement anUnvElement; // an element
                        anUnvElement.label = itrl->nod2elts[iel];
#if INTERACTIVEREAD
                        if (debuglevel >= 5) fprintf(stdout," %d",anUnvElement.label);
#endif
                        if (anUnvElement.label != itre->label)
                        {
                            itre3 = newelts.find(anUnvElement);
                            itre4 = freeelts.find(anUnvElement);
#if INTERACTIVEREAD
                            if (debuglevel >= 5) fprintf(stdout," (%d)",itre4->matid);
#endif
                            if (itre3 == newelts.end() && itre4->matid == 0)
                            {
                                newelts.insert(anUnvElement);
                            }
                        }
                    }
#if INTERACTIVEREAD
                    if (debuglevel >= 5) fprintf(stdout,"\n\t\t\tincrease newelt size=%d\n",newelts.size());
#endif
                }
            }
            oldelts.clear();
            if (newelts.size() > 0)
                oldelts = newelts; // Stupid copy instead of reference...
            else
            {
                // Look for first unpainted element:
                for (itre = freeelts.begin(); itre != freeelts.end(); itre++)
                    if (itre->matid == 0)
                    {
                        // Use another color
                        for (int k=0; k < 6; k++ )
                            arange.trange[k] = myrange[k];
                        vranges.push_back(arange);
                        myrange[0]=1.e30;
                        myrange[1]=-1.e30;
                        myrange[2]=1.e30;
                        myrange[3]=-1.e30;
                        myrange[4]=1.e30;
                        myrange[5]=-1.e30;
                        // myrange = { 1.e30, -1.e30, 1.e30, -1.e30, 1.e30, -1.e30 };
                        nbfreesets++;
                        anUnvElement.label = itre->label;
                        oldelts.insert(anUnvElement);
#if INTERACTIVEREAD
                        if (debuglevel >= 4) fprintf(stdout,"\tChanging color to %d\n",nbfreesets);
#else
                        debug4 << "\tChanging color to " << nbfreesets << endl;
#endif
                        break;
                    }

            }
            newelts.clear();
        }
        for (int k=0; k < 6; k++ )
            arange.trange[k] = myrange[k];
        vranges.push_back(arange);

        if (cdim == 3)
        {
            // Check which is the outer boundary ?
            int imax=0;
            for (int j=1; j<nbfreesets; j++)
                for (int k=0; k < 3; k++ )
                {
                    if (vranges[imax].trange[2*k] > vranges[j].trange[2*k] || vranges[imax].trange[2*k+1] < vranges[j].trange[2*k+1])
                    {
                        imax = j;
                        break;
                    }
                }

            imax++;
            int iorder = 1; // nbfreesets ou 1
#if INTERACTIVEREAD
            if (debuglevel >= 3) fprintf(stdout,"Outer boundary is %d on %d\n",imax,nbfreesets);
            for (int j=0; j<nbfreesets; j++)
                if (debuglevel >= 3) fprintf(stdout,"BBox(%d) = (%lf,%lf)\n",j,vranges[j].trange[0],vranges[j].trange[1]);
#else
            debug3 << "Outer boundary is " << imax << " on " << nbfreesets << endl;
#endif
            // Outer boundary is not the first one:
            if (imax != iorder)
                for (itre = freeelts.begin(); itre != freeelts.end(); itre++)
                    if (itre->matid == iorder)
                        itre->matid = imax;
                    else if (itre->matid == imax)
                        itre->matid = iorder;
        }

        // Paints the matid array:
#if INTERACTIVEREAD
        if (debuglevel >= 3) fprintf(stdout,"Painting the %d free faces using %d colors\n",freeUnvFaces.size(),nbfreesets);
#else
        debug3 << "Painting the " << freeUnvFaces.size() << " free faces using " << nbfreesets << " colors" << endl;
#endif
        for (int j=0; j<freeUnvFaces.size(); j++)
        {
            anUnvElement.label = j;
            itre = freeelts.find(anUnvElement);
            freeUnvFaces[j].matid = itre->matid;
        }
        // Free memory
        for (itrl = freenoeuds.begin(); itrl != freenoeuds.end(); itrl++)
            itrl->nod2elts.clear();

        freenoeuds.clear();
        // Deallocate the node list for all elements:
        for (itre = freeelts.begin(); itre != freeelts.end(); itre++)
            delete [] itre->nodes;

        freeelts.clear();
        // Return
        return(nbfreesets);
    }
}

// ****************************************************************************
//  Method: avtunvFileFormat constructor
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Thu Sep 27 17:38:20 PST 2012
//
// ****************************************************************************

avtunvFileFormat::avtunvFileFormat(const char *fn) : avtSTSDFileFormat(fn)
{
    // INITIALIZE DATA MEMBERS
    fileRead = false;
    filename = strdup(fn);
    // File handles
    handle = NULL;
    gzhandle = Z_NULL;
    // Data contents
    nb3dmats = 0;
    nb2dmats = 0;
    nb1dmats = 0;
    nb3dcells = 0;
    nb2dcells = 0;
    nb1dcells = 0;
    cdim = 0;
    nbnodes = 0;
    nbloadsets = 0;
    nbfalsv = 0;
    nbnolsv = 0;
    revconnect = 0;
    nbfaextv = -1;
    nbnff = -1;
    // Sets the mesh's range
    range[0] = 1.e30;
    range[1] = -1.e30;
    range[2] = 1.e30;
    range[3] = -1.e30;
    range[4] = 1.e30;
    range[5] = -1.e30;
    // Reset the number of elements for every type:
    for (int i=0; i< 7; i++)
        nbletsptyp[i] = 0;

    nbfreesets = -1; // The number of different free sets in 3D
}

// ****************************************************************************
//  Method: avtunvFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Thu Sep 27 17:38:20 PST 2012
//
// ****************************************************************************

void
avtunvFileFormat::FreeUpResources(void)
{
    // Deallocate each element if required:
    if (revconnect > 0)
    {
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg;
        for (itrg = meshUnvNodes.begin(); itrg != meshUnvNodes.end(); itrg++)
            itrg->nod2elts.clear();
        revconnect = 0;
    }
    meshUnvNodes.clear();

    // Deallocate the node list for all elements:
    set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
    for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        delete [] itre->nodes;

    meshUnvElements.clear();

    // Remove face pressures if any:
    for (int i=0; i<meshUnvFacePressures.size(); i++)
        meshUnvFacePressures[i].faces.clear();

    meshUnvFacePressures.clear();
}


// ****************************************************************************
//  Method: avtunvFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Thu Sep 27 17:38:20 PST 2012
//
// ****************************************************************************

void
avtunvFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // CODE TO ADD A MESH
    ReadFile();
    //
    string meshname = "mesh";
    //
    // AVT_RECTILINEAR_MESH, AVT_CURVILINEAR_MESH, AVT_UNSTRUCTURED_MESH,
    // AVT_POINT_MESH, AVT_SURFACE_MESH, AVT_UNKNOWN_MESH
    avtMeshType mt = AVT_UNSTRUCTURED_MESH;
    //
    int nblocks = 1;
    int block_origin = 0;
    int spatial_dimension = 3;
    int topological_dimension = cdim;
    double *extents = NULL;
    //
    // Here's the call that tells the meta-data object that we have a mesh:
    //
    AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                      spatial_dimension, topological_dimension);

    if (nb3dcells > 0  && (nb2dcells > 0 || nb1dcells > 0))
        AddMeshToMetaData(md, "volmesh", mt, extents, nblocks, block_origin,
                          spatial_dimension, 3);
    // In case there is a 3D mesh and a 2D one:
    if (nb3dcells > 0  && nb2dcells > 0)
        AddMeshToMetaData(md, "surfmesh", mt, extents, nblocks, block_origin,
                          spatial_dimension, 2);
    if ((nb3dcells > 0 || nb2dcells > 0)  && nb1dcells > 0)
        AddMeshToMetaData(md, "wiremesh", mt, extents, nblocks, block_origin,
                          spatial_dimension, 1);

    // Add the face presure stuffs:
    if (nbloadsets > 0)
    {
        debug2 << "GRF: Populate AVT_UNSTRUCTURED_MESH name=facemesh" << endl;
        AddMeshToMetaData(md, "facemesh", AVT_UNSTRUCTURED_MESH, extents, nblocks, block_origin,
                          spatial_dimension, 2);
        AddScalarVarToMetaData(md, "sets_surfaces", "facemesh", AVT_ZONECENT);
    }
    if (nb3dcells > 0 || nb2dcells > 0)
    {
        // Add the free faces mesh:
        debug2 << "GRF: Populate AVT_UNSTRUCTURED_MESH name=freemesh" << endl;
        AddMeshToMetaData(md, "freemesh", AVT_UNSTRUCTURED_MESH, extents, nblocks, block_origin,
                          spatial_dimension, cdim-1);
        AddScalarVarToMetaData(md, "free_surfaces", "freemesh", AVT_ZONECENT);
    }

    //
    // CODE TO ADD A SCALAR VARIABLE
    //
    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    // string varname = ...
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    // AddScalarVarToMetaData(md, varname, mesh_for_this_var, cent);
    //

    //
    // CODE TO ADD A VECTOR VARIABLE
    //
    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    // string varname = ...
    // int vector_dim = 2;
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    // AddVectorVarToMetaData(md, varname, mesh_for_this_var, cent,vector_dim);
    //

    //
    // CODE TO ADD A TENSOR VARIABLE
    //
    // string mesh_for_this_var = meshname; // ??? -- could be multiple meshes
    // string varname = ...
    // int tensor_dim = 9;
    //
    // AVT_NODECENT, AVT_ZONECENT, AVT_UNKNOWN_CENT
    // avtCentering cent = AVT_NODECENT;
    //
    //
    // Here's the call that tells the meta-data object that we have a var:
    //
    // AddTensorVarToMetaData(md, varname, mesh_for_this_var, cent,tensor_dim);
    //

    //
    // CODE TO ADD A MATERIAL
    //
    string mesh_for_mat = "mesh"; // ??? -- could be multiple meshes
    string matname = "materials";
    int nmats = max(nb3dmats, nb2dmats);
    vector<string> mnames;
    for (int i = 0; i < nmats; i++)
    {
        char str[32];
        sprintf(str, "mat%d", i+1);
#if INTERACTIVEREAD
        if (debuglevel >= 4) fprintf(stdout,"Material %s.\n",str);
#else
        debug4 << "Material " << str << endl;
#endif
        mnames.push_back(str);
    }
    //
    // Here's the call that tells the meta-data object that we have a mat:
    //
    AddMaterialToMetaData(md, matname, mesh_for_mat, nmats, mnames);
    // In case there is a 3D mesh and a 2D one:
    if (nb3dcells > 0  && (nb2dcells > 0 || nb1dcells > 0))
        AddMaterialToMetaData(md, "volmats", "volmesh", nmats, mnames);

    if (nb3dcells > 0  && nb2dcells > 0)
        AddMaterialToMetaData(md, "surfmats", "surfmesh", nmats, mnames);

    if ((nb3dcells > 0 || nb2dcells > 0)  && nb1dcells > 0)
        AddMaterialToMetaData(md, "wiremats", "wiremesh", nmats, mnames);

    mnames.clear();

    if (nbloadsets > 0)
    {
        vector<string> lnames;
        for (int i=0; i<meshUnvFacePressures.size(); i++)
            lnames.push_back(meshUnvFacePressures[i].name);

        AddMaterialToMetaData(md, "load_sets", "facemesh", nbloadsets, lnames);
        lnames.clear();
        // Add the normals mesh here only:
        AddVectorVarToMetaData(md, "sets_normals", "facemesh", AVT_ZONECENT, 3);
        Expression my_expr;
        my_expr.SetName("rev_sets_normals");
        my_expr.SetDefinition("-sets_normals");
        my_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&my_expr);
    }
    if (nb3dcells > 0 || nb2dcells > 0)
    {
        // How to build the boundaries ?
        AddVectorVarToMetaData(md, "free_normals", "freemesh", AVT_ZONECENT, 3);
        // Here's the way to add expressions:
        Expression my_expr;
        my_expr.SetName("rev_free_normals");
        my_expr.SetDefinition("-free_normals");
        my_expr.SetType(Expression::VectorMeshVar);
        md->AddExpression(&my_expr);
        // Read the data or assume a reasonable number of boundaries ?
        int nmats = 20; // avtunvFileFormat::getNbfreeSets();
        if (nmats > 0)
        {
            // Propose a list of boundaries:
            vector<string> fnames;
            for (int i = 0; i < nmats; i++)
            {
                char str[32];
                if (i == 0 && cdim == 3)
                    sprintf(str, "OUTER");
                else
                    sprintf(str, "INN%d", i);

#if INTERACTIVEREAD
                if (debuglevel >= 4) fprintf(stdout,"Boundary %s.\n",str);
#else
                debug4 << "Boundary " << str << endl;
#endif
                fnames.push_back(str);
            }
            //
            // Here's the call that tells the meta-data object that we have a mat:
            //
            AddMaterialToMetaData(md, "boundaries", "freemesh", nbfreesets, fnames);
            fnames.clear();
        }
    }
    //
    //
    // Here's the way to add expressions:
    //Expression momentum_expr;
    //momentum_expr.SetName("momentum");
    //momentum_expr.SetDefinition("{u, v}");
    //momentum_expr.SetType(Expression::VectorMeshVar);
    //md->AddExpression(&momentum_expr);
    //Expression KineticEnergy_expr;
    //KineticEnergy_expr.SetName("KineticEnergy");
    //KineticEnergy_expr.SetDefinition("0.5*(momentum*momentum)/(rho*rho)");
    //KineticEnergy_expr.SetType(Expression::ScalarMeshVar);
    //md->AddExpression(&KineticEnergy_expr);
}


// ****************************************************************************
//  Method: avtunvFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Thu Sep 27 17:38:20 PST 2012
//
// ****************************************************************************

vtkDataSet *
avtunvFileFormat::GetMesh(const char *meshname)
{
#if INTERACTIVEPLOT
    if (debuglevel >= 2) fprintf(stdout, "meshname=%s\n",meshname);
#else
    debug2 << "meshname=" << meshname << endl;
#endif
    if (strcmp(meshname, "mesh") == 0)
    {
        // Builds the VTK data structure:
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        vtkPoints   *pts = vtkPoints::New();
        // Builds the VTK data structure:
#if INTERACTIVEPLOT
        if (debuglevel >= 3) fprintf(stdout,"Building #nodes=%d\n",nbnodes);
#else
        debug3 << "Building #nodes=" << nbnodes << endl;
#endif
        pts->SetNumberOfPoints(nbnodes);
        ugrid->SetPoints(pts);
        pts->Delete();

        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrn;
        for (itrn = meshUnvNodes.begin(); itrn != meshUnvNodes.end(); itrn++)
        {
            pts->SetPoint(itrn->number, itrn->x, itrn->y, itrn->z);
#if INTERACTIVEPLOT
            if (debuglevel >= 5) fprintf(stdout,"Node %d at position (%lf,%lf,%lf)\n",itrn->number, itrn->x, itrn->y, itrn->z);
#endif
        }
        ugrid->Allocate(nb3dcells+nb2dcells+nb1dcells);
        vtkIdType verts[8]; // Element nodes in Ideas convention
        vtkIdType cverts[8]; // Element nodes in VTK convention, may differ on some kinds of elements
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
        UnvNode anUnvNode;
#if INTERACTIVEPLOT
        if (debuglevel >= 3) fprintf(stdout,"Building #elements=%d\n",nb3dcells+nb2dcells);
#else
        debug3 << "Building #elements=" << nb3dcells+nb2dcells << endl;
#endif
        for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        {
            switch (itre->typelt)
            {
            case 111:
                for (int i=0; i < 4; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
                    verts[i] = itrn->number;
                }
#if INTERACTIVEPLOT
                if (debuglevel >= 5) fprintf(stdout, "elt %d, node=%d\n",
                                                 itre->number,itre->nodes[0]);
#endif
                ugrid->InsertNextCell(VTK_TETRA, 4, verts);
                break;
            case 115:
                for (int i=0; i < 8; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
#if UNVCHECK
                    if (itrn == meshUnvNodes.end())
                    {
                        fprintf(stderr,"Probleme element %d local node %d\n",itre->label,i);
                    }
#endif
                    verts[i] = itrn->number;
                }
#if INTERACTIVEPLOT
                if (debuglevel >= 5) fprintf(stdout, "elt %d, nodes=(%d,%d)\n",
                                                 itre->number,itre->nodes[0],itre->nodes[7]);
#endif
                cverts[0]=verts[1];
                cverts[1]=verts[2];
                cverts[2]=verts[3];
                cverts[3]=verts[0];
                cverts[4]=verts[5];
                cverts[5]=verts[6];
                cverts[6]=verts[7];
                cverts[7]=verts[4];
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, cverts);
                break;
            case 112:
                for (int i=0; i < 6; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
#if UNVCHECK
                    if (itrn == meshUnvNodes.end())
                    {
                        fprintf(stderr,"Probleme element %d local node %d\n",itre->label,i);
                    }
#endif
                    verts[i] = itrn->number;
                }
#if INTERACTIVEPLOT
                if (debuglevel >= 5) fprintf(stdout, "elt %d, nodes=(%d,%d)\n",
                                                 itre->number,itre->nodes[0],itre->nodes[5]);
#endif
                cverts[0]=verts[0];
                cverts[1]=verts[2];
                cverts[2]=verts[1];
                cverts[3]=verts[3];
                cverts[4]=verts[5];
                cverts[5]=verts[4];
                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
                break;
            case 312:
                for (int i=0; i < 5; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
#if UNVCHECK
                    if (itrn == meshUnvNodes.end())
                    {
                        fprintf(stderr,"Probleme element %d local node %d\n",itre->label,i);
                    }
#endif
                    verts[i] = itrn->number;
                }
#if INTERACTIVEPLOT
                if (debuglevel >= 5) fprintf(stdout, "elt %d, nodes=(%d,%d)\n",
                                                 itre->number,itre->nodes[0],itre->nodes[4]);
#endif
                ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
                break;
            case 91:
                for (int i=0; i < 3; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
                    verts[i] = itrn->number;
                }
#if INTERACTIVEPLOT
                if (debuglevel >= 5) fprintf(stdout, "elt %d, node=%d\n",itre->number,itre->nodes[0]);
#endif
                ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                break;
            case 94:
                for (int i=0; i < 4; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
                    verts[i] = itrn->number;
                }
#if INTERACTIVEPLOT
                if (debuglevel >= 5) fprintf(stdout, "elt %d, node=%d\n",itre->number,itre->nodes[0]);
#endif
                ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                break;
            case 21:
                for (int i=0; i < 2; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
                    verts[i] = itrn->number;
                }
#if INTERACTIVEPLOT
                if (debuglevel >= 5) fprintf(stdout, "elt %d, node=%d\n",itre->number,itre->nodes[0]);
#endif
                ugrid->InsertNextCell(VTK_LINE, 2, verts);
                break;
            default:
                break;
            }
        }
        return ugrid;
    }
    else if (strcmp(meshname, "surfmesh") == 0)
    {
        // Builds the VTK data structure:
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        vtkPoints   *pts = vtkPoints::New();
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrn;
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg;
        UnvNode anUnvNode;
        std::set<UnvNode, UnvNode::compare_UnvNode> maliste;  // Temporary node list
        if (nb3dcells > nb2dcells)
        {
            // Make a reduced set of nodes:
#if INTERACTIVEPLOT
            if (debuglevel >= 3) fprintf(stdout, "Making a reduced set of nodes\n");
#else
            debug3 << "Making a reduced set of nodes" << endl ;
#endif
            anUnvNode.number = 0 ;
            for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
                if (avtunvFileFormat::is2DKnownElt(itre->typelt) >= 0) {
                    int nn = avtunvFileFormat::getNbnodes(itre->typelt);
                    for (int i=0; i < nn; i++)
                    {
                        anUnvNode.label = itre->nodes[i];
                        itrn = maliste.find(anUnvNode);
                        if (itrn == maliste.end()) {
                            maliste.insert(anUnvNode) ;
                            anUnvNode.number++; // This is the actual number of nodes
                        }
                    }
                }
            pts->SetNumberOfPoints(anUnvNode.number);
#if INTERACTIVEPLOT
            if (debuglevel >= 3) fprintf(stdout, "Reduced set of #nodes=%d\n",anUnvNode.number);
#else
            debug3 << "Reduced set of #nodes=" << anUnvNode.number << endl ;
#endif
       }
        else
        {
            pts->SetNumberOfPoints(nbnodes);
        }
        ugrid->SetPoints(pts);
        pts->Delete();

        if (nb3dcells > nb2dcells)
            // Loop on reduced list of nodes:
            for (itrn = maliste.begin(); itrn != maliste.end(); itrn++)
            {
                anUnvNode.label = itrn->label ;
                itrg = meshUnvNodes.find(anUnvNode);
                pts->SetPoint(itrn->number, itrg->x, itrg->y, itrg->z);
            }
        else
            for (itrn = meshUnvNodes.begin(); itrn != meshUnvNodes.end(); itrn++)
                pts->SetPoint(itrn->number, itrn->x, itrn->y, itrn->z);
        
        ugrid->Allocate(nb2dcells);
        vtkIdType verts[8]; // Element nodes in Ideas convention
        vtkIdType cverts[8]; // Element nodes in VTK convention, may differ on some kinds of elements
        if (nb3dcells > nb2dcells)
        {
            for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
            {
                switch (itre->typelt)
                {
                case 91:
                    for (int i=0; i < 3; i++)
                    {
                        anUnvNode.label = itre->nodes[i];
                        itrn = maliste.find(anUnvNode);
                        verts[i] = itrn->number;
                    }
                    ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                    break ;
                case 94:
                    for (int i=0; i < 4; i++)
                    {
                        anUnvNode.label = itre->nodes[i];
                        itrn = maliste.find(anUnvNode);
                        verts[i] = itrn->number;
                    }
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                    break ;
                default:
                    break ;
                }
            }
            maliste.clear() ;
        }
        else
            for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
            {
                switch (itre->typelt)
                {
                case 91:
                    for (int i=0; i < 3; i++)
                    {
                        anUnvNode.label = itre->nodes[i];
                        itrn = meshUnvNodes.find(anUnvNode);
                        verts[i] = itrn->number;
                    }
                    ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                    break ;
                case 94:
                    for (int i=0; i < 4; i++)
                    {
                        anUnvNode.label = itre->nodes[i];
                        itrn = meshUnvNodes.find(anUnvNode);
                        verts[i] = itrn->number;
                    }
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);
                    break ;
                default:
                    break ;
                }
            }
        
        return ugrid;
    }
    else if (strcmp(meshname, "wiremesh") == 0)
    {
        // Builds the VTK data structure:
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        vtkPoints   *pts = vtkPoints::New();
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrn;
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg;
        UnvNode anUnvNode;
        std::set<UnvNode, UnvNode::compare_UnvNode> maliste;  // Temporary node list
        if (nb3dcells+nb2dcells > nb1dcells)
        {
            // Make a reduced set of nodes:
#if INTERACTIVEPLOT
            if (debuglevel >= 3) fprintf(stdout, "Making a reduced set of nodes");
#else
            debug3 << "Making a reduced set of nodes" << endl ;
#endif
            anUnvNode.number = 0 ;
            for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
                if (avtunvFileFormat::is1DKnownElt(itre->typelt) >= 0) {
                    int nn = avtunvFileFormat::getNbnodes(itre->typelt);
                    for (int i=0; i < nn; i++)
                    {
                        anUnvNode.label = itre->nodes[i];
                        itrn = maliste.find(anUnvNode);
                        if (itrn == maliste.end()) {
                            maliste.insert(anUnvNode) ;
                            anUnvNode.number++; // This is the actual number of nodes
                        }
                    }
                }
            pts->SetNumberOfPoints(anUnvNode.number);
#if INTERACTIVEPLOT
            if (debuglevel >= 3) fprintf(stdout, "Reduced set of #nodes=",anUnvNode.number);
#else
            debug3 << "Reduced set of #nodes=" << anUnvNode.number << endl ;
#endif
        }
        else
        {
            pts->SetNumberOfPoints(nbnodes);
        }
        ugrid->SetPoints(pts);
        pts->Delete();
        
        if (nb3dcells+nb2dcells > nb1dcells)
            // Loop on reduced list of nodes:
            for (itrn = maliste.begin(); itrn != maliste.end(); itrn++)
            {
                anUnvNode.label = itrn->label ;
                itrg = meshUnvNodes.find(anUnvNode);
                pts->SetPoint(itrn->number, itrg->x, itrg->y, itrg->z);
            }
        else
            for (itrn = meshUnvNodes.begin(); itrn != meshUnvNodes.end(); itrn++)
                pts->SetPoint(itrn->number, itrn->x, itrn->y, itrn->z);
        
        ugrid->Allocate(nb1dcells);
        vtkIdType verts[8]; // Element nodes in Ideas convention
        vtkIdType cverts[8]; // Element nodes in VTK convention, may differ on some kinds of elements
        if (nb3dcells+nb2dcells > nb1dcells)
        {
            for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
            {
                if (itre->typelt == 21)
                {
                    for (int i=0; i < 2; i++)
                    {
                        anUnvNode.label = itre->nodes[i];
                        itrn = maliste.find(anUnvNode);
                        verts[i] = itrn->number;
                    }
                    ugrid->InsertNextCell(VTK_LINE, 2, verts);
                }
            }
            maliste.clear() ;
        }
        else
            for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
            {
                if (itre->typelt == 21)
                {
                    for (int i=0; i < 2; i++)
                    {
                        anUnvNode.label = itre->nodes[i];
                        itrn = meshUnvNodes.find(anUnvNode);
                        verts[i] = itrn->number;
                    }
                    ugrid->InsertNextCell(VTK_LINE, 2, verts);
                }
            }
        
        return ugrid;
    }
    else if (strcmp(meshname, "volmesh") == 0)
    {
        // Builds the VTK data structure:
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        vtkPoints   *pts = vtkPoints::New();
        pts->SetNumberOfPoints(nbnodes);
        ugrid->SetPoints(pts);
        pts->Delete();

        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrn;
        for (itrn = meshUnvNodes.begin(); itrn != meshUnvNodes.end(); itrn++)
            pts->SetPoint(itrn->number, itrn->x, itrn->y, itrn->z);
        
        ugrid->Allocate(nb3dcells);
        vtkIdType verts[8]; // Element nodes in Ideas convention
        vtkIdType cverts[8]; // Element nodes in VTK convention, may differ on some kinds of elements
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
        UnvNode anUnvNode;
        for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
        {
            switch (itre->typelt)
            {
            case 111:
                for (int i=0; i < 4; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
                    verts[i] = itrn->number;
                }
                ugrid->InsertNextCell(VTK_TETRA, 4, verts);
                break ;
            case 115:
                for (int i=0; i < 8; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
                    verts[i] = itrn->number;
                }
                cverts[0]=verts[1];
                cverts[1]=verts[2];
                cverts[2]=verts[3];
                cverts[3]=verts[0];
                cverts[4]=verts[5];
                cverts[5]=verts[6];
                cverts[6]=verts[7];
                cverts[7]=verts[4];
                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, cverts);
                break ;
            case 112:
                for (int i=0; i < 6; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
                    verts[i] = itrn->number;
                }
                cverts[0]=verts[0];
                cverts[1]=verts[2];
                cverts[2]=verts[1];
                cverts[3]=verts[3];
                cverts[4]=verts[5];
                cverts[5]=verts[4];
                ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
                break ;
            case 312:
                for (int i=0; i < 5; i++)
                {
                    anUnvNode.label = itre->nodes[i];
                    itrn = meshUnvNodes.find(anUnvNode);
                    verts[i] = itrn->number;
                }
                ugrid->InsertNextCell(VTK_PYRAMID, 5, verts);
                break ;
            default:
                break ;
            }
        }
        return ugrid;
    }
    else if (strcmp(meshname, "freemesh") == 0)
    {
        // Builds the VTK data structure:
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        vtkPoints   *pts = vtkPoints::New();
        // Build the free faces mesh data structure:
        int nbcells = avtunvFileFormat::getNbfaextv(); // Number of faces
#if INTERACTIVEPLOT
        if (debuglevel >= 2) fprintf(stdout,"Number of free faces=%d\n",nbcells);
#else
        debug2 << "Number of free faces=" << nbcells << endl;
#endif
        // Computes the lowest nodes number:
        int nbn = avtunvFileFormat::getNbnodesFreeFaces(); // Number of nodes for the structure
#if INTERACTIVEPLOT
        if (debuglevel >= 2) fprintf(stdout,"Number of free nodes=%d\n",nbn);
#else
        debug2 << "Number of free nodes=" << nbn << endl;
#endif
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrl; // Sub-list node iterator
        UnvElement anUnvElement; // an element
        UnvNode anotherUnvNode; // a node
        UnvNode anUnvNode; // a node
        set<UnvNode, UnvNode::compare_UnvNode> maliste;  // Sub list temporary
        // Fills the nodes:
        vtkIdType verts[4];
        // Builds the VTK data structure:
        pts->SetNumberOfPoints(nbn);
        ugrid->SetPoints(pts);
        pts->Delete();
        ugrid->Allocate(nbcells);
#if INTERACTIVEPLOT
        if (debuglevel >= 2) fprintf(stdout, "Free boundary mesh made of %d faces and %d nodes\n",nbcells,nbn);
#else
        debug2 << "Free boundary mesh made of " << nbcells << " faces and " << nbn << " nodes " << endl;
#endif
        // Fills the VTK data structures, both nodes and elements right at the same time
        anotherUnvNode.number=0;
        nbcells = 0;
        for (int j=0; j<freeUnvFaces.size(); j++)
        {
            int iel = freeUnvFaces[j].element;
            int facloc = freeUnvFaces[j].facloc;
            // Fetch the element:
            anUnvElement.label = iel;
            itre = meshUnvElements.find(anUnvElement);
            int iflo3 = -1, iflo2 = -1;
            iflo3 = avtunvFileFormat::is3DKnownElt(itre->typelt);
            if (iflo3 >= 0)
            {
                nbcells++;
                for (int ln=0; ln<4; ln++)
                {
                    int in = nodefac[iflo3][facloc-1][ln]; // local node number
                    if (in > 0)
                    {
                        anUnvNode.label = itre->nodes[in-1];
                        itrg = meshUnvNodes.find(anUnvNode);
#if INTERACTIVEPLOT
                        if (debuglevel >= 5) fprintf(stdout," Conn Elt %d fac %d Node %d at (%lf,%lf,%lf)\n",
                                                         iel,facloc,itrg->label,itrg->x,itrg->y,itrg->z);
#endif
                        itrl = maliste.find(anUnvNode);
                        if (itrl != maliste.end())
                            // Already known stuff:
                            verts[ln] = itrl->number;
                        else
                        {
                            anotherUnvNode.label = anUnvNode.label;
                            maliste.insert(anotherUnvNode);
                            pts->SetPoint(anotherUnvNode.number, itrg->x, itrg->y, itrg->z);
                            verts[ln] = anotherUnvNode.number;
                            anotherUnvNode.number++; // This is the actual number of nodes
                        }
                    }
                }
                // Note that all nodes are known when creating the VTK element:
                if (nodefac[iflo3][facloc-1][3] == 0)
                    ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                else
                    ugrid->InsertNextCell(VTK_QUAD, 4, verts);

            }
            else
            {
                iflo2 = avtunvFileFormat::is2DKnownElt(itre->typelt);
                if (iflo2 >= 0)
                {
                    nbcells++;
                    for (int ln=0; ln<2; ln++)
                    {
                        int in = nodefac2[iflo2][facloc-1][ln]; // local node number
                        if (in > 0)
                        {
                            anUnvNode.label = itre->nodes[in-1];
                            itrg = meshUnvNodes.find(anUnvNode);
#if INTERACTIVEPLOT
                            if (debuglevel >= 5) fprintf(stdout," Conn Elt %d fac %d Node %d at (%lf,%lf,%lf)\n",
                                                             iel,facloc,itrg->label,itrg->x,itrg->y,itrg->z);
#endif
                            itrl = maliste.find(anUnvNode);
                            if (itrl != maliste.end())
                                // Already known stuff:
                                verts[ln] = itrl->number;
                            else
                            {
                                anotherUnvNode.label = anUnvNode.label;
                                maliste.insert(anotherUnvNode);
                                pts->SetPoint(anotherUnvNode.number, itrg->x, itrg->y, itrg->z);
                                verts[ln] = anotherUnvNode.number;
                                anotherUnvNode.number++; // This is the actual number of nodes
                            }
                        }
                    }
                    ugrid->InsertNextCell(VTK_LINE, 2, verts);
                }
            }
        }
        maliste.clear();
        return ugrid;
    }
    else if (strcmp(meshname, "facemesh") == 0)
    {
        // Builds the VTK data structure:
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        vtkPoints   *pts = vtkPoints::New();
        int nbn; // Number of nodes for the structure
        int nbcells = 0; // Number of faces
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrl; // Sub-list node iterator
        UnvElement anUnvElement; // an element
        UnvNode anotherUnvNode; // a node
        UnvNode anUnvNode;
        set<UnvNode, UnvNode::compare_UnvNode> maliste;  // Sub list
        // Fills the nodes:
        vtkIdType verts[4];
        // Builds the VTK data structure:
        nbn = avtunvFileFormat::getNbnolsv();
        pts->SetNumberOfPoints(nbn);
        ugrid->SetPoints(pts);
        pts->Delete();
        ugrid->Allocate(nbfalsv);
#if INTERACTIVEPLOT
        if (debuglevel >= 2) fprintf(stdout, "Face Pressure mesh made of %d faces and %d nodes\n",nbfalsv,nbn);
#else
        debug2 << "Face Pressure mesh made of " << nbfalsv << " faces and " << nbn << " nodes " << endl;
#endif
        anotherUnvNode.number=0;
        for (int i=0; i<meshUnvFacePressures.size(); i++)
            for (int j=0; j<meshUnvFacePressures[i].faces.size(); j++)
            {
                int iel = meshUnvFacePressures[i].faces[j].element;
                int facloc = meshUnvFacePressures[i].faces[j].facloc;
                // Fetch the element:
                anUnvElement.label = iel;
                itre = meshUnvElements.find(anUnvElement);
                int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                if (iflo >= 0)
                {
                    nbcells++;
                    for (int ln=0; ln<4; ln++)
                    {
                        int in = nodefac[iflo][facloc-1][ln]; // local node number
                        if (in > 0)
                        {
                            anUnvNode.label = itre->nodes[in-1];
                            itrg = meshUnvNodes.find(anUnvNode);
                            itrl = maliste.find(anUnvNode);
                            if (itrl != maliste.end())
                                // Already known stuff:
                                verts[ln] = itrl->number;
                            else
                            {
                                anotherUnvNode.label = anUnvNode.label;
                                maliste.insert(anotherUnvNode);
                                pts->SetPoint(anotherUnvNode.number, itrg->x, itrg->y, itrg->z);
                                verts[ln] = anotherUnvNode.number;
                                anotherUnvNode.number++; // This is the actual number of nodes
                            }
                        }
                    }
                    if (nodefac[iflo][facloc-1][3] == 0)
                        ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
                    else
                        ugrid->InsertNextCell(VTK_QUAD, 4, verts);

                }
            }

        maliste.clear();
        return ugrid;
    }
}


// ****************************************************************************
//  Method: avtunvFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Thu Sep 27 17:38:20 PST 2012
//
// ****************************************************************************

vtkDataArray *
avtunvFileFormat::GetVar(const char *varname)
{
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION1(InvalidVariableException, varname);

    //
    // If you do have a scalar variable, here is some code that may be helpful.
    //
    if (strcmp(varname, "sets_surfaces") == 0 || strcmp(varname, "free_surfaces") == 0)
    {
        int ntuples; // this is the number of entries in the variable.
        vtkFloatArray *rv = vtkFloatArray::New();
        float *one_entry = new float[3];
        if (strcmp(varname, "sets_surfaces") == 0)
            rv->SetNumberOfTuples(nbfalsv);
        else if (strcmp(varname, "free_surfaces") == 0)
        {
            ntuples = avtunvFileFormat::getNbfaextv();
            rv->SetNumberOfTuples(ntuples);
        }
        ntuples = 0;
        UnvElement anUnvElement; // an element
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
        if (strcmp(varname, "sets_surfaces") == 0)
        {
            for (int i=0; i<meshUnvFacePressures.size(); i++)
                for (int j=0; j<meshUnvFacePressures[i].faces.size(); j++)
                {
                    int iel = meshUnvFacePressures[i].faces[j].element;
                    // Fetch the element:
                    anUnvElement.label = iel;
                    itre = meshUnvElements.find(anUnvElement);
                    int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                    if (iflo >= 0)
                    {
                        // Set one_entry...
                        int facloc = meshUnvFacePressures[i].faces[j].facloc;
                        avtunvFileFormat::getNormal3D(one_entry, itre, iflo, facloc);
                        rv->SetTuple1(ntuples,sqrt(one_entry[0]*one_entry[0]+one_entry[1]*one_entry[1]+one_entry[2]*one_entry[2]));
                        ntuples++;
                    }
                    else
                    {
                        iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
                        if (iflo >= 0)
                        {
                            int facloc = meshUnvFacePressures[i].faces[j].facloc;
                            avtunvFileFormat::getNormal2D(one_entry, itre, facloc);
                            rv->SetTuple1(ntuples,sqrt(one_entry[0]*one_entry[0]+one_entry[1]*one_entry[1]+one_entry[2]*one_entry[2]));
                            ntuples++;
                        }
                    }
                }

        }
        else if (strcmp(varname, "free_surfaces") == 0)
        {
            for (int j=0; j<freeUnvFaces.size(); j++)
            {
                int iel = freeUnvFaces[j].element;
                // Fetch the element:
                anUnvElement.label = iel;
                itre = meshUnvElements.find(anUnvElement);
                int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                int iko = 0;
                if (iflo >= 0)
                {
                    // Set one_entry...
                    int facloc = freeUnvFaces[j].facloc;
                    avtunvFileFormat::getNormal3D(one_entry, itre, iflo, facloc);
                    rv->SetTuple1(ntuples, sqrt(one_entry[0]*one_entry[0]+one_entry[1]*one_entry[1]+one_entry[2]*one_entry[2]));
                    ntuples++;
                }
                else
                {
                    iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
                    if (iflo >= 0)
                    {
                        int facloc = freeUnvFaces[j].facloc;
                        avtunvFileFormat::getNormal2D(one_entry, itre, facloc);
                        rv->SetTuple1(ntuples, sqrt(one_entry[0]*one_entry[0]+one_entry[1]*one_entry[1]+one_entry[2]*one_entry[2]));
                        ntuples++;
                    }
                }
            }
        }
        return rv;
    }
}


// ****************************************************************************
//  Method: avtunvFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: cessenat -- generated by xml2avt
//  Creation:   Thu Sep 27 17:38:20 PST 2012
//
// ****************************************************************************

vtkDataArray *
avtunvFileFormat::GetVectorVar(const char *varname)
{
    // YOU MUST IMPLEMENT THIS

    debug2 << "GRF: GetVectorVar function match to set normals = " << varname << endl;
    if (strcmp(varname, "sets_normals") == 0 || strcmp(varname, "free_normals") == 0)
    {
        //
        // If you have a file format where variables don't apply (for example a
        // strictly polygonal format like the STL (Stereo Lithography) format,
        // then uncomment the code below.
        //
        // EXCEPTION1(InvalidVariableException, varname);
        //
        //
        // If you do have a vector variable, here is some code that may be helpful.
        //
        int ncomps = 3;  // This is the rank of the vector - typically 2 or 3.
        int ntuples = 0; // this is the number of entries in the variable.
        vtkFloatArray *rv = vtkFloatArray::New();
        rv->SetNumberOfComponents(ncomps);
        float *one_entry = new float[ncomps];
        UnvElement anUnvElement; // an element
        set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
        UnvNode anUnvNode;
        set<UnvNode, UnvNode::compare_UnvNode>::iterator itrg; // Global node iterator
        double x1,x2,y1,y2,z1,z2;
        if (strcmp(varname, "sets_normals") == 0)
            rv->SetNumberOfTuples(nbfalsv);
        else if (strcmp(varname, "free_normals") == 0)
        {
            ntuples = avtunvFileFormat::getNbfaextv();
            rv->SetNumberOfTuples(ntuples);
        }
        ntuples = 0;
        if (strcmp(varname, "sets_normals") == 0)
            for (int i=0; i<meshUnvFacePressures.size(); i++)
                for (int j=0; j<meshUnvFacePressures[i].faces.size(); j++)
                {
                    int iel = meshUnvFacePressures[i].faces[j].element;
                    // Fetch the element:
                    anUnvElement.label = iel;
                    itre = meshUnvElements.find(anUnvElement);
                    int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                    if (iflo >= 0)
                    {
                        // Set one_entry...
                        int facloc = meshUnvFacePressures[i].faces[j].facloc;
                        avtunvFileFormat::getNormal3D(one_entry, itre, iflo, facloc);
#if INTERACTIVEPLOT
                        if (debuglevel >= 5) fprintf(stdout," Normal(%d)=(%lf,%lf,%lf)\n",
                                                         ntuples,one_entry[0],one_entry[1],one_entry[2]);
#endif
                        rv->SetTuple(ntuples, one_entry);
                        ntuples++;
                    }
                    else
                    {
                        iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
                        if (iflo >= 0)
                        {
                            int facloc = meshUnvFacePressures[i].faces[j].facloc;
                            avtunvFileFormat::getNormal2D(one_entry, itre, facloc);
#if INTERACTIVEPLOT
                            if (debuglevel >= 5) fprintf(stdout," Normal(%d)=(%lf,%lf,%lf)\n",
                                                             ntuples,one_entry[0],one_entry[1],one_entry[2]);
#endif
                            rv->SetTuple(ntuples, one_entry);
                            ntuples++;
                        }
                    }
                }
        else if (strcmp(varname, "free_normals") == 0)
            for (int j=0; j<freeUnvFaces.size(); j++)
            {
                int iel = freeUnvFaces[j].element;
                // Fetch the element:
                anUnvElement.label = iel;
                itre = meshUnvElements.find(anUnvElement);
                int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                int iko = 0;
                if (iflo >= 0)
                {
                    // Set one_entry...
                    int facloc = freeUnvFaces[j].facloc;
                    avtunvFileFormat::getNormal3D(one_entry, itre, iflo, facloc);
#if INTERACTIVEPLOT
                    if (debuglevel >= 5) fprintf(stdout," Normal(%d)=(%lf,%lf,%lf)\n",
                                                     ntuples,one_entry[0],one_entry[1],one_entry[2]);
#endif
                    rv->SetTuple(ntuples, one_entry);
                    ntuples++;
                }
                else
                {
                    iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
                    if (iflo >= 0)
                    {
                        int facloc = freeUnvFaces[j].facloc;
                        avtunvFileFormat::getNormal2D(one_entry, itre, facloc);
#if INTERACTIVEPLOT
                        if (debuglevel >= 5) fprintf(stdout," Normal(%d)=(%lf,%lf,%lf)\n",
                                                         ntuples,one_entry[0],one_entry[1],one_entry[2]);
#endif
                        rv->SetTuple(ntuples, one_entry);
                        ntuples++;
                    }
                }
            }

#if INTERACTIVEPLOT
        if (debuglevel >= 3) fprintf(stdout,"Constructed %d normals\n",ntuples);
#else
        debug3 << "Constructed " << ntuples << " normals" << endl;
#endif
        delete [] one_entry;
        return rv;
    }
}

void
avtunvFileFormat::ReadFile()
{
    if (fileRead)
        return;

    ifstream in(filename.c_str());
    if (!in)
    {
        fprintf(stdout, "Raise an exception for %s\n",filename.c_str());
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }


    debuglevel = 0;
    if (DebugStream::Level5())
        debuglevel = 5;
    else if (DebugStream::Level4())
        debuglevel = 4;
    else if (DebugStream::Level3())
        debuglevel = 3;
    else if (DebugStream::Level2())
        debuglevel = 2;
    else if  (DebugStream::Level1())
        debuglevel = 1;

    TRY
    {
        nb3dcells = 0;
        nb2dcells = 0;
        nb3dmats = 0;
        nb2dmats = 0;
        nbloadsets = 0;
        nbfalsv=0;
        nbnolsv=0;
        int readingFile = visitTimer->StartTimer();
        if (strstr(filename.c_str(), ".unv") != NULL && strstr(filename.c_str(), ".unv.gz") == NULL)
        {
            handle = fopen(filename.c_str(), "r");
            if (handle == NULL)
            {
                EXCEPTION1(InvalidDBTypeException, "This unv file could not be openend.");
            }

#if INTERACTIVEREAD
            if (debuglevel >= 1) fprintf(stdout,"On the way to read unv file %s\n",filename.c_str());
#else
            debug1 << "On the way to read unv file " << filename << endl;
#endif
            int len = 2048; // Longest line length
            char buf[len]; // A line length
            int code;
            int label;
            double fac = 1.;
            while (fgets(buf, len, handle) != NULL)
            {
                if (strstr(buf, "    -1") != NULL)
                {
                    // Adding another ideas block
                    // fprintf(stdout,"Found -1 code\n");
                    //fscanf(handle, "%d", &code);
                    if (fgets(buf, len, handle) != NULL)
                    {
                        sscanf(buf, "%d", &code);
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"Found code=%d\n",code);
#else
                        debug3 << "Found code=" << code << endl;
#endif
                        if (code == 2412)
                        {
                            UnvElement anUnvElement;
                            int nod[8];
                            anUnvElement.number=0;
#if INTERACTIVEREAD
                            if (debuglevel >= 3) fprintf(stdout,"Found Element code\n");
#else
                            debug3 << "Found Element code" << endl;
#endif
                            while (fgets(buf, len, handle) != NULL)
                            {
                                int typelt, numprop,numat,colour,nbnel;
                                sscanf(buf, "%d%d%d%d%d%d\n", &label, &typelt, &numprop, &numat, &colour, &nbnel);
                                if (label < 0)
                                {
#if INTERACTIVEREAD
                                    if (debuglevel >= 2) fprintf(stdout,"Found Element section end, nb3dcells=%d, nb2dcells=%d, nb1dcells=%d\n",nb3dcells,nb2dcells,nb1dcells);
#else
                                    debug2 << "Found Element section end, #cells=(" << nb3dcells << "," << nb2dcells << "," << nb1dcells << ")" << endl;
#endif
                                    break;
                                }
                                if (fgets(buf, len, handle) != NULL)
                                {
                                    int ier; //  = avtunvFileFormat::isKnownElt(typelt);
                                    //if (typelt == 111) {
                                    switch (typelt)
                                    {
                                    case 111:
                                        sscanf(buf, "%d %d %d %d\n", &nod[0], &nod[1], &nod[2], &nod[3]);
                                        nb3dcells++;
                                        nb3dmats = max(nb3dmats, numat);
                                        ier = 0;
                                        break;
                                    case 115:
                                        //} else if (typelt == 115) {
                                        sscanf(buf, "%d %d %d %d %d %d %d %d\n", &nod[0], &nod[1], &nod[2], &nod[3], &nod[4], &nod[5], &nod[6], &nod[7]);
                                        nb3dcells++;
                                        nb3dmats = max(nb3dmats, numat);
                                        ier = 1;
                                        break;
                                    case 112:
                                        //} else if (typelt == 112) {
                                        sscanf(buf, "%d %d %d %d %d %d\n", &nod[0], &nod[1], &nod[2], &nod[3], &nod[4], &nod[5]);
                                        nb3dcells++;
                                        nb3dmats = max(nb3dmats, numat);
                                        ier = 2;
                                        break;
                                    case 312:
                                        //} else if (typelt == 312) {
                                        sscanf(buf, "%d %d %d %d %d\n", &nod[0], &nod[1], &nod[2], &nod[3], &nod[4]);
                                        nb3dcells++;
                                        nb3dmats = max(nb3dmats, numat);
                                        ier = 3;
                                        break;
                                    case 91:
                                        //} else if (typelt == 91) {
                                        sscanf(buf, "%d %d %d\n", &nod[0], &nod[1], &nod[2]);
                                        nb2dcells++;
                                        nb2dmats = max(nb2dmats, numat);
                                        ier = 4;
                                        break;
                                    case 94:
                                        //} else if (typelt == 94) {
                                        sscanf(buf, "%d %d %d %d\n", &nod[0], &nod[1], &nod[2], &nod[3]);
                                        nb2dcells++;
                                        nb2dmats = max(nb2dmats, numat);
                                        ier = 5;
                                        break;
                                    case 21:
                                        //} else if (typelt == 21) {
                                        // Linear beams
                                        if (fgets(buf, len, handle) != NULL)
                                            if (typelt == 21)
                                            {
                                                sscanf(buf, "%d %d\n", &nod[0], &nod[1]);
                                                nb1dcells++;
                                                nb1dmats = max(nb1dmats, numat);
                                                ier = 6;
                                            }
                                        break;
                                    default:
                                        //} else {
                                        ier = -1;
                                        if (typelt < 25)
                                            if (fgets(buf, len, handle) == NULL)
                                            {
                                                EXCEPTION1(InvalidDBTypeException, "This unv file has illegal typeelt.");
                                            }

                                        break;
                                    }
                                    if (ier != -1)
                                    {
                                        nbletsptyp[ier]++;
#if !defined(MDSERVER)
                                        anUnvElement.label = label;
                                        anUnvElement.typelt = typelt;
                                        anUnvElement.matid = numat;
                                        anUnvElement.nodes = new int[nbnel];
                                        for (int i=0; i < nbnel; i++)
                                            anUnvElement.nodes[i] = nod[i];

#if INTERACTIVEREAD
                                        if (debuglevel >= 4) fprintf(stdout, "\telt %d, #nodes=%d, first/last node=(%d,%d)\n",
                                            anUnvElement.label,nbnel,anUnvElement.nodes[0],anUnvElement.nodes[nbnel-1]);
#else
                                        debug4 << "\telt " << anUnvElement.label << ", #nodes=" << nbnel << ", first/last node=(" << anUnvElement.nodes[0] << "," << anUnvElement.nodes[nbnel-1] << ")" << endl;
#endif
                                        meshUnvElements.insert(anUnvElement);
                                        anUnvElement.number++;
#endif
                                    }
                                }
                            }
                        }
                        else if (code == 2411)
                        {
                            UnvNode anode;
                            anode.number=0;
#if INTERACTIVEREAD
                            if (debuglevel >= 2) fprintf(stdout,"Found Node code\n");
#else
                            debug2 << "Found Node code "  << endl;
#endif
                            int i1, i2, i3;
                            double x, y, z;
                            while (fgets(buf, len, handle) != NULL)
                            {
                                sscanf(buf, "%d%d%d%d\n", &label, &i1, &i2, &i3);
                                if (label < 0)
                                    break;

                                if (fgets(buf, len, handle) != NULL)
                                {
#if defined(MDSERVER)
                                    nbnodes++;
#else
                                    for (i1=0; i1<strlen(buf); i1++)
                                        if (buf[i1] == 'D')
                                            buf[i1] = 'E';

                                    sscanf(buf, "%lf %lf %lf\n", &x, &y, &z);
                                    x *= fac;
                                    y *= fac;
                                    z *= fac;
                                    range[0] = min(range[0],x);
                                    range[1] = max(range[1],x);
                                    range[2] = min(range[2],y);
                                    range[3] = max(range[3],y);
                                    range[4] = min(range[4],z);
                                    range[5] = max(range[5],z);
                                    anode.label = label;
                                    anode.x = x;
                                    anode.y = y;
                                    anode.z = z;
                                    meshUnvNodes.insert(anode);
                                    anode.number++;
#endif
#if INTERACTIVEREAD
                                    if (debuglevel >= 5) fprintf(stdout,"label=%d, n=%d, pos=(%lf,%lf,%lf)\n",label,anode.number,x,y,z);
#endif
                                }
                            }
#if !defined(MDSERVER)
                            nbnodes = anode.number;
#endif
#if INTERACTIVEREAD
                            if (debuglevel >= 2) fprintf(stdout,"Found Node section end, nbnodes=%d\n",nbnodes);
#else
                            debug2 << "Found Node section end, nbnodes=" << nbnodes << endl;
#endif
                        }
                        else if (code == 164)
                        {
                            char str[11]; // A line length
                            int iunit;
                            if (fgets(buf, len, handle) != NULL)
                            {
                                strncpy(str, buf, 10);
                                str[10] = '\0';
                                sscanf(str, "%d\n", &iunit);
#if INTERACTIVEREAD
                                if (debuglevel >= 3) fprintf(stdout, "unit code=%d\n",iunit);
#else
                                debug3 << "unit code=" << iunit << endl;
#endif
                                if (iunit == 5)
                                {
                                    fac = 1.e-3;
                                }
                                else if (iunit == 6)
                                {
                                    fac = 1.e-2;
                                }
                            }
                            while (fgets(buf, len, handle) != NULL)
                            {
                                if (strstr(buf, "    -1") != NULL)
                                {
#if INTERACTIVEREAD
                                    if (debuglevel >= 3) fprintf(stdout,"Found End unit section code=%d\n",code);
#else
                                    debug3 << "Found End unit section code=" << code << endl;
#endif
                                    break;
                                }
                            }
                        }
                        else if (code == 790)
                        {
                            int id, i1, i3;
                            if (fgets(buf, len, handle) != NULL)
                                sscanf(buf, "%d%d\n", &id, &i3);

                            if (fgets(buf, len, handle) != NULL)
                            {
                                int imax = strlen(buf)-1;
                                // fprintf(stdout, "imax=%d\n",imax);
                                for (i1=0; i1<strlen(buf); i1++)
                                    if (buf[i1] == ' ')
                                        buf[i1] = '_';

                                for (i1=strlen(buf)-2; i1>0; i1--)
                                    if (buf[i1] != '_')
                                    {
                                        imax = i1+1;
                                        break;
                                    }

                                // fprintf(stdout, "imax=%d\n",imax);
                                buf[imax] = '\0';
                                UnvFacePressure anfp;
                                anfp.number = nbloadsets;
                                anfp.label = id;
                                anfp.name = buf;
                                nbloadsets++;
#if INTERACTIVEREAD
                                if (debuglevel >= 3) cout << "Adding a load set id=" << anfp.label << " name='" << anfp.name << "'." << endl;
                                if (debuglevel >= 3) fprintf(stdout, "Adding a load set id=%d name='%s'.\n",anfp.label,buf);
#else
                                debug3 << "Adding a load set id=" << anfp.label << " name='" << anfp.name << "'." << endl;
#endif
                                UnvFace anUnvFace; // Elementary face pressure object
                                UnvElement anUnvElement; // an element object, assuming already built
                                set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
                                while (fgets(buf, len, handle) != NULL)
                                {
                                    if (strstr(buf, "    -1") != NULL)
                                    {
                                        meshUnvFacePressures.push_back(anfp);
#if INTERACTIVEREAD
                                        if (debuglevel >= 3) fprintf(stdout,"Found End section code=%d, fp=%d, #faces=%d\n",code,anfp.label,nbfalsv);
#else
                                        debug3 << "Found End section code=" << code << "fp=" << anfp.label << ", #faces=" << nbfalsv << endl;
#endif
                                        break;
                                    }
                                    else
                                    {
                                        int label, iel, fac, colour;
                                        double pressure;
                                        sscanf(buf, "%d%d%d%d\n", &label, &iel, &fac, &colour);
                                        anUnvFace.number = label;
                                        anUnvFace.element = iel;
                                        anUnvFace.facloc = fac;
                                        anUnvFace.matid = colour;
                                        if (fgets(buf, len, handle) != NULL)
                                        {
                                            for (i1=0; i1<strlen(buf); i1++)
                                                if (buf[i1] == 'D')
                                                    buf[i1] = 'E';

                                            sscanf(buf, "%lf\n", &pressure);
                                            anUnvFace.pressure = pressure;
                                        }
                                        if (fgets(buf, len, handle) != NULL)
                                        {
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout, "\tAdding a face presure %d iel=%d\n",anUnvFace.number,anUnvFace.element);
#else
                                            debug4 << "\tAdding a face presure " << anUnvFace.number << " iel=" << anUnvFace.element << endl;
#endif
                                            anfp.faces.push_back(anUnvFace);
                                            // Now increase the number of elements:
#if defined(MDSERVER)
                                            nbfalsv++;
#else
                                            anUnvElement.label = iel;
                                            itre = meshUnvElements.find(anUnvElement);
                                            if (itre != meshUnvElements.end())
                                            {
                                                int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                                                if (iflo >= 0)
                                                {
                                                    nbfalsv++;
                                                }
                                                else
                                                {
                                                    iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
                                                    if (iflo >= 0)
                                                    {
                                                        nbfalsv++;
                                                    }
                                                }
                                            }
#endif
                                        }
                                    }
                                }
                            }
                        }
                        else
                            while (fgets(buf, len, handle) != NULL)
                                if (strstr(buf, "    -1") != NULL)
                                {
#if INTERACTIVEREAD
                                    if (debuglevel >= 3) fprintf(stdout,"Found End section code=%d\n",code);
#else
                                    debug3 << "Found End section code=" << code << endl;
#endif
                                    break;
                                }


                    }
                }
            }
            fclose(handle);
        }
#if GZSTUFF
        else if (strstr(filename.c_str(), ".unv.gz") != NULL)
        {
#if INTERACTIVEREAD
            if (debuglevel >= 1) fprintf(stdout,"File %s is a compressed one, using C reader\n",filename.c_str());
#else
            debug1 << "File is a compressed one, using C reader for " << filename << endl;
#endif
            gzhandle = gzopen(filename.c_str(), "r"); // Open the target file for reading
            if (gzhandle == Z_NULL)
            {
                EXCEPTION1(InvalidDBTypeException, "This unv.gz file could not be openend.");
            }

            int len = 2048; // Longest line length
            char buf[len]; // A line length
            int code;
            int label;
            double fac = 1.;
            while (gzgets(gzhandle, buf, len) != Z_NULL)
            {
                if (strstr(buf, "    -1") != NULL)
                {
                    if (gzgets(gzhandle, buf, len) != Z_NULL)
                    {
                        sscanf(buf, "%d", &code);
#if INTERACTIVEREAD
                        if (debuglevel >= 3) fprintf(stdout,"Found code=%d\n",code);
#else
                        debug3 << "Found code=" << code << endl;
#endif
                        if (code == 2412)
                        {
                            UnvElement anUnvElement;
                            int nodes[8];
                            anUnvElement.number=0;
#if INTERACTIVEREAD
                            if (debuglevel >= 3) fprintf(stdout,"Found Element code\n");
#else
                            debug3 << "Found Element code" << endl;
#endif
                            while (gzgets(gzhandle, buf, len) != Z_NULL)
                            {
                                if (strstr(buf, "    -1") != NULL)
                                {
#if INTERACTIVEREAD
                                    if (debuglevel >= 2) fprintf(stdout,"Found Element section end, nb3dcells=%d, nb2dcells=%d, nb1dcells=%d\n",nb3dcells,nb2dcells,nb1dcells);
#else
                                    debug2 << "Found Element section end, #cells=(" << nb3dcells << "," << nb2dcells << "," << nb1dcells << ")" << endl;
#endif
                                    break;
                                }
                                else
                                {
                                    int typelt, numprop,numat,colour,nbnel;
                                    sscanf(buf, "%d%d%d%d%d%d\n", &label, &typelt, &numprop, &numat, &colour, &nbnel);
                                    // fprintf(stdout,"Read element label=%d type=%d\n",label, typelt);
                                    if (gzgets(gzhandle, buf, len) != Z_NULL)
                                    {
                                        int ier; // = avtunvFileFormat::isKnownElt(typelt);
                                        switch (typelt)
                                        {
                                        case 111:
                                            //if (typelt == 111) {
                                            sscanf(buf, "%d %d %d %d\n", &nodes[0], &nodes[1], &nodes[2], &nodes[3]);
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout,"\t Nodes=(%d,%d,%d,%d)\n",nodes[0], nodes[1], nodes[2], nodes[3]);
#else
                                            debug4 << "\t Nodes=(" << nodes[0] << "," << nodes[1] << "," <<  nodes[2] << "," <<  nodes[3]  << ")" << endl;
#endif
                                            nb3dcells++;
                                            nb3dmats = max(nb3dmats, numat);
                                            ier = 0;
                                            break;
                                        case 115:
                                            //} else if (typelt == 115) {
                                            sscanf(buf, "%d %d %d %d %d %d %d %d\n", &nodes[0], &nodes[1], &nodes[2], &nodes[3], &nodes[4], &nodes[5], &nodes[6], &nodes[7]);
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout,"\t Nodes=(%d,%d,%d,%d,%d,%d,%d,%d)\n",nodes[0], nodes[1], nodes[2], nodes[3], nodes[4], nodes[5], nodes[6], nodes[7]);
#else
                                            debug4 << "\t Nodes=(" << nodes[0] << "," << nodes[1] << "," <<  nodes[2] << "," <<  nodes[3] << "," << nodes[4] << "," << nodes[5] << "," <<  nodes[6] << "," <<  nodes[7]  << ")" << endl;
#endif
                                            nb3dcells++;
                                            nb3dmats = max(nb3dmats, numat);
                                            ier = 1;
                                            break;
                                        case 112:
                                            //} else if (typelt == 112) {
                                            sscanf(buf, "%d %d %d %d %d %d\n", &nodes[0], &nodes[1], &nodes[2], &nodes[3], &nodes[4], &nodes[5]);
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout,"\t Nodes=(%d,%d,%d,%d,%d,%d)\n",nodes[0], nodes[1], nodes[2], nodes[3], nodes[4], nodes[5]);
#else
                                            debug4 << "\t Nodes=(" << nodes[0] << "," << nodes[1] << "," <<  nodes[2] << "," <<  nodes[3] << "," << nodes[4] << "," << nodes[5] << ")" << endl;
#endif
                                            nb3dcells++;
                                            nb3dmats = max(nb3dmats, numat);
                                            ier = 2;
                                            break;
                                        case 312:
                                            // } else if (typelt == 312) {
                                            sscanf(buf, "%d %d %d %d %d\n", &nodes[0], &nodes[1], &nodes[2], &nodes[3], &nodes[4]);
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout,"\t Nodes=(%d,%d,%d,%d,%d)\n",nodes[0], nodes[1], nodes[2], nodes[3], nodes[4]);
#else
                                            debug4 << "\t Nodes=(" << nodes[0] << "," << nodes[1] << "," <<  nodes[2] << "," <<  nodes[3] << "," << nodes[4] << ")" << endl;
#endif
                                            nb3dcells++;
                                            nb3dmats = max(nb3dmats, numat);
                                            ier = 3;
                                            break;
                                        case 91:
                                            //} else if (typelt == 91) {
                                            sscanf(buf, "%d %d %d\n", &nodes[0], &nodes[1], &nodes[2]);
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout,"\t Nodes=(%d,%d,%d)\n",nodes[0], nodes[1], nodes[2]);
#else
                                            debug4 << "\t Nodes=(" << nodes[0] << "," << nodes[1] << "," <<  nodes[2] << ")" << endl;
#endif
                                            nb2dcells++;
                                            nb2dmats = max(nb2dmats, numat);
                                            ier = 4;
                                            break;
                                        case 94:
                                            //} else if (typelt == 94) {
                                            sscanf(buf, "%d %d %d %d\n", &nodes[0], &nodes[1], &nodes[2], &nodes[3]);
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout,"\t Nodes=(%d,%d,%d,%d)\n",nodes[0], nodes[1], nodes[2], nodes[3]);
#else
                                            debug4 << "\t Nodes=(" << nodes[0] << "," << nodes[1] << "," <<  nodes[2] << "," <<  nodes[3]  << ")" << endl;
#endif
                                            nb2dcells++;
                                            nb2dmats = max(nb2dmats, numat);
                                            ier = 5;
                                            break;
                                        case 21:
                                            //} else if (typelt < 25) {
                                            // Linear beams
                                            if (gzgets(gzhandle, buf, len) != Z_NULL)
                                                if (typelt == 21)
                                                {
                                                    sscanf(buf, "%d %d %d %d\n", &nodes[0], &nodes[1]);
#if INTERACTIVEREAD
                                                    if (debuglevel >= 4) fprintf(stdout,"\t Nodes=(%d,%d)\n",nodes[0], nodes[1]);
#else
                                                    debug4 << "\t Nodes=(" << nodes[0] << "," << nodes[1]  << ")" << endl;
#endif
                                                    nb1dcells++;
                                                    nb1dmats = max(nb1dmats, numat);
                                                }

                                        default:
                                            //} else {
                                            ier = -1;
                                            if (typelt < 25)
                                                if (gzgets(gzhandle, buf, len) != Z_NULL)
                                                {
                                                    EXCEPTION1(InvalidDBTypeException, "This unv file has illegal typeelt.");
                                                }

                                            break;
                                        }
                                        if (ier != -1)
                                        {
                                            nbletsptyp[ier]++;
#if !defined(MDSERVER)
                                            anUnvElement.label = label;
                                            anUnvElement.typelt = typelt;
                                            anUnvElement.matid = numat;
                                            anUnvElement.nodes = new int[nbnel];
                                            for (int i=0; i < nbnel; i++)
                                            {
                                                anUnvElement.nodes[i] = nodes[i];
                                            }
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout, "\telt %d, #nodes=%d, first/last node=(%d,%d)\n",anUnvElement.label,nbnel,anUnvElement.nodes[0],anUnvElement.nodes[nbnel-1]);
#else
                                            debug4 << "\telt " << anUnvElement.label << ", #nodes=" << nbnel << ", first/last node=(" << anUnvElement.nodes[0] << "," << anUnvElement.nodes[nbnel-1] << ")" << endl;
#endif
                                            meshUnvElements.insert(anUnvElement);
                                            anUnvElement.number++;
#endif
                                        }
                                    }
                                }
                            }
                        }
                        else if (code == 2411)
                        {
                            UnvNode anode;
                            anode.number=0;
#if INTERACTIVEREAD
                            if (debuglevel >= 3) fprintf(stdout,"Found Node code\n");
#else
                            debug3 << "Found Node code "  << endl;
#endif
                            while (gzgets(gzhandle, buf, len) != Z_NULL)
                            {
                                if (strstr(buf, "    -1") != NULL)
                                {
#if !defined(MDSERVER)
                                    nbnodes = anode.number;
#endif
#if INTERACTIVEREAD
                                    if (debuglevel >= 2) fprintf(stdout,"Found Node section end, nbnodes=%d\n",nbnodes);
#else
                                    debug2 << "Found Node section end, nbnodes=" << nbnodes << endl;
#endif
                                    break;
                                }
                                else
                                {
                                    int i1, i2, i3;
                                    double x, y, z;
                                    sscanf(buf, "%d%d%d%d\n", &label, &i1, &i2, &i3);
                                    if (gzgets(gzhandle, buf, len) != Z_NULL)
                                    {
#if defined(MDSERVER)
                                        nbnodes++;
#else
                                        //fprintf(stdout,"buf len=%d, '%s'\n",strlen(buf), buf);
                                        for (i1=0; i1<strlen(buf); i1++)
                                            if (buf[i1] == 'D')
                                                buf[i1] = 'E';

                                        //fprintf(stdout,"buf len=%d, '%s'\n",strlen(buf), buf);
                                        sscanf(buf, "%lf %lf %lf\n", &x, &y, &z);
                                        x *= fac;
                                        y *= fac;
                                        z *= fac;
                                        range[0] = min(range[0],x);
                                        range[1] = max(range[1],x);
                                        range[2] = min(range[2],y);
                                        range[3] = max(range[3],y);
                                        range[4] = min(range[4],z);
                                        range[5] = max(range[5],z);
                                        // fprintf(stdout,"x,y,z=%lf,%lf,%lf\n",x,y,z);
                                        anode.label = label;
                                        anode.x = x;
                                        anode.y = y;
                                        anode.z = z;
                                        meshUnvNodes.insert(anode);
                                        anode.number++;
#endif
                                    }
                                }
                            }
                        }
                        else if (code == 164)
                        {
                            char str[11]; // A line length
                            int iunit;
                            if (gzgets(gzhandle, buf, len) != Z_NULL)
                            {
                                strncpy(str, buf, 10);
                                str[10] = '\0';
                                sscanf(str, "%d\n", &iunit);
#if INTERACTIVEREAD
                                if (debuglevel >= 3) fprintf(stdout, "unit code=%d\n",iunit);
#else
                                debug3 << "unit code=" << iunit << endl;
#endif
                                if (iunit == 5)
                                    fac = 1.e-3;
                                else if (iunit == 6)
                                    fac = 1.e-2;

                            }
                            while (gzgets(gzhandle, buf, len) != Z_NULL)
                            {
                                if (strstr(buf, "    -1") != NULL)
                                {
#if INTERACTIVEREAD
                                    if (debuglevel >= 3) fprintf(stdout,"Found End unit section code=%d\n",code);
#else
                                    debug3 << "Found End unit section code=" << code << endl;
#endif
                                    break;
                                }
                            }
                        }
                        else if (code == 790)
                        {
                            int id, i1, i3;
                            if (gzgets(gzhandle, buf, len) != NULL)
                                sscanf(buf, "%d%d\n", &id, &i3);

                            if (gzgets(gzhandle, buf, len) != NULL)
                            {
                                int imax = strlen(buf)-1;
                                // fprintf(stdout, "imax=%d\n",imax);
                                for (i1=0; i1<strlen(buf); i1++)
                                    if (buf[i1] == ' ')
                                        buf[i1] = '_';


                                for (i1=strlen(buf)-2; i1>0; i1--)
                                    if (buf[i1] != '_')
                                    {
                                        imax = i1+1;
                                        break;
                                    }

                                // fprintf(stdout, "imax=%d\n",imax);
                                buf[imax] = '\0';
                                UnvFacePressure anfp;
                                anfp.number = nbloadsets;
                                anfp.label = id;
                                anfp.name = buf;
                                nbloadsets++;
#if INTERACTIVEREAD
                                if (debuglevel >= 3) cout << "Adding a load set id=" << anfp.label << " name='" << anfp.name << "'." << endl;
                                if (debuglevel >= 3) fprintf(stdout, "Adding a load set id=%d name='%s'.\n",anfp.label,buf);
#else
                                debug3 << "Adding a load set id=" << anfp.label << " name='" << anfp.name << "'." << endl;
#endif
                                UnvFace anUnvFace; // Elementary face pressure object
                                UnvElement anUnvElement; // an element object, assuming already built
                                set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
                                while (gzgets(gzhandle, buf, len) != NULL)
                                {
                                    if (strstr(buf, "    -1") != NULL)
                                    {
                                        meshUnvFacePressures.push_back(anfp);
#if INTERACTIVEREAD
                                        if (debuglevel >= 3) fprintf(stdout,"Found End section code=%d, fp=%d, #faces=%d\n",code,anfp.label,nbfalsv);
#else
                                        debug3 << "Found End section code=" << code << "fp=" << anfp.label << ", #faces=" << nbfalsv << endl;
#endif
                                        break;
                                    }
                                    else
                                    {
                                        int label, iel, fac, colour;
                                        double pressure;
                                        sscanf(buf, "%d%d%d%d\n", &label, &iel, &fac, &colour);
                                        anUnvFace.number = label;
                                        anUnvFace.element = iel;
                                        anUnvFace.facloc = fac;
                                        anUnvFace.matid = colour;
                                        if (gzgets(gzhandle, buf, len) != NULL)
                                        {
                                            for (i1=0; i1<strlen(buf); i1++)
                                                if (buf[i1] == 'D')
                                                    buf[i1] = 'E';

                                            sscanf(buf, "%lf\n", &pressure);
                                            anUnvFace.pressure = pressure;
                                        }
                                        if (gzgets(gzhandle, buf, len) != NULL)
                                        {
#if INTERACTIVEREAD
                                            if (debuglevel >= 4) fprintf(stdout, "\tAdding a face presure %d iel=%d\n",anUnvFace.number,anUnvFace.element);
#else
                                            debug4 << "\tAdding a face presure " << anUnvFace.number << " iel=" << anUnvFace.element << endl;
#endif
                                            anfp.faces.push_back(anUnvFace);
                                            // Now increase the number of elements:
#if defined(MDSERVER)
                                            nbfalsv++;
#else
                                            anUnvElement.label = iel;
                                            itre = meshUnvElements.find(anUnvElement);
                                            if (itre != meshUnvElements.end())
                                            {
                                                int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                                                if (iflo >= 0)
                                                {
                                                    nbfalsv++;
                                                }
                                                else
                                                {
                                                    iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
                                                    if (iflo >= 0)
                                                    {
                                                        nbfalsv++;
                                                    }
                                                }
                                            }
#endif
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            while (gzgets(gzhandle, buf, len) != Z_NULL)
                            {
                                if (strstr(buf, "    -1") != NULL)
                                {
#if INTERACTIVEREAD
                                    if (debuglevel >= 3) fprintf(stdout,"Found End section code=%d\n",code);
#else
                                    debug3 << "Found End section code=" << code << endl;
#endif
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            gzclose(gzhandle);
#if INTERACTIVEREAD
            if (debuglevel >= 1) fprintf(stdout,"Closing file, nbnodes=%d, nbelts=%d\n",nbnodes,nb3dcells+nb2dcells);
#else
            debug1 << "Closing file, nbnodes=" << nbnodes << ", nbelts=" << nb3dcells+nb2dcells << endl;
#endif
        }
#endif
#if defined(MDSERVER)
        visitTimer->StopTimer(readingFile, "Reading file");
#else
        visitTimer->StopTimer(readingFile, "Reading file and allocating data");
#endif
    }

    CATCH(InvalidFilesException)
    {
        debug1 << "Unable to read unv()" << endl;
        RETHROW;
    }
    CATCHALL
    {
        debug1 << "Unable to read unv()" << endl;
        fprintf(stdout,"Unable to read unv()\n");
    }
    ENDTRY
    if (handle == NULL && gzhandle == Z_NULL)
    {
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    else
    {
        fileRead = true;
        cdim = 0;
        if (nb3dcells > 0)
            cdim = 3;
        else if (nb2dcells > 0)
            cdim = 2;
        else if (nb1dcells > 0)
            cdim = 1;
    }
}

#include <avtMaterial.h>
void *
avtunvFileFormat::GetAuxiliaryData(const char *var, const char *type, void *,DestructorFunction &df)
{
    void *retval = 0;
#if INTERACTIVEPLOT
    if (debuglevel >= 3) fprintf(stdout,"var='%s', type='%s'\n",var,type);
#else
    debug3 << "var='" << var << "', type='" << type << "%s'" << endl;
#endif
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        if (strcmp(var, "materials") == 0 || strcmp(var, "volmats") == 0 || strcmp(var, "surfmats") == 0 || strcmp(var, "wiremats") == 0)
        {
            avtMaterial *mat = NULL;
            int nmats = max(nb3dmats, nb2dmats);
            nmats = max(nmats,nb1dmats) ;
            int *matnos = new int[nmats];
            char **names = new char *[nmats];
            char str[32];
            for (int i=0; i<nmats; i++)
            {
                matnos[i] = i+1;
                sprintf(str, "mat%d", i+1);
                names[i] = new char[strlen(str)+1];
                sprintf(names[i], "%s", str);
#if INTERACTIVEPLOT
                if (debuglevel >= 4) fprintf(stdout,"Material %s.\n",names[i]);
#else
                debug4 << "Material " << names[i] << endl;
#endif
            }
            set<UnvElement, UnvElement::compare_UnvElement>::iterator itre;
            int ndims = 1;
            int dims[3];
            if (strcmp(var, "volmats") == 0)
                dims[0] = nb3dcells ;
            else if (strcmp(var, "surfmats") == 0)
                dims[0] = nb2dcells ;
            else if (strcmp(var, "wiremats") == 0)
                dims[0] = nb1dcells ;
            else
                dims[0] = nb3dcells+nb2dcells+nb1dcells;

#if INTERACTIVEPLOT
            if (debuglevel >= 3) fprintf(stdout,"Material #cells=%d\n",dims[0]);
#else
            debug3 << "Material #cells=" << dims[0] << endl;
#endif
            int *matlist = new int[dims[0]];
            int k = 0 ;
            string meshname = "mesh";
            if (strcmp(var, "volmats") == 0)
            {
                meshname = "volmesh";
#if INTERACTIVEPLOT
                if (debuglevel >= 3) fprintf(stdout,"3D Material #cells=%d\n",dims[0]);
#endif
                for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
                    if (avtunvFileFormat::is3DKnownElt(itre->typelt) >= 0) 
                    {
                        matlist[k] = itre->matid;
                        k++ ;
                    }
            }
            else if (strcmp(var, "surfmats") == 0)
            {
                meshname = "surfmesh";
#if INTERACTIVEPLOT
                if (debuglevel >= 3) fprintf(stdout,"2D Material #cells=%d\n",dims[0]);
#endif
                for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
                    if (avtunvFileFormat::is2DKnownElt(itre->typelt) >= 0)
                    {
                        matlist[k] = itre->matid;
                        k++ ;
                    }
            }
            else if (strcmp(var, "wiremats") == 0)
            {
                meshname = "wiremesh";
#if INTERACTIVEPLOT
                if (debuglevel >= 3) fprintf(stdout,"1D Material #cells=%d\n",dims[0]);
#endif
                for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
                    if (avtunvFileFormat::is1DKnownElt(itre->typelt) >= 0)
                    {
                        matlist[k] = itre->matid;
                        k++ ;
                    }
            }
            else
            {
#if INTERACTIVEPLOT
                if (debuglevel >= 3) fprintf(stdout,"xD Material #cells=%d\n",dims[0]);
#endif
                for (itre = meshUnvElements.begin(); itre != meshUnvElements.end(); itre++)
                    matlist[itre->number] = itre->matid;
            }

#if INTERACTIVEPLOT
            if (debuglevel >= 3) fprintf(stdout,"Material #cells=%d\n",k);
#else
            debug3 << "Material #cells=" << k << endl;
#endif

            mat = new avtMaterial(nmats,matnos,names,ndims,dims,0,matlist,
                                  0, // length of mix arrays
                                  0, // mix_mat array
                                  0, // mix_next array
                                  0, // mix_zone array
                                  0 // mix_vf array
                                  ,meshname.c_str(),0 // Sets the allowmat0 option
                                 );
            delete [] matnos;
            delete [] matlist;
            for(int i = 0; i < nmats; ++i)
                delete [] names[i];

            // Set the return values.
            retval = (void *)mat;
            df = avtMaterial::Destruct;
        }
        else if (strcmp(var, "load_sets") == 0)
        {
            avtMaterial *mat = NULL;
            int nmats = nbloadsets;
            int *matnos = new int[nmats];
            char **names = new char *[nmats];
            for (int i=0; i<meshUnvFacePressures.size(); i++)
            {
                char str[32];
                matnos[i] = i;
                sprintf(str, "%s", meshUnvFacePressures[i].name.c_str());
                names[i] = new char[strlen(str)+1];
                sprintf(names[i], "%s", str);
#if INTERACTIVEPLOT
                if (debuglevel >= 4) fprintf(stdout,"Load-Set %s.\n",names[i]);
#else
                debug4 << "Load-Set " << names[i] << endl;
#endif
            }
            int ndims = 1;
            int dims[3];
            int *matlist = NULL;
            int nbcells = 0; // Number of faces
            UnvElement anUnvElement; // an element
            set<UnvElement, UnvElement::compare_UnvElement>::iterator itre; // Global elements iterator
            dims[0] = nbfalsv;
            matlist = new int[nbfalsv];
            nbcells = 0;
            for (int i=0; i<meshUnvFacePressures.size(); i++)
                for (int j=0; j<meshUnvFacePressures[i].faces.size(); j++)
                {
                    int iel = meshUnvFacePressures[i].faces[j].element;
                    // Fetch the element:
                    anUnvElement.label = iel;
                    itre = meshUnvElements.find(anUnvElement);
                    int iflo = avtunvFileFormat::is3DKnownElt(itre->typelt);
                    if (iflo >= 0)
                    {
                        // Following could be used to get load sets by color:
                        // matlist[nbcells] = itre->matid;
                        matlist[nbcells] = i;
                        nbcells++;
                    }
                    else
                    {
                        iflo = avtunvFileFormat::is2DKnownElt(itre->typelt);
                        if (iflo >= 0)
                        {
                            matlist[nbcells] = i;
                            nbcells++;
                        }
                    }
                }

            mat = new avtMaterial(nmats,matnos,names,ndims,dims,0,matlist,
                                  0, // length of mix arrays
                                  0, // mix_mat array
                                  0, // mix_next array
                                  0, // mix_zone array
                                  0 // mix_vf array
                                  ,"facemesh",0 // Sets the allowmat0 option
                                 );
            delete [] matnos;
            delete [] matlist;
            for(int i = 0; i < nmats; ++i)
                delete [] names[i];

            // Set the return values.
            retval = (void *)mat;
            df = avtMaterial::Destruct;
        }
        else if (strcmp(var, "boundaries") == 0)
        {
            avtMaterial *mat = NULL;
            // Make sure stuff is already computed:
            int nmats = avtunvFileFormat::getNbfreeSets();
            int *matnos = new int[nmats];
            char **names = new char *[nmats];
            for (int i=0; i<nmats; i++)
            {
                char str[32];
                matnos[i] = i+1;
                if (i == 0 && cdim == 3)
                    sprintf(str, "OUTER");
                else
                    sprintf(str, "INN%d", i);

                names[i] = new char[strlen(str)+1];
                sprintf(names[i], "%s", str);
#if INTERACTIVEPLOT
                if (debuglevel >= 3) fprintf(stdout,"Boundary %s\n",names[i]);
#else
                debug3 << "Boundary " << names[i] << endl;
#endif
            }
            int ndims = 1;
            int dims[3];
            int *matlist = NULL;
            dims[0] = nbfaextv;
#if INTERACTIVEPLOT
            if (debuglevel >= 2) fprintf(stdout,"mat nbfaextv=%d\n",nbfaextv);
#else
            debug2 << "mat nbfaextv " << nbfaextv << endl;
#endif
            matlist = new int[nbfaextv];
            for (int j=0; j<freeUnvFaces.size(); j++)
                matlist[j] = freeUnvFaces[j].matid;

            mat = new avtMaterial(nmats,matnos,names,ndims,dims,0,matlist,
                                  0, // length of mix arrays
                                  0, // mix_mat array
                                  0, // mix_next array
                                  0, // mix_zone array
                                  0 // mix_vf array
                                  ,"freemesh",0 // Sets the allowmat0 option
                                 );
            delete [] matnos;
            delete [] matlist;
            for(int i = 0; i < nmats; ++i)
                delete [] names[i];

            // Set the return values.
            retval = (void *)mat;
            df = avtMaterial::Destruct;
        }
    }
    else if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        if (strstr(var, "mesh") != NULL)
        {
#if INTERACTIVEPLOT
            if (debuglevel >= 3) fprintf(stdout,"range = %lf,%lf\n",range[0],range[1]);
#else
            debug3 << "range = (" << range[0] << ", " << range[1] << ")" << endl;
#endif
            avtIntervalTree *itree = new avtIntervalTree(1, 3);
            itree->AddElement(0, range);
            itree->Calculate(true);
            df = avtIntervalTree::Destruct;
            return ((void *) itree);
        }
        else
            return retval;

    }
    return retval;
}
