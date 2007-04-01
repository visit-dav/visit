#include <visitstream.h>

// ****************************************************************************
//  File:  gentetcases.cpp
//
//  Purpose:
//    Generates the 48 transforms of a hex.
//    Points are 0-8, edges are A-L.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2003
//
// ****************************************************************************


static int tetVerticesFromEdges[6][2] =
{
    { 0, 1 },   /* Edge 0 */
    { 1, 2 },   /* Edge 1 */
    { 2, 0 },   /* Edge 2 */
    { 0, 3 },   /* Edge 3 */
    { 1, 3 },   /* Edge 4 */
    { 2, 3 }    /* Edge 5 */
};

void
GenCase(int n1, int n2, int n3, int n4, bool flip)
{
    cerr << "{{"
         << n1 <<","
         << n2 <<","
         << n3 <<","
         << n4 << "},{";

    int allnodes[4] = {n1, n2, n3, n4};
    for (int i=0; i<6; i++)
    {
        int e0 = tetVerticesFromEdges[i][0];
        int e1 = tetVerticesFromEdges[i][1];

        int n0 = allnodes[e0];
        int n1 = allnodes[e1];

        for (int j=0; j<6; j++)
        {
            int j0 = tetVerticesFromEdges[j][0];
            int j1 = tetVerticesFromEdges[j][1];

            if (n0==j0 && n1==j1)
            {
                cerr << "'" << char(int('a')+j) << "'";
            }
            else if (n0==j1 && n1==j0)
            {
                cerr << "'" << char(int('a')+j) << "'";
            }

        }
        if (i<5) cerr << ",";
    }
    cerr << "},";

    cerr << (flip ? "true " : "false");
    cerr << " }," << endl;
}

int main()
{
    int counter = 0;
    for (int n1=0; n1<4; n1++)
    {
        for (int n2=0; n2<4; n2++)
        {
            if (n2==n1)
                continue;
            for (int n3=0; n3<4; n3++)
            {
                if (n3==n1 || n3==n2)
                    continue;
                for (int n4=0; n4<4; n4++)
                {
                    if (n4==n1 || n4==n2 || n4==n3)
                        continue;

                    bool flip = (counter%4)==1 || (counter%4)==2;

                    GenCase(n1,n2,n3,n4,flip);
                    counter++;
                }
            }
        }
    }
}
