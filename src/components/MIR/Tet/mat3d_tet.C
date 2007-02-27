/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

/* ***************************************************************** */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*        This file generated automatically by mat3d_tables.C        */
/*                                                                   */
/* ----------------------------------------------------------------- */
/* ***************************************************************** */



/* ***************************************************************** */
/*                          Structured tables                        */
/* ***************************************************************** */


/* ***************************************************************** */
/*                         Unstructured tables                       */
/* ***************************************************************** */


/* ***************************** Voxel ***************************** */

/* -------------------------------------------------------------- */
/* unstructured voxel tetrahedralization table for subdiv level 1 */
/* -------------------------------------------------------------- */
int ntet_mat3d_vox2tet_subdiv1[8] = {5,6,6,6,6,6,6,6};
int mat3d_vox2tet_subdiv1_node[8][6][4] = {
{ /***** case 0 *****/
{4,0,1,2},
{4,7,6,2},
{3,7,1,2},
{5,7,4,1},
{4,7,2,1},
{0,0,0,0}
},
{ /***** case 1 *****/
{4,5,7,1},
{4,2,0,1},
{6,4,7,3},
{6,2,4,3},
{1,7,4,3},
{1,4,2,3}
},
{ /***** case 2 *****/
{4,6,2,7},
{4,1,5,7},
{0,4,2,3},
{0,1,4,3},
{7,2,4,3},
{7,4,1,3}
},
{ /***** case 3 *****/
{4,1,5,7},
{1,4,0,3},
{1,4,3,7},
{0,4,2,3},
{4,6,2,3},
{4,7,6,3}
},
{ /***** case 4 *****/
{4,0,1,2},
{4,7,6,2},
{5,4,1,3},
{5,7,4,3},
{2,1,4,3},
{2,4,7,3}
},
{ /***** case 5 *****/
{4,2,0,1},
{2,4,6,3},
{2,4,3,1},
{6,4,7,3},
{4,5,7,3},
{4,1,5,3}
},
{ /***** case 6 *****/
{4,7,6,2},
{7,4,5,3},
{7,4,3,2},
{5,4,1,3},
{4,0,1,3},
{4,2,0,3}
},
{ /***** case 7 *****/
{5,7,4,3},
{5,4,1,3},
{4,0,1,3},
{4,2,0,3},
{4,7,6,3},
{4,6,2,3}
}
};


int mat3d_vox2tet_subdiv1_flip[2][2][2][8] = {
{{{0,1,2,3,4,5,6,7},
  {2,3,0,1,6,7,4,5}},
 {{4,5,6,7,0,1,2,3},
  {6,7,4,5,2,3,0,1}}},
{{{1,0,3,7,5,4,6,2},
  {3,7,1,0,6,2,5,4}},
 {{5,4,6,2,1,0,3,7},
  {6,2,5,4,3,7,1,0}}},
};


/* *************************** Hexahedron ************************** */

/* ------------------------------------------------------------------- */
/* unstructured hexahedral tetrahedralization table for subdiv level 1 */
/* ------------------------------------------------------------------- */
int ntet_mat3d_hex2tet_subdiv1[8] = {5,6,6,6,6,6,6,6};
int mat3d_hex2tet_subdiv1_node[8][6][4] = {
{ /***** case 0 *****/
{4,0,1,3},
{4,6,7,3},
{2,6,1,3},
{5,6,4,1},
{4,6,3,1},
{0,0,0,0}
},
{ /***** case 1 *****/
{4,5,6,1},
{4,3,0,1},
{7,4,6,2},
{7,3,4,2},
{1,6,4,2},
{1,4,3,2}
},
{ /***** case 2 *****/
{4,7,3,6},
{4,1,5,6},
{0,4,3,2},
{0,1,4,2},
{6,3,4,2},
{6,4,1,2}
},
{ /***** case 3 *****/
{4,1,5,6},
{1,4,0,2},
{1,4,2,6},
{0,4,3,2},
{4,7,3,2},
{4,6,7,2}
},
{ /***** case 4 *****/
{4,0,1,3},
{4,6,7,3},
{5,4,1,2},
{5,6,4,2},
{3,1,4,2},
{3,4,6,2}
},
{ /***** case 5 *****/
{4,3,0,1},
{3,4,7,2},
{3,4,2,1},
{7,4,6,2},
{4,5,6,2},
{4,1,5,2}
},
{ /***** case 6 *****/
{4,6,7,3},
{6,4,5,2},
{6,4,2,3},
{5,4,1,2},
{4,0,1,2},
{4,3,0,2}
},
{ /***** case 7 *****/
{5,6,4,2},
{5,4,1,2},
{4,0,1,2},
{4,3,0,2},
{4,6,7,2},
{4,7,3,2}
}
};


int mat3d_hex2tet_subdiv1_flip[2][2][2][8] = {
{{{0,1,2,3,4,5,6,7},
  {3,2,1,0,7,6,5,4}},
 {{4,5,6,7,0,1,2,3},
  {7,6,5,4,3,2,1,0}}},
{{{1,0,3,2,5,4,7,6},
  {2,3,0,1,6,7,4,5}},
 {{5,4,7,6,1,0,3,2},
  {6,7,4,5,2,3,0,1}}},
};


/* ************************** Tetrahedron ************************** */

/* -------------------------------------------------------------------- */
/* unstructured tetrahedral tetrahedralization table for subdiv level 1 */
/* -------------------------------------------------------------------- */
int ntet_mat3d_tet2tet_subdiv1 = 1;
int mat3d_tet2tet_subdiv1[1][1][4] = {
{ /***** case 0 *****/
{0,1,2,3}
}
};


/* **************************** Pyramid **************************** */

/* ---------------------------------------------------------------- */
/* unstructured pyramid tetrahedralization table for subdiv level 1 */
/* ---------------------------------------------------------------- */
int ntet_mat3d_pyr2tet_subdiv1 = 2;
int mat3d_pyr2tet_subdiv1_node[1][2][4] = {
{ /***** case 0 *****/
{0,1,2,4},
{0,2,3,4}
}
};


int mat3d_pyr2tet_subdiv1_flip[2][5] = {
{0,1,2,3,4},
{1,0,3,2,4}
};


/* ***************************** Wedge ***************************** */

/* -------------------------------------------------------------- */
/* unstructured wedge tetrahedralization table for subdiv level 1 */
/* -------------------------------------------------------------- */
int ntet_mat3d_wedge2tet_subdiv1 = 3;
int mat3d_wedge2tet_subdiv1_node[2][3][4] = {
{ /***** case 0 *****/
{0,3,4,5},
{0,4,1,2},
{0,2,5,4}
},
{ /***** case 1 *****/
{0,3,4,5},
{0,4,1,5},
{0,2,5,1}
}
};


int mat3d_wedge2tet_subdiv1_fliprot[2][3][6] = {
{{0,1,2,3,4,5},
 {1,2,0,4,5,3},
 {2,0,1,5,3,4}},
{{3,4,5,0,1,2},
 {4,5,3,1,2,0},
 {5,3,4,2,0,1}}
};


