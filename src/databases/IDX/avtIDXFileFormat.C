/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
#include <vtkCellType.h>
#include <vtkCellData.h>
#include <vtkLongArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkSmartPointer.h>
#include <vtkHexahedron.h>
#include <vtkUnstructuredGrid.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix4x4.h>
#include <vtkXMLDataElement.h>
#include <vtkXMLDataParser.h>

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
#include <avtView2D.h>
#include <avtView3D.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <dirent.h>

#ifdef PARALLEL
#include <avtParallel.h>
#endif

typedef std::string String;

#define USE_AMR 0

bool debug_format = true;
bool debug_input = true;

using namespace VisitIDXIO;

int        cint   (String s) {int    value;std::istringstream iss(s);iss>>value;return value;}
float      cfloat (String s) {float  value;std::istringstream iss(s);iss>>value;return value;}
double     cdouble(String s) {double value;std::istringstream iss(s);iss>>value;return value;}
// trim from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}

void avtIDXFileFormat::pidx_decomposition(int process_count){
#if 0  
  if (process_count == 1) return;
  
    int maxdir = 0; // largest extent axis
    int maxextent = 0;
    int maxbox = 0;

    for(int i=0; i < boxes.size(); i++){
        Box& box = boxes.at(i);
        
        for(int j=0; j < 3; j++){
            int extent = box.p2[j]-box.p1[j];
            if(extent > maxextent){
                maxdir = j;
                maxextent = extent;
                maxbox = i;
            }
        }
    }

    int total_extent = 0;
    int avg_ext = 0;
    
    for(int i=0; i < boxes.size(); i++){
        Box& box = boxes.at(i);
        
        total_extent += box.p2[maxdir]-box.p1[maxdir];
    }

    if(debug_format)
      printf("total_extent %d\n", total_extent);
   
    avg_ext = (total_extent+1) / nprocs;
    int res_ext = (total_extent+1) % nprocs;

    Box box = boxes[0];
    int global_size[3] = {box.p2.x-box.p1.x+1,box.p2.y-box.p1.y+1,box.p2.z-box.p1.z+1};
    if(debug_format)
      printf("global box size %d %d %d avg_ext %d res_ext %d max_dir %d\n", global_size[0],global_size[1],global_size[2], avg_ext, res_ext, maxdir);


  boxes.clear();
  phyboxes.clear();
  
  Point3d log2phy;
     
  for (int k = 0; k < dim; k++){
    log2phy[k] = (physicalBox.p2[k] - physicalBox.p1[k])/(global_size[k]-1);
  }
    
  for(int r=0; r < process_count; r++){
    
    int local_size[3];
    /*
    for(int d=0; d < dim; d++){
        if(d == maxdir){
            local_size[d] = avg_ext;
            if(r == process_count-1)
                local_size[d] += res_ext;
        }else
            local_size[d] = global_size[d];
    }

    int local_offset[3];*/

    Box newbox;
    for (int k = 0; k < dim; k++){
      if(k!=maxdir){
            newbox.p1[k] = 0;
            newbox.p2[k] = global_size[k]-1;
      }
      else {
        newbox.p1[k] = r*avg_ext;
        //if(boxes.size()>1)
        //  newbox.p1[k]--;
        newbox.p2[k] = newbox.p1[k] + avg_ext +1;
        if(r == process_count-1){
           newbox.p2[k] = global_size[k]-1; 
        }
      }
    }

    /*
    newbox.p1[0] = local_offset[0];
    newbox.p1[1] = local_offset[1];
    newbox.p1[2] = local_offset[2];
    newbox.p2[0] = local_offset[0]+local_size[0];//-1;
    newbox.p2[1] = local_offset[1]+local_size[1];//-1;
    newbox.p2[2] = local_offset[2]+local_size[2];//-1;
    */
    if(debug_format)
      printf("%d: created p1 %f %f %f p2 %f %f %f\n", r, newbox.p1[0],newbox.p1[1],newbox.p1[2], newbox.p2[0],newbox.p2[1],newbox.p2[2]);
    
    boxes.push_back(newbox);
         
    Point3d phyOffset = physicalBox.p1;
//      std::cout << "log2phy " <<log2phy << std::endl;
                
    Box newphybox;
    
    for (int k = 0; k < dim; k++){
      newphybox.p1[k] = newbox.p1[k] * log2phy[k] + phyOffset[k];

      if(k != maxdir)
        newphybox.p2[k] = physicalBox.p2[k];//newbox.p2[k] * log2phy[k] + phyOffset[k] + log2phy[k];
      else{
        newphybox.p2[k] = newbox.p2[k] * log2phy[k] + phyOffset[k];//newphybox.p1[k] + (avg_ext)*log2phy[k];// +log2phy[k];
        if(r == process_count-1)
           newphybox.p2[k] = physicalBox.p2[k];
      }
    }       

    if(debug_format)
      printf("New phy box p1: %f %f %f p2: %f %f %f\n", newphybox.p1.x,newphybox.p1.y,newphybox.p1.z ,newphybox.p2.x,newphybox.p2.y,newphybox.p2.z);

    phyboxes.push_back(newphybox);
    //physicalBox = physicalBox.getUnion((const Box)phyboxes.at(r));
    
  }

  /*if(process_count == 1)
    physicalBox = reader->getLogicBox();
  */
#endif
}

void avtIDXFileFormat::loadBalance(){

  //std::cout << "Load balancing";
    
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
    
    int total_extent = 0;
    int avg_ext = 0;
    
    for(int i=0; i < level_info.patchInfo.size(); i++){
        PatchInfo& box = level_info.patchInfo[i];
        int box_low[3];
        int box_high[3];

        box.getBounds(box_low,box_high,"CC");
        
        total_extent += box_high[maxdir] - box_low[maxdir];
    }
    //total_extent++; // DIM
    avg_ext = total_extent / nprocs;
    int res_ext = (total_extent % nprocs);
    
    if(debug_format && rank==0)
      std::cout << "tot ext " << total_extent << " avg ext " << avg_ext << " res ext " << res_ext << " n boxes " << level_info.patchInfo.size() <<std::endl;
    
    std::vector<PatchInfo> newboxes;
    // std::vector<Box> newphyboxes;
    
    for(int i=0; i < level_info.patchInfo.size(); i++){
        PatchInfo& box = level_info.patchInfo[i];
        int box_low[3];
        int box_high[3];

        box.getBounds(box_low,box_high,"CC");

        int loc_avg_ext = box_high[maxdir] - box_low[maxdir];
        int loc_res = 0;
        
        if(loc_avg_ext > avg_ext){
            loc_res = loc_avg_ext % avg_ext;
            loc_avg_ext = avg_ext;
        }

        if(debug_format && rank==0)
               std::cout << "local avg ext " << loc_avg_ext << " local res " << loc_res;
        
        int part_p1 = box_low[maxdir];
        int part_p2 = box_low[maxdir] + loc_avg_ext;
        
        int low[3]; 
        int high[3];
        int eCells[6];
        box.getBounds(low,high,eCells,"CC");

        // Point3d p1(box.p1);
        // Point3d p2(box.p2);

        // Point3d log2phy;
   //      Box& phybox = phyboxes.at(i);
   //      if(debug_format && rank==0)
          // std::cout << "Old box p1: " << p1 << " p2: "<< p2 << " phy " << phybox.p1 << " p2 " << phybox.p2<< std::endl<< std::flush;
      
   //      for (int k = 0; k < dim; k++){
   //        log2phy[k] = (phybox.p2[k] - phybox.p1[k])/(box.p2[k] - box.p1[k] + 1);
   //     }
      
        //Point3d phyOffset = phybox.p1;
//      std::cout << "log2phy " <<log2phy << std::endl;
      
        while(part_p2 <= box_high[maxdir]){
            
            low[maxdir] = part_p1;
            high[maxdir] = (part_p2 < box_high[maxdir]) ? part_p2+1 : part_p2;
           
            PatchInfo newbox;

            newbox.setBounds(low,high,eCells,"CC");
            newboxes.push_back(newbox);

            // // Box newbox(p1,p2);
            // // newboxes.push_back(newbox);
          
            // Box newphybox;
          
            // for (int k = 0; k < dim; k++){
            //   newphybox.p1[k] = newbox.p1[k] * log2phy[k];// + phyOffset[k];
            //   newphybox.p2[k] = newbox.p2[k] * log2phy[k];// + phyOffset[k];;
            // }
          
            // newphyboxes.push_back(newphybox);
            
            if(debug_format && rank ==0)
                   std::cout << "New box: " << newbox.toString();//p1 << " p2: "<< p2 << <<std:endl;//" phy " << newphybox.p1 << " p2 " << newphybox.p2<< std::endl << std::flush;
          
            part_p1 += loc_avg_ext;
            part_p2 += loc_avg_ext;
           
        }
        
        if(loc_res > 0){
            PatchInfo& boxres = newboxes[newboxes.size()-1];
            int boxres_low[3];
            int boxres_high[3];
            int ecellres[6];

            boxres.getBounds(boxres_low,boxres_high,ecellres,"CC");
            boxres_high[maxdir] += loc_res-1;
            boxres.setBounds(boxres_low,boxres_high,ecellres,"CC");
          
            // Box& phyboxres = newphyboxes.at(newphyboxes.size()-1);
            // phyboxres.p2[maxdir] += loc_res*log2phy[maxdir];
            
            if(debug_format && rank ==0)       
              std::cout << "Residual " << loc_res-1 <<" added to box "<< newboxes.size()-1 << boxres.toString();
        }

    }
    
    level_info.patchInfo.swap(newboxes);
    // phyboxes.swap(newphyboxes);
  
    if(debug_format && rank == 0){
      std::cout << "Total number of boxes/domains: " << level_info.patchInfo.size() << std::endl<< std::flush;
      std::cout << "----------Boxes----------" << std::endl<< std::flush;
        for(int i=0; i< level_info.patchInfo.size(); i++){
            std::cout << i << " = "<<level_info.patchInfo[i].toString();
            //boxes.at(i).p1 << " , " << boxes.at(i).p2 << " phy: "
                      //<< phyboxes.at(i).p1 << " , " << phyboxes.at(i).p2 << std::endl<< std::flush;
        }
        std::cout << "-------------------------" << std::endl<< std::flush;
    }

}


template <typename Type>
Type* avtIDXFileFormat::convertComponents(const unsigned char* src, int src_ncomponents, int dst_ncomponents, long long totsamples){
    int n=src_ncomponents;
    int m=dst_ncomponents;
    int ncomponents=std::min(m,n);
    
    Type* dst = (Type*)calloc(totsamples*m, sizeof(Type));
    
    //for each component...
    for (int C=0;C<ncomponents;C++)
    {
        Type* src_p=((Type*)src)+C;
        Type* dst_p=((Type*)dst)+C;
        for (long long I=0; I<totsamples; I++,src_p+=n,dst_p+=m)
        {
            *dst_p=*src_p;
            
            //std::cout << "c " << C << " I " <<I << std::endl;
        }
        
    }
    
    //std::cout << "data converted " << std::endl;
    
    return dst;
}

// TODO consider the physical box
// void avtIDXFileFormat::calculateBoundsAndExtents(){
    
//     // TODO deallocate this stuff
//     for(int i=0; i< boxes.size(); i++){
//         Box& box = boxes.at(i);
//         int* my_bounds = new int[3];
            
//         my_bounds[0] = box.p2.x-box.p1.x+1;
//         my_bounds[1] = box.p2.y-box.p1.y+1;
//         my_bounds[2] = box.p2.z-box.p1.z+1;
        
//         boxes_bounds.push_back(my_bounds);
        
//     }
    
// }

void avtIDXFileFormat::parseVector(vtkXMLDataElement *el, double* vec){
  String el_str(el->GetCharacterData());
          
  el_str = trim(el_str);
  String el_vals = el_str.substr(1,el_str.length()-2);
  if(debug_input)
    std::cout << "Found vec " << el_vals << std::endl;
  std::string anchs;
  std::stringstream anch_ss(el_vals);
  for (int k=0; k < dim; k++){
    std::getline(anch_ss, anchs, ',');
           
    vec[k] = cfloat(anchs);
  }
  if(debug_input)
    std::cout << "read " << vec[0] << " "<< vec[1] << " " << vec[2] << std::endl;
}

void avtIDXFileFormat::parseVector(vtkXMLDataElement *el, int* vec){
  String el_str(el->GetCharacterData());
      
  el_str = trim(el_str);
  String el_vals = el_str.substr(1,el_str.length()-2);
  if(debug_input)
    std::cout << "Found vec " << el_vals << std::endl;
  std::string anchs;
  std::stringstream anch_ss(el_vals);
  for (int k=0; k < dim; k++){
    std::getline(anch_ss, anchs, ',');
       
    vec[k] = cint(anchs);
  }
  if(debug_input)
    std::cout << "read " << vec[0] << " "<< vec[1] << " " << vec[2] << std::endl;
}


void avtIDXFileFormat::createBoxes(){
    
    size_t found = dataset_filename.find_last_of("/\\");
    String folder = dataset_filename.substr(0,found);
    
    bool found_cellspacing = false;

    bool use_boxes = false;

    String upsfilename = "noupsfile.ups";
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (folder.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            String name(ent->d_name);
            if(name.substr(name.find_last_of(".") + 1) == "ups"){
                upsfilename = name;
                if(debug_input)
                    std::cout<< ".ups file found " << upsfilename << std::endl;
                upsfilename = folder + "/" +upsfilename;
                break;
            }
        }
        closedir (dir);
    } else {
        if(debug_input)
            std::cout<< "No .ups file found" << std::endl;
    }
    
    vtkSmartPointer<vtkXMLDataParser> parser = vtkSmartPointer<vtkXMLDataParser>::New();
    
    upsfilename.replace(upsfilename.end()-3, upsfilename.end(),"ups");
    
    uintah_metadata = false;

    parser->SetFileName(upsfilename.c_str());
    if (!parser->Parse())
    {
        if(debug_input)
            std::cout<< "No .ups file found (Uintah only)" << std::endl;

        parser->SetFileName(metadata_filename.c_str());
        //std::cout << "trying metadata " << idxmetadata << std::endl;
        if (!parser->Parse()){        
            use_boxes = false;
            if(debug_input)
             std::cout << "Single-box mode" << std::endl;
        }else{
            use_boxes = true;
        }       
        
    }else{
        uintah_metadata = true;
        use_boxes = true;
        if(debug_input)
            std::cout << "Multi-box mode" << std::endl;
    }
    
    if(use_boxes){
        vtkXMLDataElement *root = parser->GetRootElement();
        vtkXMLDataElement *level = NULL;

        if(uintah_metadata)
            level = root->FindNestedElementWithName("Grid")->FindNestedElementWithName("Level");
        else 
            level = root->FindNestedElementWithName("level");

        if(uintah_metadata){

                vtkXMLDataElement *anchor_el = NULL;
                anchor_el = root->FindNestedElementWithName("Grid")->FindNestedElementWithName("anchor");

                if(anchor_el == NULL){
                  if(debug_input)
                    std::cout << "anchor not found" << std::endl;
                  level_info.anchor[0]=0;
                  level_info.anchor[1]=0;
                  level_info.anchor[2]=0;
                 }
                else{
                  parseVector(anchor_el, level_info.anchor);       
                }
                
                //cellspacing
                vtkXMLDataElement *cellspacing_el = NULL;
                cellspacing_el = root->FindNestedElementWithName("Grid")->FindNestedElementWithName("cellspacing");

                if(cellspacing_el == NULL){
                  if(debug_input)
                    std::cout << "cellspacing not found" << std::endl;
                  found_cellspacing = false;
                }else{
                  parseVector(cellspacing_el, level_info.spacing);
                  found_cellspacing = true;

                }
        }

        int nboxes = level->GetNumberOfNestedElements();
        
        if(debug_input)
            std::cout << "Found " << nboxes << " boxes" << std::endl;
        int last_log = 0;
        for(int i=0; i < nboxes; i++){

            vtkXMLDataElement *xmlbox = level->GetNestedElement(i);
            String lower(xmlbox->FindNestedElementWithName("lower")->GetCharacterData());
            String upper(xmlbox->FindNestedElementWithName("upper")->GetCharacterData());
            
            vtkXMLDataElement* p1log_el = xmlbox->FindNestedElementWithName("p1log");
            vtkXMLDataElement* p2log_el = xmlbox->FindNestedElementWithName("p2log");

            String extra_cells = "[0 0 0]";

            if(uintah_metadata)
                extra_cells = String(xmlbox->FindNestedElementWithName("extraCells")->GetCharacterData());
            
            String resolution(xmlbox->FindNestedElementWithName("resolution")->GetCharacterData());
            lower = trim(lower);
            upper = trim(upper);
            
            resolution = trim(resolution);
          
            lower = lower.substr(1,lower.length()-2);
            upper = upper.substr(1,upper.length()-2);
            
            // TODO Do we still need extra cells managements as a flag?
            extra_cells = extra_cells.substr(1,extra_cells.length()-2);
            resolution = resolution.substr(1,resolution.length()-2);
            
            if(debug_input)
                std::cout<< "lower " << lower << " upper " << upper << " resolution " << resolution << std::endl;
          
            // Point3d p1phy(0,0,0), p1log(0,0,0);
            // Point3d p2phy(0,0,0), p2log(0,0,0), logOffset(0,0,0);

            Point3d p1phy(0,0,0), p2phy(0,0,0);

            int low[3];
            int high[3];
          
            int eCells[6];
            int resdata[3];
            double phy2log[3];
            
            std::stringstream ress(resolution);
            std::stringstream ss1(lower);
            std::stringstream ss2(upper);
            std::stringstream ssSpace(extra_cells);
            std::string p1s, p2s, espace, res;
            for (int k=0; k < dim; k++){
                std::getline(ss1, p1s, ',');
                std::getline(ss2, p2s, ',');
                std::getline(ssSpace, espace, ',');
                std::getline(ress, res, ',');
                
                eCells[k] = cint(espace);
                resdata[k] = cint(res);
                
                p1phy[k] = cfloat(p1s);
                p2phy[k] = cfloat(p2s);
              
                phy2log[k] = (p2phy[k]-p1phy[k])/(resdata[k]);
                //p2phy[k] += phy2log[k];
                
                /*if(boxes.size() == 0){
                  logOffset[k] = std::abs(p1phy[k]) / phy2log[k];//phy2log[k];
                        }*/
               
                if(nboxes == 1){ // single box case
                  low[k] = 0;
                  high[k] = low[k] + resdata[k];
                }
                else{ // multibox case (all inside the same domain)
                  low[k] = std::fabs(p1phy[k]-level_info.anchor[k]) / phy2log[k] + eCells[k];
                  /*
                  if(level_info.patchInfo.size() > 0){ 

                    if(k==0){
                      int temp_low[3];
                      int temp_high[3];
                      level_info.patchInfo.back().getBounds(temp_low,temp_high,"CC");
                      
                      low[k] = temp_high[k]-1; // force overlap
                    }
                 
                    }*/

                  high[k] = low[k] + resdata[k];
                  /*
                  if(level_info.patchInfo.size() == nboxes-1){
                    if(k==0){
                      Box log_box = reader->getLogicBox();
                      high[k] = log_box.p2[k]-1-eCells[k];
                    }
                    }*/
                }
                
                if(p1log_el != NULL)
                  parseVector(p1log_el, low);
                if(p2log_el != NULL)
                  parseVector(p2log_el, high);   
                
            }
            
            PatchInfo box;
            // Simmetric extracells !!!! Not true
            eCells[3] = eCells[0];
            eCells[4] = eCells[1];
            eCells[5] = eCells[2];
            box.setBounds(low,high,eCells,"CC");

            level_info.patchInfo.push_back(box);

            if(debug_input){
              std::cout <<"Read box phy: p1 " << p1phy << " p2 "<< p2phy << std::endl;
              std::cout << level_info.patchInfo.back().toString();
              //std::cout <<"     box log: p1 " << p1log << " p2 "<< p2log << std::endl;
            }

       if(!found_cellspacing){
        printf("Cellspacing not found, calculating\n");

        PatchInfo& box = level_info.patchInfo[0];

        int low[3];
        int high[3];
        box.getBounds(low,high,"CC",use_extracells);

        for(int k=0; k<3; k++){
          level_info.spacing[k] = (p2phy[k]-p1phy[k])/(high[k]-low[k]+1);;
          printf("%f - %f / %d - %d +1\n",p2phy[k],p1phy[k],high[k],low[k]);
        }

      }


     //    phyboxes.push_back(Box(p1phy, p2phy));
     //    if(phyboxes.size()==1)
            //   physicalBox = phyboxes[0];
            // else
            //   physicalBox = physicalBox.getUnion((const Box)phyboxes.at(i));
     //      boxes.push_back(Box(p1log,p2log));
            
      }

        
    }
    else{

        Box log_box = reader->getLogicBox();
        int low[3];
        int high[3];
        int eCells[6] = {0,0,0,0,0,0};

        low[0] = log_box.p1[0];
        low[1] = log_box.p1[1];
        low[2] = log_box.p1[2];
        high[0] = log_box.p2[0]-1;
        high[1] = log_box.p2[1]-1;
        high[2] = log_box.p2[2]-1;

        PatchInfo box;
        box.setBounds(low,high,eCells,"CC");
        level_info.patchInfo.push_back(box);

        for(int k=0; k<3; k++){
          level_info.spacing[k]= 1.f;
          level_info.anchor[k] = 0.f;
        }
        std::cout << "Single Box: ";
        std::cout << level_info.patchInfo.back().toString();
    }

}

void avtIDXFileFormat::createTimeIndex(){
    timeIndex.clear();
    logTimeIndex.clear();

    vtkSmartPointer<vtkXMLDataParser> parser = vtkSmartPointer<vtkXMLDataParser>::New();
    size_t folder_point = dataset_filename.find_last_of("/\\");
    String folder = dataset_filename.substr(0,folder_point);
    
    String udafilename = folder + "/index.xml";

    //std::cout << "trying metadata " << udafilename << std::endl;
    parser->SetFileName(udafilename.c_str());
    if (!parser->Parse()){
        parser->SetFileName(metadata_filename.c_str());
        //std::cout << "trying metadata " << idxmetadata << std::endl;
        if (!parser->Parse()){
            std::cout<< "No metadata XML file found " << udafilename << std::endl;
            
            std::vector<double> times = reader->getTimes();
            
            for(int i=0; i< times.size(); i++){
                timeIndex.push_back(times.at(i));
                logTimeIndex.push_back(times.at(i));
            }
            
            return;
        }
    }

    if(debug_input)
      std::cout << "Found metadata file" << std::endl;

    vtkXMLDataElement *root = parser->GetRootElement();
    vtkXMLDataElement *level = root->FindNestedElementWithName("timesteps");
    if(level != NULL){
    int ntimesteps = level->GetNumberOfNestedElements();
    
    if(debug_format)
        std::cout << "Found " << ntimesteps << " timesteps" << std::endl;
    
    for(int i=0; i < ntimesteps; i++){
        
        vtkXMLDataElement *xmltime = level->GetNestedElement(i);
        String timestr(xmltime->GetAttribute("time"));
        String logtimestr(xmltime->GetCharacterData());

        if(debug_input)
            std::cout << "time " << timestr << " index " << logtimestr << std::endl;
        
        double time = cdouble(timestr);
        int logtime = cint(logtimestr);

        logTimeIndex.push_back(logtime);
      
        timeIndex.push_back(time);
    }
    }
    else{
      fprintf(stderr, "No timesteps field found in index.xml, no physical time available\n");
    
      if(is_gidx)
      {
        for(int i=0; i< gidx_datasets.size(); i++){
         timeIndex.push_back(gidx_datasets[i].log_time);
         logTimeIndex.push_back(gidx_datasets[i].log_time);            
        } 

      }else
      {
          std::vector<double> times = reader->getTimes();
                
          for(int i=0; i< times.size(); i++){
            timeIndex.push_back(times.at(i));
            logTimeIndex.push_back(times.at(i));            
          }
      }
           
    }
  
    if(debug_input){
        std::cout << "loaded " << timeIndex.size() << " timesteps"<< std::endl;
        std::cout << reader->getNTimesteps() << " in the timesteps range of the IDX file" << std::endl;
    }
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
bool avtIDXFileFormat::data_query = false;

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
    
    std::cout << "--------------------------" << std::endl;
    if(use_extracells)
        std::cout << "Using extra cells" << std::endl;
    else
        std::cout << "Not using extra cells" << std::endl;
    
    if(use_raw)
        std::cout << "Using RAW format" << std::endl;
    else
        std::cout << "Not using RAW format" << std::endl;

    if(reverse_endian)
        std::cout << "Using Big Endian" << std::endl;
    else
        std::cout << "Using Little Endian" << std::endl;
    std::cout << "--------------------------" << std::endl;

#ifdef PARALLEL
    rank = PAR_Rank();
    nprocs = PAR_Size();
#else
    rank = 0;
    nprocs = 1;
#endif
    
    //std::cout << "~~~PROC " << rank << " / " << nprocs << std::endl;
  
#ifdef USE_VISUS
    reader = new VisusIDXIO(); // USE VISUS
#else
    reader = new PIDXIO(use_raw);     // USE PIDX
#endif
    
    dataset_filename = filename;

    size_t folder_point = dataset_filename.find_last_of("/\\");
    size_t ext_point = dataset_filename.find_last_of(".");
    String extension = dataset_filename.substr(ext_point+1, dataset_filename.size());
    is_gidx = extension.compare("gidx") == 0;

    memset(sfc_offset,0,sizeof(int)*3);

    if(dataset_filename.substr(folder_point+1,3).compare("SFC")==0){
      const char* sfc_v = dataset_filename.substr(folder_point+4,1).c_str();
      std::cout << "Use SFC "<< sfc_v << std::endl;
      if(*sfc_v == 'X')
            sfc_offset[0] = 1;
      else if(*sfc_v == 'Y')
        sfc_offset[1] = 1;
      else if(*sfc_v == 'Z')
        sfc_offset[2] = 1;
    }
    
    if(is_gidx){
        std::cout << "Using GIDX file" << std::endl;

        vtkSmartPointer<vtkXMLDataParser> parser = vtkSmartPointer<vtkXMLDataParser>::New();
    
        parser->SetFileName(dataset_filename.c_str());
        if (parser->Parse()){
            vtkXMLDataElement *root = parser->GetRootElement();

            int ntimesteps = root->GetNumberOfNestedElements();
    
            if(debug_format)
                std::cout << "Found " << ntimesteps << " timesteps in GIDX file" << std::endl;
    
            for(int i=0; i < ntimesteps; i++){
        
                vtkXMLDataElement *xmltime = root->GetNestedElement(i);
                String urlstr(xmltime->GetAttribute("url"));
                String timestr(xmltime->GetAttribute("log_time"));

                gidx_info ginfo;
                ginfo.url = urlstr.substr(7);
                ginfo.log_time = cint(timestr.c_str());
                gidx_datasets.push_back(ginfo);

                if(debug_input)
                    std::cout << "added dataset " << ginfo.url << " time " << ginfo.log_time << std::endl;

                logTimeIndex.push_back(ginfo.log_time);
                timeIndex.push_back(ginfo.log_time);
             }

             if(ntimesteps > 0)
                if (!reader->openDataset(gidx_datasets[0].url)) // open first dataset 
                {
                    std::cout <<"could not load "<<filename << std::endl;
                    return;
                }

        }else{
            std::cerr << "Cannot parse GIDX file " << dataset_filename << std::endl;
        }

    }
    else
    {
        String folder = dataset_filename.substr(0,folder_point);
        String dataset_name = dataset_filename.substr(folder_point,ext_point-folder_point);
        
        // "Standard" IDX metadata file (not yet standardized)
        metadata_filename = folder + dataset_name+"/"+dataset_name+".xml";

        if (!reader->openDataset(filename))
        {
            std::cout <<"could not load "<<filename << std::endl;
            return;
        }
    }
//    std::cout <<"dataset loaded";
    dim = reader->getDimension(); //<ctc> //NOTE: it doesn't work like we want. Instead, when a slice (or box) is added, the full data is read from disk then cropped to the desired subregion. Thus, I/O is never avoided.
  
    createBoxes();

    createTimeIndex();

    int old_size = level_info.patchInfo.size();
#ifdef PARALLEL
    //if(boxes.size()>1)
       loadBalance();
    // else
    //    pidx_decomposition(nprocs);
    fprintf(stderr,"NO PARALLEL YET\n");
#endif

    if(level_info.patchInfo.size()>old_size)
      parallel_boxes = true;
    else
      parallel_boxes = false;

// #ifdef USE_VISUS
//     loadBalance();
// #endif
  
   // calculateBoundsAndExtents();
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
    if(debug_format)
        std::cout<<"(avtIDXFileFormat destructor)" << std::endl;

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
    if(debug_format)
        std::cout<<"avtIDXFileFormat::FreeUpResources..." << std::endl;
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
    if(debug_format)
        std::cout << rank << ": Meta data" << std::endl;

    md->ClearMeshes();
    md->ClearScalars(); 
    md->ClearVectors();
    md->ClearLabels();

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "CC_Mesh";

#if USE_AMR    
    mesh->meshType = AVT_AMR_MESH;
    int totalPatches = level_info.patchInfo.size();
    mesh->numBlocks = totalPatches;
    mesh->blockOrigin = 0;
    //mesh->LODs = reader->getMaxResolution();
    mesh->spatialDimension = dim;
    mesh->topologicalDimension = dim;
    mesh->blockTitle = "patches";
    mesh->blockPieceName = "patch";
    
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
    mesh->groupTitle = "levels";
    mesh->groupPieceName = "level";
    mesh->numGroups = numLevels; // n AMR levels
    mesh->containsExteriorBoundaryGhosts = false;
    mesh->blockNames = pieceNames;

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
    level_info.getBounds(low,high,"CC_Mesh",use_extracells);

    //this can be done once for everything because the spatial range is the same for all meshes
    double box_min[3] = { level_info.anchor[0] + low[0] * level_info.spacing[0],
                            level_info.anchor[1] + low[1] * level_info.spacing[1],
                            level_info.anchor[2] + low[2] * level_info.spacing[2] };
    double box_max[3] = { level_info.anchor[0] + high[0] * level_info.spacing[0],
                        level_info.anchor[1] + high[1] * level_info.spacing[1],
                        level_info.anchor[2] + high[2] * level_info.spacing[2] };

    if(debug_format){
        printf("Dimensions %d\n", dim);
        printf("anchor %f %f %f spacing %f %f %f\n",level_info.anchor[0],level_info.anchor[1],level_info.anchor[2], level_info.spacing[0],level_info.spacing[1],level_info.spacing[2]);
        printf("global log %d %d %d - %d %d %d\n", low[0],low[1],low[2],high[0],high[1],high[2]);
        printf("global phy %f %f %f - %f %f %f\n", box_min[0],box_min[1],box_min[2],box_max[0],box_max[1],box_max[2]);
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
   
    //md->AddDefaultSILRestrictionDescription(std::string("!TurnOnAll"));
    
#ifdef PARALLEL // only PIDX
   // md->SetFormatCanDoDomainDecomposition(true);
#endif
   
    //if(timestate == 0){
    const std::vector<Field>& fields = reader->getFields();
    if(debug_format)
        std::cout << "adding " << fields.size() << "fields" << std::endl;
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
    
    if(debug_format)
      printf("%d: end meta\n", rank);

#if USE_AMR
    computeDomainBoundaries(mesh->name.c_str(), timestate);
#else
    SetUpDomainConnectivity(mesh->name.c_str());
#endif
    return;
}

void
avtIDXFileFormat::SetUpDomainConnectivity(const char* meshname)
{
    avtRectilinearDomainBoundaries *rdb =
      new avtRectilinearDomainBoundaries(true);
    int ndomains =level_info.patchInfo.size();
    rdb->SetNumDomains(ndomains);
    printf("Rect: Setting number of domains %d for mesh %s\n", ndomains, meshname);

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

    avtRectilinearDomainBoundaries *rdb =
    new avtRectilinearDomainBoundaries(true);
    rdb->SetNumDomains(level_info.patchInfo.size());
    printf("Setting number of domains %d for mesh %s\n", level_info.patchInfo.size(), meshname);
    for (long long i = 0 ; i < level_info.patchInfo.size() ; i++)
    {
        int low[3],high[3];
        level_info.patchInfo[i].getBounds(low,high,meshname, use_extracells);

        int e[6] = { low[0], high[0],
                   low[1], high[1],
                   low[2], high[2] };
        
        rdb->SetIndicesForAMRPatch(i,0,e);//SetIndicesForRectGrid(i, e);

    }

    rdb->CalculateBoundaries();
  
    this->mesh_boundaries[meshname] = void_ref_ptr(rdb,
                                   avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, timestate, -1, this->mesh_boundaries[meshname]);

    void_ref_ptr vrTmp = cache->GetVoidRef("any_mesh", // MUST be called any_mesh
                                           AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                           timestate, -1);
    if (*vrTmp == NULL || *vrTmp != *this->mesh_boundaries[meshname])
      fprintf(stderr,"pidx boundary mesh not registered\n");
    

    /*
    int totalPatches = level_info.patchInfo.size();
    int num_levels = 1;
    avtStructuredDomainNesting *dn = new avtStructuredDomainNesting(totalPatches, num_levels);
    
    dn->SetNumDimensions(3);
    
    std::vector< std::vector<int> > childPatches(totalPatches);
    */

    /* //OLD
    for (int p1=0; p1<totalPatches ; p1++) {
      int child_low[3],child_high[3];                                                                     
      level_info.patchInfo[p1].getBounds(child_low,child_high,meshname,use_extracells);                                            
      for (int p2=0; p2<totalPatches; p2++) {
              if(p1==p2) continue;

        PatchInfo &parentPatchInfo = level_info.patchInfo[p2];                                   
        int parent_low[3],parent_high[3];                                                                 
        parentPatchInfo.getBounds(parent_low,parent_high,meshname,use_extracells);                                 
        int mins[3], maxs[3];                                                                             
        for (int i=0; i<3; i++) {                                                                        
          mins[i] = std::max(child_low[i],  parent_low[i]);
          // *levelInfoChild.refinementRatio[i]);
          maxs[i] = std::min(child_high[i], parent_high[i]);// *levelInfoChild.refinementRatio[i]);     
        } 

        bool overlap = (mins[0]<maxs[0] &&
                        mins[1]<maxs[1] &&
                        mins[2]<maxs[2]);
        
        if (overlap) {
          printf("overlap %d %d \n", p1,p2); 
          int child_gpatch = p1;
          int parent_gpatch = p2;
          childPatches[parent_gpatch].push_back(child_gpatch);                                            
        }
      }
    }
    */
    /* 
    for (int p=0; p<totalPatches ; p++) {
      int my_level =0;
      int local_patch = p;

      PatchInfo &patchInfo = level_info.patchInfo[local_patch];
      int low[3],high[3];
      patchInfo.getBounds(low,high,meshname,use_extracells);
      
      std::vector<int> e(6);
      for (int i=0; i<3; i++) {
        e[i+0] = low[i];
        e[i+3] = high[i]-1;
      }
      
      childPatches[p].push_back(p);

      dn->SetNestingForDomain(p, my_level, childPatches[p], e);


      
    }

    this->mesh_domains[meshname]=void_ref_ptr(dn, avtStructuredDomainNesting::Destruct);
    vrTmp = cache->GetVoidRef("any_mesh", // MUST be called any_mesh
                              AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                              timestate, -1);
    if (*vrTmp == NULL || *vrTmp != *this->mesh_domains[meshname])
      fprintf(stderr,"pidx domain mesh not registered");
*/
    //}


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
    if(debug_format)
        std::cout<< rank << ": start getMesh "<< meshname << " domain " << domain << std::endl;   

    Box slice_box;
    
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    int my_dims[3];
    float *arrayX;
    float *arrayZ;
    float *arrayY;
    vtkFloatArray *coordsX;
    vtkFloatArray *coordsY;
    vtkFloatArray *coordsZ;

    int glow[3], ghigh[3];
    level_info.getBounds(glow,ghigh,meshname,use_extracells);

    //get patch bounds
    int low[3], high[3];
    level_info.patchInfo[domain].getBounds(low,high,meshname,use_extracells);

    for(int k=0; k<3; k++){
      my_dims[k] = high[k]-low[k]+1 +1; // for NON-nodeCentered no +1 ??(patch end is on high boundary)
      
      if(use_extracells && uintah_metadata)
        my_dims[k]--;
    }

    if(debug_format)
        std::cout << rank << ": dims " << my_dims[0] << " " << my_dims[1] << " " << my_dims[2] << std::endl;

    rgrid->SetDimensions(my_dims[0], my_dims[1], my_dims[2]);
     
    printf("global %d %d %d - %d %d %d local %d %d %d - %d %d %d\n",glow[0],glow[1],glow[2],ghigh[0],ghigh[1],ghigh[2],low[0],low[1],low[2],high[0],high[1],high[2]);
    printf("cellspacing %f %f %f\n", level_info.spacing[0],level_info.spacing[1],level_info.spacing[2]);

    for (int c=0; c<3; c++) {
      vtkFloatArray *coords = vtkFloatArray::New(); 
      coords->SetNumberOfTuples(my_dims[c]); 
      float *array = (float *)coords->GetVoidPointer(0); 
          
        for (int i=0; i<my_dims[c]; i++)
        {
          // Face centered data gets shifted towards -inf by half a cell.
          // Boundary patches are special shifted to preserve global domain.
          // Internal patches are always just shifted.
          float face_offset= 0;//-1.f;

          if (sfc_offset[c]) 
          {
              if (i==0)
                if (low[c]==glow[c]) // patch is on low boundary
                  face_offset += 0.0;
                else
                  face_offset += -0.5;       // patch boundary is internal to the domain
              else if (i==my_dims[c]-1)
                if (high[c]==ghigh[c]-1) // patch is on high boundary (added -1)
                  //if (levelInfo.periodic[c])  // periodic means one less value in the face-centered direction
                  //  face_offset += 0.0;
                  //else
                  face_offset += -1;
                else                        // patch boundary is internal to the domain
                  face_offset += -0.5;
              else
                face_offset += -0.5;
           }
    
            array[i] = level_info.anchor[c] + (i + low[c] + face_offset) * level_info.spacing[c];
          
            if(i==0)
              printf("low %d[%d]: %f\n", domain,c, array[i]);
            if(i==my_dims[c]-1)
              printf("high %d[%d]: %f\n", domain,c, array[i]);
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

    if(debug_format)
      printf("end mesh\n");
    return rgrid;
    
}

void
avtIDXFileFormat::GetCycles(std::vector<int> &cycles)
{
  // if(debug_format)
  //   printf("%d: entering getcycles\n", rank);
  for(int i = 0; i < reader->getNTimesteps(); ++i)
      cycles.push_back(i);

  // if(debug_format)
  //   printf("%d: exiting getcycles\n", rank);
}

void
avtIDXFileFormat::GetTimes(std::vector<double> &times)
{
  // if(debug_format)
  //   printf("%d: entering gettimes\n", rank);
  times.swap(timeIndex);

  // if(debug_format)
  //   printf("%d: exiting gettimes\n", rank);

}

vtkDataArray* avtIDXFileFormat::queryToVtk(int timestate, int domain, const char *varname){
    
    Box my_box;
    int low[3];
    int high[3];
    level_info.patchInfo[domain].getBounds(low,high,"CC_Mesh", use_extracells);

    std::cout << "read data " << level_info.patchInfo[domain].toString();
    for(int k=0; k<3; k++){
      if(uintah_metadata && use_extracells){
        low[k]++;
        //high[k]++;
      }
        my_box.p1[k] = low[k];
        my_box.p2[k] = high[k];
    }
    
    unsigned char* data = reader->getData(my_box, timestate, varname);

    if(debug_format)
      printf("read data done\n");
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
        std::cout << " NO DATA " << std::endl;
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
        ncomponents = 3; // Visit wants 3 components vectors
    
    long long ntotal = ntuples * ncomponents;
  
    // do not reverse endianess if data is compressed
    reverse_endian = reverse_endian * !reader->isCompressed();
  
    if(type == VisitIDXIO::IDX_UINT8){
        vtkUnsignedCharArray*rv = vtkUnsignedCharArray::New();
        rv->SetNumberOfComponents(ncomponents); //<ctc> eventually handle vector data, since visit can actually render it!
        
        if(isVector && dim < 3){
            
            unsigned char* newdata = convertComponents<unsigned char>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((unsigned char*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned char>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else
            rv->SetArray((unsigned char*)data,ncomponents*ntuples,1/*delete when done*/,vtkDataArrayTemplate<unsigned char>::VTK_DATA_ARRAY_FREE);
        return rv;
    }
    else if(type == VisitIDXIO::IDX_UINT16){
    
        vtkUnsignedShortArray *rv = vtkUnsignedShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
        
        if(isVector && dim < 3){
            
            unsigned short* newdata = convertComponents<unsigned short>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((unsigned short*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned short>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else
            rv->SetArray((unsigned short*)data,ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned short>::VTK_DATA_ARRAY_FREE);
        
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
            
            unsigned int* newdata = convertComponents<unsigned int>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((unsigned int*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned int>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else
            rv->SetArray((unsigned int*)data,ncomponents*ntuples,1,vtkDataArrayTemplate<unsigned int>::VTK_DATA_ARRAY_FREE);
        
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
            
            char* newdata = convertComponents<char>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((char*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<char>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else
            rv->SetArray((char*)data,ncomponents*ntuples,1,vtkDataArrayTemplate<char>::VTK_DATA_ARRAY_FREE);
        
        return rv;
    }
    else if(type == VisitIDXIO::IDX_INT16){
        vtkShortArray *rv = vtkShortArray::New();
        rv->SetNumberOfComponents(ncomponents);
        
        if(isVector && dim < 3){
            
            short* newdata = convertComponents<short>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((short*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<short>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else
            rv->SetArray((short*)data,ncomponents*ntuples,1,vtkDataArrayTemplate<short>::VTK_DATA_ARRAY_FREE);
        
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
            
            int* newdata = convertComponents<int>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((int*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else
            rv->SetArray((int*)data,ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        
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
            
            long* newdata = convertComponents<long>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((long*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<long>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else
            rv->SetArray((long*)data,ncomponents*ntuples,1,vtkDataArrayTemplate<long>::VTK_DATA_ARRAY_FREE);
        
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
            float* newdata = convertComponents<float>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((float*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else{
            rv->SetArray((float*)data,ncomponents*ntuples,1,vtkDataArrayTemplate<int>::VTK_DATA_ARRAY_FREE);
        }
        
        if(reverse_endian){
            float *buff = (float *) rv->GetVoidPointer(0);

            float min_value = 99999999999999.f;
            float max_value = -99999999999999.f;

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
            printf("range %f , %f\n", min_value, max_value);
            
            }
        return rv;
    }
    else if(type == VisitIDXIO::IDX_FLOAT64){
        
      //printf("DOUBLE creating array ncomp %d \n", ncomponents);
        vtkDoubleArray *rv = vtkDoubleArray::New();
        rv->SetNumberOfComponents(ncomponents);
        
        if(isVector && dim < 3){
            
            double* newdata = convertComponents<double>(data, field.ncomponents, 3, ntuples);
            rv->SetArray((double*)newdata,ncomponents*ntuples,1,vtkDataArrayTemplate<double>::VTK_DATA_ARRAY_FREE);
            
            delete data;
        }
        else{
         // printf("DOUBLE converting %d \n", ncomponents*ntuples);
            rv->SetArray((double*)data,ncomponents*ntuples,1,vtkDataArrayTemplate<double>::VTK_DATA_ARRAY_FREE);
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
  
    if(debug_format)
      printf("done data loading\n");
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

//vtkDataArray * avtIDXFileFormat::datatoreturn = NULL;
//const char *avtIDXFileFormat::curr_varname = NULL;
//int avtIDXFileFormat::activations = 0;

vtkDataArray *
avtIDXFileFormat::GetVar(int timestate, int domain, const char *varname)
{
    //std::cout<< rank << ": start getvar " << varname << " domain "<< domain;
  
  // get correspondig logic time
  if(debug_format)
   printf("Requested index time %d using logical time (IDX) %d\n", timestate, logTimeIndex[timestate]);

  if(is_gidx){
//     delete reader;

// #ifdef USE_VISUS
//     reader = new VisusIDXIO(); // USE VISUS
// #else
//     reader = new PIDXIO(use_raw);     // USE PIDX
// #endif
    reader->openDataset(gidx_datasets[timestate].url);
  }

  timestate = logTimeIndex[timestate];
  return queryToVtk(timestate, domain, varname);
    
}

void avtIDXFileFormat::ActivateTimestep(int ts){
    //printf("Activate timestep\n");

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

  std::cout<< rank << ": start getVectorVar " << varname << " domain "<< domain;
    timestate = logTimeIndex[timestate];
    return queryToVtk(timestate, domain, varname);
    
}
