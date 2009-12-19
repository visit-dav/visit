/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
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

#include <errno.h>

#include <teem/air.h>
#include <teem/biff.h>
#include <teem/hest.h>
#include <teem/nrrd.h>

#define TKWB "talkweb"

#define TKWB_TAG_TOC 0
#define TKWB_TAG_TITLE 1
#define TKWB_TAG_IMAGE 2
#define TKWB_TAG_FIRST 3
#define TKWB_TAG_PREV 4
#define TKWB_TAG_NEXT 5
#define TKWB_TAG_LAST 6
#define TKWB_TAG_TEXT 7

#define TKWB_TAG_MAX 7

char *tkwbInfo = 
("Generates HTML pages from slide images and text. "
 "This program takes multiple inputs: a template for the table of contents "
 "that will become \"index.html\" (\"-i\"), a template for the pages "
 "generated for each slide e.g. \"slide000.html\" (\"-t\"), and a script "
 "text file that contains all the information that will go into the slide "
 "pages.  The format of this file is:\n "
 "\t\t- Seperator line indicating slide transitions, e.g. \"-------------\"\n "
 "\t\t- Title of first slide (one line)\n "
 "\t\t- Filename for image to put on first slide (one line)\n "
 "\t\t- Body of HTML text to put with the slide image (multiple lines)\n "
 "\t\t- Seperator\n "
 "followed by information for the second slide, and so forth.  Textual "
 "subtitutions are performed in the template files, according to the "
 "replacement tags (\"-r\", see below).  Within the slide pages, navigation "
 "arrows are based on remaining command-line options."
);

int tkwbArrayIncr = 16;

typedef struct {
  char *title, *image, *text;
} tkwbSlide;

tkwbSlide *
tkwbSlideNew(char *title, char *image, char *text) {
  tkwbSlide *ret;

  ret = (tkwbSlide*)calloc(1, sizeof(tkwbSlide));
  if (ret) {
    ret->title = airStrdup(title);
    ret->image = airStrdup(image);
    ret->text = airStrdup(text);
  }
  return ret;
}

tkwbSlide *
tkwbSlideNix(tkwbSlide *slide) {

  slide->title = (char *)airFree(slide->title);
  slide->image = (char *)airFree(slide->image);
  slide->text = (char *)airFree(slide->text);
  slide = (tkwbSlide *)airFree(slide);
  return NULL;
}

typedef union {
  tkwbSlide ***ps;
  char ***pc;
  void **v;
} _tkwbU;

int
tkwbReadFileToString(char **strP, int *hitEOF, FILE *file, char *stop) {
  char **all, line[AIR_STRLEN_HUGE];
  airArray *allArr;
  unsigned int allLen;
  unsigned int lineLen, lineIdx, totalLen;
  _tkwbU uu;
  
  uu.pc = &all;
  allArr = airArrayNew(uu.v, &allLen, sizeof(char*), tkwbArrayIncr);
  airArrayPointerCB(allArr, airNull, airFree);
  lineLen = airOneLine(file, line, AIR_STRLEN_HUGE);
  totalLen = 0;
  while (lineLen && (!( airStrlen(stop) && !strcmp(line, stop) )) ) {
    lineIdx = airArrayLenIncr(allArr, 1); /* HEY error checking */
    all[lineIdx] = (char *)calloc(strlen(line) + strlen("\n") + 1,
                                  sizeof(char));
    sprintf(all[lineIdx], "%s\n", line);
    totalLen += strlen(line) + 1;
    lineLen = airOneLine(file, line, AIR_STRLEN_HUGE);
  }
  if (hitEOF) {
    *hitEOF = !lineLen;
  }
  
  *strP = (char*)calloc(totalLen+1, sizeof(char));
  strcpy(*strP, "");
  for (lineIdx=0; lineIdx<allLen; lineIdx++) {
    strcat(*strP, all[lineIdx]);
  }
  
  airArrayNuke(allArr);
  return 0;
}

int
tkwbReadTemplate(char **tmplSP, char *filename) {
  char me[]="tkwbReadTemplate", err[BIFF_STRLEN];
  FILE *file;
  airArray *mop;
  
  mop = airMopNew();
  if (!( file = airFopen(filename, stdin, "rb") )) {
    sprintf(err, "%s: couldn't open %s: %s", me, filename, strerror(errno));
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);

  if (tkwbReadFileToString(tmplSP, NULL, file, NULL)) {
    sprintf(err, "%s: couldn't read in template file %s", me, filename);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}

int
tkwbReadSlides(tkwbSlide ***slideP, char *filename, airArray *pmop) {
  char me[]="tkwbReadSlides", err[BIFF_STRLEN];
  FILE *file;
  airArray *mop, *slideArr;
  tkwbSlide **slide = NULL;
  char *title, *image, *text, stop[AIR_STRLEN_HUGE], line[AIR_STRLEN_HUGE];
  int slideIdx=0, hitEOF, notReally;
  unsigned int len;
  _tkwbU uu;
  
  mop = airMopNew();
  if (!( file = airFopen(filename, stdin, "rb") )) {
    sprintf(err, "%s: couldn't open %s: %s", me, filename, strerror(errno));
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);

  len = airOneLine(file, stop, AIR_STRLEN_HUGE);
  if (!( len > 1 )) {
    sprintf(err, "%s: didn't get a stop delimiter from %s", me, filename);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }

  uu.ps = &slide;
  slideArr = airArrayNew(uu.v, NULL,
                         sizeof(tkwbSlide*), tkwbArrayIncr);
  airMopAdd(mop, slideArr, (airMopper)airArrayNix, airMopAlways);
  hitEOF = notReally = AIR_FALSE;
  while (!hitEOF) {
    slideIdx = airArrayLenIncr(slideArr, 1); /* HEY error checking */
    len = airOneLine(file, line, AIR_STRLEN_HUGE);
    if (!len) {
      /* got EOF after a division marker, that's okay */
      notReally = AIR_TRUE;
      break;
    }
    title = airStrdup(line);
    len = airOneLine(file, line, AIR_STRLEN_HUGE);
    if (!len) {
      break;
    }
    image = airStrdup(line);
    if (tkwbReadFileToString(&text, &hitEOF, file, stop)) {
      sprintf(err, "%s: couldn't read in slide %d", me, slideIdx);
      biffAdd(TKWB, err); airMopError(mop); return 1;
    }
    slide[slideIdx] = tkwbSlideNew(title, image, text);
    airMopAdd(pmop, slide[slideIdx], (airMopper)tkwbSlideNix, airMopAlways);
  }
  if (!hitEOF && !notReally) {
    sprintf(err, "%s: got incomplete slide info for slide %d\n", me, slideIdx);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  if (!notReally) {
    slideIdx = airArrayLenIncr(slideArr, 1); /* HEY error checking */
  }
  slide[slideIdx] = NULL;
  
  *slideP = slide;
  airMopOkay(mop);
  return 0;
}

int
tkwbExpandImageInfo(tkwbSlide **slide) {
  char me[]="tkwbExpandImageInfo", err[BIFF_STRLEN], *image;
  Nrrd *nimg;
  int si, sx, sy, len;
  airArray *mop;

  mop = airMopNew();
  nimg = nrrdNew();
  airMopAdd(mop, nimg, (airMopper)nrrdNuke, airMopAlways);
  for (si=0; slide[si]; si++) {
    if (nrrdLoad(nimg, slide[si]->image, NULL)) {
      sprintf(err, "%s: trouble reading slide image \"%s\"",
              me, slide[si]->image);
      biffMove(TKWB, err, NRRD); airMopError(mop); return 1;
    }
    if (!nrrdFormatPNG->fitsInto(nimg, nrrdEncodingGzip, AIR_TRUE)) {
      sprintf(err, "%s: slide image \"%s\" doesn't seem to be an image",
              me, slide[si]->image);
      biffMove(TKWB, err, NRRD); airMopError(mop); return 1;
    }
    sx = nimg->axis[nimg->dim-2].size;
    sy = nimg->axis[nimg->dim-1].size;
    len = (strlen("<img width=xxxx height=xxxx src=\"\">") 
           + strlen(slide[si]->image) + 1);
    image = (char *)calloc(len, sizeof(char));
    sprintf(image, "<img width=%d height=%d src=\"%s\">",
            sx, sy, slide[si]->image);
    free(slide[si]->image);
    slide[si]->image = image;
  }

  airMopOkay(mop);
  return 0;
}

int
tkwbWriteStringToFile(char *filename, char *content) {
  char me[]="tkwbWriteStringToFile", err[BIFF_STRLEN];
  FILE *file;

  if (!(file = fopen(filename, "wb"))) {
    sprintf(err, "%s: trouble opening file \"%s\": %s", 
            me, filename, strerror(errno));
    biffAdd(TKWB, err); return 1;
  }
  fprintf(file, "%s", content);
  fclose(file);

  return 0;
}

int
_tkwbStringSubst(char **sP,  /* string to search in */
                 char *f,    /* find */
                 char *r) {  /* replace */
  char *p,                   /* place where find was found */
    *n;                      /* new string */

  p = strstr(*sP, f);
  if (!p) {
    /* nothing to do */
    return 0;
  }
  n = (char*)calloc(strlen(*sP) - strlen(f) + strlen(r) + 1, sizeof(char));
  strncpy(n, *sP, p - *sP);
  strncpy(n + (p - *sP), r, strlen(r));
  strcpy(n + (p - *sP) + strlen(r), p + strlen(f));
  free(*sP);
  *sP = n;
  return _tkwbStringSubst(sP, f, r);
}

/*
** NOTE: this will re-allocate *stringP if a substitution is done
*/
void
tkwbStringSubst(char **sP,  /* string to search in */
                char *f,    /* find */
                char *r) {  /* replace */

  _tkwbStringSubst(sP, f, r);
  return;
}

int
tkwbWriteIndex(char *_index, tkwbSlide **slide, char *tag[TKWB_TAG_MAX+1]) {
  char me[]="tkwbWriteIndex", err[BIFF_STRLEN],
    *repl, *index, tmp[AIR_STRLEN_MED];
  int replLen, si;
  airArray *mop;
  
  mop = airMopNew();
  replLen = 0;
  replLen += strlen("<ol>\n");
  for (si=0; slide[si]; si++) {
    replLen += (strlen("<li> <a href=\"slideXXX.html\"></a>\n") 
                + strlen(slide[si]->title));
  }
  replLen += strlen("</ol>\n");
  
  if (!(repl = (char*)calloc(replLen+1, sizeof(char)))) {
    sprintf(err, "%s: couldn't allocate link buffer!", me);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, repl, airFree, airMopAlways);

  strcpy(repl, "<ol>\n");
  for (si=0; slide[si]; si++) {
    sprintf(tmp, "<li> <a href=\"slide%03d.html\">%s</a>\n", 
            si+1, slide[si]->title);
    strcat(repl, tmp);
  }
  strcat(repl, "</ol>");

  index = airStrdup(_index);
  tkwbStringSubst(&index, tag[TKWB_TAG_TOC], repl);
  airMopAdd(mop, index, airFree, airMopAlways);
  if (tkwbWriteStringToFile("index.html", index)) {
    sprintf(err, "%s: couldn't write \"index.html\"", me);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}

int
tkwbWriteSlides(tkwbSlide **slide, int numSlides, char *tmpl, 
                char *tag[TKWB_TAG_MAX+1], char *link[4]) {
  char me[]="tkwbWriteSlides", err[BIFF_STRLEN];
  char *text, name[AIR_STRLEN_MED], frst[AIR_STRLEN_MED], prev[AIR_STRLEN_MED],
    next[AIR_STRLEN_MED], last[AIR_STRLEN_MED];
  int si;
  airArray *mop;

  mop = airMopNew();
  sprintf(frst, "<a href=\"slide001.html\">%s</a>", link[0]);
  sprintf(last, "<a href=\"slide%03d.html\">%s</a>", numSlides, link[3]);
  for (si=0; si<numSlides; si++) {
    text = airStrdup(tmpl);
    tkwbStringSubst(&text, tag[TKWB_TAG_TITLE], slide[si]->title);
    tkwbStringSubst(&text, tag[TKWB_TAG_IMAGE], slide[si]->image);
    tkwbStringSubst(&text, tag[TKWB_TAG_TEXT], slide[si]->text);
    if (si) {
      tkwbStringSubst(&text, tag[TKWB_TAG_FIRST], frst);
      sprintf(prev, "<a href=\"slide%03d.html\">%s</a>", si, link[1]);
      tkwbStringSubst(&text, tag[TKWB_TAG_PREV], prev);
    }
    if (si < numSlides-1) {
      tkwbStringSubst(&text, tag[TKWB_TAG_LAST], last);
      sprintf(next, "<a href=\"slide%03d.html\">%s</a>", si+2, link[2]);
      tkwbStringSubst(&text, tag[TKWB_TAG_NEXT], next);
    }
    airMopAdd(mop, text, airFree, airMopAlways);
    sprintf(name, "slide%03d.html", si+1);
    if (tkwbWriteStringToFile(name, text)) {
      sprintf(err, "%s: couldn't write \"%s\"", me, name);
      biffAdd(TKWB, err); airMopError(mop); return 1;
    }
  }
  
  airMopOkay(mop);
  return 0;
}

int
tkwbDoit(char *indexS, char *tmplS, char *scriptS,
         char *tag[TKWB_TAG_MAX+1], char *link[4]) {
  char me[]="tkwbDoit", err[BIFF_STRLEN];
  char *index, *tmpl;
  tkwbSlide **slide;
  airArray *mop;
  int numSlides;
  
  mop = airMopNew();
  if (tkwbReadTemplate(&index, indexS)) {
    sprintf(err, "%s: trouble reading in index template file", me);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, index, airFree, airMopAlways);

  if (tkwbReadTemplate(&tmpl, tmplS)) {
    sprintf(err, "%s: trouble reading in slide template file", me);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, tmpl, airFree, airMopAlways);

  if (tkwbReadSlides(&slide, scriptS, mop)) {
    sprintf(err, "%s: trouble reading in slide script", me);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  airMopAdd(mop, slide, airFree, airMopAlways);
  
  if (tkwbExpandImageInfo(slide)) {
    sprintf(err, "%s: trouble learning details of images", me);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }

  if (tkwbWriteIndex(index, slide, tag)) {
    sprintf(err, "%s: trouble writing index.html", me);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  
  for (numSlides=0; slide[numSlides]; numSlides++);
  if (tkwbWriteSlides(slide, numSlides, tmpl, tag, link)) {
    sprintf(err, "%s: trouble writing slide pages", me);
    biffAdd(TKWB, err); airMopError(mop); return 1;
  }
  
  airMopOkay(mop);
  return 0;
}

int
main(int argc, char *argv[]) {
  char *me, *err, *indexS, *tmplS, *scriptS, *pretag[TKWB_TAG_MAX+1],
    *tag[AIR_STRLEN_MED],
    *frstLink, *prevLink, *nextLink, *lastLink, *link[4];
  hestOpt *hopt = NULL;
  airArray *mop;
  int ti;

  me = argv[0];
  hestOptAdd(&hopt, "i", "index", airTypeString, 1, 1, &indexS, NULL,
             "*index* template HTML filename.  This will be turned into "
             "the \"index.html\" index file, after the links to all the "
             "slides have been substituted in.");
  hestOptAdd(&hopt, "t", "slide", airTypeString, 1, 1, &tmplS, NULL,
             "*slide* template HTML filename.  "
             "The text of this includes the tags "
             "that are replaced with their per-slide values, to produce the "
             "HTML file for each slide's page. ");
  hestOptAdd(&hopt, "s", "script", airTypeString, 1, 1, &scriptS, NULL,
             "script filename.  This file contains information about each "
             "slide: the slide title, the slide image filename, and the "
             "HTML text to accompany the slide image.");
  hestOptAdd(&hopt, "r", "tags", airTypeString,
             TKWB_TAG_MAX+1, TKWB_TAG_MAX+1, pretag,
             "TOC TITLE IMAGE FIRST PREV NEXT LAST TEXT",
             "replacement tags that will be converted into links. "
             "The actual replcement tag is the string given here embedded "
             "in an HTML comment (no space).  So saying \"TOC\" means the "
             "actual replacement tag will be \"<!--TOC-->\". The first tag "
             "is replaced in the index template; all others are in the "
             "slide template. "
             "In order, the tags are for:\n "
             "\b\bo In the index template, the list of links to slide pages\n "
             "\b\bo The slide title\n "
             "\b\bo The slide image\n "
             "\b\bo The link to the first slide\n "
             "\b\bo The link to the previous slide\n "
             "\b\bo The link to the next slide\n "
             "\b\bo The link to the last slide\n "
             "\b\bo The text accompanying each slide");
  hestOptAdd(&hopt, "first", "text", airTypeString, 1, 1, &frstLink,
             "<b>|&lt;&lt;</b>", "Snippet of HTML text to be converted into "
             "link to first slide.  Some image could be used here. "
             "Following three arguments are similar. ");
  hestOptAdd(&hopt, "prev", "text", airTypeString, 1, 1, &prevLink,
             "<b>&lt;--</b>", "HTML for link to previous slide");
  hestOptAdd(&hopt, "next", "text", airTypeString, 1, 1, &nextLink,
             "<b>--&gt;</b>", "HTML for link to next slide");
  hestOptAdd(&hopt, "last", "text", airTypeString, 1, 1, &lastLink,
             "<b>&gt;&gt;|</b>", "HTML for link to last slide");
  hestParseOrDie(hopt, argc-1, argv+1, NULL, me, tkwbInfo,
                 AIR_TRUE, AIR_TRUE, AIR_TRUE);
  mop = airMopNew();
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  link[0] = frstLink;
  link[1] = prevLink;
  link[2] = nextLink;
  link[3] = lastLink;
  for (ti=0; ti<=TKWB_TAG_MAX; ti++) {
    tag[ti] = (char *)calloc(strlen(pretag[ti]) + strlen("<!---->") + 1,
                             sizeof(char));
    airMopAdd(mop, tag[ti], airFree, airMopAlways);
    sprintf(tag[ti], "<!--%s-->", pretag[ti]);
  }
  if (tkwbDoit(indexS, tmplS, scriptS, tag, link)) {
    airMopAdd(mop, err = biffGetDone(TKWB), airFree, airMopAlways);
    fprintf(stderr, "%s: error:\n%s", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
