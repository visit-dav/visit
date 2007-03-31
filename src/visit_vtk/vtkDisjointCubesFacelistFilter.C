// ************************************************************************* //
//                        vtkDisjointCubesFacelistFilter.C                   //
// ************************************************************************* //

#include "vtkDisjointCubesFacelistFilter.h"
#include "vtkObjectFactory.h"
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <float.h>

#include <vector>
using std::vector;


struct quadRecord {
  float Xmin,Xmax;
  float Ymin,Ymax;
  float Zmin,Zmax;
};

static void  findExteriorQuads(int nzones, float *bb, 
                     float *dataSetMinMaxCoord,  float minZoneWidth,
                     struct quadRecord **Xquads, struct quadRecord **Yquads,
                     struct quadRecord **Zquads, int **XcellIDs,
                     int **YcellIDs, int **ZcellIDs, int *numXquads,
                     int *numYquads, int *numZquads);

vtkStandardNewMacro(vtkDisjointCubesFacelistFilter); 

void
vtkDisjointCubesFacelistFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
}

// ****************************************************************************
//  Method: vtkDisjointCubesFacelistFilter::Execute
//
//  Purpose:
//      Finds the faces that are external to the unstructured grid input.
//
//  Programmer: Hank Childs
//  Creation:   October 2, 2003
//
// ****************************************************************************

void
vtkDisjointCubesFacelistFilter::Execute()
{
    int  i, j;

    vtkUnstructuredGrid *ugrid = GetInput();
    int ncells = ugrid->GetNumberOfCells();
    float *cells = new float[ncells*9];
    vtkIdType *ptr = ugrid->GetCells()->GetPointer();
    float *pts = (float *) ugrid->GetPoints()->GetVoidPointer(0);
    float bounds[6] = { FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX,
                        FLT_MAX, -FLT_MAX };
    float minCellWidth = FLT_MAX;
    for (i = 0 ; i < ncells ; i++)
    {
        ptr += 1;  // npts for this cell.
        float *cell = cells + 9*i;
        cell[0] = +FLT_MAX;
        cell[1] = -FLT_MAX;
        cell[2] = +FLT_MAX;
        cell[3] = -FLT_MAX;
        cell[4] = +FLT_MAX;
        cell[5] = -FLT_MAX;
        cell[6] = i;
        cell[7] = 0;
        cell[8] = 0;

        for (j = 0 ; j < 8 ; j++)
        {
            int pt = ptr[j];
            float *this_pt = pts + 3*pt;
            cell[0] = (this_pt[0] < cell[0] ? this_pt[0] : cell[0]);
            cell[1] = (this_pt[0] > cell[1] ? this_pt[0] : cell[1]);
            cell[2] = (this_pt[1] < cell[2] ? this_pt[1] : cell[2]);
            cell[3] = (this_pt[1] > cell[3] ? this_pt[1] : cell[3]);
            cell[4] = (this_pt[2] < cell[4] ? this_pt[2] : cell[4]);
            cell[5] = (this_pt[2] > cell[5] ? this_pt[2] : cell[5]);
        }

        if (cell[0] < bounds[0])
            bounds[0] = cell[0];
        if (cell[1] > bounds[1])
            bounds[1] = cell[1];
        if (cell[2] < bounds[2])
            bounds[2] = cell[2];
        if (cell[3] > bounds[3])
            bounds[3] = cell[3];
        if (cell[4] < bounds[4])
            bounds[4] = cell[4];
        if (cell[5] > bounds[5])
            bounds[5] = cell[5];
        if ((cell[1] - cell[0]) < minCellWidth)
            minCellWidth = cell[1] - cell[0];
        if ((cell[3] - cell[2]) < minCellWidth)
            minCellWidth = cell[3] - cell[2];
        if ((cell[5] - cell[4]) < minCellWidth)
            minCellWidth = cell[5] - cell[4];

        ptr += 8;  // 8 coords for the hex.
    }
    double delX = bounds[1] - bounds[0];
    bounds[0] -= 0.1*delX;
    bounds[1] += 0.1*delX;
    double delY = bounds[3] - bounds[2];
    bounds[2] -= 0.1*delY;
    bounds[3] += 0.1*delY;
    double delZ = bounds[5] - bounds[4];
    bounds[4] -= 0.1*delZ;
    bounds[5] += 0.1*delZ;

    quadRecord *Xquads = NULL;
    quadRecord *Yquads = NULL;
    quadRecord *Zquads = NULL;
    int *xcellIds = NULL;
    int *ycellIds = NULL;
    int *zcellIds = NULL;
    int numXquads = 0;
    int numYquads = 0;
    int numZquads = 0;

    findExteriorQuads(ncells, cells, bounds, minCellWidth, &Xquads, &Yquads, &Zquads,
                      &xcellIds, &ycellIds, &zcellIds, &numXquads, &numYquads, &numZquads);

    int nquads = numXquads + numYquads + numZquads;
    int npts = 4*nquads;

    vtkPolyData *output = GetOutput();
    vtkPoints *newpts = vtkPoints::New();
    newpts->SetNumberOfPoints(npts);
    output->SetPoints(newpts);
    newpts->Delete();

    vtkCellData *in_cd  = ugrid->GetCellData();
    output->Allocate(nquads);
    vtkCellData *out_cd = output->GetCellData();
    out_cd->CopyAllocate(in_cd, nquads);

    int cellCount = 0;

    for (i = 0 ; i < numXquads ; i++)
    {
        quadRecord &q = Xquads[i];
        int cellid = xcellIds[i];
        float pt[3];
        pt[0] = q.Xmin;
        pt[1] = q.Ymin;
        pt[2] = q.Zmin;
        newpts->SetPoint(4*cellCount, pt);
        pt[1] = q.Ymax;
        newpts->SetPoint(4*cellCount+1, pt);
        pt[2] = q.Zmax;
        newpts->SetPoint(4*cellCount+2, pt);
        pt[1] = q.Ymin;
        newpts->SetPoint(4*cellCount+3, pt);
        out_cd->CopyData(in_cd, cellid, cellCount);
        vtkIdType coords[4];
        coords[0] = 4*cellCount;
        coords[1] = coords[0] + 1;
        coords[2] = coords[1] + 1;
        coords[3] = coords[2] + 1;
        output->InsertNextCell(VTK_QUAD, 4, coords);
        cellCount++;
    }

    for (i = 0 ; i < numYquads ; i++)
    {
        quadRecord &q = Yquads[i];
        int cellid = ycellIds[i];
        float pt[3];
        pt[0] = q.Xmin;
        pt[1] = q.Ymin;
        pt[2] = q.Zmin;
        newpts->SetPoint(4*cellCount, pt);
        pt[0] = q.Xmax;
        newpts->SetPoint(4*cellCount+1, pt);
        pt[2] = q.Zmax;
        newpts->SetPoint(4*cellCount+2, pt);
        pt[0] = q.Xmin;
        newpts->SetPoint(4*cellCount+3, pt);
        out_cd->CopyData(in_cd, cellid, cellCount);
        vtkIdType coords[4];
        coords[0] = 4*cellCount;
        coords[1] = coords[0] + 1;
        coords[2] = coords[1] + 1;
        coords[3] = coords[2] + 1;
        output->InsertNextCell(VTK_QUAD, 4, coords);
        cellCount++;
    }

    for (i = 0 ; i < numZquads ; i++)
    {
        quadRecord &q = Zquads[i];
        int cellid = zcellIds[i];
        float pt[3];
        pt[0] = q.Xmin;
        pt[1] = q.Ymin;
        pt[2] = q.Zmin;
        newpts->SetPoint(4*cellCount, pt);
        pt[0] = q.Xmax;
        newpts->SetPoint(4*cellCount+1, pt);
        pt[1] = q.Ymax;
        newpts->SetPoint(4*cellCount+2, pt);
        pt[0] = q.Xmin;
        newpts->SetPoint(4*cellCount+3, pt);
        out_cd->CopyData(in_cd, cellid, cellCount);
        vtkIdType coords[4];
        coords[0] = 4*cellCount;
        coords[1] = coords[0] + 1;
        coords[2] = coords[1] + 1;
        coords[3] = coords[2] + 1;
        output->InsertNextCell(VTK_QUAD, 4, coords);
        cellCount++;
    }
}


// The below code was contributed by Peter Williams, plw@llnl.gov
/* ff.h */

#define INF 10E32F


/* Here is what the array of bounding boxes should look like,
   note last 2 fields need to be initialized to 0.
bounding Box Array (bb) format:
                bb[offset+0] = min_x;
                bb[offset+1] = max_x;
                bb[offset+2] = min_y;
                bb[offset+3] = max_y;
                bb[offset+4] = min_z;
                bb[offset+5] = max_z;
                bb[offset+6] = cellID;
                bb[offset+7] = minFlag; // init to 0 
                bb[offset+8] = maxFlag; // init to 0
*/
#define BB_DS_SIZE 9 // see above
#define XMIN   bb[BB_DS_SIZE*zone+0]
#define XMAX   bb[BB_DS_SIZE*zone+1]
#define YMIN   bb[BB_DS_SIZE*zone+2]
#define YMAX   bb[BB_DS_SIZE*zone+3]
#define ZMIN   bb[BB_DS_SIZE*zone+4]
#define ZMAX   bb[BB_DS_SIZE*zone+5]
#define ZONEID bb[BB_DS_SIZE*zone+6]
#define NFLAG  bb[BB_DS_SIZE*zone+7]
#define FFLAG  bb[BB_DS_SIZE*zone+8]

void  findExteriorQuads(int nzones, float *bb, float *dataSetMinMaxCoord,  float minZoneWidth,
                         struct quadRecord **Xquads, struct quadRecord **Yquads,
                         struct quadRecord **Zquads, int **XcellIDs, int **YcellIDs, 
                        int **ZcellIDs, int *numXquads,int *numYquads,int *numZquads);




#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define GRIDFACTOR 1.0   // determines granularity of sweeping plane grid.
#define CHUNKFACTOR 0.05 // determines how big (as fn of nzones) to 
                         // allocate exterior face array.

// used to see if more space is needed for exterior face array, if so
// it allocates it using realloc()
#define CHECKARRAYBOUNDS(numQuads,quads,cellIDs)                   \
if (currentQuad >=  numQuads) {                                    \
  allocateQuads(&quads,&cellIDs,numQuads+quadsChunkSize,numQuads); \
  numQuads += quadsChunkSize;                                      \
}

// next 3 macros used to snap each zone to cells in sweep plane grid, i.e. finds begining
// and ending indices over grid that are covered by a zone.
#define EPS 0.9999999 
  // to force max values that lie on grid cell bdry to just under

#define SNAP2GRIDX() {                                                                     \
  min1idx = floor( ((YMIN - gridYmin)/gridSpacing));                                       \
  max1idx = floor( ((YMAX - gridYmin)/gridSpacing) *EPS);                                  \
  min2idx = floor( ((ZMIN - gridZmin)/gridSpacing));                                       \
  max2idx = floor( ((ZMAX - gridZmin)/gridSpacing) *EPS);                                  \
  if ((min1idx < 0)||(max1idx > (gridYsize-1))||(min2idx < 0)||(max2idx > (gridZsize-1))) {\
    /* ymin, ymax, zmin or zmax are out of bounds */                                       \
    debug1 << "Error: Min-Max Coords of Data Set provided by user have been exceeded!!!"   \
           << endl;                                                                        \
    EXCEPTION0(ImproperUseException);                                                      \
  }                                                                                        \
}

#define SNAP2GRIDY() {                                                                     \
  min1idx = floor( ((XMIN - gridXmin)/gridSpacing));                                       \
  max1idx = floor( ((XMAX - gridXmin)/gridSpacing) *EPS);                                  \
  min2idx = floor( ((ZMIN - gridZmin)/gridSpacing));                                       \
  max2idx = floor( ((ZMAX - gridZmin)/gridSpacing) *EPS);                                  \
  if ((min1idx < 0)||(max1idx > (gridXsize-1))||(min2idx < 0)||(max2idx > (gridZsize-1))) {\
    /* ymin, ymax, zmin or zmax are out of bounds */                                       \
    debug1 << "Error: Min-Max Coords of Data Set provided by user have been exceeded!!!"   \
           << endl;                                                                        \
    EXCEPTION0(ImproperUseException);                                                      \
  }                                                                                        \
}

#define SNAP2GRIDZ() {                                                                     \
  min1idx = floor( ((XMIN - gridXmin)/gridSpacing));                                       \
  max1idx = floor( ((XMAX - gridXmin)/gridSpacing) *EPS);                                  \
  min2idx = floor( ((YMIN - gridYmin)/gridSpacing));                                       \
  max2idx = floor( ((YMAX - gridYmin)/gridSpacing) *EPS);                                  \
  if ((min2idx < 0)||(max2idx > (gridYsize-1))||(min1idx < 0)||(max1idx > (gridXsize-1))) {\
    /* ymin, ymax, zmin or zmax are out of bounds */                                       \
    debug1 << "Error: Min-Max Coords of Data Set provided by user have been exceeded!!!"   \
           << endl;                                                                        \
    EXCEPTION0(ImproperUseException);                                                      \
  }                                                                                        \
}


#define INITBB() {                  \
  int offset;                       \
  for (zone=0;zone<nzones;zone++) { \
     offset = BB_DS_SIZE*zone;      \
     bb[offset+7] = 0;              \
     bb[offset+8] = 0;              \
  }                                 \
}

// Some macros for debugging purposes, followed by the two main macros, RUNSWEEP &
// ADD_FARFACES_IN_GRID.

#define PRINTGRID(size1, size2)                                                 \
{ int i,j;                                                                      \
  for (i=0; i < size1; i++) {                                                   \
    for (j=0; j < size2; j++) {                                                 \
        printf("(grid[i][j]).startedFlag = %d\n",(grid[i][j]).startedFlag);     \
        printf("(grid[i][j]).farFaceZoneNo = %d\n",(grid[i][j]).farFaceZoneNo); \
        printf("(grid[i][j]).maxFarFace = %f\n",(grid[i][j]).maxFarFace);       \
        printf("diff = %f\n",(grid[i][j]).maxFarFace +INF);                     \
        if (grid[i][j].maxFarFace == INF ) printf("equal\n");                   \
    }                                                                           \
  }                                                                             \
}

#define PRINTQUADS()                               \
  {int i;                                          \
   for (i=0;i<*numXquads;i++) {                    \
     printf("Xquads[i].Xmin=%f\n",Xquads[i].Xmin); \
     printf("Xquads[i].Xmax=%f\n",Xquads[i].Xmax); \
     printf("Xquads[i].Ymin=%f\n",Xquads[i].Ymin); \
     printf("Xquads[i].Ymax=%f\n",Xquads[i].Ymax); \
     printf("Xquads[i].Zmin=%f\n",Xquads[i].Zmin); \
     printf("Xquads[i].Zmax=%f\n",Xquads[i].Zmax); \
     printf("XcellIDs[i]=%d\n",XcellIDs[i]);       \
   }                                               \
  }   

#define PRINTGRIDSIZE(size1,size2,min1,max1,min2,max2) {                    \
  printf("\ngridSize1 = %d gridSize2 = %d\n",size1,size2);                  \
  printf("gridSpacing = %f  minZoneWidth = %f\n",gridSpacing,minZoneWidth); \
  printf("gridmin1 = %f gridmax1 = %f\n",min1,max1);                        \
  printf("gridmin2 = %f gridmax2 = %f\n\n",min2,max2);                      \
}

#define   PRINTBB()                                             \
{ int i;                                                        \
 for (i=0;i<nzones*BB_DS_SIZE;i+=BB_DS_SIZE) {                  \
   printf(" %d  {%f %f}   {%f %f}   {%f %f}        {%f %f}\n",  \
   (int)bb[i+6], bb[i+0], bb[i+1], bb[i+2], bb[i+3],            \
    bb[i+4], bb[i+5],  bb[i+7], bb[i+8]);                       \
  }                                                             \
}

#define PRINTDATASETMINMAXCOORDS() {                                     \
  printf("datasetMinMax: %f %f   %f %f   %f %f\n",dataSetMinMaxCoord[0], \
      dataSetMinMaxCoord[1],dataSetMinMaxCoord[2],dataSetMinMaxCoord[3], \
      dataSetMinMaxCoord[4],dataSetMinMaxCoord[5]);                      \
}

#define RUNSWEEP(XMIN1,XMAX1,YMIN1,YMAX1,ZMIN1,ZMAX1,       \
                 XMIN2,XMAX2,YMIN2,YMAX2,ZMIN2,ZMAX2,NF,FF, \
                 QUADS,NUMQUADS,CELLIDS,GRIDSIZE1,GRIDSIZE2)\
{                                                           \
  for (i1=min1idx; i1<=max1idx; i1++) {                     \
    for (j1=min2idx; j1<=max2idx; j1++) {                   \
      thisGridCell = &(grid[i1][j1]);                       \
      if ( thisGridCell->startedFlag == 0) {                \
        /* add this zone to grid cell */                    \
        thisGridCell->startedFlag = 1;                      \
        thisGridCell->farFaceZoneNo = (int)ZONEID;          \
        thisGridCell->maxFarFace = FF;                      \
        /* output nearFace this zone if not already done */ \
        if (!(int)(NFLAG)) {                                \
          NFLAG = 1;                                        \
          /* printf("adding #1 near face zone: %d\n",(int)ZONEID);*/ \
          CHECKARRAYBOUNDS(*NUMQUADS,QUADS,CELLIDS)         \
          QUADS[currentQuad].Xmin = XMIN1;                  \
          QUADS[currentQuad].Xmax = XMAX1;                  \
          QUADS[currentQuad].Ymin = YMIN1;                  \
          QUADS[currentQuad].Ymax = YMAX1;                  \
          QUADS[currentQuad].Zmin = ZMIN1;                  \
          QUADS[currentQuad].Zmax = ZMAX1;                  \
          CELLIDS[currentQuad++] = (int)ZONEID;             \
        }                                                   \
      }                                                     \
      else { /* startedFlag == 1 */                         \
        if (FF > thisGridCell->maxFarFace) {                \
          if (NF > thisGridCell->maxFarFace) { /*  *** */   \
             /* air gap exists between this zone & zone */  \
             /* stored in this grid cell, so output  */     \
             /* nearFace this zone if not already output */ \
             if (!(int)(NFLAG)) {                           \
                 NFLAG = 1;                                 \
                 /* printf("adding #2 near face zone: %d\n",\
                           (int)ZONEID); */                 \
                 CHECKARRAYBOUNDS(*NUMQUADS,QUADS,CELLIDS)  \
                 QUADS[currentQuad].Xmin = XMIN1;           \
                 QUADS[currentQuad].Xmax = XMAX1;           \
                 QUADS[currentQuad].Ymin = YMIN1;           \
                 QUADS[currentQuad].Ymax = YMAX1;           \
                 QUADS[currentQuad].Zmin = ZMIN1;           \
                 QUADS[currentQuad].Zmax = ZMAX1;           \
                 CELLIDS[currentQuad++] = (int)ZONEID;      \
              }                                             \
              /* and also output farFace of zone stored */  \
              /* in this grid cell. (temp swap zone id */   \
              /* so can use XMIN, ... macros) */            \
              savedZone = zone;                             \
              zone = thisGridCell->farFaceZoneNo;           \
              if (!(int)(FFLAG)) {                          \
                /* printf("adding far face zone: %d\n",     \
                   (int)ZONEID); */                         \
                FFLAG = 1;                                  \
                CHECKARRAYBOUNDS(*NUMQUADS,QUADS,CELLIDS)   \
                QUADS[currentQuad].Xmin = XMIN2;            \
                QUADS[currentQuad].Xmax = XMAX2;            \
                QUADS[currentQuad].Ymin = YMIN2;            \
                QUADS[currentQuad].Ymax = YMAX2;            \
                QUADS[currentQuad].Zmin = ZMIN2;            \
                QUADS[currentQuad].Zmax = ZMAX;             \
                CELLIDS[currentQuad++] = (int)ZONEID;       \
              }                                             \
              zone = savedZone;                             \
            }  /* end *** */                                \
            /* now add this zone to grid cell */            \
            thisGridCell->farFaceZoneNo = ZONEID;           \
            thisGridCell->maxFarFace = FF;                  \
          }                                                 \
        } /* end else          */                           \
      }   /* end for j1        */                           \
    }     /* end for i1        */                           \
}


#define   ADD_FARFACES_IN_GRID( XMIN1,XMAX1,YMIN1,YMAX1,    \
               ZMIN1,ZMAX1,XMIN2,XMAX2,YMIN2,YMAX2,ZMIN2,   \
               ZMAX2,NF,FF,QUADS,NUMQUADS,CELLIDS,GRIDSIZE1,\
               GRIDSIZE2)                                   \
{                                                           \
/* for every grid cell, output cell's stored farFace */     \
  for (i1=0; i1<GRIDSIZE1; i1++) {                          \
      for (j1=0; j1<GRIDSIZE2; j1++) {                      \
         thisGridCell = &(grid[i1][j1]);                    \
         if ( thisGridCell->startedFlag != 0) {             \
           /* if cell is covered by some zone */            \
           /* add far face of cell's farFaceZoneNo */       \
           /* if it has not already been output */          \
           zone = thisGridCell->farFaceZoneNo;              \
           if (!(int)(FFLAG)) {                             \
             /* printf("at end: adding farFace zone: %d\n", \
                        (int)ZONEID); */                    \
             FFLAG = 1;                                     \
             CHECKARRAYBOUNDS(*NUMQUADS,QUADS,CELLIDS)      \
             QUADS[currentQuad].Xmin = XMIN2;               \
             QUADS[currentQuad].Xmax = XMAX2;               \
             QUADS[currentQuad].Ymin = YMIN2;               \
             QUADS[currentQuad].Ymax = YMAX2;               \
             QUADS[currentQuad].Zmin = ZMIN2;               \
             QUADS[currentQuad].Zmax = ZMAX2;               \
             CELLIDS[currentQuad++] = ZONEID;               \
           }                                                \
         }                                                  \
      }                                                     \
  }                                                         \
}

struct gridCell {
  char startedFlag;
  int farFaceZoneNo;
  float maxFarFace;
};

void dealloc(struct gridCell **grid, int size1, int size2);
void allocate2Dgrid(struct gridCell ***grid, int size1, int size2);
void  calcGridSizeX( int *gridYsize, int *gridZsize, float *gridYmin, float *gridYmax, 
                     float *gridZmin, float *gridZmax, float *dataSetMinMaxCoord, 
                     float gridSpacing);
void  calcGridSizeY( int *gridXsize, int *gridZsize, float *gridXmin, float *gridXmax, 
                     float *gridZmin, float *gridZmax, float *dataSetMinMaxCoord, 
                     float gridSpacing);
void  calcGridSizeZ( int *gridXsize, int *gridYsize, float *gridXmin, float *gridXmax, 
                     float *gridYmin, float *gridYmax, float *dataSetMinMaxCoord, 
                     float gridSpacing);
void allocateQuads(struct quadRecord **Xquads, int **cellIDs, int newsize, int oldsize);

int setQuadsChunkSize(int nzones);
int compareX(const void *x, const void *y);
int compareY(const void *x, const void *y);
int compareZ(const void *x, const void *y);

int quadsChunkSize;
int totalMemory =0;

void  findExteriorQuads(int nzones, float *bb, float *dataSetMinMaxCoord, float minZoneWidth,
                         struct quadRecord **xquads, struct quadRecord **yquads,
                         struct quadRecord **zquads, int **xcellIDs, int **ycellIDs, 
                         int **zcellIDs, int *numXquads,int *numYquads,int *numZquads)
{
  float gridXmin,gridXmax,gridYmin,gridYmax,gridZmin,gridZmax,gridSpacing;
  int gridXsize,gridYsize, gridZsize;
  struct gridCell **grid, *thisGridCell;
  int currentQuad, zone, i1, j1;
  struct quadRecord *Xquads= NULL, *Yquads= NULL, *Zquads= NULL;
  int *XcellIDs= NULL, *YcellIDs= NULL, *ZcellIDs= NULL;

  // Sweep along each of the 3 principal axes, and generate list of exterior faces
  // for each axis, as well as 3 lists of the original cellID's for these faces.
  // Comments given below only for the sweep along the X axis.

  // do X sweep

  gridSpacing = minZoneWidth*GRIDFACTOR;
  calcGridSizeX(&gridYsize,&gridZsize,&gridYmin,&gridYmax,&gridZmin,&gridZmax,
                dataSetMinMaxCoord,gridSpacing);
    /* set grid sensitivity as fn of zone size */
  //PRINTGRIDSIZE(gridYsize,gridZsize,gridYmin,gridYmax,gridZmin,gridZmax)
  allocate2Dgrid(&grid,gridYsize,gridZsize);    /* create grid and initialize each cell's fields:
                                                   startedFlag=0 farFaceZoneNo = -1 
                                                   maxFarFace = -INF */
  //  PRINTGRID(gridYsize,gridZsize)
  quadsChunkSize = setQuadsChunkSize(nzones); /* we dont know in advance how many exterior quads, so
                                                 allocate quad array in chunks as needed; here we calc
                                                 chunk size as fn of nzones */
  *numXquads = quadsChunkSize;                /* *numXquads will hold total length of exterior face
                                                 array allocated so far */
  allocateQuads(&Xquads, &XcellIDs, *numXquads,0);  
   /* allocate space for exterior faces and cellIDs, initializing face fields: 
          Xmin=Xmax=Ymin=Ymax=ZminZmax=INF, XcellIDs initialized to -1.  */
  currentQuad = 0;
  //PRINTQUADS()
  qsort(bb,nzones, BB_DS_SIZE*sizeof(float), compareX); /* sort zone bb's by Xmin */
  //PRINTBB()
  //PRINTDATASETMINMAXCOORDS()

  for (zone=0; zone<nzones; zone++) {
    int min1idx,max1idx,min2idx,max2idx,savedZone;

    SNAP2GRIDX() // calculates min1idx,max1idx,min2idx,max2idx

    RUNSWEEP(XMIN,XMIN,YMIN,YMAX,ZMIN,ZMAX,
             XMAX,XMAX,YMIN,YMAX,ZMIN,ZMAX,XMIN,XMAX,
             Xquads,numXquads,XcellIDs,gridYsize,gridZsize)
  }
  ADD_FARFACES_IN_GRID(XMIN,XMIN,YMIN,YMAX,ZMIN,ZMAX,
                       XMAX,XMAX,YMIN,YMAX,ZMIN,ZMAX,XMIN,XMAX,
                       Xquads,numXquads,XcellIDs,gridYsize,gridZsize)
  *numXquads = currentQuad;
  /*
  printf("====================================================================\n");
  printf("\n%d XQuads:\n",*numXquads);
  for (i=0;i<*numXquads;i++) {
    printf("\tZone: %d  X: %f   Y: %f %f    Z: %f %f\n",XcellIDs[i],Xquads[i].Xmin,
           Xquads[i].Ymin,Xquads[i].Ymax,Xquads[i].Zmin,
           Xquads[i].Zmax);
  }
  printf("====================================================================\n");
  */
  //printf("\nTotal Memory Allocated for one sweep: %d bytes\n",totalMemory);

  // do Y sweep

  INITBB()
  dealloc(grid,gridYsize,gridZsize);
  calcGridSizeY(&gridXsize,&gridZsize,&gridXmin,&gridXmax,&gridZmin,&gridZmax,
                dataSetMinMaxCoord,gridSpacing);
  //PRINTGRIDSIZE(gridXsize,gridZsize,gridXmin,gridXmax,gridZmin,gridZmax)
  allocate2Dgrid(&grid,gridXsize,gridZsize);
  //  PRINTGRID(gridXsize,gridZsize)
  *numYquads = quadsChunkSize;
  allocateQuads(&Yquads, &YcellIDs, *numYquads,0);
  currentQuad = 0;
  //PRINTQUADS()
  qsort(bb,nzones, BB_DS_SIZE*sizeof(float), compareY);
  //PRINTBB()
  //PRINTDATASETMINMAXCOORDS()

  for (zone=0; zone<nzones; zone++) {
    int min1idx,max1idx,min2idx,max2idx,savedZone;

    SNAP2GRIDY()
    RUNSWEEP(XMIN,XMAX,YMIN,YMIN,ZMIN,ZMAX,
             XMIN,XMAX,YMAX,YMAX,ZMIN,ZMAX,YMIN,YMAX,
             Yquads,numYquads,YcellIDs,gridXsize,gridZsize)
  }
  ADD_FARFACES_IN_GRID(XMIN,XMAX,YMIN,YMIN,ZMIN,ZMAX,
                       XMIN,XMAX,YMAX,YMAX,ZMIN,ZMAX,YMIN,YMAX,
                       Yquads,numYquads,YcellIDs,gridXsize,gridZsize)
  *numYquads = currentQuad;
  /*
  printf("====================================================================\n");
  printf("\n%d YQuads:\n",*numYquads);
  for (i=0;i<*numYquads;i++) {
    printf("\tZone: %d  X: %f %f   Z: %f %f    Y:  %f\n",YcellIDs[i],Yquads[i].Xmin,
           Yquads[i].Xmax,  Yquads[i].Zmin, Yquads[i].Zmax, Yquads[i].Ymax);
  }
  printf("====================================================================\n");
  */

  // do Z sweep

  INITBB()
  dealloc(grid,gridXsize,gridZsize);
  calcGridSizeZ(&gridXsize,&gridYsize,&gridXmin,&gridXmax,&gridYmin,&gridYmax,
                dataSetMinMaxCoord,gridSpacing);
  //PRINTGRIDSIZE(gridXsize,gridYsize,gridXmin,gridXmax,gridYmin,gridYmax)
  allocate2Dgrid(&grid,gridXsize,gridYsize);
  //  PRINTGRID(gridXsize,gridYsize)
  *numZquads = quadsChunkSize;
  allocateQuads(&Zquads, &ZcellIDs, *numZquads,0);
  currentQuad = 0;
  //PRINTQUADS()
  qsort(bb,nzones, BB_DS_SIZE*sizeof(float), compareZ);
  //PRINTBB()
  //PRINTDATASETMINMAXCOORDS()

  for (zone=0; zone<nzones; zone++) {
    int min1idx,max1idx,min2idx,max2idx,savedZone;

    SNAP2GRIDZ()
    RUNSWEEP(XMIN,XMAX,YMIN,YMAX,ZMIN,ZMIN,
             XMIN,XMAX,YMIN,YMAX,ZMAX,ZMAX,ZMIN,ZMAX,
             Zquads,numZquads,ZcellIDs,gridXsize,gridYsize)
  }
  ADD_FARFACES_IN_GRID(XMIN,XMAX,YMIN,YMAX,ZMIN,ZMIN,
                       XMIN,XMAX,YMIN,YMAX,ZMAX,ZMAX,ZMIN,ZMAX,
                       Zquads,numZquads,ZcellIDs,gridXsize,gridYsize)
  *numZquads = currentQuad;
  /*
  printf("====================================================================\n");
  printf("\n%d ZQuads:\n",*numZquads);
  for (i=0;i<*numZquads;i++) {
    printf("\tZone: %d  X: %f %f   Y: %f %f    Z:  %f\n",ZcellIDs[i],Zquads[i].Xmin,
           Zquads[i].Xmax,  Zquads[i].Ymin, Zquads[i].Ymax, Zquads[i].Zmax);
  }
  printf("====================================================================\n");
  */
  *xquads = Xquads; *yquads = Yquads; *zquads = Zquads;
  *xcellIDs = XcellIDs; *ycellIDs = YcellIDs; *zcellIDs = ZcellIDs; 
}

void allocateQuads(struct quadRecord **quads, int **cellIDs, int newsize, int oldsize) {
  int i,memsize;

    memsize = sizeof(struct quadRecord) * newsize;
    if ((*quads = (struct quadRecord *)realloc(*quads,memsize)) == 0) {
       debug1 << "Insufficient memory for realloc of " << newsize << " quads" << endl;
       EXCEPTION0(ImproperUseException);
    }
    totalMemory += memsize;
    //printf("realloc'd %d bytes for %d quads\n",memsize,newsize);
    for (i=oldsize;i<newsize;i++) {
      (*quads)[i].Xmin=INF;
      (*quads)[i].Xmax=INF;
      (*quads)[i].Ymin=INF;
      (*quads)[i].Ymax=INF;
      (*quads)[i].Zmin=INF;
      (*quads)[i].Zmax=INF;
    }
    memsize = sizeof(int) * newsize;
    if ((*cellIDs = (int *)realloc(*cellIDs,memsize)) == 0) {
       debug1 << "Insufficient memory for realloc of " << newsize << " quads" << endl;
       EXCEPTION0(ImproperUseException);
    }
    totalMemory += memsize;
    //printf("realloc'd %d bytes for %d cellIDs\n",memsize,newsize);
    for (i=oldsize;i<newsize;i++) {
      (*cellIDs)[i]=-1;
    }
}

#define DSxMIN dataSetMinMaxCoord[0]
#define DSxMAX dataSetMinMaxCoord[1]
#define DSyMIN dataSetMinMaxCoord[2]
#define DSyMAX dataSetMinMaxCoord[3]
#define DSzMIN dataSetMinMaxCoord[4]
#define DSzMAX dataSetMinMaxCoord[5]

void  calcGridSizeX( int *gridYsize, int *gridZsize, float *gridYmin, float *gridYmax, 
                     float *gridZmin, float *gridZmax, float *dataSetMinMaxCoord, 
                     float gridSpacing) {
  *gridYsize = (int) ((DSyMAX-DSyMIN)/gridSpacing)+1;
  *gridZsize = (int) ((DSzMAX-DSzMIN)/gridSpacing)+1;
  *gridYmin = DSyMIN;
  *gridYmax = *gridYmin + *gridYsize*gridSpacing;
  *gridZmin = DSzMIN;
  *gridZmax = *gridZmin + *gridZsize*gridSpacing;
}

void  calcGridSizeY( int *gridXsize, int *gridZsize, float *gridXmin, float *gridXmax, 
                     float *gridZmin, float *gridZmax, float *dataSetMinMaxCoord, 
                     float gridSpacing) {
  *gridXsize = (int) ((DSxMAX-DSxMIN)/gridSpacing)+1;
  *gridZsize = (int) ((DSzMAX-DSzMIN)/gridSpacing)+1;
  *gridXmin = DSxMIN;
  *gridXmax = *gridXmin + *gridXsize*gridSpacing;
  *gridZmin = DSzMIN;
  *gridZmax = *gridZmin + *gridZsize*gridSpacing;
}


void  calcGridSizeZ( int *gridXsize, int *gridYsize, float *gridXmin, float *gridXmax, 
                     float *gridYmin, float *gridYmax, float *dataSetMinMaxCoord, 
                     float gridSpacing) {
  *gridXsize = (int) ((DSxMAX-DSxMIN)/gridSpacing)+1;
  *gridYsize = (int) ((DSyMAX-DSyMIN)/gridSpacing)+1;
  *gridXmin = DSxMIN;
  *gridXmax = *gridXmin + *gridXsize*gridSpacing;
  *gridYmin = DSyMIN;
  *gridYmax = *gridYmin + *gridYsize*gridSpacing;
}

void dealloc(struct gridCell **grid, int size1, int size2) {
  int indx;
  for (indx=0; indx< size1; indx++) free( grid[indx]);
  free(grid);
}

void allocate2Dgrid(struct gridCell ***grid, int size1, int size2) {
  int size,indx;
  size = sizeof(grid) * size1;
  if ((*grid = (struct gridCell **)malloc(size)) == 0) {
     debug1 << "Insufficient memory for malloc of grid** of  size1 = "
            << size << endl;
     EXCEPTION0(ImproperUseException);
  }
  totalMemory += size;
  //printf("malloc'd %d bytes for **grid of size1 = %d\n",size,size1);
  size = sizeof(struct gridCell) * size2;
  for (indx=0; indx< size1; indx++){
    if (((*grid)[indx] = (struct gridCell *)malloc(size)) == 0) {
       debug1 << "Insufficient memory for malloc of grid[" << indx
              << "] of size2  = " << size2 << endl;
       EXCEPTION0(ImproperUseException);
    }
    totalMemory += size;
    //printf("malloc'd %d bytes for grid[%d] of size2 = %d\n",size,indx,size2);
  }
  {int i,j;
    for (i=0; i < size1; i++) {
      for (j=0; j < size2; j++) {
         ((*grid)[i][j]).startedFlag=0;
         ((*grid)[i][j]).farFaceZoneNo = -1;
         ((*grid)[i][j]).maxFarFace = -INF;
      }
    }
  }
}

int setQuadsChunkSize(int nzones) {
  int size;
  size = (int)(CHUNKFACTOR * nzones);
  if (size < 1000) { size = 1000;}
  return size;
}

int compareX(const void *xv, const void *yv) {
   float *x = (float *) xv;
   float *y = (float *) yv;
   if(x[0] < y[0]) return -1;
   if(x[0] > y[0]) return 1;
   return 0;
}

int compareY(const void *xv, const void *yv) {
   float *x = (float *) xv;
   float *y = (float *) yv;
   if(x[2] < y[2]) return -1;
   if(x[2] > y[2]) return 1;
   return 0;
}

int compareZ(const void *xv, const void *yv) {
   float *x = (float *) xv;
   float *y = (float *) yv;
   if(x[4] < y[4]) return -1;
   if(x[4] > y[4]) return 1;
   return 0;
}




