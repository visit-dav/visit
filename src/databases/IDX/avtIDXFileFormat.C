/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// *************************************************************************
//                           avtIDXFileFormat.C
// *************************************************************************

#include <avtIDXFileFormat.h>

#include <string>
#include <sstream>

#include <vtkFloatArray.h>
#include <vtkTypeFloat32Array.h>
#include <vtkDoubleArray.h>
#include <vtkCharArray.h>
#include <vtkShortArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkCellType.h>
#include <vtkCellData.h>
#include <vtkLongArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkSmartPointer.h>
#include <DebugStream.h>
#include <vtkHexahedron.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix4x4.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <avtResolutionSelection.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtVariableCache.h>
#include <avtParallel.h>
#include <avtDatabaseMetaData.h>
#include <avtMultiresSelection.h>
#include <avtStructuredDomainNesting.h>
#include <avtCallback.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>

#ifdef _WIN32
#include <win_dirent.h>
#else
#include <dirent.h>
#endif

#include "uintah_utils.h"
#include "visit_idx_utils.h"

#ifdef PARALLEL
#include <avtParallel.h>
#endif

typedef std::string String;

#define USE_AMR 1

using namespace VisitIDXIO;
using namespace std;

bool computeDivisor(int x, int &divisor) {
  int upperBound = std::sqrt(x);
  for (int i = 2; i <= upperBound; ++i) {
    if (x % i == 0) {
      divisor = i;
      return true;
    }
  }
  return false;
}

std::vector<int> computeGrid(int num) {
  std::vector<int> grid(3);
  grid[0] = 1;
  grid[1] = 1;
  grid[2] = 1;
  int axis = 0;
  int divisor = 0;
  while (computeDivisor(num, divisor)) {
    grid[axis] *= divisor;
    num /= divisor;
    axis = (axis + 1) % 3;
  }
  if (num != 1) {
    grid[axis] *= num;
  }
  return grid;
}

void avtIDXFileFormat::domainDecomposition(){

#if 1
  level_info.patchInfo.clear();

  int n = nprocs;
  int b = input_patches.patchInfo.size();

  std::vector<int> global_size = reader->getGlobalSize();

  if (n < b){
    b = n;
    level_info.patchInfo.clear();

    int low[3]={0,0,0};
    int high[3]={global_size[0]-1,global_size[1]-1,global_size[2]-1};
    int eCells[6]={0,0,0,0,0,0};
    PatchInfo box;
    box.setBounds(low,high,eCells,grid_type);
    level_info.patchInfo.push_back(box);
  }

  int c = b > n ? b/n : n/b; // how many patches per box
  std::vector<PatchInfo> newboxes;

  // need to consider extra cells?
  //printf("%d: n %d b %d c %d\n", rank, n,b,c);
  // printf("global %d %d %d\n", global_size[0],global_size[1],global_size[2]);

  for(int i=0; i < input_patches.patchInfo.size(); i++){

    std::vector<int> block_decomp = computeGrid(c);

    // printf("decomposing box %d in [%d %d %d]\n", i, block_decomp[0],block_decomp[1],block_decomp[2]);

    const PatchInfo& box = input_patches.patchInfo[i];
    int box_low[3];
    int box_high[3];
    int eCells[6];

    box.getBounds(box_low,box_high,eCells,grid_type);

    int box_dim[3] = {box_high[0]-box_low[0],box_high[1]-box_low[1],box_high[2]-box_low[2]};  // should be +1
    int block_dim[3] = {box_dim[0]/block_decomp[0],box_dim[1]/block_decomp[1],box_dim[2]/block_decomp[2]};

    //printf("block dim [%d %d %d]\n", block_dim[0],block_dim[1],block_dim[2]);
    //printf("box low [%d %d %d] [%d %d %d]\n", box_low[0],box_low[1],box_low[2],box_high[0],box_high[1],box_high[2]);
    for(int nb=0; nb<c; nb++){

      int bid[3] = {nb % block_decomp[0], (nb / block_decomp[0]) % block_decomp[1], nb / (block_decomp[0] * block_decomp[1])};
      int curr_p1[3] = {box_low[0]+(bid[0]*block_dim[0]),box_low[1]+(bid[1]*block_dim[1]),box_low[2]+(bid[2]*block_dim[2])};
      int curr_p2[3] = {curr_p1[0]+block_dim[0], curr_p1[1]+block_dim[1], curr_p1[2]+block_dim[2]};

      for(int d=0; d <3; d++){
        //curr_p1[d] = curr_p1[d] > 0 ? curr_p1[d]-1 : curr_p1[d];
        //curr_p2[d] = (curr_p2[d] < global_size[d]-1) ? curr_p2[d]+1 : global_size[d]-1;
        curr_p2[d] = (curr_p2[d] < global_size[d]-1) ? curr_p2[d] : global_size[d];
      }

      PatchInfo newbox;
      newbox.setBounds(curr_p1,curr_p2,eCells,grid_type);
      newboxes.push_back(newbox);
    }

  }

#else

    int maxdir = 0; // largest extent axis
    int maxextent = 0;
    int maxbox = 0;
    
    for(int i=0; i < level_info.patchInfo.size(); i++){
      PatchInfo& box = level_info.patchInfo[i];
      int box_low[3];
      int box_high[3];

      box.getBounds(box_low,box_high,"CC");

      for(int j=0; j < 3; j++){
        int extent = box_high[j]-box_low[j];
        if(extent > maxextent){
          maxdir = j;
          maxextent = extent;
          maxbox = i;
        }
      }
    }
    
    //std::cout << "max dir " << maxdir << " max extent " << maxextent << " box " << maxbox;
    //printf("NUM PROCS %d patches %d\n", nprocs, level_info.patchInfo.size());
    std::vector<PatchInfo> newboxes;
    int n = nprocs;
    int b = level_info.patchInfo.size();
    int c = b > n ? b/n : n/b; // how many patches per box
    int d = b > n ? b%n : n%b;

    //printf("Trying to use %d patches per core, res %d\n", c, d);

    float h[b];
    int res[b];
    int slabs[b];
    //  fprintf(stderr,"n %d b %d c %d d %d\n",n,b,c,d);

//    if(d == 0){
      for(int i=0; i<b; i++){
        PatchInfo& box = level_info.patchInfo[i];
        int box_low[3];
        int box_high[3];

        box.getBounds(box_low, box_high, "CC");

        int extent = box_high[maxdir]-box_low[maxdir]+1;

        h[i] = (float)extent/c;
        slabs[i]=c;
        res[i]=extent % int(h[i]*c);

        //  printf("Even H[%d] = %d xslabs %d res %d\n", i, h[i], slabs[i], res[i]);

      }

    for(int i=0; i < b; i++){
      PatchInfo& box = level_info.patchInfo[i];
      int box_low[3];
      int box_high[3];

      int low[3];
      int high[3];
      int eCells[6];

      box.getBounds(box_low,box_high,eCells,"CC");

      memcpy(low, box_low, 3*sizeof(int));
      memcpy(high, box_high, 3*sizeof(int));

      int n_slabs = slabs[i];//floor((foat)(box_high[maxdir]-box_low[maxdir]) / h[i]);

      int residual = res[i];

      //printf("n_slabs %d residual %d\n", n_slabs, res[i]);

      int part_p1 = box_low[maxdir];
      int part_p2 = box_low[maxdir] + h[i] -1;

      int added_boxes = 0;
      while(added_boxes < n_slabs){

        low[maxdir] = part_p1 > 0 ? part_p1-1 : part_p1;
        high[maxdir] = (part_p2 < box_high[maxdir]) ? part_p2+1 : part_p2;

        PatchInfo newbox;

        newbox.setBounds(low,high,eCells,"CC");
        newboxes.push_back(newbox);
        added_boxes++;

        part_p1 = high[maxdir];
        part_p2 = round(part_p1 + h[i]) -1;

      }

      if(residual > 0 || (high[maxdir] < box_high[maxdir])){
        int lowr[3];
        int highr[3];

        PatchInfo& last = newboxes.back();
        last.getBounds(lowr,highr,eCells,"CC");

        highr[maxdir] = box_high[maxdir];//residual;

        last.setBounds(lowr,highr,eCells,"CC");

      }

    }
#endif

  level_info.patchInfo=newboxes;

  // if(rank == 0){ 
  // std::cout<< "Total number of boxes/domains: " << level_info.patchInfo.size() << std::endl<< std::flush;
  // std::cout << "----------Boxes----------" << std::endl<< std::flush;
  //   for(int i=0; i< level_info.patchInfo.size(); i++){
  //     std::cout << i << " = "<<level_info.patchInfo[i].toString();
  //   }
  //   std::cout << "-------------------------" << std::endl<< std::flush;
  //   }

  if(level_info.patchInfo.size() % nprocs != 0){
    fprintf(stderr,"ERROR: wrong domain decomposition, patches %d procs %d\n", level_info.patchInfo.size(), nprocs);
    assert(false);
  }
}

void avtIDXFileFormat::createBoxes(){

  size_t found = dataset_filename.find_last_of("/\\");
  String folder = dataset_filename.substr(0,found);

  String upsfilename = "noupsfile.ups";
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir (folder.c_str())) != NULL) {
      /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      String name(ent->d_name);
      if(name.substr(name.find_last_of(".") + 1) == "ups"){
        upsfilename = name;
        debug4 << ".ups file found " << upsfilename << std::endl;
        upsfilename = folder + "/" +upsfilename;
        break;
      }
    }
    closedir (dir);
  } else {
    debug4 << "No .ups file found" << std::endl;
  }

  vtkSmartPointer<vtkXMLDataParser> parser = vtkSmartPointer<vtkXMLDataParser>::New();

  upsfilename.replace(upsfilename.end()-3, upsfilename.end(),"ups");

  uintah_metadata = false;

  parser->SetFileName(upsfilename.c_str());

  if (!parser->Parse()){
    debug4 << "No .ups file found (Uintah only)" << std::endl;

    parser->SetFileName(metadata_filename.c_str());

    if (!parser->Parse()){
      debug4 << "Single-box mode" << std::endl;
    }
  }else{
    uintah_metadata = true;
    debug4 << "Multi-box mode" << std::endl;
  }

  if(uintah_metadata){
    /* vtkSmartPointer<vtkXMLDataParser> timestep_parser = vtkSmartPointer<vtkXMLDataParser>::New();
    size_t folder_point = dataset_filename.find_last_of("/\\");                                                                         String folder = dataset_filename.substr(0,folder_point);
  
    String timestep_filename = folder + "/timestep.xml";
    timestep_parser->SetFileName(timestep_filename.c_str());
    
    if (!timestep_parser->Parse() ){
      debug4 << "NOT found timestep .xml" <<std::endl;
    }
    else {
      debug4 << "found timestep.xml in " << timestep_filename.c_str()<<std::endl;
      parse_timestep(timestep_parser, input_patches, dim, use_extracells);
    }

    // Note: parse ups after timeste.xml so it will use the anchor
    */
    //printf("USING EXTRA CELLS %d\n", use_extracells);

    parse_ups(parser, input_patches, dim, use_extracells, grid_type);

#if 0
    input_patches.patchInfo.clear();

    std::vector<std::vector<int> > lows;
    std::vector<std::vector<int> > highs;

    std::vector<int> t(3);


    t[0]=0; t[1]=0; t[2]=0; lows.push_back(t); t[0]=36; t[1]=6; t[2]=6; highs.push_back(t);
t[0]=0; t[1]=0; t[2]=6; lows.push_back(t); t[0]=36; t[1]=6; t[2]=12; highs.push_back(t);
t[0]=0; t[1]=0; t[2]=12; lows.push_back(t); t[0]=36; t[1]=6; t[2]=18; highs.push_back(t);
t[0]=0; t[1]=0; t[2]=18; lows.push_back(t); t[0]=36; t[1]=6; t[2]=24; highs.push_back(t);
t[0]=0; t[1]=0; t[2]=24; lows.push_back(t); t[0]=36; t[1]=6; t[2]=30; highs.push_back(t);
t[0]=0; t[1]=6; t[2]=0; lows.push_back(t); t[0]=36; t[1]=12; t[2]=6; highs.push_back(t);
t[0]=0; t[1]=6; t[2]=6; lows.push_back(t); t[0]=36; t[1]=12; t[2]=12; highs.push_back(t);
t[0]=0; t[1]=6; t[2]=12; lows.push_back(t); t[0]=36; t[1]=12; t[2]=18; highs.push_back(t);
t[0]=0; t[1]=6; t[2]=18; lows.push_back(t); t[0]=36; t[1]=12; t[2]=24; highs.push_back(t);
t[0]=0; t[1]=6; t[2]=24; lows.push_back(t); t[0]=36; t[1]=12; t[2]=30; highs.push_back(t);
t[0]=0; t[1]=12; t[2]=0; lows.push_back(t); t[0]=36; t[1]=18; t[2]=6; highs.push_back(t);
t[0]=0; t[1]=12; t[2]=6; lows.push_back(t); t[0]=36; t[1]=18; t[2]=12; highs.push_back(t);
t[0]=0; t[1]=12; t[2]=12; lows.push_back(t); t[0]=36; t[1]=18; t[2]=18; highs.push_back(t);
t[0]=0; t[1]=12; t[2]=18; lows.push_back(t); t[0]=36; t[1]=18; t[2]=24; highs.push_back(t);
t[0]=0; t[1]=12; t[2]=24; lows.push_back(t); t[0]=36; t[1]=18; t[2]=30; highs.push_back(t);
t[0]=0; t[1]=18; t[2]=0; lows.push_back(t); t[0]=36; t[1]=24; t[2]=6; highs.push_back(t);
t[0]=0; t[1]=18; t[2]=6; lows.push_back(t); t[0]=36; t[1]=24; t[2]=12; highs.push_back(t);
t[0]=0; t[1]=18; t[2]=12; lows.push_back(t); t[0]=36; t[1]=24; t[2]=18; highs.push_back(t);
t[0]=0; t[1]=18; t[2]=18; lows.push_back(t); t[0]=36; t[1]=24; t[2]=24; highs.push_back(t);
t[0]=0; t[1]=18; t[2]=24; lows.push_back(t); t[0]=36; t[1]=24; t[2]=30; highs.push_back(t);
t[0]=0; t[1]=24; t[2]=0; lows.push_back(t); t[0]=36; t[1]=30; t[2]=6; highs.push_back(t);
t[0]=0; t[1]=24; t[2]=6; lows.push_back(t); t[0]=36; t[1]=30; t[2]=12; highs.push_back(t);
t[0]=0; t[1]=24; t[2]=12; lows.push_back(t); t[0]=36; t[1]=30; t[2]=18; highs.push_back(t);
t[0]=0; t[1]=24; t[2]=18; lows.push_back(t); t[0]=36; t[1]=30; t[2]=24; highs.push_back(t);
t[0]=0; t[1]=24; t[2]=24; lows.push_back(t); t[0]=36; t[1]=30; t[2]=30; highs.push_back(t);
t[0]=36; t[1]=6; t[2]=6; lows.push_back(t); t[0]=42; t[1]=12; t[2]=12; highs.push_back(t);
t[0]=36; t[1]=6; t[2]=12; lows.push_back(t); t[0]=42; t[1]=12; t[2]=18; highs.push_back(t);
t[0]=36; t[1]=6; t[2]=18; lows.push_back(t); t[0]=42; t[1]=12; t[2]=24; highs.push_back(t);
t[0]=36; t[1]=12; t[2]=6; lows.push_back(t); t[0]=42; t[1]=18; t[2]=12; highs.push_back(t);
t[0]=36; t[1]=12; t[2]=12; lows.push_back(t); t[0]=42; t[1]=18; t[2]=18; highs.push_back(t);
t[0]=36; t[1]=12; t[2]=18; lows.push_back(t); t[0]=42; t[1]=18; t[2]=24; highs.push_back(t);
t[0]=36; t[1]=18; t[2]=6; lows.push_back(t); t[0]=42; t[1]=24; t[2]=12; highs.push_back(t);
t[0]=36; t[1]=18; t[2]=12; lows.push_back(t); t[0]=42; t[1]=24; t[2]=18; highs.push_back(t);
t[0]=36; t[1]=18; t[2]=18; lows.push_back(t); t[0]=42; t[1]=24; t[2]=24; highs.push_back(t);
t[0]=36; t[1]=24; t[2]=6; lows.push_back(t); t[0]=42; t[1]=30; t[2]=12; highs.push_back(t);
t[0]=36; t[1]=24; t[2]=12; lows.push_back(t); t[0]=42; t[1]=30; t[2]=18; highs.push_back(t);
t[0]=36; t[1]=24; t[2]=18; lows.push_back(t); t[0]=42; t[1]=30; t[2]=24; highs.push_back(t);
t[0]=36; t[1]=0; t[2]=6; lows.push_back(t); t[0]=42; t[1]=6; t[2]=12; highs.push_back(t);
t[0]=36; t[1]=0; t[2]=12; lows.push_back(t); t[0]=42; t[1]=6; t[2]=18; highs.push_back(t);
t[0]=36; t[1]=0; t[2]=18; lows.push_back(t); t[0]=42; t[1]=6; t[2]=24; highs.push_back(t);
t[0]=36; t[1]=6; t[2]=24; lows.push_back(t); t[0]=42; t[1]=12; t[2]=30; highs.push_back(t);
t[0]=36; t[1]=12; t[2]=24; lows.push_back(t); t[0]=42; t[1]=18; t[2]=30; highs.push_back(t);
t[0]=36; t[1]=18; t[2]=24; lows.push_back(t); t[0]=42; t[1]=24; t[2]=30; highs.push_back(t);
t[0]=36; t[1]=6; t[2]=0; lows.push_back(t); t[0]=42; t[1]=12; t[2]=6; highs.push_back(t);
t[0]=36; t[1]=12; t[2]=0; lows.push_back(t); t[0]=42; t[1]=18; t[2]=6; highs.push_back(t);
t[0]=36; t[1]=18; t[2]=0; lows.push_back(t); t[0]=42; t[1]=24; t[2]=6; highs.push_back(t);
//     t={0,0,0}; lows.push_back(t); t={36,6,6}; highs.push_back(t);
// t={0,0,6}; lows.push_back(t); t={36,6,12}; highs.push_back(t);
// t={0,0,12}; lows.push_back(t); t={36,6,18}; highs.push_back(t);
// t={0,0,18}; lows.push_back(t); t={36,6,24}; highs.push_back(t);
// t={0,0,24}; lows.push_back(t); t={36,6,30}; highs.push_back(t);
// t={0,6,0}; lows.push_back(t); t={36,12,6}; highs.push_back(t);
// t={0,6,6}; lows.push_back(t); t={36,12,12}; highs.push_back(t);
// t={0,6,12}; lows.push_back(t); t={36,12,18}; highs.push_back(t);
// t={0,6,18}; lows.push_back(t); t={36,12,24}; highs.push_back(t);
// t={0,6,24}; lows.push_back(t); t={36,12,30}; highs.push_back(t);
// t={0,12,0}; lows.push_back(t); t={36,18,6}; highs.push_back(t);
// t={0,12,6}; lows.push_back(t); t={36,18,12}; highs.push_back(t);
// t={0,12,12}; lows.push_back(t); t={36,18,18}; highs.push_back(t);
// t={0,12,18}; lows.push_back(t); t={36,18,24}; highs.push_back(t);
// t={0,12,24}; lows.push_back(t); t={36,18,30}; highs.push_back(t);
// t={0,18,0}; lows.push_back(t); t={36,24,6}; highs.push_back(t);
// t={0,18,6}; lows.push_back(t); t={36,24,12}; highs.push_back(t);
// t={0,18,12}; lows.push_back(t); t={36,24,18}; highs.push_back(t);
// t={0,18,18}; lows.push_back(t); t={36,24,24}; highs.push_back(t);
// t={0,18,24}; lows.push_back(t); t={36,24,30}; highs.push_back(t);
// t={0,24,0}; lows.push_back(t); t={36,30,6}; highs.push_back(t);
// t={0,24,6}; lows.push_back(t); t={36,30,12}; highs.push_back(t);
// t={0,24,12}; lows.push_back(t); t={36,30,18}; highs.push_back(t);
// t={0,24,18}; lows.push_back(t); t={36,30,24}; highs.push_back(t);
// t={0,24,24}; lows.push_back(t); t={36,30,30}; highs.push_back(t);
// t={36,6,6}; lows.push_back(t); t={42,12,12}; highs.push_back(t);
// t={36,6,12}; lows.push_back(t); t={42,12,18}; highs.push_back(t);
// t={36,6,18}; lows.push_back(t); t={42,12,24}; highs.push_back(t);
// t={36,12,6}; lows.push_back(t); t={42,18,12}; highs.push_back(t);
// t={36,12,12}; lows.push_back(t); t={42,18,18}; highs.push_back(t);
// t={36,12,18}; lows.push_back(t); t={42,18,24}; highs.push_back(t);
// t={36,18,6}; lows.push_back(t); t={42,24,12}; highs.push_back(t);
// t={36,18,12}; lows.push_back(t); t={42,24,18}; highs.push_back(t);
// t={36,18,18}; lows.push_back(t); t={42,24,24}; highs.push_back(t);
// t={36,24,6}; lows.push_back(t); t={42,30,12}; highs.push_back(t);
// t={36,24,12}; lows.push_back(t); t={42,30,18}; highs.push_back(t);
// t={36,24,18}; lows.push_back(t); t={42,30,24}; highs.push_back(t);
// t={36,0,6}; lows.push_back(t); t={42,6,12}; highs.push_back(t);
// t={36,0,12}; lows.push_back(t); t={42,6,18}; highs.push_back(t);
// t={36,0,18}; lows.push_back(t); t={42,6,24}; highs.push_back(t);
// t={36,6,24}; lows.push_back(t); t={42,12,30}; highs.push_back(t);
// t={36,12,24}; lows.push_back(t); t={42,18,30}; highs.push_back(t);
// t={36,18,24}; lows.push_back(t); t={42,24,30}; highs.push_back(t);
// t={36,6,0}; lows.push_back(t); t={42,12,6}; highs.push_back(t);
// t={36,12,0}; lows.push_back(t); t={42,18,6}; highs.push_back(t);
// t={36,18,0}; lows.push_back(t); t={42,24,6}; highs.push_back(t);


  for(int i=0; i< lows.size(); i++){
    PatchInfo box;
    int eCells[6]= {0,0,0,0,0,0};
    box.setBounds(&lows[i][0],&highs[i][0],eCells, grid_type);

    input_patches.patchInfo.push_back(box);

  }
#endif 

  }
  else{
    Box log_box = reader->getLogicBox();
    int low[3];
    int high[3];
    int eCells[6] = {0,0,0,0,0,0};
    use_extracells = false;
    grid_type = "CC";
    mesh_name = "CC_Mesh";

    debug1 << "Disabling extra cells (no Uintah)" << std::endl;
 
    low[0] = (int)log_box.p1[0];
    low[1] = (int)log_box.p1[1];
    low[2] = (int)log_box.p1[2];
    high[0] = (int)log_box.p2[0]-1;
    high[1] = (int)log_box.p2[1]-1;
    high[2] = (int)log_box.p2[2]-1;

    PatchInfo box;
    box.setBounds(low,high,eCells,grid_type);
    input_patches.patchInfo.clear();
    input_patches.patchInfo.push_back(box);

    for(int k=0; k<3; k++){
      input_patches.spacing[k]= 1.f;
      input_patches.anchor[k] = 0.f;
    }

    debug4 << "Single Box: ";
    debug4 << input_patches.patchInfo.back().toString();

  }

  debug4 << "anchor point " << input_patches.anchor[0] <<","<< input_patches.anchor[1] <<","<< input_patches.anchor[2] <<std::endl;
}

void avtIDXFileFormat::createTimeIndex(){
  timeIndex.clear();
  logTimeIndex.clear();

  // try to get timesteps metadata from uintah index.xml file
  vtkSmartPointer<vtkXMLDataParser> parser = vtkSmartPointer<vtkXMLDataParser>::New();
  size_t folder_point = dataset_filename.find_last_of("/\\");
  String folder = dataset_filename.substr(0,folder_point);

  bool time_from_uintah = false;
  String udafilename = folder + "/index.xml";
  debug5 <<"looking for index.xml here " << udafilename.c_str() << std::endl;

  parser->SetFileName(udafilename.c_str());
  if (parser->Parse()){
    parser->SetFileName(metadata_filename.c_str());
  
    debug4 << udafilename << " file found" << std::endl;

    vtkXMLDataElement *root = parser->GetRootElement();
    vtkXMLDataElement *tsteps = root->FindNestedElementWithName("timesteps");
    if(tsteps != NULL){
      time_from_uintah=true;
      int ntimesteps = tsteps->GetNumberOfNestedElements();

      debug4 << "Found " << ntimesteps << " timesteps" << std::endl;

      for(int i=0; i < ntimesteps; i++){

        vtkXMLDataElement *xmltime = tsteps->GetNestedElement(i);
        String timestr(xmltime->GetAttribute("time"));
        String logtimestr(xmltime->GetCharacterData());

        debug4 << "time " << timestr << " index " << logtimestr << std::endl;
       
        double time = cdouble(timestr);
        int logtime = cint(logtimestr);

        logTimeIndex.push_back(logtime);

        timeIndex.push_back(time);
      }
    }
    else
      fprintf(stderr, "No timesteps field found in index.xml, no physical time available\n");
  }

  if(!time_from_uintah){
    std::vector<double> times = reader->getTimes();
    debug4 << "adding " << times.size() << " timesteps " << std::endl;

    if(is_gidx){
        for(int i=0; i< gidx_datasets.size(); i++){
         timeIndex.push_back(gidx_datasets[i].log_time);
         logTimeIndex.push_back(gidx_datasets[i].log_time);            
           } 
      }else{
      for(int i=0; i< times.size(); i++){
        timeIndex.push_back(times.at(i));
        logTimeIndex.push_back(times.at(i));            
      }
    }
  }

  debug4 << "loaded " << timeIndex.size() << " timesteps"<< std::endl;
  debug4 << reader->getNTimesteps() << " in the timesteps range of the IDX file" << std::endl;

      //if(timeIndex.size() != reader->getNTimesteps())
      //  std::cout << "ERROR: the timesteps in the IDX file and in the index.xml are not consistent!\n You will not be able to use the physical time"<< std::endl;

  return;

}

// ****************************************************************************
//  Method: avtIDXFileFormat constructor
//
//  Programmer: bremer5 -- generated by xml2avt
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

avtIDXFileFormat::avtIDXFileFormat(const char *filename, DBOptionsAttributes* attrs)
: avtMTMDFileFormat(filename)
{

  for (int i=0; attrs!=0 && i<attrs->GetNumberOfOptions(); ++i) {
       // printf("reading option %d %s\n",i, attrs->GetName(i).c_str() );
    if (attrs->GetName(i) == "Big Endian") {
      reverse_endian = attrs->GetBool("Big Endian");
    }
    else if (attrs->GetName(i) == "Use extra cells") {
      use_extracells = attrs->GetBool("Use extra cells");
    }else if (attrs->GetName(i) == "Use RAW format") {
      use_raw = attrs->GetBool("Use RAW format");
    }
  }

  debug4 << "--------------------------" << std::endl;
  if(use_extracells)
    debug4 << "Using extra cells" << std::endl;
  else
    debug4 << "Not using extra cells" << std::endl;

  if(use_raw)
    debug4 << "Using RAW format" << std::endl;
  else
    debug4 << "Not using RAW format" << std::endl;

  if(reverse_endian)
    debug4 << "Using Big Endian" << std::endl;
  else
    debug4 << "Using Little Endian" << std::endl;
  debug4 << "--------------------------" << std::endl;

#ifdef PARALLEL
  debug4 << "THIS IS PARALLEL AVT"<< std::endl;
  rank = PAR_Rank();
  nprocs = PAR_Size();
#else
  debug4 << "THIS IS NOT PARALLEL AVT"<< std::endl;
  rank = 0;
  nprocs = 1;
#endif
  
#ifdef USE_VISUS
    reader = new VisusIDXIO(); // USE VISUS
#else
    reader = new PIDXIO();     // USE PIDX
#endif
    
    dataset_filename = filename;

    size_t folder_point = dataset_filename.find_last_of("/\\");
    size_t ext_point = dataset_filename.find_last_of(".");
    String extension = dataset_filename.substr(ext_point+1, dataset_filename.size());
    is_gidx = extension.compare("gidx") == 0;

    memset(sfc_offset,0,sizeof(int)*3);

    if(dataset_filename.substr(folder_point+1,3).compare("SFC")==0){
      const char* sfc_v = dataset_filename.substr(folder_point+4,1).c_str();
      debug4 << "Use SFC "<< sfc_v << std::endl;
      if(*sfc_v == 'X'){
        grid_type = "SFCX";
        sfc_offset[0] = 1;
      }
      else if(*sfc_v == 'Y'){
        grid_type = "SFCY";
        sfc_offset[1] = 1;
      }
      else if(*sfc_v == 'Z'){
        grid_type = "SFCZ";
        sfc_offset[2] = 1;
      }
    }
    else if(dataset_filename.substr(folder_point+1,2).compare("CC")==0)
      grid_type = "CC";

    mesh_name = grid_type+"_Mesh";

  if(is_gidx){
    debug4 << "Using GIDX file" << std::endl;

    vtkSmartPointer<vtkXMLDataParser> parser = vtkSmartPointer<vtkXMLDataParser>::New();

    parser->SetFileName(dataset_filename.c_str());
    if (parser->Parse()){
      vtkXMLDataElement *root = parser->GetRootElement();

      int ntimesteps = root->GetNumberOfNestedElements();

      debug4 << "Found " << ntimesteps << " timesteps in GIDX file" << std::endl;

      for(int i=0; i < ntimesteps; i++){
        vtkXMLDataElement *xmltime = root->GetNestedElement(i);
        String urlstr(xmltime->GetAttribute("url"));
        String timestr(xmltime->GetAttribute("log_time"));

        gidx_info ginfo;
        ginfo.url = urlstr.substr(7);
        ginfo.log_time = cint(timestr.c_str());
        gidx_datasets.push_back(ginfo);

        debug5 << "added dataset " << ginfo.url << " time " << ginfo.log_time << std::endl;

        logTimeIndex.push_back(ginfo.log_time);
        timeIndex.push_back(ginfo.log_time);
      }

      if(ntimesteps > 0)
        if (!reader->openDataset(gidx_datasets[0].url)) // open first dataset 
        {
          std::cerr << "Cannot load "<< filename << std::endl;
          return;
        }

      }else{
        std::cerr << "Cannot parse GIDX file " << dataset_filename << std::endl;
      }

    }else{
      String folder = dataset_filename.substr(0,folder_point);
      String dataset_name = dataset_filename.substr(folder_point,ext_point-folder_point);

    // "Standard" IDX metadata file (not yet standardized)
      metadata_filename = folder + dataset_name+"/"+dataset_name+".xml";

      if (!reader->openDataset(filename))
      {
        std::cerr <<"Could not load "<<filename << std::endl;
        return;
      }
    }

    dim = reader->getDimension(); //<ctc> //NOTE: it doesn't work like we want. Instead, when a slice (or box) is added, the full data is read from disk then cropped to the desired subregion. Thus, I/O is never avoided.

    createBoxes();

    createTimeIndex();

//#ifdef PARALLEL
    domainDecomposition();
//#endif
  }


// ****************************************************************************
//  Method: avtIDXFileFormat destructor
//
//  Programmer: Cameron Christensen
//  Creation:   Monday, November 04, 2013
//
// ****************************************************************************

  avtIDXFileFormat::~avtIDXFileFormat()
  {
    debug5 <<"(avtIDXFileFormat destructor)" << std::endl;

    // for(int i=0; i < boxes_bounds.size(); i++)
    //     if(boxes_bounds.at(i) != NULL)
    //         delete [] boxes_bounds.at(i);

    if(reader != NULL)
      delete reader;

  }

// ****************************************************************************
//  Method: avtIDXFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

  int
  avtIDXFileFormat::GetNTimesteps(void)
  {
    if(timeIndex.size() == 0)
      createTimeIndex();

    //printf("Ntimesteps %d \n", timeIndex.size());
    return timeIndex.size();// reader->getNTimesteps();
  }


// ****************************************************************************
//  Method: avtIDXFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

  void
  avtIDXFileFormat::FreeUpResources(void)
  {
    debug5 <<"avtIDXFileFormat::FreeUpResources..." << std::endl;
    //<ctc> todo... something (is destructor also called?)
  }


//bool avtIDXFileFormat::CanCacheVariable(const char *var)
//{
//    return false;
//}

// ****************************************************************************
//  Method: avtIDXFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

void
avtIDXFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
  int timestate) 
{
  debug5 << rank << ": Meta data" << std::endl;

  ActivateTimestep(timestate);

  md->ClearMeshes();
  md->ClearScalars(); 
  md->ClearVectors();
  md->ClearLabels();

  avtMeshMetaData *mesh = new avtMeshMetaData;
  mesh->name = mesh_name;

#if USE_AMR    
  mesh->meshType = AVT_AMR_MESH;
  mesh->topologicalDimension = dim;
  mesh->spatialDimension = dim;

  int totalPatches = level_info.patchInfo.size();
  
  mesh->numBlocks = totalPatches;
  
  //mesh->blockOrigin = 0;
  //mesh->LODs = reader->getMaxResolution();

  mesh->blockTitle = "patches";
  mesh->blockPieceName = "patch";

  int numLevels = 1; // only 1 AMR level for now
  mesh->numGroups = numLevels;
  mesh->groupTitle = "levels";
  mesh->groupPieceName = "level";

  std::vector<int> groupIds(totalPatches);
  std::vector<std::string> pieceNames(totalPatches);
  
  for (int i = 0; i < mesh->numBlocks; i++) {
    char tmpName[64];
    int level = 0; // only 1 level
    int local_patch = i;
    sprintf(tmpName,"level%d, patch%d", level, local_patch);

    //printf("Setting id %d = %d name %s\n", i, level, tmpName);
    groupIds[i] = level;
    pieceNames[i] = tmpName;
  }

  mesh->blockNames = pieceNames;
  mesh->groupIds = groupIds;

  mesh->containsExteriorBoundaryGhosts = false;
  mesh->hasSpatialExtents = false;
  
  md->AddGroupInformation(numLevels, totalPatches, groupIds);
#else
  mesh->meshType = AVT_RECTILINEAR_MESH;
  int totalPatches = level_info.patchInfo.size();
  mesh->numBlocks = totalPatches;
  mesh->blockOrigin = 0;
  mesh->cellOrigin = 1;
  //mesh->LODs = reader->getMaxResolution();
  mesh->spatialDimension = dim;
  mesh->topologicalDimension = dim;
  mesh->blockTitle = "blocks";
  mesh->blockPieceName = "piece";//%06d";
  mesh->groupPieceName = "global_index";
  
  /*mesh->groupTitle = "levels";
  mesh->groupPieceName = "level";
  mesh->blockTitle = "patches";
  mesh->blockPieceName = "patch%06d";
     
  std::vector<int> groupIds(totalPatches);
  std::vector<std::string> pieceNames(totalPatches);
  
  int numLevels = 1;
  for (int i = 0; i < mesh->numBlocks; i++) {
    char tmpName[64];
    int level = 0; // only 1 level
    int local_patch = i;
    sprintf(tmpName,"level%d, patch%d", level, local_patch);

    groupIds[i] = level;
    pieceNames[i] = tmpName;
  }
  //mesh->groupTitle = "levels";
  //mesh->groupPieceName = "level";
  mesh->numGroups = numLevels; // n AMR levels
  mesh->containsExteriorBoundaryGhosts = false;
  mesh->blockNames = pieceNames;

  md->AddGroupInformation(numLevels, totalPatches, groupIds);
  */
#endif
  
  int low[3],high[3];
  input_patches.getBounds(low,high,mesh_name,use_extracells);
 
  debug4 << "global low " << low[0] << ","<< low[1]<<","<< low[2] <<std::endl;
  debug4 << "global high " << high[0] << ","<< high[1]<<","<< high[2] <<std::endl;

  //this can be done once for everything because the spatial range is the same for all meshes
  double box_min[3] = { input_patches.anchor[0] + low[0] * input_patches.spacing[0],
  input_patches.anchor[1] + low[1] * input_patches.spacing[1],
  input_patches.anchor[2] + low[2] * input_patches.spacing[2] };
  double box_max[3] = { input_patches.anchor[0] + high[0] * input_patches.spacing[0],
    input_patches.anchor[1] + high[1] * input_patches.spacing[1],
    input_patches.anchor[2] + high[2] * input_patches.spacing[2] };
    {
      debug5 << "Dimensions " << dim <<std::endl;
      char* debug_str = new char[1024];
      sprintf(debug_str, "anchor %f %f %f spacing %f %f %f\n",input_patches.anchor[0],input_patches.anchor[1],input_patches.anchor[2], input_patches.spacing[0],input_patches.spacing[1],input_patches.spacing[2]);
      debug5 << debug_str;
      sprintf(debug_str, "global log %d %d %d - %d %d %d\n", low[0],low[1],low[2],high[0],high[1],high[2]);
      debug5 << debug_str;

      sprintf(debug_str, "global phy %f %f %f - %f %f %f\n", box_min[0],box_min[1],box_min[2],box_max[0],box_max[1],box_max[2]);
      debug5 << debug_str;

     delete [] debug_str;
    }

    int logical[3];
    for (int i=0; i<3; i++)
      logical[i] = high[i]-low[i];

    mesh->hasSpatialExtents = true; 
    mesh->minSpatialExtents[0] = box_min[0];
    mesh->maxSpatialExtents[0] = box_max[0];
    mesh->minSpatialExtents[1] = box_min[1];
    mesh->maxSpatialExtents[1] = box_max[1];
    mesh->minSpatialExtents[2] = box_min[2];
    mesh->maxSpatialExtents[2] = box_max[2];

    mesh->hasLogicalBounds = true;
    mesh->logicalBounds[0] = logical[0];
    mesh->logicalBounds[1] = logical[1];
    mesh->logicalBounds[2] = logical[2];

    md->Add(mesh);

    md->AddDefaultSILRestrictionDescription(std::string("!TurnOnAll"));
    md->SetCyclesAreAccurate(true);

//if(timestate == 0){
  const std::vector<Field>& fields = reader->getFields();
  debug5 << "adding " << fields.size() << "fields" << std::endl;

  int ndtype;
// char testvar[128];
// sprintf(testvar, "AA_var_%d",timestate);
// AddScalarVarToMetaData(md, testvar, mesh->name, AVT_ZONECENT);
  for (int i = 0; i < (int) fields.size(); i++)
  {
    const Field& field = fields[i];
  // printf("adding field %s\n", field.name.c_str());
    if (!field.isVector){
      AddScalarVarToMetaData(md, field.name, mesh->name, AVT_ZONECENT);
      //md->Add(new avtScalarMetaData(field.name,mesh->name,AVT_ZONECENT));
    }
    else
      AddVectorVarToMetaData(md, field.name, mesh->name, AVT_ZONECENT,field.ncomponents);
      //md->Add(new avtVectorMetaData(field.name,mesh->name,AVT_ZONECENT, field.ncomponents));
  }

  std::vector<int> cycles;
  cycles.resize(GetNTimesteps());

  for (int ti=0;ti<(int)cycles.size();ti++){
    cycles[ti] = ti;
  }

  md->SetCycles( cycles );
  //md->SetMustRepopulateOnStateChange(true);

  debug5 << rank << ": end meta" <<std::endl;
}

void
avtIDXFileFormat::SetUpDomainConnectivity(const char* meshname)
{
  avtRectilinearDomainBoundaries *rdb =
  new avtRectilinearDomainBoundaries(true);
  int ndomains =level_info.patchInfo.size();
  rdb->SetNumDomains(ndomains);
  //printf("Rect: Setting number of domains %d for mesh %s\n", ndomains, meshname);

  for(int n=0; n < ndomains; n++) {
    int low[3],high[3];
    level_info.patchInfo[n].getBounds(low,high,meshname, use_extracells);

    int e[6] = { low[0], high[0],
     low[1], high[1],
     low[2], high[2] };

     rdb->SetIndicesForRectGrid(n, e);
   }
   rdb->CalculateBoundaries();
   void_ref_ptr vr =
   void_ref_ptr(rdb, avtStructuredDomainBoundaries::Destruct);
   cache->CacheVoidRef("any_mesh",
     AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1, vr);

  void_ref_ptr vrTmp = cache->GetVoidRef("any_mesh", // MUST be called any_mesh
    AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
    -1, -1);
  if (*vrTmp == NULL )//|| *vrTmp != *this->mesh_boundaries[meshname])
    fprintf(stderr,"pidx boundary mesh not registered\n");


}

void avtIDXFileFormat::computeDomainBoundaries(const char* meshname, int timestate){
  //if (!avtDatabase::OnlyServeUpMetaData() && level_info.patchInfo.size()>0 ){

#ifdef MDSERVER
    return;
#else
  
  avtRectilinearDomainBoundaries *rdb =
  new avtRectilinearDomainBoundaries(true);
  rdb->SetNumDomains(level_info.patchInfo.size());
  //printf("Setting number of domains %d for mesh %s\n", level_info.patchInfo.size(), meshname);
    // for (long long i = 0 ; i < level_info.patchInfo.size() ; i++)
    // {
    //     int low[3],high[3];

    //     level_info.patchInfo[i].getBounds(low,high,meshname, use_extracells);

    //     int e[6] = { low[0], high[0],
    //                low[1], high[1],
    //                low[2], high[2] };

    //     rdb->SetIndicesForAMRPatch(i,0,e);//SetIndicesForRectGrid(i, e);
    // }
  int glow[3], ghigh[3];
  level_info.getBounds(glow,ghigh,meshname,use_extracells);

  for(int domain=0; domain < level_info.patchInfo.size(); domain++){

    int low[3], high[3];//, extracells[6];
    level_info.patchInfo[domain].getBounds(low,high,meshname);

    // int e[6] = { low[0]+extracells[0], high[0]+extracells[1],
    //  low[1]+extracells[2], high[1]+extracells[3],
    //  low[2]+extracells[4], high[2]+extracells[5]};

    int e[6] = { low[0], high[0],
                 low[1], high[1],
                 low[2], high[2]};

    rdb->SetIndicesForAMRPatch(domain,0,e);

    //printf("domain %d ext %d %d %d, %d %d %d\n", domain, low[0],low[1],low[2],high[0],high[1],high[2]);

#if 0
    for(int b=0; b < level_info.patchInfo.size(); b++){
      int tlow[3], thigh[3];
      level_info.patchInfo[b].getBounds(tlow,thigh,meshname,use_extracells);

      if (b == domain) continue;
      
      if(overlap(low[0],high[0],tlow[0],thigh[0])&&
         overlap(low[1],high[1],tlow[1],thigh[1])&&
         overlap(low[2],high[2],tlow[2],thigh[2]))
      {
        int neig_low[3] = {max(low[0],tlow[0]),max(low[1],tlow[1]),max(low[2],tlow[2])};
        int neig_high[3] = {min(high[0],thigh[0]),min(high[1],thigh[1]),min(high[2],thigh[2])};

        rdb->AddNeighbor(domain, domain, b,
                                 neig_low, neig_high);

        //printf("%d add neighbour %d [%d %d %d, %d %d %d]\n", domain, b, neig_low[0],neig_low[1],neig_low[2], neig_high[0],neig_high[1],neig_high[2]);

      }
    }
#endif
    }

    rdb->CalculateBoundaries();

    this->mesh_boundaries[meshname] = void_ref_ptr(rdb,
     avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, 
      timestate, -1, this->mesh_boundaries[meshname]);

    void_ref_ptr vrTmp = cache->GetVoidRef("any_mesh", // MUST be called any_mesh
      AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
      timestate, -1);
    if (*vrTmp == NULL || *vrTmp != *this->mesh_boundaries[meshname])
      fprintf(stderr,"pidx boundary mesh not registered\n");

    //canDoStreaming = false;

    //printf("%d: DONE compute domain boundary\n", rank);
#endif
  }


// ****************************************************************************
//  Method: avtIDXFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: spetruzza
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

  vtkDataSet *
  avtIDXFileFormat::GetMesh(int timestate, int domain, const char *meshname)
  {
    debug5 << rank << ": start getMesh "<< meshname << " domain " << domain << std::endl;

    Box slice_box;

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();

#if USE_AMR
  //SetUpDomainConnectivity(mesh->name.c_str());
  computeDomainBoundaries(meshname, timestate);
#else
  SetUpDomainConnectivity(meshname);
#endif

    int my_dims[3];
    float *arrayX;
    float *arrayZ;
    float *arrayY;
    vtkFloatArray *coordsX;
    vtkFloatArray *coordsY;
    vtkFloatArray *coordsZ;

    int glow[3], ghigh[3];
    input_patches.getBounds(glow,ghigh,meshname,use_extracells);

    //get patch bounds
    int low[3], high[3];
    level_info.patchInfo[domain].getBounds(low,high,meshname,use_extracells);

    //printf("MESH domain %d ext %d %d %d, %d %d %d\n", domain, low[0],low[1],low[2],high[0],high[1],high[2]);
    
    for(int k=0; k<3; k++){
      int offset = 1 + sfc_offset[k]; 

      if(!uintah_metadata)
        offset = 0;
      
      my_dims[k] = high[k]-low[k]+offset; // for NON-nodeCentered no +1 ??(patch end is on high boundary)

    }

    debug5 << rank << ": dims " << my_dims[0] << " " << my_dims[1] << " " << my_dims[2] << std::endl;

    rgrid->SetDimensions(my_dims);

    //printf("global %d %d %d - %d %d %d local[%d] %d %d %d - %d %d %d\n",glow[0],glow[1],glow[2],ghigh[0],ghigh[1],ghigh[2],domain,low[0],low[1],low[2],high[0],high[1],high[2]);
    // printf("cellspacing %f %f %f\n", level_info.spacing[0],level_info.spacing[1],level_info.spacing[2]);
    //printf("anchor %f %f %f\n", input_patches.anchor[0],input_patches.anchor[1],input_patches.anchor[2]);

    for (int c=0; c<3; c++) {
      vtkFloatArray *coords = vtkFloatArray::New(); 
      coords->SetNumberOfTuples(my_dims[c]); 
      float *array = (float *)coords->GetVoidPointer(0); 

      for (int i=0; i < my_dims[c]; i++)
      {
        // Face centered data gets shifted towards -inf by half a cell.
        // Boundary patches are special shifted to preserve global domain.
        // Internal patches are always just shifted.
       float face_offset= 0;

       if (sfc_offset[c]) 
       {
          if (i==0)
          {
            // No neighbor, so the patch is on low boundary
            if (low[c]==glow[c]) 
              face_offset = 0.0;
            // Patch boundary is internal to the domain
            else{
              if(c==0)
                face_offset = -0.5;
              else
                face_offset = -0.0;
            }
          }
          else if (i == my_dims[c]-1)
          {
            face_offset = -1.0;
          }
          else
            face_offset = -0.5;
        }

          array[i] = input_patches.anchor[c] + 
                     (i + low[c] + face_offset) * input_patches.spacing[c];

         // if(i==0)
         //   printf("low %d[%d]: %f\n", domain,c, array[i]);
         // if(i==my_dims[c]-1)
         //   printf("high %d[%d]: %f\n", domain,c, array[i]);
        }

        switch(c) {
          case 0:
          rgrid->SetXCoordinates(coords); break;
          case 1:
          rgrid->SetYCoordinates(coords); break;
          case 2:
          rgrid->SetZCoordinates(coords); break;
        }

        coords->Delete();
      }

#if 0 // compute ghost cells
    size_t nCells = rgrid->GetNumberOfCells();
    char *blanks = new char[nCells];
    memset(blanks, 0, nCells*sizeof(char));

    int nNodes = rgrid->GetNumberOfPoints();
    int *blanksN = new int[nNodes];
    memset(blanksN, 0, nNodes*sizeof(int));

    unsigned char realVal = 0, ghost = 0, ghostN = 0, noghostN = 0;

    avtGhostData::AddGhostZoneType(ghost, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
    ghostCells->SetName("avtGhostZones");
    ghostCells->Allocate(nCells);

    avtGhostData::AddGhostNodeType(ghostN,
                                        DUPLICATED_NODE);
    avtGhostData::RemoveGhostNodeType(noghostN,
                                        DUPLICATED_NODE);//NODE_NOT_APPLICABLE_TO_PROBLEM);
    vtkUnsignedCharArray *ghostNodes = vtkUnsignedCharArray::New();
    ghostNodes->SetName("avtGhostNodes");
    ghostNodes->Allocate(nNodes);

    int dim_block[3] = {my_dims[0]-1,my_dims[1]-1,my_dims[2]-1};

    //printf("NCELLS %d dims %d %d %d\n", nCells, dim_block[0],dim_block[1],dim_block[2]);

    for(int b=0; b < level_info.patchInfo.size(); b++){
      if (b == domain) continue;

      int tlow[3], thigh[3];
      level_info.patchInfo[b].getBounds(tlow,thigh,meshname,use_extracells);

      int count_ghost = 0;
      bool over[3];  
            
      int neig_low[3]={0,0,0};
      int neig_high[3]={0,0,0};

      int inter_low[3];
      int inter_high[3];
      //bool box_intersect = intersect(over, low,high,tlow,thigh, inter_low,inter_high);

      bool box_touch = touch(over, low,high,tlow,thigh, inter_low,inter_high);

      for(int d=0; d < 3; d++){
        int maxv=inter_low[d];
        int minv=inter_high[d];
        neig_low[d] = maxv;
        neig_high[d] = minv;

#if 0
        if(box_touch && over[d]){
          if(d==0){
            
            if(low[d] < tlow[d]){
              
              neig_low[d] = maxv;
              neig_high[d] = minv;
              //printf("<<< low[%d] = %d high[d] = %d\n", d,neig_low[d], neig_high[d]);
            }
            else{
               neig_low[d] = maxv;
               neig_high[d] = minv;
               //printf(">>>> low[%d] = %d high[d] = %d\n", d,neig_low[d], neig_high[d]);
            }
          }
          else{
            if(low[d] < tlow[d]){
              neig_low[d] = maxv+1;
              neig_high[d] = minv;
            }
            else{
              neig_low[d] = maxv;
              neig_high[d] = minv-1;
            }
          }

         /* 
          if(d==0){
            if(low[d] < tlow[d]){
              neig_low[d] = maxv+1;//+use_extracells;
              neig_high[d] = minv;
            }
            else{
              neig_low[d] = maxv;
              neig_high[d] = minv-1;//-use_extracells;
            }
          }
          else{
            if(low[d] < tlow[d]){
              neig_low[d] = maxv+2;//+use_extracells;
              neig_high[d] = minv;
            }
            else{
               neig_low[d] = maxv;
               neig_high[d] = minv-1;//-use_extracells;
            }
          }
          */
      }
      else{
        neig_high[d] = minv;
        neig_low[d] = maxv;
      }

 #endif     
    }

      printf("%d->%d Ghost zone [%d %d %d, %d %d %d]\n", domain, b, neig_low[0],neig_low[1],neig_low[2], neig_high[0],neig_high[1],neig_high[2]);
      for(int k=neig_low[2]; k <= neig_high[2]; k++)
        for(int j=neig_low[1]; j <= neig_high[1]; j++)
          for(int i=neig_low[0]; i <= neig_high[0]; i++){

            // if(i == glow[0] || j == glow[1] || k == glow[2] || i == ghigh[0] || j == ghigh[1] || k == ghigh[2])
            // if(i == ghigh[0]-1 || j == ghigh[1]-1 || k == ghigh[2]-1)
            //    continue;

            int ii = i - low[0];
            int jj = j - low[1];
            int kk = k - low[2];

            blanks[(size_t)(ii) + (size_t)dim_block[0] * (size_t)((size_t)(jj) + (size_t)dim_block[1] * (size_t)(kk))] = 1;

            blanksN[(size_t)(ii) + (size_t)dim_block[0] * (size_t)((size_t)(jj) + (size_t)dim_block[1] * (size_t)(kk))] = 1;
            count_ghost++;
          }
    }

      //printf("%d found %d ghosts %d\n", domain, count_ghost, ghost);
  //}
    //printf("cells %d blanks size %d\n", nCells, dim_block[0]*dim_block[1]*dim_block[2]);

  for (int i = 0; i < nCells; i++) {
    // if (!blanks[i]){
    //   ghostCells->InsertNextValue(realVal);
    // }
    // else{
    //   ghostCells->InsertNextValue(ghost);
    // }

    if(!blanksN[i]){
      ghostNodes->InsertNextValue(noghostN);
    }
    else
      ghostNodes->InsertNextValue(ghostN);
    }

  //rgrid->GetCellData()->AddArray(ghostCells);
  rgrid->GetPointData()->AddArray(ghostNodes);

  vtkStreamingDemandDrivenPipeline::SetUpdateGhostLevel(
    rgrid->GetInformation(), 0);
  ghostCells->Delete();
  ghostNodes->Delete();

  delete [] blanks;
#endif

  debug5 << rank << ": end mesh" << std::endl;

  return rgrid;

}

void
avtIDXFileFormat::GetCycles(std::vector<int> &cycles)
{
  cycles.clear();

  if(logTimeIndex.size()>0)
    cycles=logTimeIndex;
  else{
    for(int i = 0; i < reader->getNTimesteps(); ++i)
      cycles.push_back(i);
  }
}

void
avtIDXFileFormat::GetTimes(std::vector<double> &times)
{
  times=timeIndex;
}

vtkDataArray* avtIDXFileFormat::queryToVtk(int timestate, int domain, const char *varname){

  Box my_box;
  int low[3];
  int high[3];
  level_info.patchInfo[domain].getBounds(low,high, mesh_name, use_extracells);

  debug5 << "read data " << level_info.patchInfo[domain].toString();
  for(int k=0; k<3; k++){
    if(uintah_metadata){
      if(!use_extracells) // cause uintah low starts from -1
        low[k]++;

      high[k] += sfc_offset[k];
    }
    else{
      high[k] -= 2;
    }

    my_box.p1[k] = low[k];
    my_box.p2[k] = high[k];
  }

  unsigned char* data = reader->getData(my_box, timestate, varname);

  debug5 << rank << ": read data done" << std::endl;
    /*
    std::ofstream out;
    char outname[128];
    sprintf(outname, "out_%d_p1_%d_%d_%d_p2_%d_%d_%d.raw", domain,low[0],low[1],low[2],high[0],high[1],high[2]);
    out.open(outname);

    uint buffer_size = sizeof(float)*(my_box.p2.x-my_box.p1.x+1)*(my_box.p2.y-my_box.p1.y+1)*(my_box.p2.z-my_box.p1.z+1);
    printf("dumping domain %d size %d\n", domain, buffer_size);
  
    //  for(int i = 0; i< exp_size/sizeof(float); i++){
    //    //if((double)data[i] != 0.0)
    //      printf("d %f\n", (float)data[i]);
    //  }
  
    out.write((const char*)data, buffer_size);
    out.close();
    */
  if(data == NULL){
    std::cerr << " NO DATA " << std::endl;
    return NULL;
  }

  Field field = reader->getCurrField();
  DTypes type = field.type;

  int my_bounds[3];
  for(int k=0; k<3; k++)
   my_bounds [k] = high[k]-low[k]+1;

  int ztuples = (dim == 2) ? 1 : (my_bounds[2]);
  long long ntuples = (my_bounds[0])*(my_bounds[1])*ztuples;

  int ncomponents = 1;
  bool isVector = field.isVector;

// printf("is vector? %d\n", isVector);

  if(isVector)
    ncomponents = 3; // Visit wants always 3 components vectors

  long long ntotal = ntuples * ncomponents;

    // do not reverse endianess if data is compressed
  reverse_endian = reverse_endian * !reader->isCompressed();

  if(type == VisitIDXIO::IDX_UINT8){

    vtkUnsignedCharArray*rv = vtkUnsignedCharArray::New();
    rv->SetNumberOfComponents(ncomponents); //<ctc> eventually handle vector data, since visit can actually render it!
    
    if(isVector && dim < 3){

      unsigned char* newdata = convertTo3Components<unsigned char>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((unsigned char*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else
      rv->SetArray((unsigned char*)data,ncomponents*ntuples,1/*delete when done*/,0);
      return rv;
  }
  else if(type == VisitIDXIO::IDX_UINT16){

    vtkUnsignedShortArray *rv = vtkUnsignedShortArray::New();
    rv->SetNumberOfComponents(ncomponents);
    
    if(isVector && dim < 3){

      unsigned short* newdata = convertTo3Components<unsigned short>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((unsigned short*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else
      rv->SetArray((unsigned short*)data,ncomponents*ntuples,1,0);
    
    if(reverse_endian){
      unsigned short *buff = (unsigned short *) rv->GetVoidPointer(0);
      for (long long i = 0 ; i < ntotal ; i++)
      {
        int tmp;
        int16_Reverse_Endian(buff[i], (unsigned char *) &tmp);
        buff[i] = tmp;
      }
    }

    return rv;
  }
  else if(type == VisitIDXIO::IDX_UINT32){
    vtkUnsignedIntArray *rv = vtkUnsignedIntArray::New();
    rv->SetNumberOfComponents(ncomponents);
    
    if(isVector && dim < 3){

      unsigned int* newdata = convertTo3Components<unsigned int>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((unsigned int*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else
      rv->SetArray((unsigned int*)data,ncomponents*ntuples,1,0);
    
    if(reverse_endian){
      unsigned int *buff = (unsigned int *) rv->GetVoidPointer(0);
      for (long long i = 0 ; i < ntotal ; i++)
      {
        int tmp;
        int32_Reverse_Endian(buff[i], (unsigned char *) &tmp);
        buff[i] = tmp;
      }
    }
    
    return rv;
  }
  else if(type == VisitIDXIO::IDX_INT8){
    vtkCharArray*rv = vtkCharArray::New();
    rv->SetNumberOfComponents(ncomponents);
    
    if(isVector && dim < 3){

      char* newdata = convertTo3Components<char>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((char*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else
      rv->SetArray((char*)data,ncomponents*ntuples,1,0);
    
    return rv;
  }
  else if(type == VisitIDXIO::IDX_INT16){
    vtkShortArray *rv = vtkShortArray::New();
    rv->SetNumberOfComponents(ncomponents);
    
    if(isVector && dim < 3){

      short* newdata = convertTo3Components<short>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((short*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else
      rv->SetArray((short*)data,ncomponents*ntuples,1,0);
    
    if(reverse_endian){
      short *buff = (short *) rv->GetVoidPointer(0);
      for (long long i = 0 ; i < ntotal ; i++)
      {
        int tmp;
        int16_Reverse_Endian(buff[i], (unsigned char *) &tmp);
        buff[i] = tmp;
      }
    }
    
    return rv;
  }
  else if(type == VisitIDXIO::IDX_INT32){
    vtkIntArray *rv = vtkIntArray::New();
    rv->SetNumberOfComponents(ncomponents);
    
    if(isVector && dim < 3){

      int* newdata = convertTo3Components<int>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((int*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else
      rv->SetArray((int*)data,ncomponents*ntuples,1,0);
    
    if(reverse_endian){
      int *buff = (int *) rv->GetVoidPointer(0);
      for (long long i = 0 ; i < ntotal ; i++)
      {
        int tmp;
        int32_Reverse_Endian(buff[i], (unsigned char *) &tmp);
        buff[i] = tmp;
      }
    }
    
    return rv;
  }
  else if(type == VisitIDXIO::IDX_INT64){
    vtkLongArray *rv = vtkLongArray::New();
    rv->SetNumberOfComponents(ncomponents);

    // ?? is it correct to use long here ??
    if(isVector && dim < 3){

      long* newdata = convertTo3Components<long>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((long*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else
      rv->SetArray((long*)data,ncomponents*ntuples,1,0);
    
    if(reverse_endian){
      long *buff = (long *) rv->GetVoidPointer(0);
      for (long long i = 0 ; i < ntotal ; i++)
      {
        long tmp;
        double64_Reverse_Endian(buff[i], (unsigned char *) &tmp);
        buff[i] = tmp;
      }
    }
    
    return rv;
  }
  else if(type == VisitIDXIO::IDX_FLOAT32){
  //  printf("FLOAT32 creating array ncomp %d tuples %d \n", ncomponents, ntuples);
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfComponents(ncomponents);
    
    if(isVector && dim < 3){
      float* newdata = convertTo3Components<float>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((float*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else{
      rv->SetArray((float*)data,ncomponents*ntuples,1,0);
    }
    
    if(reverse_endian){
      float *buff = (float *) rv->GetVoidPointer(0);

      float min_value = 99999999999999.f;
      float max_value = -99999999999999.f;

      // TODO remove the range calculation
      for (long long i = 0 ; i < ntotal ; i++)
      {
        float tmp;
        float32_Reverse_Endian(buff[i], (unsigned char *) &tmp);
        buff[i] = tmp;

        float value = buff[i];
        if(value < min_value)
          min_value = value;

        if(value > max_value)
          max_value = value;
      }
      //printf("range %f , %f\n", min_value, max_value);

    }
    return rv;
  }
  else if(type == VisitIDXIO::IDX_FLOAT64){

  //printf("DOUBLE creating array ncomp %d \n", ncomponents);
    vtkDoubleArray *rv = vtkDoubleArray::New();
    rv->SetNumberOfComponents(ncomponents);
    
    if(isVector && dim < 3){

      double* newdata = convertTo3Components<double>(data, field.ncomponents, 3, ntuples);
      rv->SetArray((double*)newdata,ncomponents*ntuples,1,0);

      delete data;
    }
    else{
     // printf("DOUBLE converting %d \n", ncomponents*ntuples);
      rv->SetArray((double*)data,ncomponents*ntuples,1,0);
    }

    if(reverse_endian){
      double *buff = (double *) rv->GetVoidPointer(0);
      for (unsigned long long i = 0 ; i < ntotal ; i++)
      {
        double tmp;
        double64_Reverse_Endian(buff[i], (unsigned char *) &tmp);
        buff[i] = tmp;
      }
    }

    return rv;
  }else{

    fprintf(stderr, "Type %d not found\n", type);
  }

  debug5 << rank << ": done data loading" << std::endl;

  return NULL;
}

// ****************************************************************************
//  Method: avtIDXFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

vtkDataArray *
avtIDXFileFormat::GetVar(int timestate, int domain, const char *varname)
{
// get correspondig logic time
  debug5 << "Requested index time " << timestate << " using logical time (IDX) " << logTimeIndex[timestate] << std::endl;

  if(is_gidx){
    reader->openDataset(gidx_datasets[timestate].url);
  }

  timestate = logTimeIndex[timestate];
  return queryToVtk(timestate, domain, varname);

}

void avtIDXFileFormat::ActivateTimestep(int ts){
  if(is_gidx){
    reader->openDataset(gidx_datasets[ts].url);
  }
}

// ****************************************************************************
//  Method: avtIDXFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: spetruzza, bremer5
//  Creation:   Mon Dec 10 15:06:44 PST 2012
//
// ****************************************************************************

vtkDataArray *
avtIDXFileFormat::GetVectorVar(int timestate, int domain, const char *varname)
{
  timestate = logTimeIndex[timestate];
  return queryToVtk(timestate, domain, varname);

}
