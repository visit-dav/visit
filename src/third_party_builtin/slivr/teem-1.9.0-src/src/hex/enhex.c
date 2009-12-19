/*
  Copyright (C) 2004, 2003, 2002 University of Utah

  This software,  is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/*
** enhex: simple stand-alone hex encoder
**
** Compile with:
**    cc -o enhex enhex.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int
enhexColumns = 70;  /* number of characters per line */

void
enhexUsage(char *me) {
  /*                       0   1     2   (2/3) */
  fprintf(stderr, "usage: %s <in> [<out>]\n", me);
  fprintf(stderr, " <in>: file to read raw data from\n");
  fprintf(stderr, "<out>: file to write hex data to; "
          "uses stdout by default\n");
  fprintf(stderr, " \"-\" can be used to refer to stdin/stdout\n");
  exit(1);
}

void
enhexFclose(FILE *file) {
  
  if (!( stdin == file || stdout == file )) {
    fclose(file);
  }
}

int
enhexTable[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};


int
main(int argc, char *argv[]) {
  char *me, *inS, *outS;
  FILE *fin, *fout;
  int car=0, col;

  me = argv[0];
  if (!( 2 == argc || 3 == argc ))
    enhexUsage(me);

  inS = argv[1];
  if (!strcmp("-", inS)) {
    fin = stdin;
#ifdef _WIN32
    _setmode(_fileno(fin), _O_BINARY);
#endif
  } else {
    fin = fopen(inS, "rb");
    if (!fin) {
      fprintf(stderr, "\n%s: couldn't fopen(\"%s\",\"rb\"): %s\n\n",
              me, inS, strerror(errno));
      enhexUsage(me);
    }
  }
  if (2 == argc) {
    fout = stdout;
  } else {
    outS = argv[2];
    if (!strcmp("-", outS)) {
      fout = stdout;
    } else {
      fout = fopen(outS, "w");
      if (!fout) {
        fprintf(stderr, "\n%s: couldn't fopen(\"%s\",\"w\"): %s\n\n",
                me, outS, strerror(errno));
        enhexUsage(me);
      }
    }
  }

  col = 0;
  car = fgetc(fin);
  while (EOF != car) {
    if (col > enhexColumns) {
      fprintf(fout, "\n");
      col = 0;
    }
    fprintf(fout, "%c%c", enhexTable[car>>4], enhexTable[car&15]);
    col += 2;
    car = fgetc(fin);
  }
  if (2 != col) {
    fprintf(fout, "\n");
  }

  enhexFclose(fin);
  enhexFclose(fout);
  exit(0);
}
