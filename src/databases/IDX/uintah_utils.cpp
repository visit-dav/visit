
#include "uintah_utils.h"
#include "visit_idx_io_types.h"

using namespace VisitIDXIO;

bool uintah_debug_input = true;

void ups_parse_vector(vtkXMLDataElement *el, double* vec, int dim){
  std::string el_str(el->GetCharacterData());
    
  el_str = trim(el_str);
  std::string el_vals = el_str.substr(1,el_str.length()-2);
  if(uintah_debug_input)
    std::cout << "Found vec " << el_vals << std::endl;
  std::string anchs;
  std::stringstream anch_ss(el_vals);
  for (int k=0; k < dim; k++){
    std::getline(anch_ss, anchs, ',');
     
    vec[k] = cfloat(anchs);
  }
  if(uintah_debug_input)
    std::cout << "read " << vec[0] << " "<< vec[1] << " " << vec[2] << std::endl;
}

void ups_parse_vector(vtkXMLDataElement *el, int* vec, int dim){
  std::string el_str(el->GetCharacterData());
      
  el_str = trim(el_str);
  std::string el_vals = el_str.substr(1,el_str.length()-2);
  if(uintah_debug_input)
    std::cout << "Found vec " << el_vals << std::endl;
  std::string anchs;
  std::stringstream anch_ss(el_vals);
  for (int k=0; k < dim; k++){
    std::getline(anch_ss, anchs, ',');
       
    vec[k] = cint(anchs);
  }
  if(uintah_debug_input)
    std::cout << "read " << vec[0] << " "<< vec[1] << " " << vec[2] << std::endl;
}

void parse_ups(vtkSmartPointer<vtkXMLDataParser> parser, LevelInfo& level_info, int dim, bool use_extracells){
  
  vtkXMLDataElement *root = parser->GetRootElement();
  vtkXMLDataElement *level = NULL;
  bool found_cellspacing = false;

  level = root->FindNestedElementWithName("Grid")->FindNestedElementWithName("Level");

  vtkXMLDataElement *anchor_el = NULL;
  anchor_el = root->FindNestedElementWithName("Grid")->FindNestedElementWithName("anchor");

  if(anchor_el == NULL){
    if(uintah_debug_input)
      std::cout << "anchor not found" << std::endl;
    level_info.anchor[0]=0;
    level_info.anchor[1]=0;
    level_info.anchor[2]=0;
  }
  else{
    ups_parse_vector(anchor_el, level_info.anchor, dim);
  }
    
    //cellspacing
  vtkXMLDataElement *cellspacing_el = NULL;
  cellspacing_el = root->FindNestedElementWithName("Grid")->FindNestedElementWithName("cellspacing");

  if(cellspacing_el == NULL){
    if(uintah_debug_input)
      std::cout << "cellspacing not found" << std::endl;

    found_cellspacing = false;
  }
      

  int nboxes = level->GetNumberOfNestedElements();
  
  if(uintah_debug_input)
      std::cout << "Found " << nboxes << " boxes" << std::endl;

  int last_log = 0;
  for(int i=0; i < nboxes; i++){

    vtkXMLDataElement *xmlbox = level->GetNestedElement(i);
    std::string lower(xmlbox->FindNestedElementWithName("lower")->GetCharacterData());
    std::string upper(xmlbox->FindNestedElementWithName("upper")->GetCharacterData());
      
    vtkXMLDataElement* p1log_el = xmlbox->FindNestedElementWithName("p1log");
    vtkXMLDataElement* p2log_el = xmlbox->FindNestedElementWithName("p2log");

    std::string extra_cells = "[0 0 0]";

    extra_cells = std::string(xmlbox->FindNestedElementWithName("extraCells")->GetCharacterData());
    
    std::string resolution(xmlbox->FindNestedElementWithName("resolution")->GetCharacterData());
    lower = trim(lower);
    upper = trim(upper);
    
    resolution = trim(resolution);
  
    lower = lower.substr(1,lower.length()-2);
    upper = upper.substr(1,upper.length()-2);
    
    // TODO Do we still need extra cells managements as a flag?
    extra_cells = extra_cells.substr(1,extra_cells.length()-2);
    resolution = resolution.substr(1,resolution.length()-2);
    
    if(uintah_debug_input)
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
 
      if(nboxes == 1){ // single box case
        low[k] = 0;
        high[k] = low[k] + resdata[k];
      }
      else{ // multibox case (all inside the same domain)
        low[k] = std::fabs(p1phy[k]-level_info.anchor[k]) / phy2log[k] + eCells[k];
        high[k] = low[k] + resdata[k];
      }

      if(p1log_el != NULL)
        ups_parse_vector(p1log_el, low, dim);
      if(p2log_el != NULL)
        ups_parse_vector(p2log_el, high, dim);
    }
    
    PatchInfo box;
    // Note: Simmetric extracells !!!! Not always true
    eCells[3] = eCells[0];
    eCells[4] = eCells[1];
    eCells[5] = eCells[2];
    box.setBounds(low,high,eCells,"CC");

    level_info.patchInfo.push_back(box);

    if(uintah_debug_input){
      std::cout <<"Read box phy: p1 " << p1phy << " p2 "<< p2phy << std::endl;
      std::cout << level_info.patchInfo.back().toString();
      //std::cout <<"     box log: p1 " << p1log << " p2 "<< p2log << std::endl;
    }

    if(!found_cellspacing){
      if(uintah_debug_input)
        printf("Cellspacing not found, calculating\n");

      PatchInfo& box = level_info.patchInfo[0];

      int low[3];
      int high[3];
      box.getBounds(low,high,"CC",use_extracells);

      for(int k=0; k<3; k++){
        level_info.spacing[k] = (p2phy[k]-p1phy[k])/(high[k]-low[k]+1);
        
        if(uintah_debug_input)
          printf("%f - %f / %d - %d +1\n",p2phy[k],p1phy[k],high[k],low[k]);
      }
    }
  }
}