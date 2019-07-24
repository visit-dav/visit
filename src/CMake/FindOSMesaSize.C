// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <GL/osmesa.h>

#ifndef WIN32
  #include <fstream>
  using namespace std;
#endif

int main(int argc, char **argv)
{
    int w = 0, h = 0;
    OSMesaGetIntegerv(OSMESA_MAX_WIDTH, &w);
    OSMesaGetIntegerv(OSMESA_MAX_HEIGHT, &h);
#ifndef WIN32
    ofstream out("junk.txt");
    out << ((w < h) ? w : h) << endl;
    out.close();
#endif
    return ((w < h) ? w : h);
}

