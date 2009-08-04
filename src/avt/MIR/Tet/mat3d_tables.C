/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

/*---------------------------------------------------------------------------*
 * File        : mat3d_tables.C
 * Programmer  : Jeremy Meredith
 * Date        : December 12, 2000
 * 
 * Abstract    : Create the tables used by the probability-based
 *               material algorithm for structured and unstructured
 *               meshes, 3 different levels of subdivision
 *
 * Notes:  This has only been confirmed to work in VisIt for subdiv level 1.
 *         This file was derived from MeshTV's original version of 03/01/00.
 *
 * Modifications:
 *    Jeremy Meredith, Wed Feb 14 13:54:11 PST 2001
 *    Changed to use create a .h and a .C file simultaneously.
 *
 *    Jeremy Meredith, Wed Mar 13 15:16:31 PST 2002
 *    Fixed a case for the wedge and a compile error.
 *
 *    Jeremy Meredith, Mon Jan  6 10:09:13 PST 2003
 *    Fixed a couple problems with the hex/vox cases.
 *
 *    Jeremy Meredith, Tue Jun  5 13:23:18 EDT 2007
 *    Added copyright notice.
 *
 *---------------------------------------------------------------------------*/
#include <visitstream.h>
#include <stdio.h>

ofstream outc("mat3d_tet.C",ios::out);
ofstream outh("mat3d_tet.h",ios::out);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Tables for creating faces out of polyhedra
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int quad_face_table[6][4] = {
    {0,4,6,2},
    {1,3,7,5},
    {0,1,5,4},
    {2,6,7,3},
    {0,2,3,1},
    {4,5,7,6}
};

int ucd_face_table_hex[6][4] = {
    {0,1,2,3},
    {1,5,6,2},
    {2,6,7,3},
    {3,7,4,0},
    {0,4,5,1},
    {7,6,5,4}
};

int ucd_face_table_tet[4][3] = {
    {0,1,2},
    {0,2,3},
    {0,3,1},
    {1,3,2}
};

int ucd_face_table_pyr[5][4] = {
    {0,1,2,3},
    {4,3,2,  -1},
    {4,2,1,  -1},
    {4,1,0,  -1},
    {4,0,3,  -1}
};
int nsides_face_table_pyr[5] = {4,3,3,3,3};

int ucd_face_table_prism[5][4] = {
    {0,1,2,3},
    {3,2,5,4},
    {4,5,1,0},
    {1,5,2,  -1},
    {3,4,0,  -1}
};
int nsides_face_table_prism[5] = {4,4,4,3,3};

int tet_faces[4][3] = {
    {0,1,2},
    {0,2,3},
    {0,3,1},
    {1,3,2}
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Helper functions
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void
start_table_f(char* name, int dim1, int dim2=0, int dim3=0, int dim4=0)
{
    outc << "float " << name;
    if (dim1)
        outc << "[" << dim1 << "]";
    if (dim2)
        outc << "[" << dim2 << "]";
    if (dim3)
        outc << "[" << dim3 << "]";
    if (dim4)
        outc << "[" << dim4 << "]";
    outc << " = {" << endl;

    outh << "extern float " << name;
    if (dim1)
        outh << "[" << dim1 << "]";
    if (dim2)
        outh << "[" << dim2 << "]";
    if (dim3)
        outh << "[" << dim3 << "]";
    if (dim4)
        outh << "[" << dim4 << "]";
    outh << ";" << endl;
}

void
start_table_i(char* name, int dim1, int dim2=0, int dim3=0, int dim4=0)
{
    outc << "int " << name;
    if (dim1)
        outc << "[" << dim1 << "]";
    if (dim2)
        outc << "[" << dim2 << "]";
    if (dim3)
        outc << "[" << dim3 << "]";
    if (dim4)
        outc << "[" << dim4 << "]";
    outc << " = {" << endl;

    outh << "extern int   " << name;
    if (dim1)
        outh << "[" << dim1 << "]";
    if (dim2)
        outh << "[" << dim2 << "]";
    if (dim3)
        outh << "[" << dim3 << "]";
    if (dim4)
        outh << "[" << dim4 << "]";
    outh << ";" << endl;
}

 void
end_table()
{
    outc << "};" << endl << endl << endl;
}


struct comment {
    char *s;
    comment(char *s_) : s(s_) {};
};
ostream& operator<<(ostream& out, const comment &c)
{
    return out << "/* " << c.s << " */";
}


void
printrow(int n, float v[8], bool comma=true)
{
    char s[100];
    outc << "{";

    for (int i=0; i<n; i++)
    {
        sprintf(s,"%.6f", v[i]);
        outc << s;
        if (i+1 < n)
            outc << ",";
    }
    outc << "}";
    if (comma) outc << ",";
    outc << endl;
}

template <class T>
inline void
swap(T &a, T &b)
{
    T tmp;
    tmp = a;
    a   = b;
    b   = tmp;
}

template <class T>
inline void
rotl(T &a, T &b, T &c)
{
    T tmp;
    tmp = a;
    a   = b;
    b   = c;
    c   = tmp;
}

template <class T>
inline void
rotr(T &a, T &b, T &c)
{
    T tmp;
    tmp = c;
    c   = b;
    b   = a;
    a   = tmp;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Structured Mesh Tetrahedralization
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// Do structured tetrahedralization table for subdiv level 3 (48 tets)
void quad_3()
{
    outc << comment("------------------------------------------------------") << endl
         << comment("structured tetrahedralization table for subdiv level 3") << endl
         << comment("------------------------------------------------------") << endl;

    outh << "extern int   ntet_mat3d_quad2tet_subdiv3;" << endl;
    outc << "int ntet_mat3d_quad2tet_subdiv3 = 48;" << endl;
    start_table_f("mat3d_quad2tet_subdiv3",48,4,8);
    int t=0;
    float zone[8] = {.125,.125,.125,.125, .125,.125,.125,.125};
    for (int f=0; f<6; f++)
    {
        float face[8]={0,0,0,0,0,0,0,0};
        for (int n=0; n<4; n++)
            face[quad_face_table[f][n]] = 0.25;

        for (int e=0; e<4; e++)
        {
            float edge[8]={0,0,0,0,0,0,0,0};
            edge[quad_face_table[f][(e  )%4]] = 0.5;
            edge[quad_face_table[f][(e+1)%4]] = 0.5;

            for (int i=0; i<2; i++)
            {
                float node[8]={0,0,0,0,0,0,0,0};
                node[quad_face_table[f][(e+i)%4]] = 1;

                char s[100];
                sprintf(s,"tet %d -- face %d  edge %d  node %d",t,f,e,i);
                outc << "{ " << comment(s) << endl;

                printrow(8,zone);
                printrow(8,face);
                if (i==0)
                {
                    printrow(8,edge);
                    printrow(8,node,false);
                }
                else
                {
                    printrow(8,edge);
                    printrow(8,node,false);
                }
                if (t+1 == 48)
                    outc << "}" << endl;
                else
                    outc << "}," << endl;
                t++;
            }
        }
    }
    end_table();
}

// Do structured tetrahedralization table for subdiv level 2 (24 tets)
void quad_2()
{
    outc << comment("------------------------------------------------------") << endl
         << comment("structured tetrahedralization table for subdiv level 2") << endl
         << comment("------------------------------------------------------") << endl;
        

    outh << "extern int   ntet_mat3d_quad2tet_subdiv2;" << endl;
    outc << "int ntet_mat3d_quad2tet_subdiv2 = 24;" << endl;
    start_table_f("mat3d_quad2tet_subdiv2",24,4,8);
    int t=0;
    float zone[8] = {.125,.125,.125,.125, .125,.125,.125,.125};
    for (int f=0; f<6; f++)
    {
        float face[8]={0,0,0,0,0,0,0,0};
        for (int n=0; n<4; n++)
            face[quad_face_table[f][n]] = 0.25;

        for (int e=0; e<4; e++)
        {
            float node1[8]={0,0,0,0,0,0,0,0};
            float node2[8]={0,0,0,0,0,0,0,0};
            node1[quad_face_table[f][(e  )%4]] = 1;
            node2[quad_face_table[f][(e+1)%4]] = 1;

            char s[100];
            sprintf(s,"tet %d -- face %d  edge %d",t,f,e);
            outc << "{ " << comment(s) << endl;

            printrow(8,zone);
            printrow(8,face);
            printrow(8,node1);
            printrow(8,node2,false);
            if (t+1 == 24)
                outc << "}" << endl;
            else
                outc << "}," << endl;
            t++;
        }
    }
    end_table();
}

// Do structured tetrahedralization table for subdiv level 1 (5 tets)
void quad_1()
{
    outc << comment("------------------------------------------------------") << endl
         << comment("structured tetrahedralization table for subdiv level 1") << endl
         << comment("------------------------------------------------------") << endl;

    outh << "extern int   ntet_mat3d_quad2tet_subdiv1;" << endl;
    outc << "int ntet_mat3d_quad2tet_subdiv1 = 5;" << endl;
    start_table_f("mat3d_quad2tet_subdiv1",5,4,8);
    for (int t=0; t<5; t++)
    {
        int a,b,c,d;
        switch (t)
        {
          case 0:   a=0; b=1; c=4; d=2;   break;
          case 1:   a=1; b=3; c=7; d=2;   break;
          case 2:   a=1; b=5; c=4; d=7;   break;
          case 3:   a=7; b=6; c=4; d=2;   break;
          case 4:   a=1; b=4; c=2; d=7;   break;
        }
        float node[4][8]={{0,0,0,0,0,0,0,0},
                          {0,0,0,0,0,0,0,0}, 
                          {0,0,0,0,0,0,0,0}, 
                          {0,0,0,0,0,0,0,0}};
        node[0][a] = 1;
        node[1][b] = 1;
        node[2][c] = 1;
        node[3][d] = 1;

        char s[100];
        sprintf(s,"tet %d",t);
        outc << "{ " << comment(s) << endl;

        printrow(8,node[0]);
        printrow(8,node[1]);
        printrow(8,node[2]);
        printrow(8,node[3],false);
        if (t+1 == 5)
            outc << "}" << endl;
        else
            outc << "}," << endl;
    }
    end_table();

    start_table_i("mat3d_quad2tet_subdiv1_flip",2,2,2,8);
    for (int x=0; x<=1; x++)
    {
        outc << "{";
        for (int y=0; y<=1; y++)
        {
            outc << "{";
            for (int z=0; z<=1; z++)
            {
                outc << "{";
                int nodes[8] = {0,1,2,3,4,5,6,7};
                if (x)
                {
                    swap(nodes[0], nodes[1]);
                    swap(nodes[2], nodes[3]);
                    swap(nodes[4], nodes[5]);
                    swap(nodes[6], nodes[7]);
                }
                if (y)
                {
                    swap(nodes[0], nodes[2]);
                    swap(nodes[1], nodes[3]);
                    swap(nodes[4], nodes[6]);
                    swap(nodes[5], nodes[7]);
                }
                if (z)
                {
                    swap(nodes[0], nodes[4]);
                    swap(nodes[1], nodes[5]);
                    swap(nodes[2], nodes[6]);
                    swap(nodes[3], nodes[7]);
                }
                outc << nodes[0] << "," << nodes[1] << ","
                     << nodes[2] << "," << nodes[3] << ","
                     << nodes[4] << "," << nodes[5] << ","
                     << nodes[6] << "," << nodes[7];

                outc << "}";
                if (z==1)
                    outc << "}";
                if (z==1 && y==1)
                    outc << "}";
                outc << ",";

                outc << endl;

                if (z==0)
                    outc << "  ";
                else if (y==0)
                    outc << " ";
            }
        }
    }
    end_table();

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Unstructured Hexahedral Tetrahedralization
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// Do unstructured hexahedral tetrahedralization table for subdiv level 3 (48 tets)
void hex_3()
{
    outc << comment("-------------------------------------------------------------------") << endl
         << comment("unstructured hexahedral tetrahedralization table for subdiv level 3") << endl
         << comment("-------------------------------------------------------------------") << endl;

    outh << "extern int   ntet_mat3d_hex2tet_subdiv3;" << endl;
    outc << "int ntet_mat3d_hex2tet_subdiv3 = 48;" << endl;
    start_table_f("mat3d_hex2tet_subdiv3",48,4,8);
    int t=0;
    float zone[8] = {.125,.125,.125,.125, .125,.125,.125,.125};
    for (int f=0; f<6; f++)
    {
        float face[8]={0,0,0,0,0,0,0,0};
        for (int n=0; n<4; n++)
            face[ucd_face_table_hex[f][n]] = 0.25;

        for (int e=0; e<4; e++)
        {
            float edge[8]={0,0,0,0,0,0,0,0};
            edge[ucd_face_table_hex[f][(e  )%4]] = 0.5;
            edge[ucd_face_table_hex[f][(e+1)%4]] = 0.5;

            for (int i=0; i<2; i++)
            {
                float node[8]={0,0,0,0,0,0,0,0};
                node[ucd_face_table_hex[f][(e+i)%4]] = 1;

                char s[100];
                sprintf(s,"tet %d -- face %d  edge %d  node %d",t,f,e,i);
                outc << "{ " << comment(s) << endl;

                printrow(8,zone);
                printrow(8,face);
                if (i != 0)
                {
                    printrow(8,node);
                    printrow(8,edge,false);
                }
                else
                {
                    printrow(8,edge);
                    printrow(8,node,false);
                }
                if (t+1 == 48)
                    outc << "}" << endl;
                else
                    outc << "}," << endl;
                t++;
            }
        }
    }
    end_table();
}

// Do unstructured hexahedral tetrahedralization table for subdiv level 2 (24 tets)
void hex_2()
{
    outc << comment("-------------------------------------------------------------------") << endl
         << comment("unstructured hexahedral tetrahedralization table for subdiv level 2") << endl
         << comment("-------------------------------------------------------------------") << endl;
        

    outh << "extern int   ntet_mat3d_hex2tet_subdiv2;" << endl;
    outc << "int ntet_mat3d_hex2tet_subdiv2 = 24;" << endl;
    start_table_f("mat3d_hex2tet_subdiv2",24,4,8);
    int t=0;
    float zone[8] = {.125,.125,.125,.125, .125,.125,.125,.125};
    for (int f=0; f<6; f++)
    {
        float face[8]={0,0,0,0,0,0,0,0};
        for (int n=0; n<4; n++)
            face[ucd_face_table_hex[f][n]] = 0.25;

        for (int e=0; e<4; e++)
        {
            float node1[8]={0,0,0,0,0,0,0,0};
            float node2[8]={0,0,0,0,0,0,0,0};
            node1[ucd_face_table_hex[f][(e+1)%4]] = 1;
            node2[ucd_face_table_hex[f][(e  )%4]] = 1;

            char s[100];
            sprintf(s,"tet %d -- face %d  edge %d",t,f,e);
            outc << "{ " << comment(s) << endl;

            printrow(8,zone);
            printrow(8,face);
            printrow(8,node1);
            printrow(8,node2,false);
            if (t+1 == 24)
                outc << "}" << endl;
            else
                outc << "}," << endl;
            t++;
        }
    }
    end_table();
}

// Do unstructured hexahedral tetrahedralization table for subdiv level 1 (6 tets * 8 cases)
void hex_1()
{
    outc << comment("-------------------------------------------------------------------") << endl
         << comment("unstructured hexahedral tetrahedralization table for subdiv level 1") << endl
         << comment("-------------------------------------------------------------------") << endl;

    int case000[5][4][3] = {
        {{0,0,0}, {0,1,0}, {1,1,0}, {0,1,1}},
        {{0,0,0}, {1,0,1}, {0,0,1}, {0,1,1}},
        {{1,1,1}, {1,0,1}, {1,1,0}, {0,1,1}},
        {{1,0,0}, {1,0,1}, {0,0,0}, {1,1,0}},
        {{0,0,0}, {1,0,1}, {0,1,1}, {1,1,0}}
    };
    int ncase000 = 5;
  
    int case100[6][4][3] = {
        {{0,0,0}, {0,1,0}, {1,1,0}, {0,1,1}},
        {{0,0,0}, {1,0,1}, {0,0,1}, {0,1,1}},
        {{1,0,0}, {0,0,0}, {1,1,0}, {1,1,1}},
        {{1,0,0}, {1,0,1}, {0,0,0}, {1,1,1}},
        {{0,1,1}, {1,1,0}, {0,0,0}, {1,1,1}},
        {{0,1,1}, {0,0,0}, {1,0,1}, {1,1,1}}
    };
    int ncase100 = 6;

    int case011[6][4][3] = {
        {{0,0,0}, {1,1,0}, {1,0,0}, {1,0,1}},
        {{1,1,0}, {0,0,0}, {0,1,0}, {1,1,1}},
        {{1,1,0}, {0,0,0}, {1,1,1}, {1,0,1}},
        {{0,1,0}, {0,0,0}, {0,1,1}, {1,1,1}},
        {{0,0,0}, {0,0,1}, {0,1,1}, {1,1,1}},
        {{0,0,0}, {1,0,1}, {0,0,1}, {1,1,1}}
    };
    int ncase011 = 6;

    int case111[6][4][3] = {
        {{1,0,0}, {1,0,1}, {0,0,0}, {1,1,1}},
        {{1,0,0}, {0,0,0}, {1,1,0}, {1,1,1}},
        {{0,0,0}, {0,1,0}, {1,1,0}, {1,1,1}},
        {{0,0,0}, {0,1,1}, {0,1,0}, {1,1,1}},
        {{0,0,0}, {1,0,1}, {0,0,1}, {1,1,1}},
        {{0,0,0}, {0,0,1}, {0,1,1}, {1,1,1}}
    };
    int ncase111 = 6;

    int maxtet = 6;

    outh << "extern int   ntet_mat3d_hex2tet_subdiv1[8];" << endl;
    outc << "int ntet_mat3d_hex2tet_subdiv1[8] = {5,6,6,6,6,6,6,6};" << endl;
    start_table_i("mat3d_hex2tet_subdiv1_node",8,6,4);
    for (int c=0; c<8; c++)
    {
        int (*tcase)[4][3];
        int  ntcase;
        int  nrot;
        switch (c)
        {
          case 0:  tcase=case000;  ntcase=ncase000;  nrot=0;  break;
          case 1:  tcase=case100;  ntcase=ncase100;  nrot=1;  break;
          case 2:  tcase=case100;  ntcase=ncase100;  nrot=2;  break;
          case 3:  tcase=case011;  ntcase=ncase011;  nrot=0;  break;
          case 4:  tcase=case100;  ntcase=ncase100;  nrot=0;  break;
          case 5:  tcase=case011;  ntcase=ncase011;  nrot=2;  break;
          case 6:  tcase=case011;  ntcase=ncase011;  nrot=1;  break;
          case 7:  tcase=case111;  ntcase=ncase111;  nrot=0;  break;
        }

        outc << "{ " << "/***** case " << c << " *****/" << endl;
        for (int t=0; t<maxtet; t++)
        {
            int node[4] = {0,0,0,0};
            int ucdnodes[2][2][2] = {{{4,7},{0,3}},{{5,6},{1,2}}};

            if (t<ntcase)
            {
                for (int n=0; n<4; n++)
                {
                    int x = tcase[t][n][(0+nrot)%3];
                    int y = tcase[t][n][(1+nrot)%3];
                    int z = tcase[t][n][(2+nrot)%3];
                    node[n] = ucdnodes[x][y][z];
                }
            }

            outc << "{"
                 << node[0] << ","
                 << node[1] << ","
                 << node[2] << ","
                 << node[3];

        if (t+1 == maxtet)
            outc << "}" << endl;
        else
            outc << "}," << endl;
        }
        if (c+1 == 8)
            outc << "}" << endl;
        else
            outc << "}," << endl;
    }
    end_table();


    start_table_i("mat3d_hex2tet_subdiv1_flip",2,2,2,8);
    for (int x=0; x<=1; x++)
    {
        outc << "{";
        for (int y=0; y<=1; y++)
        {
            outc << "{";
            for (int z=0; z<=1; z++)
            {
                outc << "{";
                int nodes[8] = {0,1,2,3,4,5,6,7};
                if (x)
                {
                    swap(nodes[0], nodes[1]);
                    swap(nodes[3], nodes[2]);
                    swap(nodes[4], nodes[5]);
                    swap(nodes[7], nodes[6]);
                }
                if (y)
                {
                    swap(nodes[7], nodes[3]);
                    swap(nodes[4], nodes[0]);
                    swap(nodes[5], nodes[1]);
                    swap(nodes[6], nodes[2]);
                }
                if (z)
                {
                    swap(nodes[3], nodes[0]);
                    swap(nodes[2], nodes[1]);
                    swap(nodes[7], nodes[4]);
                    swap(nodes[6], nodes[5]);
                }
                outc << nodes[0] << "," << nodes[1] << ","
                     << nodes[2] << "," << nodes[3] << ","
                     << nodes[4] << "," << nodes[5] << ","
                     << nodes[6] << "," << nodes[7];

                outc << "}";
                if (z==1)
                    outc << "}";
                if (z==1 && y==1)
                    outc << "}";
                outc << ",";

                outc << endl;

                if (z==0)
                    outc << "  ";
                else if (y==0)
                    outc << " ";
            }
        }
    }
    end_table();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Unstructured Voxel Tetrahedralization
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// Do unstructured voxel tetrahedralization table for subdiv level 1 (6 tets * 8 cases)
void vox_1()
{
    outc << comment("--------------------------------------------------------------") << endl
         << comment("unstructured voxel tetrahedralization table for subdiv level 1") << endl
         << comment("--------------------------------------------------------------") << endl;

    int case000[5][4][3] = {
        {{0,0,0}, {0,1,0}, {1,1,0}, {0,1,1}},
        {{0,0,0}, {1,0,1}, {0,0,1}, {0,1,1}},
        {{1,1,1}, {1,0,1}, {1,1,0}, {0,1,1}},
        {{1,0,0}, {1,0,1}, {0,0,0}, {1,1,0}},
        {{0,0,0}, {1,0,1}, {0,1,1}, {1,1,0}}
    };
    int ncase000 = 5;
  
    int case100[6][4][3] = {
        {{0,0,0}, {0,1,0}, {1,1,0}, {0,1,1}},
        {{0,0,0}, {1,0,1}, {0,0,1}, {0,1,1}},
        {{1,0,0}, {0,0,0}, {1,1,0}, {1,1,1}},
        {{1,0,0}, {1,0,1}, {0,0,0}, {1,1,1}},
        {{0,1,1}, {1,1,0}, {0,0,0}, {1,1,1}},
        {{0,1,1}, {0,0,0}, {1,0,1}, {1,1,1}}
    };
    int ncase100 = 6;

    int case011[6][4][3] = {
        {{0,0,0}, {1,1,0}, {1,0,0}, {1,0,1}},
        {{1,1,0}, {0,0,0}, {0,1,0}, {1,1,1}},
        {{1,1,0}, {0,0,0}, {1,1,1}, {1,0,1}},
        {{0,1,0}, {0,0,0}, {0,1,1}, {1,1,1}},
        {{0,0,0}, {0,0,1}, {0,1,1}, {1,1,1}},
        {{0,0,0}, {1,0,1}, {0,0,1}, {1,1,1}}
    };
    int ncase011 = 6;

    int case111[6][4][3] = {
        {{1,0,0}, {1,0,1}, {0,0,0}, {1,1,1}},
        {{1,0,0}, {0,0,0}, {1,1,0}, {1,1,1}},
        {{0,0,0}, {0,1,0}, {1,1,0}, {1,1,1}},
        {{0,0,0}, {0,1,1}, {0,1,0}, {1,1,1}},
        {{0,0,0}, {1,0,1}, {0,0,1}, {1,1,1}},
        {{0,0,0}, {0,0,1}, {0,1,1}, {1,1,1}}
    };
    int ncase111 = 6;

    int maxtet = 6;

    outh << "extern int   ntet_mat3d_vox2tet_subdiv1[8];" << endl;
    outc << "int ntet_mat3d_vox2tet_subdiv1[8] = {5,6,6,6,6,6,6,6};" << endl;
    start_table_i("mat3d_vox2tet_subdiv1_node",8,6,4);
    for (int c=0; c<8; c++)
    {
        int (*tcase)[4][3];
        int  ntcase;
        int  nrot;
        switch (c)
        {
          case 0:  tcase=case000;  ntcase=ncase000;  nrot=0;  break;
          case 1:  tcase=case100;  ntcase=ncase100;  nrot=1;  break;
          case 2:  tcase=case100;  ntcase=ncase100;  nrot=2;  break;
          case 3:  tcase=case011;  ntcase=ncase011;  nrot=0;  break;
          case 4:  tcase=case100;  ntcase=ncase100;  nrot=0;  break;
          case 5:  tcase=case011;  ntcase=ncase011;  nrot=2;  break;
          case 6:  tcase=case011;  ntcase=ncase011;  nrot=1;  break;
          case 7:  tcase=case111;  ntcase=ncase111;  nrot=0;  break;
        }

        outc << "{ " << "/***** case " << c << " *****/" << endl;
        for (int t=0; t<maxtet; t++)
        {
            int node[4] = {0,0,0,0};
            int ucdnodes[2][2][2] = {{{4,6},{0,2}},{{5,7},{1,3}}};

            if (t<ntcase)
            {
                for (int n=0; n<4; n++)
                {
                    int x = tcase[t][n][(0+nrot)%3];
                    int y = tcase[t][n][(1+nrot)%3];
                    int z = tcase[t][n][(2+nrot)%3];
                    node[n] = ucdnodes[x][y][z];
                }
            }

            outc << "{"
                 << node[0] << ","
                 << node[1] << ","
                 << node[2] << ","
                 << node[3];

        if (t+1 == maxtet)
            outc << "}" << endl;
        else
            outc << "}," << endl;
        }
        if (c+1 == 8)
            outc << "}" << endl;
        else
            outc << "}," << endl;
    }
    end_table();


    start_table_i("mat3d_vox2tet_subdiv1_flip",2,2,2,8);
    for (int x=0; x<=1; x++)
    {
        outc << "{";
        for (int y=0; y<=1; y++)
        {
            outc << "{";
            for (int z=0; z<=1; z++)
            {
                outc << "{";
                int nodes[8] = {0,1,2,3,4,5,6,7};
                if (x)
                {
                    swap(nodes[0], nodes[1]);
                    swap(nodes[2], nodes[3]);
                    swap(nodes[4], nodes[5]);
                    swap(nodes[3], nodes[7]);
                }
                if (y)
                {
                    swap(nodes[0], nodes[4]);
                    swap(nodes[1], nodes[5]);
                    swap(nodes[2], nodes[6]);
                    swap(nodes[3], nodes[7]);
                }
                if (z)
                {
                    swap(nodes[0], nodes[2]);
                    swap(nodes[1], nodes[3]);
                    swap(nodes[4], nodes[6]);
                    swap(nodes[5], nodes[7]);
                }
                outc << nodes[0] << "," << nodes[1] << ","
                     << nodes[2] << "," << nodes[3] << ","
                     << nodes[4] << "," << nodes[5] << ","
                     << nodes[6] << "," << nodes[7];

                outc << "}";
                if (z==1)
                    outc << "}";
                if (z==1 && y==1)
                    outc << "}";
                outc << ",";

                outc << endl;

                if (z==0)
                    outc << "  ";
                else if (y==0)
                    outc << " ";
            }
        }
    }
    end_table();
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Unstructured Tetrahedral Tetrahedralization
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// Do unstructured tetrahedral tetrahedralization table for subdiv level 3 (24 tets)
void tet_3()
{
    outc << comment("--------------------------------------------------------------------") << endl
         << comment("unstructured tetrahedral tetrahedralization table for subdiv level 3") << endl
         << comment("--------------------------------------------------------------------") << endl;

    outh << "extern int   ntet_mat3d_tet2tet_subdiv3;" << endl;
    outc << "int ntet_mat3d_tet2tet_subdiv3 = 24;" << endl;
    start_table_f("mat3d_tet2tet_subdiv3",24,4,4);
    int t=0;
    float zone[4] = {.25,.25,.25,.25};
    for (int f=0; f<4; f++)
    {
        float face[4]={0,0,0,0};
        for (int n=0; n<3; n++)
            face[ucd_face_table_tet[f][n]] = 1./3.;

        for (int e=0; e<3; e++)
        {
            float edge[4]={0,0,0,0};
            edge[ucd_face_table_tet[f][(e  )%3]] = 0.5;
            edge[ucd_face_table_tet[f][(e+1)%3]] = 0.5;

            for (int i=0; i<2; i++)
            {
                float node[4]={0,0,0,0};
                node[ucd_face_table_tet[f][(e+i)%3]] = 1;

                char s[100];
                sprintf(s,"tet %d -- face %d  edge %d  node %d",t,f,e,i);
                outc << "{ " << comment(s) << endl;

                printrow(4,zone);
                printrow(4,face);
                if (i != 0)
                {
                    printrow(4,edge);
                    printrow(4,node,false);
                }
                else
                {
                    printrow(4,node);
                    printrow(4,edge,false);
                }
                if (t+1 == 24)
                    outc << "}" << endl;
                else
                    outc << "}," << endl;
                t++;
            }
        }
    }
    end_table();
}

// Do unstructured tetrahedral tetrahedralization table for subdiv level 2 (12 tets)
void tet_2()
{
    outc << comment("--------------------------------------------------------------------") << endl
         << comment("unstructured tetrahedral tetrahedralization table for subdiv level 2") << endl
         << comment("--------------------------------------------------------------------") << endl;
        

    outh << "extern int   ntet_mat3d_tet2tet_subdiv2;" << endl;
    outc << "int ntet_mat3d_tet2tet_subdiv2 = 12;" << endl;
    start_table_f("mat3d_tet2tet_subdiv2",12,4,4);
    int t=0;
    float zone[4] = {.25,.25,.25,.25};
    for (int f=0; f<4; f++)
    {
        float face[4]={0,0,0,0};
        for (int n=0; n<3; n++)
            face[ucd_face_table_tet[f][n]] = 1./3.;

        for (int e=0; e<3; e++)
        {
            float node1[4]={0,0,0,0};
            float node2[4]={0,0,0,0};
            node1[ucd_face_table_tet[f][(e+1)%3]] = 1;
            node2[ucd_face_table_tet[f][(e  )%3]] = 1;

            char s[100];
            sprintf(s,"tet %d -- face %d  edge %d",t,f,e);
            outc << "{ " << comment(s) << endl;

            printrow(4,zone);
            printrow(4,face);
            printrow(4,node1);
            printrow(4,node2,false);
            if (t+1 == 12)
                outc << "}" << endl;
            else
                outc << "}," << endl;
            t++;
        }
    }
    end_table();
}

// Do unstructured tetrahedral tetrahedralization table for subdiv level 1 (1 tet)
void tet_1()
{
    outc << comment("--------------------------------------------------------------------") << endl
         << comment("unstructured tetrahedral tetrahedralization table for subdiv level 1") << endl
         << comment("--------------------------------------------------------------------") << endl;

    outh << "extern int   ntet_mat3d_tet2tet_subdiv1;" << endl;
    outc << "int ntet_mat3d_tet2tet_subdiv1 = 1;" << endl;
    start_table_i("mat3d_tet2tet_subdiv1",1,1,4);
    int nodes[4] = {0,1,2,3};

    outc << "{ " << "/***** case " << 0 << " *****/" << endl;
    outc << "{"
         << nodes[0] << ","
         << nodes[1] << ","
         << nodes[2] << ","
         << nodes[3]
         << "}";
    outc << endl;

    outc << "}" << endl;
    end_table();
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Unstructured Pyramidal Tetrahedralization
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// Do unstructured pyramidal tetrahedralization table for subdiv level 3 (32 tets)
void pyr_3()
{
    outc << comment("------------------------------------------------------------------") << endl
         << comment("unstructured pyramidal tetrahedralization table for subdiv level 3") << endl
         << comment("------------------------------------------------------------------") << endl;

    outh << "extern int   ntet_mat3d_pyr2tet_subdiv3;" << endl;
    outc << "int ntet_mat3d_pyr2tet_subdiv3 = 32;" << endl;
    start_table_f("mat3d_pyr2tet_subdiv3",32,4,5);
    int t=0;
    float zone[5] = {.2,.2,.2,.2,.2};
    for (int f=0; f<5; f++)
    {
        int nedges = nsides_face_table_pyr[f];
        float face[5]={0,0,0,0,0};
        for (int n=0; n<nedges; n++)
            face[ucd_face_table_pyr[f][n]] = 1./float(nedges);

        for (int e=0; e<nedges; e++)
        {
            float edge[5]={0,0,0,0,0};
            edge[ucd_face_table_pyr[f][(e  )%nedges]] = 0.5;
            edge[ucd_face_table_pyr[f][(e+1)%nedges]] = 0.5;

            for (int i=0; i<2; i++)
            {
                float node[5]={0,0,0,0,0};
                node[ucd_face_table_tet[f][(e+i)%nedges]] = 1;

                char s[100];
                sprintf(s,"tet %d -- face %d  edge %d  node %d",t,f,e,i);
                outc << "{ " << comment(s) << endl;

                printrow(5,zone);
                printrow(5,face);
                if (i != 0)
                {
                    printrow(5,edge);
                    printrow(5,node,false);
                }
                else
                {
                    printrow(5,node);
                    printrow(5,edge,false);
                }
                if (t+1 == 32)
                    outc << "}" << endl;
                else
                    outc << "}," << endl;
                t++;
            }
        }
    }
    end_table();
}

// Do unstructured pyramidal tetrahedralization table for subdiv level 2 (16 tets)
void pyr_2()
{
    outc << comment("------------------------------------------------------------------") << endl
         << comment("unstructured pyramidal tetrahedralization table for subdiv level 2") << endl
         << comment("------------------------------------------------------------------") << endl;
        

    outc << "int ntet_mat3d_pyr2tet_subdiv2 = 16;" << endl;
    start_table_f("mat3d_pyr2tet_subdiv2",16,4,5);
    int t=0;
    float zone[5] = {.2,.2,.2,.2,.2};
    for (int f=0; f<4; f++)
    {
        int nedges = nsides_face_table_pyr[f];
        float face[5]={0,0,0,0,0};
        for (int n=0; n<nedges; n++)
            face[ucd_face_table_pyr[f][n]] = 1./float(nedges);

        for (int e=0; e<nedges; e++)
        {
            float node1[5]={0,0,0,0,0};
            float node2[5]={0,0,0,0,0};
            node1[ucd_face_table_pyr[f][(e+1)%nedges]] = 1;
            node2[ucd_face_table_pyr[f][(e  )%nedges]] = 1;

            char s[100];
            sprintf(s,"tet %d -- face %d  edge %d",t,f,e);
            outc << "{ " << comment(s) << endl;

            printrow(5,zone);
            printrow(5,face);
            printrow(5,node1);
            printrow(5,node2,false);
            if (t+1 == 16)
                outc << "}" << endl;
            else
                outc << "}," << endl;
            t++;
        }
    }
    end_table();
}


// Do unstructured pyramidal tetrahedralization table for subdiv level 3 (3 tets * 2 cases)
void pyr_1()
{
    outc << comment("----------------------------------------------------------------") << endl
         << comment("unstructured pyramid tetrahedralization table for subdiv level 1") << endl
         << comment("----------------------------------------------------------------") << endl;

    int case0[2][4] = {
        {0,1,2,4},
        {0,2,3,4}
    };

    outh << "extern int   ntet_mat3d_pyr2tet_subdiv1;" << endl;
    outc << "int ntet_mat3d_pyr2tet_subdiv1 = 2;" << endl;
    start_table_i("mat3d_pyr2tet_subdiv1_node",1,2,4);
    for (int c=0; c<1; c++)
    {
        outc << "{ " << "/***** case " << c << " *****/" << endl;
        for (int t=0; t<2; t++)
        {
            int *node;
            node = case0[t];

            outc << "{"
                 << node[0] << ","
                 << node[1] << ","
                 << node[2] << ","
                 << node[3];

            if (t+1 == 2)
                outc << "}" << endl;
            else
                outc << "}," << endl;
        }

        if (c+1 == 1)
            outc << "}" << endl;
        else
            outc << "}," << endl;
    }
    end_table();


    start_table_i("mat3d_pyr2tet_subdiv1_flip",2,5);
    for (int f=0; f<2; f++)
    {
        outc << "{";
        int nodes[5] = {0,1,2,3,4};

        if (f==0)
        {
            // no flip
        }
        else
        {
            swap(nodes[0],nodes[1]);
            swap(nodes[3],nodes[2]);
        }

        outc << nodes[0] << ","
             << nodes[1] << ","
             << nodes[2] << ","
             << nodes[3] << ","
             << nodes[4];

        outc << "}";
        if (f==0)
            outc << ",";

        outc << endl;
    }
    end_table();
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Unstructured Prismatic Tetrahedralization
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// Do unstructured prismatic tetrahedralization table for subdiv level 3 (36 tets)
void prism_3()
{
    outc << comment("------------------------------------------------------------------") << endl
         << comment("unstructured prismatic tetrahedralization table for subdiv level 3") << endl
         << comment("------------------------------------------------------------------") << endl;

    outh << "extern int   ntet_mat3d_prism2tet_subdiv3;" << endl;
    outc << "int ntet_mat3d_prism2tet_subdiv3 = 36;" << endl;
    start_table_f("mat3d_prism2tet_subdiv3",36,4,6);
    int t=0;
    float zone[6] = {1./6., 1./6., 1./6., 1./6., 1./6., 1./6.};
    for (int f=0; f<6; f++)
    {
        int nedges = nsides_face_table_prism[f];
        float face[6]={0,0,0,0,0};
        for (int n=0; n<nedges; n++)
            face[ucd_face_table_prism[f][n]] = 1./float(nedges);

        for (int e=0; e<nedges; e++)
        {
            float edge[6]={0,0,0,0,0};
            edge[ucd_face_table_prism[f][(e  )%nedges]] = 0.5;
            edge[ucd_face_table_prism[f][(e+1)%nedges]] = 0.5;

            for (int i=0; i<2; i++)
            {
                float node[6]={0,0,0,0,0};
                node[ucd_face_table_tet[f][(e+i)%nedges]] = 1;

                char s[100];
                sprintf(s,"tet %d -- face %d  edge %d  node %d",t,f,e,i);
                outc << "{ " << comment(s) << endl;

                printrow(6,zone);
                printrow(6,face);
                if (i != 0)
                {
                    printrow(6,edge);
                    printrow(6,node,false);
                }
                else
                {
                    printrow(6,node);
                    printrow(6,edge,false);
                }
                if (t+1 == 36)
                    outc << "}" << endl;
                else
                    outc << "}," << endl;
                t++;
            }
        }
    }
    end_table();
}

// Do unstructured prismatic tetrahedralization table for subdiv level 2 (18 tets)
void prism_2()
{
    outc << comment("------------------------------------------------------------------") << endl
         << comment("unstructured prismatic tetrahedralization table for subdiv level 2") << endl
         << comment("------------------------------------------------------------------") << endl;
        

    outh << "extern int   ntet_mat3d_prism2tet_subdiv2;" << endl;
    outc << "int ntet_mat3d_prism2tet_subdiv2 = 18;" << endl;
    start_table_f("mat3d_prism2tet_subdiv2",18,4,6);
    int t=0;
    float zone[6] = {.2,.2,.2,.2,.2};
    for (int f=0; f<4; f++)
    {
        int nedges = nsides_face_table_prism[f];
        float face[6]={0,0,0,0,0};
        for (int n=0; n<nedges; n++)
            face[ucd_face_table_prism[f][n]] = 1./float(nedges);

        for (int e=0; e<nedges; e++)
        {
            float node1[6]={0,0,0,0,0};
            float node2[6]={0,0,0,0,0};
            node1[ucd_face_table_prism[f][(e+1)%nedges]] = 1;
            node2[ucd_face_table_prism[f][(e  )%nedges]] = 1;

            char s[100];
            sprintf(s,"tet %d -- face %d  edge %d",t,f,e);
            outc << "{ " << comment(s) << endl;

            printrow(6,zone);
            printrow(6,face);
            printrow(6,node1);
            printrow(6,node2,false);
            if (t+1 == 18)
                outc << "}" << endl;
            else
                outc << "}," << endl;
            t++;
        }
    }
    end_table();
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//   Unstructured Wedgic Tetrahedralization
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// Do unstructured wedgic tetrahedralization table for subdiv level 3 (3 tets * 2 cases)
void wedge_1()
{
    outc << comment("--------------------------------------------------------------") << endl
         << comment("unstructured wedge tetrahedralization table for subdiv level 1") << endl
         << comment("--------------------------------------------------------------") << endl;

    int case0[3][4] = {
        {0,3,4,5},
        {0,4,1,2},
        {0,2,5,4}
    };
    int case1[3][4] = {
        {0,3,4,5},
        {0,4,1,5},
        {0,2,5,1}
    };

    outh << "extern int   ntet_mat3d_wedge2tet_subdiv1;" << endl;
    outc << "int ntet_mat3d_wedge2tet_subdiv1 = 3;" << endl;
    start_table_i("mat3d_wedge2tet_subdiv1_node",2,3,4);
    for (int c=0; c<2; c++)
    {
        outc << "{ " << "/***** case " << c << " *****/" << endl;
        for (int t=0; t<3; t++)
        {
            int *node;
            if (c==0)
                node = case0[t];
            else
                node = case1[t];

            outc << "{"
                 << node[0] << ","
                 << node[1] << ","
                 << node[2] << ","
                 << node[3];

            if (t+1 == 3)
                outc << "}" << endl;
            else
                outc << "}," << endl;
        }

        if (c+1 == 2)
            outc << "}" << endl;
        else
            outc << "}," << endl;
    }
    end_table();


    start_table_i("mat3d_wedge2tet_subdiv1_fliprot",2,3,6);
    for (int f=0; f<2; f++)
    {
        outc << "{";
        for (int r=0; r<3; r++)
        {
            outc << "{";
            int nodes0[3] = {0,1,2};
            int nodes1[3] = {3,4,5};
            if (r==0)
            {
                // no rotation
            }
            else if (r==1)
            {
                rotl(nodes0[0],nodes0[1],nodes0[2]);
                rotl(nodes1[0],nodes1[1],nodes1[2]);
            }
            else if (r==2)
            {
                rotr(nodes0[0],nodes0[1],nodes0[2]);
                rotr(nodes1[0],nodes1[1],nodes1[2]);
            }

            int *nodes_top;
            int *nodes_bot;
            if (f == 0)
            {
                nodes_top = nodes0;
                nodes_bot = nodes1;
            }
            else
            {
                nodes_top = nodes1;
                nodes_bot = nodes0;
            }

            outc << nodes_top[0] << ","
                 << nodes_top[1] << ","
                 << nodes_top[2] << ","
                 << nodes_bot[0] << ","
                 << nodes_bot[1] << ","
                 << nodes_bot[2];

            outc << "}";
            if (r==2)
                outc << "}";
            if (r!=2 || f!=1)
            outc << ",";

            outc << endl;

            if (r==0 || r==1)
                outc << " ";
        }
    }
    end_table();
}

static void WriteCopyright(ostream &out)
{
    out << "/*****************************************************************************" << endl;
    out << "*" << endl;
    out << "* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC" << endl;
    out << "* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124" << endl;
    out << "* All rights reserved." << endl;
    out << "*" << endl;
    out << "* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The" << endl;
    out << "* full copyright notice is contained in the file COPYRIGHT located at the root" << endl;
    out << "* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html." << endl;
    out << "*" << endl;
    out << "* Redistribution  and  use  in  source  and  binary  forms,  with  or  without" << endl;
    out << "* modification, are permitted provided that the following conditions are met:" << endl;
    out << "*" << endl;
    out << "*  - Redistributions of  source code must  retain the above  copyright notice," << endl;
    out << "*    this list of conditions and the disclaimer below." << endl;
    out << "*  - Redistributions in binary form must reproduce the above copyright notice," << endl;
    out << "*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the" << endl;
    out << "*    documentation and/or other materials provided with the distribution." << endl;
    out << "*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may" << endl;
    out << "*    be used to endorse or promote products derived from this software without" << endl;
    out << "*    specific prior written permission." << endl;
    out << "*" << endl;
    out << "* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS \"AS IS\"" << endl;
    out << "* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE" << endl;
    out << "* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE" << endl;
    out << "* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY," << endl;
    out << "* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY" << endl;
    out << "* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL" << endl;
    out << "* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR" << endl;
    out << "* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER" << endl;
    out << "* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT" << endl;
    out << "* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY" << endl;
    out << "* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH" << endl;
    out << "* DAMAGE." << endl;
    out << "*" << endl;
    out << "*****************************************************************************/" << endl;
    out << "" << endl;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//                                Main
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int
main()
{
    WriteCopyright(outc);
    WriteCopyright(outh);
    outc << comment("*****************************************************************") << endl
         << comment("-----------------------------------------------------------------") << endl
         << comment("                                                                 ") << endl
         << comment("       This file generated automatically by mat3d_tables.C       ") << endl
         << comment("                                                                 ") << endl
         << comment("-----------------------------------------------------------------") << endl
         << comment("*****************************************************************") << endl
         << endl << endl << endl;
    outh << comment("*****************************************************************") << endl
         << comment("-----------------------------------------------------------------") << endl
         << comment("                                                                 ") << endl
         << comment("       This file generated automatically by mat3d_tables.C       ") << endl
         << comment("                                                                 ") << endl
         << comment("-----------------------------------------------------------------") << endl
         << comment("*****************************************************************") << endl
         << endl << endl << endl;

    outc << comment("*****************************************************************") << endl
         << comment("                         Structured tables                       ") << endl   
         << comment("*****************************************************************") << endl
         << endl << endl;

    //quad_3();
    //quad_2();
    //quad_1();

    outc << comment("*****************************************************************") << endl
         << comment("                        Unstructured tables                      ") << endl   
         << comment("*****************************************************************") << endl
         << endl << endl;

    outc << comment("***************************** Voxel *****************************") << endl
         << endl;

    //vox_3();
    //vox_2();
    vox_1();

    outc << comment("*************************** Hexahedron **************************") << endl
         << endl;

    //hex_3();
    //hex_2();
    hex_1();

    outc << comment("************************** Tetrahedron **************************") << endl
         << endl;

    //tet_3();
    //tet_2();
    tet_1();

    outc << comment("**************************** Pyramid ****************************") << endl
         << endl;

    //pyr_3();
    //pyr_2();
    pyr_1();

    outc << comment("***************************** Wedge *****************************") << endl
         << endl;

    //wedge_3();
    //wedge_2();
    wedge_1();

    /*
       - this is silo's wedge -- different node ordering from vtk's wedge
    outc << comment("***************************** Prism *****************************") << endl
         << endl;

    prism_3();
    prism_2();
    prism_1();
    */

    return 0;
}


