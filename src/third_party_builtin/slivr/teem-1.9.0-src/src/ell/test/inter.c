/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../ell.h"

char *interInfo = ("Recreates memories of college physics");

typedef struct {
  FILE *file;
  double psc;
  double bbox[4];
  
  double maxX, maxY, yscale;
} wheelPS;

#define WPS_X(x) AIR_AFFINE(wps->bbox[0], (x), wps->bbox[2], 0, wps->maxX)
#define WPS_Y(y) AIR_AFFINE(wps->bbox[1], (y), wps->bbox[3], 0, wps->maxY)
#define WPS_S(s)  AIR_DELTA(wps->bbox[1], (s), wps->bbox[3], 0, wps->maxY)

void
wheelPreamble(wheelPS *wps) {

  wps->maxX = wps->psc*(wps->bbox[2] - wps->bbox[0]);
  wps->maxY = wps->psc*(wps->bbox[3] - wps->bbox[1]);

  fprintf(wps->file, "%%!PS-Adobe-2.0 EPSF-2.0\n");
  fprintf(wps->file, "%%%%Creator: limn\n");
  fprintf(wps->file, "%%%%Pages: 1\n");
  fprintf(wps->file, "%%%%BoundingBox: 0 0 %d %d\n", 
          (int)(wps->maxX),
          (int)(wps->maxY));
  fprintf(wps->file, "%%%%EndComments\n");
  fprintf(wps->file, "%%%%EndProlog\n");
  fprintf(wps->file, "%%%%Page: 1 1\n");
  fprintf(wps->file, "gsave\n");
  fprintf(wps->file, "0 0 moveto\n");
  fprintf(wps->file, "%g 0 lineto\n", wps->maxX);
  fprintf(wps->file, "%g %g lineto\n", wps->maxX, wps->maxY);
  fprintf(wps->file, "0 %g lineto\n", wps->maxY);
  fprintf(wps->file, "closepath\n");
  fprintf(wps->file, "clip\n");
  fprintf(wps->file, "gsave newpath\n");
  fprintf(wps->file, "1 setlinejoin\n");
  fprintf(wps->file, "1 setlinecap\n");
  fprintf(wps->file, "/M {moveto} bind def\n");
  fprintf(wps->file, "/L {lineto} bind def\n");
  fprintf(wps->file, "/W {setlinewidth} bind def\n");
  fprintf(wps->file, "/F {fill} bind def\n");
  fprintf(wps->file, "/S {stroke} bind def\n");
  fprintf(wps->file, "/CP {closepath} bind def\n");
  fprintf(wps->file, "/RGB {setrgbcolor} bind def\n");
  fprintf(wps->file, "/Gr {setgray} bind def\n");
  fprintf(wps->file, "\n");
  return;
}

void
wheelWidth(wheelPS *wps, double width) {

  fprintf(wps->file, "%g W\n", width);
  return;
}

void
wheelGray(wheelPS *wps, double gray) {

  fprintf(wps->file, "%g Gr\n", gray);
  return;
}

void
wheelArrow(wheelPS *wps, double x0, double y0, double x1, double y1,
           double alen, double awidth) {
  double len, dir[2], perp[2];
  
  dir[0] = x0 - x1;
  dir[1] = y0 - y1;
  ELL_2V_NORM(dir, dir, len);
  ELL_2V_SET(perp, -dir[1], dir[0]);
  fprintf(wps->file, "%g %g M\n", WPS_X(x0), WPS_Y(y0));
  fprintf(wps->file, "%g %g L S\n",
          WPS_X(x1 + alen*dir[0]/2), 
          WPS_Y(y1 + alen*dir[1]/2));
  if (alen && awidth) {
    if (len < alen) {
      awidth *= len/alen;
      alen = len;
    }
    fprintf(wps->file, "%g %g M\n", 
            WPS_X(x1 + alen*dir[0] + awidth*perp[0]), 
            WPS_Y(y1 + alen*dir[1] + awidth*perp[1]));
    fprintf(wps->file, "%g %g L\n", WPS_X(x1), WPS_Y(y1));
    fprintf(wps->file, "%g %g L CP F\n", 
            WPS_X(x1 + alen*dir[0] - awidth*perp[0]), 
            WPS_Y(y1 + alen*dir[1] - awidth*perp[1]));
  }
  return;
}

void
wheelEpilog(wheelPS *wps) {

  fprintf(wps->file, "grestore\n");
  fprintf(wps->file, "grestore\n");
  fprintf(wps->file, "%%%%Trailer\n");
  return;
}

int
main(int argc, char *argv[]) {
  char *me, *outS;
  hestOpt *hopt;
  hestParm *hparm;
  airArray *mop;
  
  int fidx, aidx, num, frames;
  double psc, width[2], arrowWidth, lineWidth, angle, seglen,
    x0, y0, x1, y1, cc, ss;
  wheelPS wps;
  char filename[AIR_STRLEN_MED];

  me = argv[0];
  mop = airMopNew();
  hparm = hestParmNew();
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
  hopt = NULL;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  hestOptAdd(&hopt, "w", "arrowWidth lineWidth", airTypeDouble, 2, 2, width, 
             "1.0 0.2", "widths");
  hestOptAdd(&hopt, "n", "number", airTypeInt, 1, 1, &num, "10",
             "number of arrows");
  hestOptAdd(&hopt, "f", "frames", airTypeInt, 1, 1, &frames, "10",
             "number of frames");
  hestOptAdd(&hopt, "psc", "scale", airTypeDouble, 1, 1, &psc, "200",
             "scaling from world space to PostScript points");
  hestOptAdd(&hopt, "o", "prefix", airTypeString, 1, 1, &outS, NULL,
             "prefix of file names");
  hestParseOrDie(hopt, argc-1, argv+1, hparm,
                 me, interInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  for (fidx=0; fidx<frames; fidx++) {
    sprintf(filename, "%s%03d.eps", outS, fidx);
    if (!(wps.file = airFopen(filename, stdout, "wb"))) {
      fprintf(stderr, "%s: couldn't open output file\n", me);
      airMopError(mop); return 1;
    }
    
    lineWidth = width[0];
    arrowWidth = width[1];
    wps.psc = psc;
    ELL_4V_SET(wps.bbox, -0.45, -0.85, 1.1, 0.85);
    wheelPreamble(&wps);

    fprintf(wps.file, "0 setlinecap\n");
    
    wheelGray(&wps, 0.4);
    wheelWidth(&wps, lineWidth);
    
    x0 = 0;
    y0 = 0;
    seglen = 1.0/num;
    angle = AIR_AFFINE(0, fidx, frames, 0, 2*AIR_PI);
    for (aidx=1; aidx<=num; aidx++) {
      cc = cos(angle*aidx)*seglen;
      ss = sin(angle*aidx)*seglen;
      x1 = x0 + 0.90*cc;
      y1 = y0 + 0.90*ss;
      wheelArrow(&wps, x0, y0, x1, y1, arrowWidth, arrowWidth*0.4);
      x0 += cc;
      y0 += ss;
    }

    wheelGray(&wps, 0.0);
    wheelArrow(&wps, 0, 0, x0, y0, arrowWidth, arrowWidth*0.4);
    
    wheelEpilog(&wps);
    airFclose(wps.file);
  }

  airMopOkay(mop);
  exit(0);
}


/*
mkdir inter04
test/inter -w 4 0.08 -o inter04/ -n 4 -f 300

mkdir inter08
test/inter -w 2.5 0.04 -o inter08/ -n 8 -f 300

mkdir inter16
test/inter -w 2.2 0.02 -o inter16/ -n 16 -f 300

mkdir inter32
test/inter -w 2 0.01 -o inter32/ -n 32 -f 300

mkdir inter

foreach I ( 000 001 002 003 004 005 006 007 008 009 \
            010 011 012 013 014 015 016 017 018 019 \
            020 021 022 023 024 025 026 027 028 029 \
            030 031 032 033 034 035 036 037 038 039 \
            040 041 042 043 044 045 046 047 048 049 \
            050 051 052 053 054 055 056 057 058 059 \
            060 061 062 063 064 065 066 067 068 069 \
            070 071 072 073 074 075 076 077 078 079 \
            080 081 082 083 084 085 086 087 088 089 \
            090 091 092 093 094 095 096 097 098 099 \
            100 101 102 103 104 105 106 107 108 109 \
            110 111 112 113 114 115 116 117 118 119 \
            120 121 122 123 124 125 126 127 128 129 \
            130 131 132 133 134 135 136 137 138 139 \
            140 141 142 143 144 145 146 147 148 149 \
            150 151 152 153 154 155 156 157 158 159 \
            160 161 162 163 164 165 166 167 168 169 \
            170 171 172 173 174 175 176 177 178 179 \
            180 181 182 183 184 185 186 187 188 189 \
            190 191 192 193 194 195 196 197 198 199 \
            200 201 202 203 204 205 206 207 208 209 \
            210 211 212 213 214 215 216 217 218 219 \
            220 221 222 223 224 225 226 227 228 229 \
            230 231 232 233 234 235 236 237 238 239 \
            240 241 242 243 244 245 246 247 248 249 \
            250 251 252 253 254 255 256 257 258 259 \
            260 261 262 263 264 265 266 267 268 269 \
            270 271 272 273 274 275 276 277 278 279 \
            280 281 282 283 284 285 286 287 288 289 \
            290 291 292 293 294 295 296 297 298 299 )
  echo $I
  foreach J ( 04 08 16 32 )
    eps2ppm inter{$J}/${I}.eps 250 \
     | unu slice -a 0 -p 0 \
     | unu resample -s x0.2 x0.2 -o inter${J}/${I}.png
  end
  unu pad -i inter04/${I}.png -min 0 0 -max M+1 M -b pad -v 128 \
   | unu join -i - inter08/${I}.png -a 0 \
   | unu pad -min 0 0 -max M M+1 -b pad -v 128 -o a.png
  unu pad -i inter16/${I}.png -min 0 0 -max M+1 M -b pad -v 128 \
   | unu join -i - inter32/${I}.png -a 0 -o b.png
  unu join -i a.png b.png -a 1 \
   | unu 2op - 255 - -o inter/${I}.png
end

rm -f a.png b.png

*/
