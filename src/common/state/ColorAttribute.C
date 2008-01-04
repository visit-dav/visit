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

#include <ColorAttribute.h>
#include <DataNode.h>

// **************************************************************************
// Constant: namedColors
//
// Purpose: List of color names and their rgb values
//
// Programmer: Mark C. Miller,
// Creation:   July 13, 2006
// **************************************************************************

typedef struct { unsigned char r,g,b; char *n; } named_color_t;

static named_color_t namedColors[] =
{
// XConsortium: rgb.txt,v 10.41 94/02/20 18:39:36 rws Exp
    {255 ,250 ,250,        "snow"},
    {248 ,248 ,255,        "ghost white"},
    {248 ,248 ,255,        "GhostWhite"},
    {245 ,245 ,245,        "white smoke"},
    {245 ,245 ,245,        "WhiteSmoke"},
    {220 ,220 ,220,        "gainsboro"},
    {255 ,250 ,240,        "floral white"},
    {255 ,250 ,240,        "FloralWhite"},
    {253 ,245 ,230,        "old lace"},
    {253 ,245 ,230,        "OldLace"},
    {250 ,240 ,230,        "linen"},
    {250 ,235 ,215,        "antique white"},
    {250 ,235 ,215,        "AntiqueWhite"},
    {255 ,239 ,213,        "papaya whip"},
    {255 ,239 ,213,        "PapayaWhip"},
    {255 ,235 ,205,        "blanched almond"},
    {255 ,235 ,205,        "BlanchedAlmond"},
    {255 ,228 ,196,        "bisque"},
    {255 ,218 ,185,        "peach puff"},
    {255 ,218 ,185,        "PeachPuff"},
    {255 ,222 ,173,        "navajo white"},
    {255 ,222 ,173,        "NavajoWhite"},
    {255 ,228 ,181,        "moccasin"},
    {255 ,248 ,220,        "cornsilk"},
    {255 ,255 ,240,        "ivory"},
    {255 ,250 ,205,        "lemon chiffon"},
    {255 ,250 ,205,        "LemonChiffon"},
    {255 ,245 ,238,        "seashell"},
    {240 ,255 ,240,        "honeydew"},
    {245 ,255 ,250,        "mint cream"},
    {245 ,255 ,250,        "MintCream"},
    {240 ,255 ,255,        "azure"},
    {240 ,248 ,255,        "alice blue"},
    {240 ,248 ,255,        "AliceBlue"},
    {230 ,230 ,250,        "lavender"},
    {255 ,240 ,245,        "lavender blush"},
    {255 ,240 ,245,        "LavenderBlush"},
    {255 ,228 ,225,        "misty rose"},
    {255 ,228 ,225,        "MistyRose"},
    {255 ,255 ,255,        "white"},
    {  0 ,  0 ,  0,        "black"},
    { 47 , 79 , 79,        "dark slate gray"},
    { 47 , 79 , 79,        "DarkSlateGray"},
    { 47 , 79 , 79,        "dark slate grey"},
    { 47 , 79 , 79,        "DarkSlateGrey"},
    {105 ,105 ,105,        "dim gray"},
    {105 ,105 ,105,        "DimGray"},
    {105 ,105 ,105,        "dim grey"},
    {105 ,105 ,105,        "DimGrey"},
    {112 ,128 ,144,        "slate gray"},
    {112 ,128 ,144,        "SlateGray"},
    {112 ,128 ,144,        "slate grey"},
    {112 ,128 ,144,        "SlateGrey"},
    {119 ,136 ,153,        "light slate gray"},
    {119 ,136 ,153,        "LightSlateGray"},
    {119 ,136 ,153,        "light slate grey"},
    {119 ,136 ,153,        "LightSlateGrey"},
    {190 ,190 ,190,        "gray"},
    {190 ,190 ,190,        "grey"},
    {211 ,211 ,211,        "light grey"},
    {211 ,211 ,211,        "LightGrey"},
    {211 ,211 ,211,        "light gray"},
    {211 ,211 ,211,        "LightGray"},
    { 25 , 25 ,112,        "midnight blue"},
    { 25 , 25 ,112,        "MidnightBlue"},
    {  0 ,  0 ,128,        "navy"},
    {  0 ,  0 ,128,        "navy blue"},
    {  0 ,  0 ,128,        "NavyBlue"},
    {100 ,149 ,237,        "cornflower blue"},
    {100 ,149 ,237,        "CornflowerBlue"},
    { 72 , 61 ,139,        "dark slate blue"},
    { 72 , 61 ,139,        "DarkSlateBlue"},
    {106 , 90 ,205,        "slate blue"},
    {106 , 90 ,205,        "SlateBlue"},
    {123 ,104 ,238,        "medium slate blue"},
    {123 ,104 ,238,        "MediumSlateBlue"},
    {132 ,112 ,255,        "light slate blue"},
    {132 ,112 ,255,        "LightSlateBlue"},
    {  0 ,  0 ,205,        "medium blue"},
    {  0 ,  0 ,205,        "MediumBlue"},
    { 65 ,105 ,225,        "royal blue"},
    { 65 ,105 ,225,        "RoyalBlue"},
    {  0 ,  0 ,255,        "blue"},
    { 30 ,144 ,255,        "dodger blue"},
    { 30 ,144 ,255,        "DodgerBlue"},
    {  0 ,191 ,255,        "deep sky blue"},
    {  0 ,191 ,255,        "DeepSkyBlue"},
    {135 ,206 ,235,        "sky blue"},
    {135 ,206 ,235,        "SkyBlue"},
    {135 ,206 ,250,        "light sky blue"},
    {135 ,206 ,250,        "LightSkyBlue"},
    { 70 ,130 ,180,        "steel blue"},
    { 70 ,130 ,180,        "SteelBlue"},
    {176 ,196 ,222,        "light steel blue"},
    {176 ,196 ,222,        "LightSteelBlue"},
    {173 ,216 ,230,        "light blue"},
    {173 ,216 ,230,        "LightBlue"},
    {176 ,224 ,230,        "powder blue"},
    {176 ,224 ,230,        "PowderBlue"},
    {175 ,238 ,238,        "pale turquoise"},
    {175 ,238 ,238,        "PaleTurquoise"},
    {  0 ,206 ,209,        "dark turquoise"},
    {  0 ,206 ,209,        "DarkTurquoise"},
    { 72 ,209 ,204,        "medium turquoise"},
    { 72 ,209 ,204,        "MediumTurquoise"},
    { 64 ,224 ,208,        "turquoise"},
    {  0 ,255 ,255,        "cyan"},
    {224 ,255 ,255,        "light cyan"},
    {224 ,255 ,255,        "LightCyan"},
    { 95 ,158 ,160,        "cadet blue"},
    { 95 ,158 ,160,        "CadetBlue"},
    {102 ,205 ,170,        "medium aquamarine"},
    {102 ,205 ,170,        "MediumAquamarine"},
    {127 ,255 ,212,        "aquamarine"},
    {  0 ,100 ,  0,        "dark green"},
    {  0 ,100 ,  0,        "DarkGreen"},
    { 85 ,107 , 47,        "dark olive green"},
    { 85 ,107 , 47,        "DarkOliveGreen"},
    {143 ,188 ,143,        "dark sea green"},
    {143 ,188 ,143,        "DarkSeaGreen"},
    { 46 ,139 , 87,        "sea green"},
    { 46 ,139 , 87,        "SeaGreen"},
    { 60 ,179 ,113,        "medium sea green"},
    { 60 ,179 ,113,        "MediumSeaGreen"},
    { 32 ,178 ,170,        "light sea green"},
    { 32 ,178 ,170,        "LightSeaGreen"},
    {152 ,251 ,152,        "pale green"},
    {152 ,251 ,152,        "PaleGreen"},
    {  0 ,255 ,127,        "spring green"},
    {  0 ,255 ,127,        "SpringGreen"},
    {124 ,252 ,  0,        "lawn green"},
    {124 ,252 ,  0,        "LawnGreen"},
    {  0 ,255 ,  0,        "green"},
    {127 ,255 ,  0,        "chartreuse"},
    {  0 ,250 ,154,        "medium spring green"},
    {  0 ,250 ,154,        "MediumSpringGreen"},
    {173 ,255 , 47,        "green yellow"},
    {173 ,255 , 47,        "GreenYellow"},
    { 50 ,205 , 50,        "lime green"},
    { 50 ,205 , 50,        "LimeGreen"},
    {154 ,205 , 50,        "yellow green"},
    {154 ,205 , 50,        "YellowGreen"},
    { 34 ,139 , 34,        "forest green"},
    { 34 ,139 , 34,        "ForestGreen"},
    {107 ,142 , 35,        "olive drab"},
    {107 ,142 , 35,        "OliveDrab"},
    {189 ,183 ,107,        "dark khaki"},
    {189 ,183 ,107,        "DarkKhaki"},
    {240 ,230 ,140,        "khaki"},
    {238 ,232 ,170,        "pale goldenrod"},
    {238 ,232 ,170,        "PaleGoldenrod"},
    {250 ,250 ,210,        "light goldenrod yellow"},
    {250 ,250 ,210,        "LightGoldenrodYellow"},
    {255 ,255 ,224,        "light yellow"},
    {255 ,255 ,224,        "LightYellow"},
    {255 ,255 ,  0,        "yellow"},
    {255 ,215 ,  0,        "gold"},
    {238 ,221 ,130,        "light goldenrod"},
    {238 ,221 ,130,        "LightGoldenrod"},
    {218 ,165 , 32,        "goldenrod"},
    {184 ,134 , 11,        "dark goldenrod"},
    {184 ,134 , 11,        "DarkGoldenrod"},
    {188 ,143 ,143,        "rosy brown"},
    {188 ,143 ,143,        "RosyBrown"},
    {205 , 92 , 92,        "indian red"},
    {205 , 92 , 92,        "IndianRed"},
    {139 , 69 , 19,        "saddle brown"},
    {139 , 69 , 19,        "SaddleBrown"},
    {160 , 82 , 45,        "sienna"},
    {205 ,133 , 63,        "peru"},
    {222 ,184 ,135,        "burlywood"},
    {245 ,245 ,220,        "beige"},
    {245 ,222 ,179,        "wheat"},
    {244 ,164 , 96,        "sandy brown"},
    {244 ,164 , 96,        "SandyBrown"},
    {210 ,180 ,140,        "tan"},
    {210 ,105 , 30,        "chocolate"},
    {178 , 34 , 34,        "firebrick"},
    {165 , 42 , 42,        "brown"},
    {233 ,150 ,122,        "dark salmon"},
    {233 ,150 ,122,        "DarkSalmon"},
    {250 ,128 ,114,        "salmon"},
    {255 ,160 ,122,        "light salmon"},
    {255 ,160 ,122,        "LightSalmon"},
    {255 ,165 ,  0,        "orange"},
    {255 ,140 ,  0,        "dark orange"},
    {255 ,140 ,  0,        "DarkOrange"},
    {255 ,127 , 80,        "coral"},
    {240 ,128 ,128,        "light coral"},
    {240 ,128 ,128,        "LightCoral"},
    {255 , 99 , 71,        "tomato"},
    {255 , 69 ,  0,        "orange red"},
    {255 , 69 ,  0,        "OrangeRed"},
    {255 ,  0 ,  0,        "red"},
    {255 ,105 ,180,        "hot pink"},
    {255 ,105 ,180,        "HotPink"},
    {255 , 20 ,147,        "deep pink"},
    {255 , 20 ,147,        "DeepPink"},
    {255 ,192 ,203,        "pink"},
    {255 ,182 ,193,        "light pink"},
    {255 ,182 ,193,        "LightPink"},
    {219 ,112 ,147,        "pale violet red"},
    {219 ,112 ,147,        "PaleVioletRed"},
    {176 , 48 , 96,        "maroon"},
    {199 , 21 ,133,        "medium violet red"},
    {199 , 21 ,133,        "MediumVioletRed"},
    {208 , 32 ,144,        "violet red"},
    {208 , 32 ,144,        "VioletRed"},
    {255 ,  0 ,255,        "magenta"},
    {238 ,130 ,238,        "violet"},
    {221 ,160 ,221,        "plum"},
    {218 ,112 ,214,        "orchid"},
    {186 , 85 ,211,        "medium orchid"},
    {186 , 85 ,211,        "MediumOrchid"},
    {153 , 50 ,204,        "dark orchid"},
    {153 , 50 ,204,        "DarkOrchid"},
    {148 ,  0 ,211,        "dark violet"},
    {148 ,  0 ,211,        "DarkViolet"},
    {138 , 43 ,226,        "blue violet"},
    {138 , 43 ,226,        "BlueViolet"},
    {160 , 32 ,240,        "purple"},
    {147 ,112 ,219,        "medium purple"},
    {147 ,112 ,219,        "MediumPurple"},
    {216 ,191 ,216,        "thistle"},
    {255 ,250 ,250,        "snow1"},
    {238 ,233 ,233,        "snow2"},
    {205 ,201 ,201,        "snow3"},
    {139 ,137 ,137,        "snow4"},
    {255 ,245 ,238,        "seashell1"},
    {238 ,229 ,222,        "seashell2"},
    {205 ,197 ,191,        "seashell3"},
    {139 ,134 ,130,        "seashell4"},
    {255 ,239 ,219,        "AntiqueWhite1"},
    {238 ,223 ,204,        "AntiqueWhite2"},
    {205 ,192 ,176,        "AntiqueWhite3"},
    {139 ,131 ,120,        "AntiqueWhite4"},
    {255 ,228 ,196,        "bisque1"},
    {238 ,213 ,183,        "bisque2"},
    {205 ,183 ,158,        "bisque3"},
    {139 ,125 ,107,        "bisque4"},
    {255 ,218 ,185,        "PeachPuff1"},
    {238 ,203 ,173,        "PeachPuff2"},
    {205 ,175 ,149,        "PeachPuff3"},
    {139 ,119 ,101,        "PeachPuff4"},
    {255 ,222 ,173,        "NavajoWhite1"},
    {238 ,207 ,161,        "NavajoWhite2"},
    {205 ,179 ,139,        "NavajoWhite3"},
    {139 ,121 , 94,        "NavajoWhite4"},
    {255 ,250 ,205,        "LemonChiffon1"},
    {238 ,233 ,191,        "LemonChiffon2"},
    {205 ,201 ,165,        "LemonChiffon3"},
    {139 ,137 ,112,        "LemonChiffon4"},
    {255 ,248 ,220,        "cornsilk1"},
    {238 ,232 ,205,        "cornsilk2"},
    {205 ,200 ,177,        "cornsilk3"},
    {139 ,136 ,120,        "cornsilk4"},
    {255 ,255 ,240,        "ivory1"},
    {238 ,238 ,224,        "ivory2"},
    {205 ,205 ,193,        "ivory3"},
    {139 ,139 ,131,        "ivory4"},
    {240 ,255 ,240,        "honeydew1"},
    {224 ,238 ,224,        "honeydew2"},
    {193 ,205 ,193,        "honeydew3"},
    {131 ,139 ,131,        "honeydew4"},
    {255 ,240 ,245,        "LavenderBlush1"},
    {238 ,224 ,229,        "LavenderBlush2"},
    {205 ,193 ,197,        "LavenderBlush3"},
    {139 ,131 ,134,        "LavenderBlush4"},
    {255 ,228 ,225,        "MistyRose1"},
    {238 ,213 ,210,        "MistyRose2"},
    {205 ,183 ,181,        "MistyRose3"},
    {139 ,125 ,123,        "MistyRose4"},
    {240 ,255 ,255,        "azure1"},
    {224 ,238 ,238,        "azure2"},
    {193 ,205 ,205,        "azure3"},
    {131 ,139 ,139,        "azure4"},
    {131 ,111 ,255,        "SlateBlue1"},
    {122 ,103 ,238,        "SlateBlue2"},
    {105 , 89 ,205,        "SlateBlue3"},
    { 71 , 60 ,139,        "SlateBlue4"},
    { 72 ,118 ,255,        "RoyalBlue1"},
    { 67 ,110 ,238,        "RoyalBlue2"},
    { 58 , 95 ,205,        "RoyalBlue3"},
    { 39 , 64 ,139,        "RoyalBlue4"},
    {  0 ,  0 ,255,        "blue1"},
    {  0 ,  0 ,238,        "blue2"},
    {  0 ,  0 ,205,        "blue3"},
    {  0 ,  0 ,139,        "blue4"},
    { 30 ,144 ,255,        "DodgerBlue1"},
    { 28 ,134 ,238,        "DodgerBlue2"},
    { 24 ,116 ,205,        "DodgerBlue3"},
    { 16 , 78 ,139,        "DodgerBlue4"},
    { 99 ,184 ,255,        "SteelBlue1"},
    { 92 ,172 ,238,        "SteelBlue2"},
    { 79 ,148 ,205,        "SteelBlue3"},
    { 54 ,100 ,139,        "SteelBlue4"},
    {  0 ,191 ,255,        "DeepSkyBlue1"},
    {  0 ,178 ,238,        "DeepSkyBlue2"},
    {  0 ,154 ,205,        "DeepSkyBlue3"},
    {  0 ,104 ,139,        "DeepSkyBlue4"},
    {135 ,206 ,255,        "SkyBlue1"},
    {126 ,192 ,238,        "SkyBlue2"},
    {108 ,166 ,205,        "SkyBlue3"},
    { 74 ,112 ,139,        "SkyBlue4"},
    {176 ,226 ,255,        "LightSkyBlue1"},
    {164 ,211 ,238,        "LightSkyBlue2"},
    {141 ,182 ,205,        "LightSkyBlue3"},
    { 96 ,123 ,139,        "LightSkyBlue4"},
    {198 ,226 ,255,        "SlateGray1"},
    {185 ,211 ,238,        "SlateGray2"},
    {159 ,182 ,205,        "SlateGray3"},
    {108 ,123 ,139,        "SlateGray4"},
    {202 ,225 ,255,        "LightSteelBlue1"},
    {188 ,210 ,238,        "LightSteelBlue2"},
    {162 ,181 ,205,        "LightSteelBlue3"},
    {110 ,123 ,139,        "LightSteelBlue4"},
    {191 ,239 ,255,        "LightBlue1"},
    {178 ,223 ,238,        "LightBlue2"},
    {154 ,192 ,205,        "LightBlue3"},
    {104 ,131 ,139,        "LightBlue4"},
    {224 ,255 ,255,        "LightCyan1"},
    {209 ,238 ,238,        "LightCyan2"},
    {180 ,205 ,205,        "LightCyan3"},
    {122 ,139 ,139,        "LightCyan4"},
    {187 ,255 ,255,        "PaleTurquoise1"},
    {174 ,238 ,238,        "PaleTurquoise2"},
    {150 ,205 ,205,        "PaleTurquoise3"},
    {102 ,139 ,139,        "PaleTurquoise4"},
    {152 ,245 ,255,        "CadetBlue1"},
    {142 ,229 ,238,        "CadetBlue2"},
    {122 ,197 ,205,        "CadetBlue3"},
    { 83 ,134 ,139,        "CadetBlue4"},
    {  0 ,245 ,255,        "turquoise1"},
    {  0 ,229 ,238,        "turquoise2"},
    {  0 ,197 ,205,        "turquoise3"},
    {  0 ,134 ,139,        "turquoise4"},
    {  0 ,255 ,255,        "cyan1"},
    {  0 ,238 ,238,        "cyan2"},
    {  0 ,205 ,205,        "cyan3"},
    {  0 ,139 ,139,        "cyan4"},
    {151 ,255 ,255,        "DarkSlateGray1"},
    {141 ,238 ,238,        "DarkSlateGray2"},
    {121 ,205 ,205,        "DarkSlateGray3"},
    { 82 ,139 ,139,        "DarkSlateGray4"},
    {127 ,255 ,212,        "aquamarine1"},
    {118 ,238 ,198,        "aquamarine2"},
    {102 ,205 ,170,        "aquamarine3"},
    { 69 ,139 ,116,        "aquamarine4"},
    {193 ,255 ,193,        "DarkSeaGreen1"},
    {180 ,238 ,180,        "DarkSeaGreen2"},
    {155 ,205 ,155,        "DarkSeaGreen3"},
    {105 ,139 ,105,        "DarkSeaGreen4"},
    { 84 ,255 ,159,        "SeaGreen1"},
    { 78 ,238 ,148,        "SeaGreen2"},
    { 67 ,205 ,128,        "SeaGreen3"},
    { 46 ,139 , 87,        "SeaGreen4"},
    {154 ,255 ,154,        "PaleGreen1"},
    {144 ,238 ,144,        "PaleGreen2"},
    {124 ,205 ,124,        "PaleGreen3"},
    { 84 ,139 , 84,        "PaleGreen4"},
    {  0 ,255 ,127,        "SpringGreen1"},
    {  0 ,238 ,118,        "SpringGreen2"},
    {  0 ,205 ,102,        "SpringGreen3"},
    {  0 ,139 , 69,        "SpringGreen4"},
    {  0 ,255 ,  0,        "green1"},
    {  0 ,238 ,  0,        "green2"},
    {  0 ,205 ,  0,        "green3"},
    {  0 ,139 ,  0,        "green4"},
    {127 ,255 ,  0,        "chartreuse1"},
    {118 ,238 ,  0,        "chartreuse2"},
    {102 ,205 ,  0,        "chartreuse3"},
    { 69 ,139 ,  0,        "chartreuse4"},
    {192 ,255 , 62,        "OliveDrab1"},
    {179 ,238 , 58,        "OliveDrab2"},
    {154 ,205 , 50,        "OliveDrab3"},
    {105 ,139 , 34,        "OliveDrab4"},
    {202 ,255 ,112,        "DarkOliveGreen1"},
    {188 ,238 ,104,        "DarkOliveGreen2"},
    {162 ,205 , 90,        "DarkOliveGreen3"},
    {110 ,139 , 61,        "DarkOliveGreen4"},
    {255 ,246 ,143,        "khaki1"},
    {238 ,230 ,133,        "khaki2"},
    {205 ,198 ,115,        "khaki3"},
    {139 ,134 , 78,        "khaki4"},
    {255 ,236 ,139,        "LightGoldenrod1"},
    {238 ,220 ,130,        "LightGoldenrod2"},
    {205 ,190 ,112,        "LightGoldenrod3"},
    {139 ,129 , 76,        "LightGoldenrod4"},
    {255 ,255 ,224,        "LightYellow1"},
    {238 ,238 ,209,        "LightYellow2"},
    {205 ,205 ,180,        "LightYellow3"},
    {139 ,139 ,122,        "LightYellow4"},
    {255 ,255 ,  0,        "yellow1"},
    {238 ,238 ,  0,        "yellow2"},
    {205 ,205 ,  0,        "yellow3"},
    {139 ,139 ,  0,        "yellow4"},
    {255 ,215 ,  0,        "gold1"},
    {238 ,201 ,  0,        "gold2"},
    {205 ,173 ,  0,        "gold3"},
    {139 ,117 ,  0,        "gold4"},
    {255 ,193 , 37,        "goldenrod1"},
    {238 ,180 , 34,        "goldenrod2"},
    {205 ,155 , 29,        "goldenrod3"},
    {139 ,105 , 20,        "goldenrod4"},
    {255 ,185 , 15,        "DarkGoldenrod1"},
    {238 ,173 , 14,        "DarkGoldenrod2"},
    {205 ,149 , 12,        "DarkGoldenrod3"},
    {139 ,101 ,  8,        "DarkGoldenrod4"},
    {255 ,193 ,193,        "RosyBrown1"},
    {238 ,180 ,180,        "RosyBrown2"},
    {205 ,155 ,155,        "RosyBrown3"},
    {139 ,105 ,105,        "RosyBrown4"},
    {255 ,106 ,106,        "IndianRed1"},
    {238 , 99 , 99,        "IndianRed2"},
    {205 , 85 , 85,        "IndianRed3"},
    {139 , 58 , 58,        "IndianRed4"},
    {255 ,130 , 71,        "sienna1"},
    {238 ,121 , 66,        "sienna2"},
    {205 ,104 , 57,        "sienna3"},
    {139 , 71 , 38,        "sienna4"},
    {255 ,211 ,155,        "burlywood1"},
    {238 ,197 ,145,        "burlywood2"},
    {205 ,170 ,125,        "burlywood3"},
    {139 ,115 , 85,        "burlywood4"},
    {255 ,231 ,186,        "wheat1"},
    {238 ,216 ,174,        "wheat2"},
    {205 ,186 ,150,        "wheat3"},
    {139 ,126 ,102,        "wheat4"},
    {255 ,165 , 79,        "tan1"},
    {238 ,154 , 73,        "tan2"},
    {205 ,133 , 63,        "tan3"},
    {139 , 90 , 43,        "tan4"},
    {255 ,127 , 36,        "chocolate1"},
    {238 ,118 , 33,        "chocolate2"},
    {205 ,102 , 29,        "chocolate3"},
    {139 , 69 , 19,        "chocolate4"},
    {255 , 48 , 48,        "firebrick1"},
    {238 , 44 , 44,        "firebrick2"},
    {205 , 38 , 38,        "firebrick3"},
    {139 , 26 , 26,        "firebrick4"},
    {255 , 64 , 64,        "brown1"},
    {238 , 59 , 59,        "brown2"},
    {205 , 51 , 51,        "brown3"},
    {139 , 35 , 35,        "brown4"},
    {255 ,140 ,105,        "salmon1"},
    {238 ,130 , 98,        "salmon2"},
    {205 ,112 , 84,        "salmon3"},
    {139 , 76 , 57,        "salmon4"},
    {255 ,160 ,122,        "LightSalmon1"},
    {238 ,149 ,114,        "LightSalmon2"},
    {205 ,129 , 98,        "LightSalmon3"},
    {139 , 87 , 66,        "LightSalmon4"},
    {255 ,165 ,  0,        "orange1"},
    {238 ,154 ,  0,        "orange2"},
    {205 ,133 ,  0,        "orange3"},
    {139 , 90 ,  0,        "orange4"},
    {255 ,127 ,  0,        "DarkOrange1"},
    {238 ,118 ,  0,        "DarkOrange2"},
    {205 ,102 ,  0,        "DarkOrange3"},
    {139 , 69 ,  0,        "DarkOrange4"},
    {255 ,114 , 86,        "coral1"},
    {238 ,106 , 80,        "coral2"},
    {205 , 91 , 69,        "coral3"},
    {139 , 62 , 47,        "coral4"},
    {255 , 99 , 71,        "tomato1"},
    {238 , 92 , 66,        "tomato2"},
    {205 , 79 , 57,        "tomato3"},
    {139 , 54 , 38,        "tomato4"},
    {255 , 69 ,  0,        "OrangeRed1"},
    {238 , 64 ,  0,        "OrangeRed2"},
    {205 , 55 ,  0,        "OrangeRed3"},
    {139 , 37 ,  0,        "OrangeRed4"},
    {255 ,  0 ,  0,        "red1"},
    {238 ,  0 ,  0,        "red2"},
    {205 ,  0 ,  0,        "red3"},
    {139 ,  0 ,  0,        "red4"},
    {255 , 20 ,147,        "DeepPink1"},
    {238 , 18 ,137,        "DeepPink2"},
    {205 , 16 ,118,        "DeepPink3"},
    {139 , 10 , 80,        "DeepPink4"},
    {255 ,110 ,180,        "HotPink1"},
    {238 ,106 ,167,        "HotPink2"},
    {205 , 96 ,144,        "HotPink3"},
    {139 , 58 , 98,        "HotPink4"},
    {255 ,181 ,197,        "pink1"},
    {238 ,169 ,184,        "pink2"},
    {205 ,145 ,158,        "pink3"},
    {139 , 99 ,108,        "pink4"},
    {255 ,174 ,185,        "LightPink1"},
    {238 ,162 ,173,        "LightPink2"},
    {205 ,140 ,149,        "LightPink3"},
    {139 , 95 ,101,        "LightPink4"},
    {255 ,130 ,171,        "PaleVioletRed1"},
    {238 ,121 ,159,        "PaleVioletRed2"},
    {205 ,104 ,137,        "PaleVioletRed3"},
    {139 , 71 , 93,        "PaleVioletRed4"},
    {255 , 52 ,179,        "maroon1"},
    {238 , 48 ,167,        "maroon2"},
    {205 , 41 ,144,        "maroon3"},
    {139 , 28 , 98,        "maroon4"},
    {255 , 62 ,150,        "VioletRed1"},
    {238 , 58 ,140,        "VioletRed2"},
    {205 , 50 ,120,        "VioletRed3"},
    {139 , 34 , 82,        "VioletRed4"},
    {255 ,  0 ,255,        "magenta1"},
    {238 ,  0 ,238,        "magenta2"},
    {205 ,  0 ,205,        "magenta3"},
    {139 ,  0 ,139,        "magenta4"},
    {255 ,131 ,250,        "orchid1"},
    {238 ,122 ,233,        "orchid2"},
    {205 ,105 ,201,        "orchid3"},
    {139 , 71 ,137,        "orchid4"},
    {255 ,187 ,255,        "plum1"},
    {238 ,174 ,238,        "plum2"},
    {205 ,150 ,205,        "plum3"},
    {139 ,102 ,139,        "plum4"},
    {224 ,102 ,255,        "MediumOrchid1"},
    {209 , 95 ,238,        "MediumOrchid2"},
    {180 , 82 ,205,        "MediumOrchid3"},
    {122 , 55 ,139,        "MediumOrchid4"},
    {191 , 62 ,255,        "DarkOrchid1"},
    {178 , 58 ,238,        "DarkOrchid2"},
    {154 , 50 ,205,        "DarkOrchid3"},
    {104 , 34 ,139,        "DarkOrchid4"},
    {155 , 48 ,255,        "purple1"},
    {145 , 44 ,238,        "purple2"},
    {125 , 38 ,205,        "purple3"},
    { 85 , 26 ,139,        "purple4"},
    {171 ,130 ,255,        "MediumPurple1"},
    {159 ,121 ,238,        "MediumPurple2"},
    {137 ,104 ,205,        "MediumPurple3"},
    { 93 , 71 ,139,        "MediumPurple4"},
    {255 ,225 ,255,        "thistle1"},
    {238 ,210 ,238,        "thistle2"},
    {205 ,181 ,205,        "thistle3"},
    {139 ,123 ,139,        "thistle4"},
    {  0 ,  0 ,  0,        "gray0"},
    {  0 ,  0 ,  0,        "grey0"},
    {  3 ,  3 ,  3,        "gray1"},
    {  3 ,  3 ,  3,        "grey1"},
    {  5 ,  5 ,  5,        "gray2"},
    {  5 ,  5 ,  5,        "grey2"},
    {  8 ,  8 ,  8,        "gray3"},
    {  8 ,  8 ,  8,        "grey3"},
    { 10 , 10 , 10,        "gray4"},
    { 10 , 10 , 10,        "grey4"},
    { 13 , 13 , 13,        "gray5"},
    { 13 , 13 , 13,        "grey5"},
    { 15 , 15 , 15,        "gray6"},
    { 15 , 15 , 15,        "grey6"},
    { 18 , 18 , 18,        "gray7"},
    { 18 , 18 , 18,        "grey7"},
    { 20 , 20 , 20,        "gray8"},
    { 20 , 20 , 20,        "grey8"},
    { 23 , 23 , 23,        "gray9"},
    { 23 , 23 , 23,        "grey9"},
    { 26 , 26 , 26,        "gray10"},
    { 26 , 26 , 26,        "grey10"},
    { 28 , 28 , 28,        "gray11"},
    { 28 , 28 , 28,        "grey11"},
    { 31 , 31 , 31,        "gray12"},
    { 31 , 31 , 31,        "grey12"},
    { 33 , 33 , 33,        "gray13"},
    { 33 , 33 , 33,        "grey13"},
    { 36 , 36 , 36,        "gray14"},
    { 36 , 36 , 36,        "grey14"},
    { 38 , 38 , 38,        "gray15"},
    { 38 , 38 , 38,        "grey15"},
    { 41 , 41 , 41,        "gray16"},
    { 41 , 41 , 41,        "grey16"},
    { 43 , 43 , 43,        "gray17"},
    { 43 , 43 , 43,        "grey17"},
    { 46 , 46 , 46,        "gray18"},
    { 46 , 46 , 46,        "grey18"},
    { 48 , 48 , 48,        "gray19"},
    { 48 , 48 , 48,        "grey19"},
    { 51 , 51 , 51,        "gray20"},
    { 51 , 51 , 51,        "grey20"},
    { 54 , 54 , 54,        "gray21"},
    { 54 , 54 , 54,        "grey21"},
    { 56 , 56 , 56,        "gray22"},
    { 56 , 56 , 56,        "grey22"},
    { 59 , 59 , 59,        "gray23"},
    { 59 , 59 , 59,        "grey23"},
    { 61 , 61 , 61,        "gray24"},
    { 61 , 61 , 61,        "grey24"},
    { 64 , 64 , 64,        "gray25"},
    { 64 , 64 , 64,        "grey25"},
    { 66 , 66 , 66,        "gray26"},
    { 66 , 66 , 66,        "grey26"},
    { 69 , 69 , 69,        "gray27"},
    { 69 , 69 , 69,        "grey27"},
    { 71 , 71 , 71,        "gray28"},
    { 71 , 71 , 71,        "grey28"},
    { 74 , 74 , 74,        "gray29"},
    { 74 , 74 , 74,        "grey29"},
    { 77 , 77 , 77,        "gray30"},
    { 77 , 77 , 77,        "grey30"},
    { 79 , 79 , 79,        "gray31"},
    { 79 , 79 , 79,        "grey31"},
    { 82 , 82 , 82,        "gray32"},
    { 82 , 82 , 82,        "grey32"},
    { 84 , 84 , 84,        "gray33"},
    { 84 , 84 , 84,        "grey33"},
    { 87 , 87 , 87,        "gray34"},
    { 87 , 87 , 87,        "grey34"},
    { 89 , 89 , 89,        "gray35"},
    { 89 , 89 , 89,        "grey35"},
    { 92 , 92 , 92,        "gray36"},
    { 92 , 92 , 92,        "grey36"},
    { 94 , 94 , 94,        "gray37"},
    { 94 , 94 , 94,        "grey37"},
    { 97 , 97 , 97,        "gray38"},
    { 97 , 97 , 97,        "grey38"},
    { 99 , 99 , 99,        "gray39"},
    { 99 , 99 , 99,        "grey39"},
    {102 ,102 ,102,        "gray40"},
    {102 ,102 ,102,        "grey40"},
    {105 ,105 ,105,        "gray41"},
    {105 ,105 ,105,        "grey41"},
    {107 ,107 ,107,        "gray42"},
    {107 ,107 ,107,        "grey42"},
    {110 ,110 ,110,        "gray43"},
    {110 ,110 ,110,        "grey43"},
    {112 ,112 ,112,        "gray44"},
    {112 ,112 ,112,        "grey44"},
    {115 ,115 ,115,        "gray45"},
    {115 ,115 ,115,        "grey45"},
    {117 ,117 ,117,        "gray46"},
    {117 ,117 ,117,        "grey46"},
    {120 ,120 ,120,        "gray47"},
    {120 ,120 ,120,        "grey47"},
    {122 ,122 ,122,        "gray48"},
    {122 ,122 ,122,        "grey48"},
    {125 ,125 ,125,        "gray49"},
    {125 ,125 ,125,        "grey49"},
    {127 ,127 ,127,        "gray50"},
    {127 ,127 ,127,        "grey50"},
    {130 ,130 ,130,        "gray51"},
    {130 ,130 ,130,        "grey51"},
    {133 ,133 ,133,        "gray52"},
    {133 ,133 ,133,        "grey52"},
    {135 ,135 ,135,        "gray53"},
    {135 ,135 ,135,        "grey53"},
    {138 ,138 ,138,        "gray54"},
    {138 ,138 ,138,        "grey54"},
    {140 ,140 ,140,        "gray55"},
    {140 ,140 ,140,        "grey55"},
    {143 ,143 ,143,        "gray56"},
    {143 ,143 ,143,        "grey56"},
    {145 ,145 ,145,        "gray57"},
    {145 ,145 ,145,        "grey57"},
    {148 ,148 ,148,        "gray58"},
    {148 ,148 ,148,        "grey58"},
    {150 ,150 ,150,        "gray59"},
    {150 ,150 ,150,        "grey59"},
    {153 ,153 ,153,        "gray60"},
    {153 ,153 ,153,        "grey60"},
    {156 ,156 ,156,        "gray61"},
    {156 ,156 ,156,        "grey61"},
    {158 ,158 ,158,        "gray62"},
    {158 ,158 ,158,        "grey62"},
    {161 ,161 ,161,        "gray63"},
    {161 ,161 ,161,        "grey63"},
    {163 ,163 ,163,        "gray64"},
    {163 ,163 ,163,        "grey64"},
    {166 ,166 ,166,        "gray65"},
    {166 ,166 ,166,        "grey65"},
    {168 ,168 ,168,        "gray66"},
    {168 ,168 ,168,        "grey66"},
    {171 ,171 ,171,        "gray67"},
    {171 ,171 ,171,        "grey67"},
    {173 ,173 ,173,        "gray68"},
    {173 ,173 ,173,        "grey68"},
    {176 ,176 ,176,        "gray69"},
    {176 ,176 ,176,        "grey69"},
    {179 ,179 ,179,        "gray70"},
    {179 ,179 ,179,        "grey70"},
    {181 ,181 ,181,        "gray71"},
    {181 ,181 ,181,        "grey71"},
    {184 ,184 ,184,        "gray72"},
    {184 ,184 ,184,        "grey72"},
    {186 ,186 ,186,        "gray73"},
    {186 ,186 ,186,        "grey73"},
    {189 ,189 ,189,        "gray74"},
    {189 ,189 ,189,        "grey74"},
    {191 ,191 ,191,        "gray75"},
    {191 ,191 ,191,        "grey75"},
    {194 ,194 ,194,        "gray76"},
    {194 ,194 ,194,        "grey76"},
    {196 ,196 ,196,        "gray77"},
    {196 ,196 ,196,        "grey77"},
    {199 ,199 ,199,        "gray78"},
    {199 ,199 ,199,        "grey78"},
    {201 ,201 ,201,        "gray79"},
    {201 ,201 ,201,        "grey79"},
    {204 ,204 ,204,        "gray80"},
    {204 ,204 ,204,        "grey80"},
    {207 ,207 ,207,        "gray81"},
    {207 ,207 ,207,        "grey81"},
    {209 ,209 ,209,        "gray82"},
    {209 ,209 ,209,        "grey82"},
    {212 ,212 ,212,        "gray83"},
    {212 ,212 ,212,        "grey83"},
    {214 ,214 ,214,        "gray84"},
    {214 ,214 ,214,        "grey84"},
    {217 ,217 ,217,        "gray85"},
    {217 ,217 ,217,        "grey85"},
    {219 ,219 ,219,        "gray86"},
    {219 ,219 ,219,        "grey86"},
    {222 ,222 ,222,        "gray87"},
    {222 ,222 ,222,        "grey87"},
    {224 ,224 ,224,        "gray88"},
    {224 ,224 ,224,        "grey88"},
    {227 ,227 ,227,        "gray89"},
    {227 ,227 ,227,        "grey89"},
    {229 ,229 ,229,        "gray90"},
    {229 ,229 ,229,        "grey90"},
    {232 ,232 ,232,        "gray91"},
    {232 ,232 ,232,        "grey91"},
    {235 ,235 ,235,        "gray92"},
    {235 ,235 ,235,        "grey92"},
    {237 ,237 ,237,        "gray93"},
    {237 ,237 ,237,        "grey93"},
    {240 ,240 ,240,        "gray94"},
    {240 ,240 ,240,        "grey94"},
    {242 ,242 ,242,        "gray95"},
    {242 ,242 ,242,        "grey95"},
    {245 ,245 ,245,        "gray96"},
    {245 ,245 ,245,        "grey96"},
    {247 ,247 ,247,        "gray97"},
    {247 ,247 ,247,        "grey97"},
    {250 ,250 ,250,        "gray98"},
    {250 ,250 ,250,        "grey98"},
    {252 ,252 ,252,        "gray99"},
    {252 ,252 ,252,        "grey99"},
    {255 ,255 ,255,        "gray100"},
    {255 ,255 ,255,        "grey100"},
    {169 ,169 ,169,        "dark grey"},
    {169 ,169 ,169,        "DarkGrey"},
    {169 ,169 ,169,        "dark gray"},
    {169 ,169 ,169,        "DarkGray"},
    {0   ,  0 ,139,        "dark blue"},
    {0   ,  0 ,139,        "DarkBlue"},
    {0   ,139 ,139,        "dark cyan"},
    {0   ,139 ,139,        "DarkCyan"},
    {139 ,  0 ,139,        "dark magenta"},
    {139 ,  0 ,139,        "DarkMagenta"},
    {139 ,  0 ,  0,        "dark red"},
    {139 ,  0 ,  0,        "DarkRed"},
    {144 ,238 ,144,        "light green"},
    {144 ,238 ,144,        "LightGreen"},
//
// Silicon Graphics special colors:
// These colors exist for backward compatibility with previous releases; we do
// not recommend you use these colors because they are unlikely to exist on
// non-SGI X servers.
//
    {  0 ,  0 ,  0,        "sgi gray 0"},
    {  0 ,  0 ,  0,        "SGIGray0"},
    {  0 ,  0 ,  0,        "sgi grey 0"},
    {  0 ,  0 ,  0,        "SGIGrey0"},
    { 10 , 10 , 10,        "sgi gray 4"},
    { 10 , 10 , 10,        "SGIGray4"},
    { 10 , 10 , 10,        "sgi grey 4"},
    { 10 , 10 , 10,        "SGIGrey4"},
    { 20 , 20 , 20,        "sgi gray 8"},
    { 20 , 20 , 20,        "SGIGray8"},
    { 20 , 20 , 20,        "sgi grey 8"},
    { 20 , 20 , 20,        "SGIGrey8"},
    { 30 , 30 , 30,        "sgi gray 12"},
    { 30 , 30 , 30,        "SGIGray12"},
    { 30 , 30 , 30,        "sgi grey 12"},
    { 30 , 30 , 30,        "SGIGrey12"},
    { 40 , 40 , 40,        "sgi gray 16"},
    { 40 , 40 , 40,        "SGIGray16"},
    { 40 , 40 , 40,        "sgi grey 16"},
    { 40 , 40 , 40,        "SGIGrey16"},
    { 51 , 51 , 51,        "sgi gray 20"},
    { 51 , 51 , 51,        "SGIGray20"},
    { 51 , 51 , 51,        "sgi grey 20"},
    { 51 , 51 , 51,        "SGIGrey20"},
    { 61 , 61 , 61,        "sgi gray 24"},
    { 61 , 61 , 61,        "SGIGray24"},
    { 61 , 61 , 61,        "sgi grey 24"},
    { 61 , 61 , 61,        "SGIGrey24"},
    { 71 , 71 , 71,        "sgi gray 28"},
    { 71 , 71 , 71,        "SGIGray28"},
    { 71 , 71 , 71,        "sgi grey 28"},
    { 71 , 71 , 71,        "SGIGrey28"},
    { 81 , 81 , 81,        "sgi gray 32"},
    { 81 , 81 , 81,        "SGIGray32"},
    { 81 , 81 , 81,        "sgi grey 32"},
    { 81 , 81 , 81,        "SGIGrey32"},
    { 91 , 91 , 91,        "sgi gray 36"},
    { 91 , 91 , 91,        "SGIGray36"},
    { 91 , 91 , 91,        "sgi grey 36"},
    { 91 , 91 , 91,        "SGIGrey36"},
    {102 ,102 ,102,        "sgi gray 40"},
    {102 ,102 ,102,        "SGIGray40"},
    {102 ,102 ,102,        "sgi grey 40"},
    {102 ,102 ,102,        "SGIGrey40"},
    {112 ,112 ,112,        "sgi gray 44"},
    {112 ,112 ,112,        "SGIGray44"},
    {112 ,112 ,112,        "sgi grey 44"},
    {112 ,112 ,112,        "SGIGrey44"},
    {122 ,122 ,122,        "sgi gray 48"},
    {122 ,122 ,122,        "SGIGray48"},
    {122 ,122 ,122,        "sgi grey 48"},
    {122 ,122 ,122,        "SGIGrey48"},
    {132 ,132 ,132,        "sgi gray 52"},
    {132 ,132 ,132,        "SGIGray52"},
    {132 ,132 ,132,        "sgi grey 52"},
    {132 ,132 ,132,        "SGIGrey52"},
    {142 ,142 ,142,        "sgi gray 56"},
    {142 ,142 ,142,        "SGIGray56"},
    {142 ,142 ,142,        "sgi grey 56"},
    {142 ,142 ,142,        "SGIGrey56"},
    {153 ,153 ,153,        "sgi gray 60"},
    {153 ,153 ,153,        "SGIGray60"},
    {153 ,153 ,153,        "sgi grey 60"},
    {153 ,153 ,153,        "SGIGrey60"},
    {163 ,163 ,163,        "sgi gray 64"},
    {163 ,163 ,163,        "SGIGray64"},
    {163 ,163 ,163,        "sgi grey 64"},
    {163 ,163 ,163,        "SGIGrey64"},
    {173 ,173 ,173,        "sgi gray 68"},
    {173 ,173 ,173,        "SGIGray68"},
    {173 ,173 ,173,        "sgi grey 68"},
    {173 ,173 ,173,        "SGIGrey68"},
    {183 ,183 ,183,        "sgi gray 72"},
    {183 ,183 ,183,        "SGIGray72"},
    {183 ,183 ,183,        "sgi grey 72"},
    {183 ,183 ,183,        "SGIGrey72"},
    {193 ,193 ,193,        "sgi gray 76"},
    {193 ,193 ,193,        "SGIGray76"},
    {193 ,193 ,193,        "sgi grey 76"},
    {193 ,193 ,193,        "SGIGrey76"},
    {204 ,204 ,204,        "sgi gray 80"},
    {204 ,204 ,204,        "SGIGray80"},
    {204 ,204 ,204,        "sgi grey 80"},
    {204 ,204 ,204,        "SGIGrey80"},
    {214 ,214 ,214,        "sgi gray 84"},
    {214 ,214 ,214,        "SGIGray84"},
    {214 ,214 ,214,        "sgi grey 84"},
    {214 ,214 ,214,        "SGIGrey84"},
    {224 ,224 ,224,        "sgi gray 88"},
    {224 ,224 ,224,        "SGIGray88"},
    {224 ,224 ,224,        "sgi grey 88"},
    {224 ,224 ,224,        "SGIGrey88"},
    {234 ,234 ,234,        "sgi gray 92"},
    {234 ,234 ,234,        "SGIGray92"},
    {234 ,234 ,234,        "sgi grey 92"},
    {234 ,234 ,234,        "SGIGrey92"},
    {244 ,244 ,244,        "sgi gray 96"},
    {244 ,244 ,244,        "SGIGray96"},
    {244 ,244 ,244,        "sgi grey 96"},
    {244 ,244 ,244,        "SGIGrey96"},
    {255 ,255 ,255,        "sgi gray 100"},
    {255 ,255 ,255,        "SGIGray100"},
    {255 ,255 ,255,        "sgi grey 100"},
    {255 ,255 ,255,        "SGIGrey100"},
    {125 ,158 ,192,        "sgi light blue"},
    {125 ,158 ,192,        "SGILightBlue"},
    { 85 , 85 , 85,        "sgi dark gray"},
    { 85 , 85 , 85,        "SGIDarkGray"},
    { 85 , 85 , 85,        "sgi dark grey"},
    { 85 , 85 , 85,        "SGIDarkGrey"},
    {198 ,113 ,113,        "sgi salmon"},
    {198 ,113 ,113,        "SGISalmon"},
    {113 ,198 ,113,        "sgi chartreuse"},
    {113 ,198 ,113,        "SGIChartreuse"},
    {142 ,142 , 56,        "sgi olive drab"},
    {142 ,142 , 56,        "SGIOliveDrab"},
    {113 ,113 ,198,        "sgi slate blue"},
    {113 ,113 ,198,        "SGISlateBlue"},
    {142 ,56 , 142,        "sgi beet"},
    {142 ,56 , 142,        "SGIBeet"},
    { 56 ,142 ,142,        "sgi teal"},
    { 56 ,142 ,142,        "SGITeal"},
    {170 ,170 ,170,        "sgi light gray"},
    {170 ,170 ,170,        "SGILightGray"},
    {170 ,170 ,170,        "sgi light grey"},
    {170 ,170 ,170,        "SGILightGrey"},
    {214 ,214 ,214,        "sgi very light gray"},
    {214 ,214 ,214,        "SGIVeryLightGray"},
    {214 ,214 ,214,        "sgi very light grey"},
    {214 ,214 ,214,        "SGIVeryLightGrey"},
    {132 ,132 ,132,        "sgi medium gray"},
    {132 ,132 ,132,        "SGIMediumGray"},
    {132 ,132 ,132,        "sgi medium grey"},
    {132 ,132 ,132,        "SGIMediumGrey"},
    { 40 , 40 , 40,        "sgi very dark gray"},
    { 40 , 40 , 40,        "SGIVeryDarkGray"},
    { 40 , 40 , 40,        "sgi very dark grey"},
    { 40 , 40 , 40,        "SGIVeryDarkGrey"},
    {197 ,193 ,170,        "sgi bright gray"},
    {197 ,193 ,170,        "SGIBrightGray"},
    {197 ,193 ,170,        "sgi bright grey"},
    {197 ,193 ,170,        "SGIBrightGrey"},
    { 75 ,  0 ,130,        "Indigo"},
    { 33 ,136 ,104,        "Indigo2"},
    {220 , 20 , 60,        "Crimson"}
};

static const int numNamedColors = sizeof(namedColors) /
                                  sizeof(named_color_t);

// Type map format string
const char *ColorAttribute::TypeMapFormatString = "U";

// ****************************************************************************
// Method: ColorAttribute::ColorAttribute
//
// Purpose: 
//   Constructor for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ColorAttribute::ColorAttribute() : 
    AttributeSubject(ColorAttribute::TypeMapFormatString)
{
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
    color[3] = 255;
}

// ****************************************************************************
// Method: ColorAttribute::ColorAttribute
//
// Purpose: 
//   Copy constructor for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ColorAttribute::ColorAttribute(const ColorAttribute &obj) : 
    AttributeSubject(ColorAttribute::TypeMapFormatString)
{
    int i;

    for(i = 0; i < 4; ++i)
        color[i] = obj.color[i];


    SelectAll();
}

// ****************************************************************************
// Method: ColorAttribute::~ColorAttribute
//
// Purpose: 
//   Destructor for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ColorAttribute::~ColorAttribute()
{
    // nothing here
}

// ****************************************************************************
// Method: ColorAttribute::operator = 
//
// Purpose: 
//   Assignment operator for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

ColorAttribute& 
ColorAttribute::operator = (const ColorAttribute &obj)
{
    if (this == &obj) return *this;
    int i;

    for(i = 0; i < 4; ++i)
        color[i] = obj.color[i];


    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: ColorAttribute::operator == 
//
// Purpose: 
//   Comparison operator == for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ColorAttribute::operator == (const ColorAttribute &obj) const
{
    int i;

    // Compare the color arrays.
    bool color_equal = true;
    for(i = 0; i < 4 && color_equal; ++i)
        color_equal = (color[i] == obj.color[i]);

    // Create the return value
    return (color_equal);
}

// ****************************************************************************
// Method: ColorAttribute::operator != 
//
// Purpose: 
//   Comparison operator != for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ColorAttribute::operator != (const ColorAttribute &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: ColorAttribute::TypeName
//
// Purpose: 
//   Type name method for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
ColorAttribute::TypeName() const
{
    return "ColorAttribute";
}

// ****************************************************************************
// Method: ColorAttribute::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ColorAttribute::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const ColorAttribute *tmp = (const ColorAttribute *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: ColorAttribute::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ColorAttribute::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new ColorAttribute(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: ColorAttribute::NewInstance
//
// Purpose: 
//   NewInstance method for the ColorAttribute class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
ColorAttribute::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new ColorAttribute(*this);
    else
        retval = new ColorAttribute;

    return retval;
}

// ****************************************************************************
// Method: ColorAttribute::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
ColorAttribute::SelectAll()
{
    Select(ID_color, (void *)color, 4);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ColorAttribute::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Wed Jul 23 11:29:38 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Oct 3 10:13:43 PDT 2003
//   I made it always save to the parentNode.
//
//   Brad Whitlock, Wed Dec 17 12:09:19 PDT 2003
//   I changed the method prototype.
//
// ****************************************************************************

bool
ColorAttribute::CreateNode(DataNode *parentNode, bool, bool)
{
    if(parentNode == 0)
        return false;

    // Create a node for ColorAttribute.
    DataNode *node = new DataNode("ColorAttribute");
    node->AddNode(new DataNode("color", color, 4));
    parentNode->AddNode(node);

    return true;
}
// ****************************************************************************
// Method: ColorAttribute::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
ColorAttribute::SetFromNode(DataNode *parentNode)
{
    int i;
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("ColorAttribute");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("color")) != 0)
        SetColor(node->AsUnsignedCharArray());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
ColorAttribute::SetColor(const unsigned char *color_)
{
    color[0] = color_[0];
    color[1] = color_[1];
    color[2] = color_[2];
    color[3] = color_[3];
    Select(ID_color, (void *)color, 4);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

const unsigned char *
ColorAttribute::GetColor() const
{
    return color;
}

unsigned char *
ColorAttribute::GetColor()
{
    return color;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
ColorAttribute::SelectColor()
{
    Select(ID_color, (void *)color, 4);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ColorAttribute::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
ColorAttribute::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_color: return "color";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ColorAttribute::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
ColorAttribute::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_color: return FieldType_ucharArray;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: ColorAttribute::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
ColorAttribute::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_color: return "ucharArray";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ColorAttribute::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:34 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ColorAttribute::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    int i;

    const ColorAttribute &obj = *((const ColorAttribute*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_color:
        {  // new scope
        // Compare the color arrays.
        bool color_equal = true;
        for(i = 0; i < 4 && color_equal; ++i)
            color_equal = (color[i] == obj.color[i]);

        retval = color_equal;
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ColorAttribute::ColorAttribute
//
// Purpose: 
//   Constructor for the ColorAttribute class.
//
// Note:       Autogenerated by classwriter.
//
// Programmer: classwriter
// Creation:   Fri Feb 23 13:08:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

ColorAttribute::ColorAttribute(int red, int green, int blue, int alpha) :
    AttributeSubject("U")
{
    color[0] = (unsigned char)(red & 255);
    color[1] = (unsigned char)(green & 255);
    color[2] = (unsigned char)(blue & 255);
    color[3] = (unsigned char)(alpha & 255);
}

void
ColorAttribute::SetRgb(int red, int green, int blue)
{
    color[0] = (unsigned char)red;
    color[1] = (unsigned char)green;
    color[2] = (unsigned char)blue;
}

void
ColorAttribute::SetRgba(int red, int green, int blue, int alpha)
{
    color[0] = (unsigned char)red;
    color[1] = (unsigned char)green;
    color[2] = (unsigned char)blue;
    color[3] = (unsigned char)alpha;
}

void
ColorAttribute::GetRgba(double c[4]) const
{
    c[0] = double(color[0]) / 255.;
    c[1] = double(color[1]) / 255.;
    c[2] = double(color[2]) / 255.;
    c[3] = double(color[3]) / 255.;
}

void
ColorAttribute::SetRed(int red)
{
    color[0] = (unsigned char)(red & 255);
}

void
ColorAttribute::SetGreen(int green)
{
    color[1] = (unsigned char)(green & 255);
}

void
ColorAttribute::SetBlue(int blue)
{
    color[2] = (unsigned char)(blue & 255);
}

void
ColorAttribute::SetAlpha(int alpha)
{
    color[3] = (unsigned char)(alpha & 255);
}

int
ColorAttribute::Red() const
{
    return (int)color[0];
}

int
ColorAttribute::Green() const
{
    return (int)color[1];
}

int
ColorAttribute::Blue() const
{
    return (int)color[2];
}

int
ColorAttribute::Alpha() const
{
    return (int)color[3];
}

void ColorAttribute::SetRgba2(double c[4])
{
    color[0] = (unsigned char)((float)c[0] * 255.f);
    color[1] = (unsigned char)((float)c[1] * 255.f);
    color[2] = (unsigned char)((float)c[2] * 255.f);
    color[3] = (unsigned char)((float)c[3] * 255.f);
}

// **************************************************************************
// Method: ColorAttribute::SetByName
//
// Purpose: Set Color by naming the color
//
// Return: True if able to recognize color name and set rgb values
//         succesfully. False otherwise.
//
// Programmer: Mark C. Miller
// Creation:   July 13, 2006
//
// Modifications:
//
//   Mark C. Miller, Tue Sep 18 17:16:16 PDT 2007
//   Made color name comparison case-INsensitive
// **************************************************************************

bool ColorAttribute::SetByName(const char *const colorName)
{
    if (!colorName)
        return false;

    //
    // check for "name" being specific rgb values using the
    // '#HHHHHH' notation where H is a hex digit.
    //
    if (colorName[0] == '#')
    {
        unsigned char rgb[3];
        if (RgbStrToRgb(&colorName[1], rgb))
        {
            SetRed(rgb[0]);
            SetGreen(rgb[1]);
            SetBlue(rgb[2]);
            return true;
        }
        return false;
    }

    //
    // Search for the named color
    //
    for (int i = 0; i < numNamedColors; i++)
    {
#if defined(_WIN32)
        if (!stricmp(colorName, namedColors[i].n))
#else
        if (!strcasecmp(colorName, namedColors[i].n))
#endif
        {
            SetRed(namedColors[i].r);
            SetGreen(namedColors[i].g);
            SetBlue(namedColors[i].b);
            return true;
        }
    }

    return false;
}

// **************************************************************************
// Method: ColorAttribute::GetName
//
// Purpose: Get name of color, if it has a name
//
// Programmer: Mark C. Miller,
// Creation:   July 13, 2006
// **************************************************************************

const char *ColorAttribute::GetByName() const
{
    for (int i = 0; i < numNamedColors; i++)
    {
        if (color[0] == namedColors[i].r &&
            color[1] == namedColors[i].g &&
            color[2] == namedColors[i].b)
            return namedColors[i].n;
    }
    return "unknown";
}

// **************************************************************************
// Function: RgbStrToRgb
//
// Purpose: Convert rgb strings of the form '#HHHHHH' where H is a hex digit
//          to actual rgb values.
//
// Return: True if able to succesfully convert. False otherwise.
//
// Programmer: Mark C. Miller
// Creation:   July 13, 2006
// **************************************************************************

bool
ColorAttribute::RgbStrToRgb(const char *const s, unsigned char rgb[3])
{
    if (!s || strlen((const char*)s) != 6)
        return false;

    for (int i = 0; i < 3; i++)
    {
        int b = 16;
        int c = 0;
        for (int j = 0; j < 2; j++)
        {
            int k = i*2 + j;

            if      ('0' <= s[k] && s[k] <= '9')
                c += (int) (s[k] - '0') * b;
            else if ('A' <= s[k] && s[k] <= 'F')
                c += ((int) (s[k] - 'A') + 10) * b;
            else if ('a' <= s[k] && s[k] <= 'f')
                c += ((int) (s[k] - 'a') + 10) * b;
            else
                return false;
            b = b >> 4;
        }
        rgb[i] = (unsigned char) c;
    }
    return true;
}

// **************************************************************************
// Function: RgbToRgbStr
//
// Purpose: Convert rgb values to a string of the form '#HHHHHH' where H is a
//          hex digit.
//
// Programmer: Mark C. Miller
// Creation:   July 13, 2006
// **************************************************************************

void 
ColorAttribute::RgbToRgbStr(unsigned char rgb[3], char *s)
{
    for (int i = 0; i < 3; i++)
    {
        int c = (int) ((unsigned int) rgb[i]);
        int hexDigits[2];

        hexDigits[0] = c / 16;
        hexDigits[1] = c % 16;

        for (int j = 0; j < 2; j++)
        {
            int k = i*2 + j;

            if (hexDigits[j] > 9)
                s[k] = 'A' + (hexDigits[j] - 10);
            else
                s[k] = '0' + hexDigits[j];
        }
    }
}

