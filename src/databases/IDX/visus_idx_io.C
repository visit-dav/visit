/***************************************************
 ** ViSUS Visualization Project                    **
 ** Copyright (c) 2010 University of Utah          **
 ** Scientific Computing and Imaging Institute     **
 ** 72 S Central Campus Drive, Room 3750           **
 ** Salt Lake City, UT 84112                       **
 **                                                **
 ** For information about this project see:        **
 ** http://www.pascucci.org/visus/                 **
 **                                                **
 **      or contact: pascucci@sci.utah.edu         **
 **                                                **
 ****************************************************/

#include <visus.h>
#include <string>
#include <visuscpp/db/dataset/visus_db_dataset.h>
#include <visuscpp/kernel/geometry/visus_position.h>
#include <visuscpp/kernel/core/visus_path.h>
#include <visuscpp/idx/visus_idx_dataset.h>

#include "visus_idx_io.h"

//using namespace VisitIDXIO;
using namespace Visus;
#ifdef USE_VISUS
static Application app;
#endif

class DatasetImpl
{
public:
    DatasetImpl(String filename)
    {
        dataset = Dataset::loadDataset(filename);
    }

    Dataset* get(){
        return dataset;
    }
    
    Dataset* dataset;
    
    ~DatasetImpl()
    {
        if(dataset != nullptr)
            delete dataset;
    }
};

VisitIDXIO::DTypes convertType(DType intype)
{
  if(intype == Visus::DTypes::INT8 || intype.isVectorOf(Visus::DTypes::INT8))
        return VisitIDXIO::IDX_INT8;
    else if(intype == Visus::DTypes::UINT8 || intype.isVectorOf(Visus::DTypes::UINT8))
        return VisitIDXIO::IDX_UINT8;
    else if(intype == Visus::DTypes::INT16 || intype.isVectorOf(Visus::DTypes::INT16))
        return VisitIDXIO::IDX_INT16;
    else if(intype == Visus::DTypes::UINT16 || intype.isVectorOf(Visus::DTypes::UINT16))
        return VisitIDXIO::IDX_UINT16;
    else if(intype == Visus::DTypes::INT32 || intype.isVectorOf(Visus::DTypes::INT32))
        return VisitIDXIO::IDX_INT32;
    else if(intype == Visus::DTypes::UINT32 || intype.isVectorOf(Visus::DTypes::INT32))
        return VisitIDXIO::IDX_UINT32;
    else if(intype == Visus::DTypes::INT64 || intype.isVectorOf(Visus::DTypes::INT64))
        return VisitIDXIO::IDX_INT64;
    else if(intype == Visus::DTypes::UINT64 || intype.isVectorOf(Visus::DTypes::UINT64))
        return VisitIDXIO::IDX_UINT64;
    else if(intype == Visus::DTypes::FLOAT32 || intype.isVectorOf(Visus::DTypes::FLOAT32))
        return VisitIDXIO::IDX_FLOAT32;
    else if(intype == Visus::DTypes::FLOAT64 || intype.isVectorOf(Visus::DTypes::FLOAT64)){
        return VisitIDXIO::IDX_FLOAT64;
    }

    VisusWarning() << "No type found for conversion";
    VisusAssert(false);
}

VisusIDXIO::~VisusIDXIO()
{
    if (datasetImpl != nullptr)
        delete datasetImpl;
}

bool VisusIDXIO::openDataset(const String filename)
{
    String name("file://"); name += Path(filename).toString();
    
    datasetImpl = new DatasetImpl(name);
    
    Dataset* dataset = datasetImpl->get();
    
    dataset_url = name;
    
    if (!dataset){
        VisusWarning() <<"Could not load dataset "<<filename << std::endl;
        return false;
    }
    
    dims=dataset->getDimension();
    
    ntimesteps = std::max(1, (int)(dataset->getTimesteps()->getMax() - dataset->getTimesteps()->getMin())+1);
    
    tsteps = dataset->getTimesteps()->asVector();
    max_resolution = dataset->getMaxResolution();
  
    NdBox orig_box = dynamic_cast<IdxDataset*>(dataset)->getOriginalBox();
  
    VisusInfo() << "original box "<< orig_box.toString();
 // VisusInfo() <<orig_box.p1().toString() << "  " << orig_box.p2().toString();
  
    if (orig_box == NdBox()){
      compressed_dataset = false;
      VisusInfo() << "NOT COMPRESSED";
    }
    else{
      compressed_dataset = true;
      VisusInfo() << "COMPRESSED";
    }
  
    const std::vector<Visus::Field>& dfields = dataset->getFields();
    
    for (int i = 0; i < (int) dfields.size(); i++)
    {
        std::string fieldname = dfields[i].name;
        
        Visus::Field field = dataset->getFieldByName(fieldname);
        VisitIDXIO::Field my_field;
        
        my_field.type = convertType(field.dtype);
        my_field.isVector = (compressed_dataset) ? false : field.dtype.isVector();
        my_field.ncomponents = (compressed_dataset) ? 1 : field.dtype.ncomponents();
        my_field.name = fieldname;
        
        fields.push_back(my_field);
    }
  
    NdBox lb = (compressed_dataset) ? orig_box : dataset->getLogicBox();
  
    //NdBox lb = dataset->getLogicBox();
    memcpy(logic_to_physic, dataset->getLogicToPhysic().mat, 16*sizeof(double));

    for(int i=0; i < 3; i++){
        logic_box.p1[i] = lb.p1()[i];
        logic_box.p2[i] = lb.p2()[i];
    }
  
  if(compressed_dataset){
    for(int i=0; i < 3; i++){
      logic_box.p2[i]--;
    }
  }
  
//    std::cout << "logic box " <<logic_box.p1 << " p2 " << logic_box.p2 << std::endl;
  
    return true;
    
}

unsigned char* VisusIDXIO::getData(const VisitIDXIO::Box box, const int timestate, const char* varname)
{
    Dataset* dataset = datasetImpl->get();
    
    if (dataset == nullptr)
    {
        VisusWarning() <<"Dataset not loaded "<<dataset_url << std::endl;
        return NULL;
    }
    
    if (!dataset->getTimesteps()->containsTimestep(timestate)){
        return NULL;
    }
    
    Access* access = dataset->createAccess();

    Visus::Field field = dataset->getFieldByName(varname);
    
    curr_field.type = convertType(field.dtype);
  // TODO better
    curr_field.isVector = (compressed_dataset) ? false : field.dtype.isVector();
    curr_field.ncomponents = (compressed_dataset) ? 1 : field.dtype.ncomponents();
    curr_field.name = varname;
    
    Visus::NdBox my_box;
    int zp2 = (dims == 2) ? 1 : box.p2.z;
    
    NdPoint p1(box.p1.x,box.p1.y,box.p1.z);
    NdPoint p2(box.p2.x,box.p2.y,zp2,1,1);
    my_box.setP1(p1);
    my_box.setP2(p2);
    
    VisusInfo() << " Box query " << p1.toString() << " p2 " << p2.toString() << " variable " << varname << " time " << timestate;
    
    Query* box_query = new Query(dataset,'r');
  
    if(!compressed_dataset){
      box_query->setLogicPosition(my_box);
    }
    else{
      box_query->setOriginalLogicPosition(my_box);
    }
    box_query->setField(dataset->getFieldByName(varname));
    
    box_query->setTime(timestate);
    
    box_query->setStartResolution(0);
    box_query->addEndResolution(max_resolution);
    box_query->setMaxResolution(max_resolution);
    
    // -------- This can be used for lower resolution queries
    //    box_query->addEndResolution(sres);
    //    box_query->addEndResolution(hr);
    //    box_query->setMergeMode(Query::InterpolateSamples);
    // --------
    
    box_query->setAccess(access);
    box_query->begin();
    
    VisusReleaseAssert(!box_query->end());
  
  if(!compressed_dataset)
      VisusReleaseAssert(box_query->execute());
  else
      VisusReleaseAssert(box_query->executeAndDecompress());

    // -------- This can be used for lower resolution queries
    //    box_query->next();
    //    VisusReleaseAssert(!box_query->end());
    // --------
    
    printf("idx query result (dim %dx%dx%d) = %lld:\n", box_query->getBuffer()->getWidth(), box_query->getBuffer()->getHeight(), box_query->getBuffer()->getDepth(), box_query->getBuffer()->c_size());
  
    delete access;
    
    SharedPtr<Array> data = box_query->getBuffer();

    if( data->c_ptr() != NULL)
         std::cout << "size data bytes " << data->c_size();

    return (unsigned char*)data->c_ptr();
   
}

