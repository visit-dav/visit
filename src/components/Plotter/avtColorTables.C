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

#include <avtColorTables.h>
#include <ColorTableAttributes.h>
#include <ColorControlPointList.h>
#include <ColorControlPoint.h>
#include <ColorTableManager.h>

//
// Static data that describes the eight default color tables.
//

static const char *predef_ct_names[]  = { "bluehot", "caleblack", "calewhite",
    "contoured", "difference", "gray", "hot", "hot_and_cold", 
    "hot_desaturated", "levels", "orangehot", "rainbow", "xray",
    "cpk_jmol", "cpk_rasmol", "amino_rasmol", "amino_shapely"
};
static const int predef_ct_ncolors[]  = {4, 7, 7, 4, 3, 2, 5, 5, 8, 30, 4, 6, 2, 109, 109, 23, 23};
static const int predef_ct_smooth[]   = {1, 1, 1, 0, 1, 1, 1, 1, 1,  0, 1, 1, 1,   0,   0,  0,  0};
static const int predef_ct_equal[]    = {0, 0, 0, 1, 0, 0, 0, 0, 0,  1, 0, 0, 0,   1,   1,  1,  1};
static const int predef_ct_discrete[] = {0, 0, 0, 0, 0, 0, 0, 0, 0,  1, 0, 0, 0,   1,   1,  1,  1};

/* Hot */
static const float ct_hot[] = {
 0.f,   0.f, 0.f, 1.f,
 0.25f, 0.f, 1.f, 1.f, 
 0.5f,  0.f, 1.f, 0.f, 
 0.75f, 1.f, 1.f, 0.f, 
 1.f,   1.f, 0.f, 0.f
};

/* Caleblack */
static const float ct_caleblack[] = {
 0.f,   0.f, 0.f, 0.f,
 0.17f, 0.f, 0.f, 1.f, 
 0.34f, 0.f, 1.f, 1.f, 
 0.50f, 0.f, 1.f, 0.f, 
 0.67f, 1.f, 1.f, 0.f, 
 0.84f, 1.f, 0.f, 0.f, 
 1.f,   1.f, 0.f, 1.f, 
};

/* Calewhite */
static const float ct_calewhite[] = {
 0.f,   1.f, 1.f, 1.f, 
 0.17f, 0.f, 0.f, 1.f, 
 0.34f, 0.f, 1.f, 1.f, 
 0.50f, 0.f, 1.f, 0.f, 
 0.67f, 1.f, 1.f, 0.f, 
 0.84f, 1.f, 0.f, 0.f, 
 1.f,   1.f, 0.f, 1.f,
};

/* Gray */
static const float ct_gray[] = {0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f};
/* Xray */
static const float ct_xray[] = {0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f};

/* Rainbow */
static const float ct_rainbow[] = {
 0.f,  1.f, 0.f, 1.f, 
 0.2f, 0.f, 0.f, 1.f, 
 0.4f, 0.f, 1.f, 1.f, 
 0.6f, 0.f, 1.f, 0.f, 
 0.8f, 1.f, 1.f, 0.f, 
 1.0f, 1.f, 0.f, 0.f, 
};

/* Contoured */
static const float ct_contoured[] = {
 0.f,    0.f, 0.f, 1.f, 
 0.333f, 0.f, 1.f, 0.f, 
 0.666f, 1.f, 1.f, 0.f, 
 1.0f,   1.f, 0.f, 0.f, 
};

/* Levels discrete color table. */
static const float ct_levels[] = {
 0.00f, 1.00f, 0.00f, 0.00f,
 0.03f, 0.00f, 1.00f, 0.00f,
 0.07f, 0.00f, 0.00f, 1.00f,
 0.10f, 0.00f, 1.00f, 1.00f,
 0.14f, 1.00f, 0.00f, 1.00f,
 0.17f, 1.00f, 1.00f, 0.00f,
 0.21f, 1.00f, 0.53f, 0.00f,
 0.24f, 1.00f, 0.00f, 0.53f,
 0.28f, 0.66f, 0.66f, 0.66f,
 0.31f, 1.00f, 0.27f, 0.27f,
 0.34f, 0.39f, 1.00f, 0.39f,
 0.38f, 0.39f, 0.39f, 1.00f,
 0.41f, 0.16f, 0.65f, 0.65f,
 0.45f, 1.00f, 0.39f, 1.00f,
 0.48f, 1.00f, 1.00f, 0.39f,
 0.52f, 1.00f, 0.67f, 0.39f,
 0.55f, 0.67f, 0.31f, 1.00f,
 0.59f, 0.59f, 0.00f, 0.00f,
 0.62f, 0.00f, 0.59f, 0.00f,
 0.66f, 0.00f, 0.00f, 0.59f,
 0.69f, 0.00f, 0.43f, 0.43f,
 0.72f, 0.59f, 0.00f, 0.59f,
 0.76f, 0.59f, 0.59f, 0.00f,
 0.79f, 0.59f, 0.33f, 0.00f,
 0.83f, 0.63f, 0.00f, 0.31f,
 0.86f, 1.00f, 0.41f, 0.11f,
 0.90f, 0.00f, 0.67f, 0.32f,
 0.93f, 0.27f, 1.00f, 0.49f,
 0.97f, 0.00f, 0.51f, 1.00f,
 1.00f, 0.51f, 0.00f, 1.00f,
};

/* Bluehot */
static const float ct_bluehot[] = {
 0.f,    0.f, 0.f,  0.f, 
 0.333f, 0.f, 0.f,  0.5f, 
 0.666f, 0.f, 0.5f, 1.f, 
 1.0f,   1.f, 1.f,  1.f, 
};

/* Orangehot */
static const float ct_orangehot[] = {
 0.f,    0.f,  0.f,  0.f, 
 0.333f, 0.5f, 0.f,  0.f, 
 0.666f, 1.f,  0.5f, 0.f, 
 1.0f,   1.f,  1.f,  1.f, 
};

/* Hot-and-cold */
static const float ct_hot_and_cold[] = {
 0.f,    0.f, 1.f, 1.f, 
 0.45f,  0.f, 0.f, 1.f, 
 0.5f,   0.f, 0.f, 0.5f, 
 0.55f,  1.f, 0.f, 0.f, 
 1.0f,   1.f, 1.f, 0.f, 
};

/* Hot desaturated */
static const float ct_hot_desaturated[] = {
 0.0f,   0.28f, 0.28f, 0.86f, 
 0.143f, 0.f,   0.f,   0.36f, 
 0.285f, 0.f,   1.f,   1.f, 
 0.429f, 0.f,   0.5f,  0.f, 
 0.571f, 1.f,   1.f,   0.f, 
 0.714f, 1.f,   0.38f, 0.f, 
 0.857f, 0.42f, 0.f,   0.f, 
 1.0f,   0.88f, 0.3f,  0.3f, 
};

/* Difference */
static const float ct_difference[] = {
 0.f,    0.f,  0.f,  1.f, 
 0.5f,   1.f,  1.f,  1.f, 
 1.0f,   1.f,  0.f,  0.f, 
};

/* CPK Molecular Colors used by Jmol */
static const float ct_jmol_colors[] = {
 0.000f, 1.000f, 1.000f, 1.000f,
 0.009f, 0.851f, 1.000f, 1.000f,
 0.019f, 0.800f, 0.502f, 1.000f,
 0.028f, 0.761f, 1.000f, 0.000f,
 0.037f, 1.000f, 0.710f, 0.710f,
 0.046f, 0.565f, 0.565f, 0.565f,
 0.056f, 0.188f, 0.314f, 0.973f,
 0.065f, 1.000f, 0.051f, 0.051f,
 0.074f, 0.565f, 0.878f, 0.314f,
 0.083f, 0.702f, 0.890f, 0.961f,
 0.093f, 0.671f, 0.361f, 0.949f,
 0.102f, 0.541f, 1.000f, 0.000f,
 0.111f, 0.749f, 0.651f, 0.651f,
 0.120f, 0.941f, 0.784f, 0.627f,
 0.130f, 1.000f, 0.502f, 0.000f,
 0.139f, 1.000f, 1.000f, 0.188f,
 0.148f, 0.122f, 0.941f, 0.122f,
 0.157f, 0.502f, 0.820f, 0.890f,
 0.167f, 0.561f, 0.251f, 0.831f,
 0.176f, 0.239f, 1.000f, 0.000f,
 0.185f, 0.902f, 0.902f, 0.902f,
 0.194f, 0.749f, 0.761f, 0.780f,
 0.204f, 0.651f, 0.651f, 0.671f,
 0.213f, 0.541f, 0.600f, 0.780f,
 0.222f, 0.612f, 0.478f, 0.780f,
 0.231f, 0.878f, 0.400f, 0.200f,
 0.241f, 0.941f, 0.565f, 0.627f,
 0.250f, 0.314f, 0.816f, 0.314f,
 0.259f, 0.784f, 0.502f, 0.200f,
 0.269f, 0.490f, 0.502f, 0.690f,
 0.278f, 0.761f, 0.561f, 0.561f,
 0.287f, 0.400f, 0.561f, 0.561f,
 0.296f, 0.741f, 0.502f, 0.890f,
 0.306f, 1.000f, 0.631f, 0.000f,
 0.315f, 0.651f, 0.161f, 0.161f,
 0.324f, 0.361f, 0.722f, 0.820f,
 0.333f, 0.439f, 0.180f, 0.690f,
 0.343f, 0.000f, 1.000f, 0.000f,
 0.352f, 0.580f, 1.000f, 1.000f,
 0.361f, 0.580f, 0.878f, 0.878f,
 0.370f, 0.451f, 0.761f, 0.788f,
 0.380f, 0.329f, 0.710f, 0.710f,
 0.389f, 0.231f, 0.620f, 0.620f,
 0.398f, 0.141f, 0.561f, 0.561f,
 0.407f, 0.039f, 0.490f, 0.549f,
 0.417f, 0.000f, 0.412f, 0.522f,
 0.426f, 0.753f, 0.753f, 0.753f,
 0.435f, 1.000f, 0.851f, 0.561f,
 0.444f, 0.651f, 0.459f, 0.451f,
 0.454f, 0.400f, 0.502f, 0.502f,
 0.463f, 0.620f, 0.388f, 0.710f,
 0.472f, 0.831f, 0.478f, 0.000f,
 0.481f, 0.580f, 0.000f, 0.580f,
 0.491f, 0.259f, 0.620f, 0.690f,
 0.500f, 0.341f, 0.090f, 0.561f,
 0.509f, 0.000f, 0.788f, 0.000f,
 0.519f, 0.439f, 0.831f, 1.000f,
 0.528f, 1.000f, 1.000f, 0.780f,
 0.537f, 0.851f, 1.000f, 0.780f,
 0.546f, 0.780f, 1.000f, 0.780f,
 0.556f, 0.639f, 1.000f, 0.780f,
 0.565f, 0.561f, 1.000f, 0.780f,
 0.574f, 0.380f, 1.000f, 0.780f,
 0.583f, 0.271f, 1.000f, 0.780f,
 0.593f, 0.188f, 1.000f, 0.780f,
 0.602f, 0.122f, 1.000f, 0.780f,
 0.611f, 0.000f, 1.000f, 0.612f,
 0.620f, 0.000f, 0.902f, 0.459f,
 0.630f, 0.000f, 0.831f, 0.322f,
 0.639f, 0.000f, 0.749f, 0.220f,
 0.648f, 0.000f, 0.671f, 0.141f,
 0.657f, 0.302f, 0.761f, 1.000f,
 0.667f, 0.302f, 0.651f, 1.000f,
 0.676f, 0.129f, 0.580f, 0.839f,
 0.685f, 0.149f, 0.490f, 0.671f,
 0.694f, 0.149f, 0.400f, 0.588f,
 0.704f, 0.090f, 0.329f, 0.529f,
 0.713f, 0.816f, 0.816f, 0.878f,
 0.722f, 1.000f, 0.820f, 0.137f,
 0.731f, 0.722f, 0.722f, 0.816f,
 0.741f, 0.651f, 0.329f, 0.302f,
 0.750f, 0.341f, 0.349f, 0.380f,
 0.759f, 0.620f, 0.310f, 0.710f,
 0.769f, 0.671f, 0.361f, 0.000f,
 0.778f, 0.459f, 0.310f, 0.271f,
 0.787f, 0.259f, 0.510f, 0.588f,
 0.796f, 0.259f, 0.000f, 0.400f,
 0.806f, 0.000f, 0.490f, 0.000f,
 0.815f, 0.439f, 0.671f, 0.980f,
 0.824f, 0.000f, 0.729f, 1.000f,
 0.833f, 0.000f, 0.631f, 1.000f,
 0.843f, 0.000f, 0.561f, 1.000f,
 0.852f, 0.000f, 0.502f, 1.000f,
 0.861f, 0.000f, 0.420f, 1.000f,
 0.870f, 0.329f, 0.361f, 0.949f,
 0.880f, 0.471f, 0.361f, 0.890f,
 0.889f, 0.541f, 0.310f, 0.890f,
 0.898f, 0.631f, 0.212f, 0.831f,
 0.907f, 0.702f, 0.122f, 0.831f,
 0.917f, 0.702f, 0.122f, 0.729f,
 0.926f, 0.702f, 0.051f, 0.651f,
 0.935f, 0.741f, 0.051f, 0.529f,
 0.944f, 0.780f, 0.000f, 0.400f,
 0.954f, 0.800f, 0.000f, 0.349f,
 0.963f, 0.820f, 0.000f, 0.310f,
 0.972f, 0.851f, 0.000f, 0.271f,
 0.981f, 0.878f, 0.000f, 0.220f,
 0.991f, 0.902f, 0.000f, 0.180f,
 1.000f, 0.922f, 0.000f, 0.149f,
};

/* CPK Molecular Colors used by Rasmol */
static const float ct_rasmol_colors[] = {
 0.000f, 1.000f, 1.000f, 1.000f,
 0.009f, 1.000f, 0.753f, 0.796f,
 0.019f, 0.698f, 0.133f, 0.133f,
 0.028f, 1.000f, 0.078f, 0.576f,
 0.037f, 0.000f, 1.000f, 0.000f,
 0.046f, 0.784f, 0.784f, 0.784f,
 0.056f, 0.561f, 0.561f, 1.000f,
 0.065f, 0.941f, 0.000f, 0.000f,
 0.074f, 0.855f, 0.647f, 0.125f,
 0.083f, 1.000f, 0.078f, 0.576f,
 0.093f, 0.000f, 0.000f, 1.000f,
 0.102f, 0.133f, 0.545f, 0.133f,
 0.111f, 0.502f, 0.502f, 0.565f,
 0.120f, 0.855f, 0.647f, 0.125f,
 0.130f, 1.000f, 0.647f, 0.000f,
 0.139f, 1.000f, 0.784f, 0.196f,
 0.148f, 0.000f, 1.000f, 0.000f,
 0.157f, 1.000f, 0.078f, 0.576f,
 0.167f, 1.000f, 0.078f, 0.576f,
 0.176f, 0.502f, 0.502f, 0.565f,
 0.185f, 1.000f, 0.078f, 0.576f,
 0.194f, 0.502f, 0.502f, 0.565f,
 0.204f, 1.000f, 0.078f, 0.576f,
 0.213f, 0.502f, 0.502f, 0.565f,
 0.222f, 0.502f, 0.502f, 0.565f,
 0.231f, 1.000f, 0.647f, 0.000f,
 0.241f, 1.000f, 0.078f, 0.576f,
 0.250f, 0.647f, 0.165f, 0.165f,
 0.259f, 0.647f, 0.165f, 0.165f,
 0.269f, 0.647f, 0.165f, 0.165f,
 0.278f, 1.000f, 0.078f, 0.576f,
 0.287f, 1.000f, 0.078f, 0.576f,
 0.296f, 1.000f, 0.078f, 0.576f,
 0.306f, 1.000f, 0.078f, 0.576f,
 0.315f, 0.647f, 0.165f, 0.165f,
 0.324f, 1.000f, 0.078f, 0.576f,
 0.333f, 1.000f, 0.078f, 0.576f,
 0.343f, 1.000f, 0.078f, 0.576f,
 0.352f, 1.000f, 0.078f, 0.576f,
 0.361f, 1.000f, 0.078f, 0.576f,
 0.370f, 1.000f, 0.078f, 0.576f,
 0.380f, 1.000f, 0.078f, 0.576f,
 0.389f, 1.000f, 0.078f, 0.576f,
 0.398f, 1.000f, 0.078f, 0.576f,
 0.407f, 1.000f, 0.078f, 0.576f,
 0.417f, 1.000f, 0.078f, 0.576f,
 0.426f, 0.502f, 0.502f, 0.565f,
 0.435f, 1.000f, 0.078f, 0.576f,
 0.444f, 1.000f, 0.078f, 0.576f,
 0.454f, 1.000f, 0.078f, 0.576f,
 0.463f, 1.000f, 0.078f, 0.576f,
 0.472f, 1.000f, 0.078f, 0.576f,
 0.481f, 0.627f, 0.125f, 0.941f,
 0.491f, 1.000f, 0.078f, 0.576f,
 0.500f, 1.000f, 0.078f, 0.576f,
 0.509f, 1.000f, 0.647f, 0.000f,
 0.519f, 1.000f, 0.078f, 0.576f,
 0.528f, 1.000f, 0.078f, 0.576f,
 0.537f, 1.000f, 0.078f, 0.576f,
 0.546f, 1.000f, 0.078f, 0.576f,
 0.556f, 1.000f, 0.078f, 0.576f,
 0.565f, 1.000f, 0.078f, 0.576f,
 0.574f, 1.000f, 0.078f, 0.576f,
 0.583f, 1.000f, 0.078f, 0.576f,
 0.593f, 1.000f, 0.078f, 0.576f,
 0.602f, 1.000f, 0.078f, 0.576f,
 0.611f, 1.000f, 0.078f, 0.576f,
 0.620f, 1.000f, 0.078f, 0.576f,
 0.630f, 1.000f, 0.078f, 0.576f,
 0.639f, 1.000f, 0.078f, 0.576f,
 0.648f, 1.000f, 0.078f, 0.576f,
 0.657f, 1.000f, 0.078f, 0.576f,
 0.667f, 1.000f, 0.078f, 0.576f,
 0.676f, 1.000f, 0.078f, 0.576f,
 0.685f, 1.000f, 0.078f, 0.576f,
 0.694f, 1.000f, 0.078f, 0.576f,
 0.704f, 1.000f, 0.078f, 0.576f,
 0.713f, 1.000f, 0.078f, 0.576f,
 0.722f, 0.855f, 0.647f, 0.125f,
 0.731f, 1.000f, 0.078f, 0.576f,
 0.741f, 1.000f, 0.078f, 0.576f,
 0.750f, 1.000f, 0.078f, 0.576f,
 0.759f, 1.000f, 0.078f, 0.576f,
 0.769f, 1.000f, 0.078f, 0.576f,
 0.778f, 1.000f, 0.078f, 0.576f,
 0.787f, 1.000f, 0.078f, 0.576f,
 0.796f, 1.000f, 0.078f, 0.576f,
 0.806f, 1.000f, 0.078f, 0.576f,
 0.815f, 1.000f, 0.078f, 0.576f,
 0.824f, 1.000f, 0.078f, 0.576f,
 0.833f, 1.000f, 0.078f, 0.576f,
 0.843f, 1.000f, 0.078f, 0.576f,
 0.852f, 1.000f, 0.078f, 0.576f,
 0.861f, 1.000f, 0.078f, 0.576f,
 0.870f, 1.000f, 0.078f, 0.576f,
 0.880f, 1.000f, 0.078f, 0.576f,
 0.889f, 1.000f, 0.078f, 0.576f,
 0.898f, 1.000f, 0.078f, 0.576f,
 0.907f, 1.000f, 0.078f, 0.576f,
 0.917f, 1.000f, 0.078f, 0.576f,
 0.926f, 1.000f, 0.078f, 0.576f,
 0.935f, 1.000f, 0.078f, 0.576f,
 0.944f, 1.000f, 0.078f, 0.576f,
 0.954f, 1.000f, 0.078f, 0.576f,
 0.963f, 1.000f, 0.078f, 0.576f,
 0.972f, 1.000f, 0.078f, 0.576f,
 0.981f, 1.000f, 0.078f, 0.576f,
 0.991f, 1.000f, 0.078f, 0.576f,
 1.000f, 1.000f, 0.078f, 0.576f,
};

static const float ct_amino_colors[] = {
 0.000f, 0.745f, 0.627f, 0.431f,
 0.009f, 0.784f, 0.784f, 0.784f,
 0.019f, 1.000f, 0.412f, 0.706f,
 0.028f, 0.902f, 0.902f, 0.000f,
 0.037f, 0.902f, 0.039f, 0.039f,
 0.046f, 0.902f, 0.039f, 0.039f,
 0.056f, 0.196f, 0.196f, 0.667f,
 0.065f, 0.922f, 0.922f, 0.922f,
 0.074f, 0.510f, 0.510f, 0.824f,
 0.083f, 0.059f, 0.510f, 0.059f,
 0.093f, 0.078f, 0.353f, 1.000f,
 0.102f, 0.059f, 0.510f, 0.059f,
 0.111f, 0.902f, 0.902f, 0.000f,
 0.120f, 0.000f, 0.863f, 0.863f,
 0.130f, 0.863f, 0.588f, 0.510f,
 0.139f, 0.000f, 0.863f, 0.863f,
 0.148f, 0.078f, 0.353f, 1.000f,
 0.157f, 0.980f, 0.588f, 0.000f,
 0.167f, 0.980f, 0.588f, 0.000f,
 0.176f, 0.059f, 0.510f, 0.059f,
 0.185f, 0.706f, 0.353f, 0.706f,
 0.194f, 0.196f, 0.196f, 0.667f,
 0.204f, 1.000f, 0.412f, 0.706f,
};

static const float ct_shapely_colors[] = {
 0.000f, 1.000f, 0.000f, 1.000f,
 0.009f, 0.549f, 1.000f, 0.549f,
 0.019f, 1.000f, 0.000f, 1.000f,
 0.028f, 1.000f, 1.000f, 0.439f,
 0.037f, 0.627f, 0.000f, 0.259f,
 0.046f, 0.400f, 0.000f, 0.000f,
 0.056f, 0.325f, 0.298f, 0.322f,
 0.065f, 1.000f, 1.000f, 1.000f,
 0.074f, 0.439f, 0.439f, 1.000f,
 0.083f, 0.000f, 0.298f, 0.000f,
 0.093f, 0.278f, 0.278f, 0.722f,
 0.102f, 0.271f, 0.369f, 0.271f,
 0.111f, 0.722f, 0.627f, 0.259f,
 0.120f, 1.000f, 0.486f, 0.439f,
 0.130f, 0.322f, 0.322f, 0.322f,
 0.139f, 1.000f, 0.298f, 0.298f,
 0.148f, 0.000f, 0.000f, 0.486f,
 0.157f, 1.000f, 0.439f, 0.259f,
 0.167f, 0.722f, 0.298f, 0.000f,
 0.176f, 1.000f, 0.549f, 1.000f,
 0.185f, 0.310f, 0.275f, 0.000f,
 0.194f, 0.549f, 0.439f, 0.298f,
 0.204f, 1.000f, 0.000f, 1.000f,
};

// Static pointer to single instance.
avtColorTables *avtColorTables::instance = NULL;

// ****************************************************************************
// Method: avtColorTables::avtColorTables
//
// Purpose: 
//   Constructor for the avtColorTables class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:04:51 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 13:34:47 PST 2002
//   I rewrote the routine.
//
//   Kathleen Bonnell, Thu Jan 13 17:58:39 PST 2005 
//   Make the opacity for the ColorControlPoint be 255. 
//
//   Hank Childs, Tue Nov 15 08:44:11 PST 2005
//   Add new color tables: blue hot, orange hot, difference, hot_desaturated,
//   hot and cold.
//
//   Jeremy Meredith, Wed Mar 22 13:44:25 PST 2006
//   Added molecular color tables.
//
// ****************************************************************************

avtColorTables::avtColorTables()
{
    ctAtts = new ColorTableAttributes();

    // Set up some pointers.
    const float *predef_ct_colors[17];
    predef_ct_colors[0]  = ct_bluehot;
    predef_ct_colors[1]  = ct_caleblack;
    predef_ct_colors[2]  = ct_calewhite;
    predef_ct_colors[3]  = ct_contoured;
    predef_ct_colors[4]  = ct_difference;
    predef_ct_colors[5]  = ct_gray;
    predef_ct_colors[6]  = ct_hot;
    predef_ct_colors[7]  = ct_hot_and_cold;
    predef_ct_colors[8]  = ct_hot_desaturated;
    predef_ct_colors[9]  = ct_levels;
    predef_ct_colors[10] = ct_orangehot;
    predef_ct_colors[11] = ct_rainbow;
    predef_ct_colors[12] = ct_xray;
    predef_ct_colors[13] = ct_jmol_colors;
    predef_ct_colors[14] = ct_rasmol_colors;
    predef_ct_colors[15] = ct_amino_colors;
    predef_ct_colors[16] = ct_shapely_colors;

    // Add each colortable.
    for(int i = 0; i < 17; ++i)
    {
        ColorControlPointList ccpl;

        const float *fptr = predef_ct_colors[i];
        for(int j = 0; j < predef_ct_ncolors[i]; ++j)
        {
            ColorControlPoint p(fptr[0],
                                (unsigned char)(fptr[1]*255),
                                (unsigned char)(fptr[2]*255),
                                (unsigned char)(fptr[3]*255),
                                255);
            ccpl.AddColorControlPoint(p);
            fptr += 4;
        }

        ccpl.SetSmoothingFlag(predef_ct_smooth[i] == 1);
        ccpl.SetEqualSpacingFlag(predef_ct_equal[i] == 1);
        ccpl.SetDiscreteFlag(predef_ct_discrete[i] == 1);
        ctAtts->AddColorTable(predef_ct_names[i], ccpl);
    }

    // Set the active continuous color table to "hot".
    ctAtts->SetActiveContinuous("hot");
    // Set the active discrete color table to "levels".
    ctAtts->SetActiveDiscrete("levels");
}

// ****************************************************************************
// Method: avtColorTables::~avtColorTables
//
// Purpose: 
//   Destructor for the avtColorTables class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:04:51 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 13:35:46 PST 2002
//   I removed some deletion code.
//
// ****************************************************************************

avtColorTables::~avtColorTables()
{
    delete ctAtts;
    ctAtts = 0;
}

// ****************************************************************************
// Method: avtColorTables::Instance
//
// Purpose: 
//   Static function that returns a pointer to the single instance of the
//   class.
//
// Returns:    A pointer to the single instance of avtColorTables.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:05:26 PST 2001
//
// Modifications:
//   
// ****************************************************************************

avtColorTables *
avtColorTables::Instance()
{
    if(instance == NULL)
        instance = new avtColorTables;

    return instance;
}

// ****************************************************************************
// Method: avtColorTables::GetDefaultContinuousColorTable
//
// Purpose: 
//   Returns the name of the default continuous color table.
//
// Returns:    The name of the default continuous color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:06:07 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 13:36:51 PST 2002
//   I renamed the method.
//
// ****************************************************************************

const std::string &
avtColorTables::GetDefaultContinuousColorTable() const
{
    return ctAtts->GetActiveContinuous();
}

// ****************************************************************************
// Method: avtColorTables::GetDefaultDiscreteColorTable
//
// Purpose: 
//   Returns the name of the default discrete color table.
//
// Returns:    The name of the default discrete color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:06:07 PST 2001
//
// Modifications:
//   
// ****************************************************************************

const std::string &
avtColorTables::GetDefaultDiscreteColorTable() const
{
    return ctAtts->GetActiveDiscrete();
}

// ****************************************************************************
// Method: avtColorTables::ColorTableExists
//
// Purpose: 
//   Determines whether or not the specified color table exists.
//
// Arguments:
//   ctName : The name of the color table to look for.
//
// Returns:    True if ctName is in avtColorTables, false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:06:43 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
avtColorTables::ColorTableExists(const std::string &ctName) const
{
    return (ctAtts->GetColorTableIndex(ctName) != -1);
}

// ****************************************************************************
// Method: avtColorTables::IsDiscrete
//
// Purpose: 
//   Returns whether or not the named color table is discrete.
//
// Arguments:
//   ctName : The name of the color table.
//
// Returns:    True if the named color table is discrete.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 14:25:34 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtColorTables::IsDiscrete(const std::string &ctName) const
{
    bool retval = false;
    int index;
    if((index = ctAtts->GetColorTableIndex(ctName)) != -1)
    {
        const ColorControlPointList &ct = ctAtts->operator[](index);
        retval = ct.GetDiscreteFlag();
    }

    return retval;
}

// ****************************************************************************
// Method: avtColorTables::SetDefaultContinuousColorTable
//
// Purpose: 
//   Sets the default continuous color table.
//
// Arguments:
//   ctName : The name of the new default continuous color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:08:36 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtColorTables::SetDefaultContinuousColorTable(const std::string &ctName)
{
    if(ColorTableExists(ctName))
    {
        ctAtts->SetActiveContinuous(ctName);
    }
}

// ****************************************************************************
// Method: avtColorTables::SetDefaultDiscreteColorTable
//
// Purpose: 
//   Sets the default discrete color table.
//
// Arguments:
//   ctName : The name of the new default discrete color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:08:36 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
avtColorTables::SetDefaultDiscreteColorTable(const std::string &ctName)
{
    if(ColorTableExists(ctName))
    {
        ctAtts->SetActiveDiscrete(ctName);
    }
}

// ****************************************************************************
// Method: avtColorTables::GetColors
//
// Purpose: 
//   Returns a pointer to the named color table's colors.
//
// Arguments:
//   ctName : The name of the color table for which we want the colors.
//
// Returns:    The color array for the color table or NULL if ctName is not
//             the name of a valid color table.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 15 14:09:03 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Dec 3 14:24:06 PST 2002
//   I rewrote the method.
//
// ****************************************************************************

const unsigned char *
avtColorTables::GetColors(const std::string &ctName)
{
    const unsigned char *retval = NULL;
    int index;
    if((index = ctAtts->GetColorTableIndex(ctName)) != -1)
    {
        const ColorControlPointList &ct = ctAtts->operator[](index);
        ct.GetColors(tmpColors, GetNumColors());
        retval = tmpColors;
    }

    return retval;
}

// ****************************************************************************
// Method: avtColorTables::GetSampledColors
//
// Purpose: 
//   Returns colors for the specified color table sampled over a certain range.
//
// Arguments:
//   ctName  : The name of the color table.
//   nColors : The number of sample colors.
//   
// Returns:    An unsigned char array that contains the colors.
//
// Note:       The caller is responsible for freeing the returned array.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 14:20:10 PST 2002
//
// Modifications:
//   
// ****************************************************************************

unsigned char *
avtColorTables::GetSampledColors(const std::string &ctName, int nColors) const
{
    unsigned char *retval = NULL;
    int index;
    if((index = ctAtts->GetColorTableIndex(ctName)) != -1)
    {
        const ColorControlPointList &ct = ctAtts->operator[](index);

        int nc = (nColors < 1) ? 1 : nColors;
        retval = new unsigned char[nc * 3];
        ct.GetColors(retval, nc);
    }
  
    return retval;
}

// ****************************************************************************
// Method: avtColorTables::GetControlPointColor
//
// Purpose: 
//   Gets the color of the i'th color control point for the specified
//   color table.
//
// Arguments:
//   ctName : The name of the discrete color table.
//   i      : The index of the color control point. The value is mod'ed so
//            that it always falls within the number of control points for
//            the specified color table.
//   rgb    : The return array for the colors.
//
// Returns:    A boolean value indicating whether or not a color was returned.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 20 13:47:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtColorTables::GetControlPointColor(const std::string &ctName, int i,
    unsigned char *rgb) const
{
    bool retval = false;
    int index;
    if((index = ctAtts->GetColorTableIndex(ctName)) != -1)
    {
        const ColorControlPointList &ct = ctAtts->operator[](index);
        int j = i % ct.GetNumColorControlPoints();

        rgb[0] = ct[j].GetColors()[0];
        rgb[1] = ct[j].GetColors()[1];
        rgb[2] = ct[j].GetColors()[2];
        retval = true;
    }

    return retval;
}

// ****************************************************************************
// Method: avtColorTables::SetColorTables
//
// Purpose: 
//   Copies new color tables into the color table attributes.
//
// Arguments:
//   atts : The new color table list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 3 14:34:48 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
avtColorTables::SetColorTables(const ColorTableAttributes &atts)
{
    *ctAtts = atts;
}

// ****************************************************************************
// Method: avtColorTables::ExportColorTable
//
// Purpose: 
//   Exports the specified color table to a file that can be shared.
//
// Arguments:
//   ctName : The name of the color table to export.
//
// Returns:    A string indicating what happened.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:28:45 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 11:59:46 PDT 2003
//   I changed how the messages are returned.
//
// ****************************************************************************

bool
avtColorTables::ExportColorTable(const std::string &ctName,
    std::string &message)
{
    const ColorControlPointList *ccpl = ctAtts->GetColorControlPoints(ctName);

    if(ccpl != 0)
    {
        ColorTableManager exporter;
        return exporter.Export(ctName, *ccpl, message);
    }

    message = std::string("VisIt could not save the color table\"") + ctName +
              std::string("\" because that color table does not exist.");
    return false;
}

// ****************************************************************************
// Method: avtColorTables::ImportColorTables
//
// Purpose: 
//   Imports the color tables in the user's home .visit directory and adds
//   them to the list of color tables.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 18:30:07 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
avtColorTables::ImportColorTables()
{
    //
    // Create a color table manager to import the color tables and store
    // them in the ctAtts.
    //
    ColorTableManager importer;
    importer.ImportColorTables(ctAtts);
}

