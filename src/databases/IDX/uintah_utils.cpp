
#include "uintah_utils.h"
#include "visit_idx_io_types.h"

using namespace VisitIDXIO;

bool uintah_debug_input = false;

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


void parse_timestep(vtkSmartPointer<vtkXMLDataParser> parser, LevelInfo& level_info, int dim, bool use_extracells){
    
  vtkXMLDataElement *root = parser->GetRootElement();
  vtkXMLDataElement *level = NULL;
  //TODO unused variable  
  bool found_cellspacing = false;
  
  vtkXMLDataElement *anchor_el = NULL;
  if(root == NULL)
    std::cerr << "malformed XML"  << std::endl;
  else{
     anchor_el = root->FindNestedElementWithName("Grid");
  if(anchor_el == NULL)
    std::cerr << "anchor not found" << std::endl;    
  else
    {
     anchor_el = anchor_el->FindNestedElementWithName("Level");
     if(anchor_el == NULL)
       std::cerr << "anchor not found" << std::endl;  
     else anchor_el = anchor_el->FindNestedElementWithName("anchor");
    }
  }
  if(anchor_el == NULL){
    if(uintah_debug_input)
      std::cerr << "anchor not found" << std::endl;
    level_info.anchor[0]=0;
    level_info.anchor[1]=0;
    level_info.anchor[2]=0;
  }
  else{
    ups_parse_vector(anchor_el, level_info.anchor, dim);
  }
    
    //cellspacing
  
  vtkXMLDataElement *cellspacing_el = NULL;
  if(root == NULL)
    std::cerr << "malformed XML"  << std::endl;
  else{
    cellspacing_el = root->FindNestedElementWithName("Grid");
    if(cellspacing_el == NULL){
      if(uintah_debug_input)
      std::cerr << "cellspacing not found" << std::endl;
  }else{
    
    cellspacing_el= cellspacing_el->FindNestedElementWithName("Level");
    if(cellspacing_el == NULL){
      if(uintah_debug_input)
        std::cerr << "cellspacing not found" << std::endl;
    }  
    else{
      cellspacing_el= cellspacing_el->FindNestedElementWithName("cellspacing");
    }
  }
  }

  if(cellspacing_el == NULL){
    if(uintah_debug_input)
      std::cout << "cellspacing not found" << std::endl;

    found_cellspacing = false;
  }
  else
    ups_parse_vector(cellspacing_el, level_info.spacing, dim);

}

void compute_anchor(vtkXMLDataElement *level, double* anchor){
  int nboxes = level->GetNumberOfNestedElements();
  
  anchor[0] = 999999.9;
  anchor[1] = 999999.9;
  anchor[2] = 999999.9;

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

    for (int k=0; k < 3; k++){
      std::getline(ss1, p1s, ',');
      std::getline(ss2, p2s, ',');
      std::getline(ssSpace, espace, ',');
      std::getline(ress, res, ',');
      
      eCells[k] = cint(espace);
      resdata[k] = cint(res);
      
      p1phy[k] = cdouble(p1s);
      p2phy[k] = cdouble(p2s);
    
      anchor[k] = std::min(anchor[k], p1phy[k]);

    }
  }
}

struct uintah_box{
  int low[3];
  int high[3];
  int eCells[6];
};

void parse_ups(vtkSmartPointer<vtkXMLDataParser> parser, LevelInfo& level_info, int dim, bool use_extracells, const std::string grid_type){
  
  vtkXMLDataElement *root = parser->GetRootElement();
  vtkXMLDataElement *level = NULL;

  level = root->FindNestedElementWithName("Grid")->FindNestedElementWithName("Level");
 
  compute_anchor(level, level_info.anchor);
  
  int nboxes = level->GetNumberOfNestedElements();
  
  if(uintah_debug_input)
      std::cout << "Found " << nboxes << " boxes" << std::endl;

  std::vector<uintah_box> input_boxes;

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

    uintah_box box;
  
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
      
      box.eCells[k] = cint(espace);
      resdata[k] = cint(res);
      
      p1phy[k] = cdouble(p1s);
      p2phy[k] = cdouble(p2s);
    
      phy2log[k] = (p2phy[k]-p1phy[k])/(resdata[k]);
 
      if(nboxes == 1){ // single box case
        box.low[k] = 0;
        box.high[k] = box.low[k] + resdata[k];
      }
      else{ // multibox case 
        box.low[k] = cround(std::fabs(p1phy[k]-level_info.anchor[k]) / phy2log[k]);// + eCells[k];
	//printf("low[%d] %d p1phy %f\n", k, box.low[k], p1phy[k]);
        // if(k==0 && low[k]>0)
        //   low[k] += 1;
        box.high[k] = box.low[k] + resdata[k];
      }

      if(p1log_el != NULL)
        ups_parse_vector(p1log_el, box.low, dim);
      if(p2log_el != NULL)
        ups_parse_vector(p2log_el, box.high, dim);

      level_info.spacing[k] = phy2log[k];

      // Impose Simmetric extracells! We ll fix them later
      box.eCells[3] = box.eCells[0];
      box.eCells[4] = box.eCells[1];
      box.eCells[5] = box.eCells[2];
    }
    
    input_boxes.push_back(box);
  }

  // check intersecting boxes and fix extra cells

  for(int i=0; i<input_boxes.size(); i++){
      int* low  = input_boxes[i].low;
      int* high = input_boxes[i].high;
      int* eCells = input_boxes[i].eCells;

    for(int j=0; j<input_boxes.size() && i!=j; j++){
        int* tlow    = input_boxes[j].low;
        int* thigh   = input_boxes[j].high;
        int* teCells = input_boxes[j].eCells;

        bool over[3];  
              
        int neig_low[3];
        int neig_high[3];

        int inter_low[3];
        int inter_high[3];
        bool box_intersect = intersect(over,low,high,tlow,thigh, inter_low,inter_high);

        for(int d=0; d < 3; d++){
          if(box_intersect && over[d]){
            if(uintah_debug_input)
              printf("INTERSECT %d & %d dir %d low %d high %d\n", i, j, d, inter_low[d], inter_high[d]);

            if(inter_low[d] == low[d]){
              int gap = inter_high[d]-low[d];
              low[d]    = inter_high[d]-gap;
              high[d]   = high[d]+gap;
              //eCells[d] = 0;  // was on, removing extracell only on one of the boxes, because they could be overlapping only partially
              //teCells[d+3] = 0;
            }
            else if(inter_low[d] == tlow[d]){
              int gap = inter_high[d]-low[d];
              tlow[d] = inter_high[d]-gap;
              thigh[d]   = thigh[d]+gap;
              //teCells[d] = 0;
              //eCells[d+3] = 0; // was on
            }

          }
      }
    }
  }

  for(int i=0; i<input_boxes.size(); i++){
    int* low  = input_boxes[i].low;
    int* high = input_boxes[i].high;
    int* eCells = input_boxes[i].eCells;

    PatchInfo box;
    
    box.setBounds(low,high,eCells, grid_type);

    level_info.patchInfo.push_back(box);

    if(uintah_debug_input){
      //std::cout <<"Read box phy: p1 " << p1phy << " p2 "<< p2phy << std::endl;
      std::cout << level_info.patchInfo.back().toString();
      //std::cout <<"     box log: p1 " << p1log << " p2 "<< p2log << std::endl;
    }
      
    if(uintah_debug_input){
      printf("anchor point %f %f %f\n",level_info.anchor[0],level_info.anchor[1],level_info.anchor[2]);
      printf("cell spacing %f %f %f\n",level_info.spacing[0],level_info.spacing[1],level_info.spacing[2]);
    }
  }

}
