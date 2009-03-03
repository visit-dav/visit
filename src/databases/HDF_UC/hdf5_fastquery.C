#include "hdf5_fastquery.h"
#include "base_api.h"
#include <assert.h>

HDF5_FQ::HDF5_FQ(){
        numHits=0;
        valid=false;
        numtimeSlice=1;
        tok=NULL;
        ibis::gParameters().add("fileManager.maxBytes", "3GB");

}

HDF5_FQ::~HDF5_FQ(){
        //delete [] dataSets;
        delete dataSets;
        deleteQuery();
}

void HDF5_FQ::deleteQuery(){
        for(int64_t i=0;i<numtimeSlice;i++){
                delete fastQuery[i];
        }
        delete [] fastQuery;
}

void HDF5_FQ::openFile(const char* name, const bool useH5PartFile){
        std::string temp = name;
        openFile(temp, useH5PartFile);
}

void HDF5_FQ::closeFile() {
    dataSets->closeHDF5File();
}

void HDF5_FQ::addNewTimeStep(double timeValue){
        dataSets->insertStep(timeValue);
}

void HDF5_FQ::openFile(const std::string & name, const bool useH5PartFile){
  //if this is a new file, close the last one
  
  if(valid){
    //std::cerr<<"HDF5_FQ::openFile() called on " << name 
    //         <<" This is a new file.. deleting old stuff " << std::endl;
    if (dataSets)
      delete dataSets;
    deleteQuery();
  }

  //std::cerr<<"HDF_FQ::openFile() allocating new dataSets, fastQuery" << std::endl;
  dataSets = new H5_Index(useH5PartFile);
  dataSets->openHDF5File(name);
  numtimeSlice=dataSets->getNumSteps();
  fastQuery = new H5_FQ_Timestep*[numtimeSlice];
  for(int64_t i=0;i<numtimeSlice;i++){
    fastQuery[i] = new H5_FQ_Timestep(*dataSets,(const int64_t)i);
  }

  valid=true;
}

void HDF5_FQ::prepareIndexes(){
        for(int64_t i=0;i<numtimeSlice;i++){
                delete fastQuery[i];
        }
        delete [] fastQuery;
        numtimeSlice=dataSets->getNumSteps();
        fastQuery = new H5_FQ_Timestep*[numtimeSlice];
        for(int64_t i=0;i<numtimeSlice;i++){
                fastQuery[i] = new H5_FQ_Timestep(*dataSets,(const int64_t)i);
        }
}

void HDF5_FQ::createNewVariable(char* variableName,
        std::vector<int64_t> dims,
        BaseFileInterface::DataType type,
        char* sData_center,
        char* sCoordsys,
        char* sScheme,
        char* sSchema_type){

        std::string temp = variableName;
        std::string temp2 = sData_center;
        std::string temp3 = sCoordsys;
        std::string temp4 = sScheme;
        std::string temp5 = sSchema_type;
        createNewVariable(temp,dims,type,temp2,temp3,temp4,temp5);
}

void HDF5_FQ::createNewVariable(const std::string & variableName,
        std::vector<int64_t> dims,
        BaseFileInterface::DataType type,
        const std::string & sData_center,
        const std::string & sCoordsys,
        const std::string & sScheme,
        const std::string & sSchema_type){
        
        dataSets->declareVariable(variableName,dims,type,sData_center,sCoordsys,sScheme,sSchema_type);
}

void HDF5_FQ::insertData(const char* variableName,
        int64_t timestep,
        const void* data,
        const BaseFileInterface::DataType type,
        const void* min,
        const void* max){

        std::string temp = variableName;
        insertData(temp,timestep,data,type,min,max);
}

void HDF5_FQ::insertData(const std::string & variableName,
        int64_t timestep,
        const void* data,
        const BaseFileInterface::DataType type,
        const void* min,
        const void* max){

        dataSets->insertVariableData(variableName,timestep,data,type,min,max);
}

void HDF5_FQ::getData(const char* variableName,
        int64_t time,
        void *data){

        std::string temp = variableName;
        getData(temp,time,data);
}

void HDF5_FQ::getData(const std::string & variableName,
        int64_t time,
        void *data){
        
        dataSets->getData(variableName,time,data);
}


void HDF5_FQ::getPointData(const char* variableName,
                           int64_t time,
                           void *data,
                           const std::vector<int32_t>& indices) {
        
    std::string temp = variableName;
        getPointData(temp,time,data, indices);
}


void HDF5_FQ::getPointData(const std::string & variableName,
                           int64_t time,
                           void *data,
                           const std::vector<int32_t>& indices) {
        
        dataSets->getPointData(variableName,time,data, indices);
}


int64_t HDF5_FQ::getDatasetSize(const std::vector<int64_t> dims,
                                       const BaseFileInterface::DataType type){
        return dataSets->getDatasetSize(dims,type);
}

void HDF5_FQ::getVariableInformation(const char* variableName,
                                int64_t time,
                                std::vector <int64_t> &dims,
                                BaseFileInterface::DataType *type){
        
        std::string temp = variableName;
        getVariableInformation(temp,time,dims,type);
}

void HDF5_FQ::getVariableInformation(const std::string & variableName,
                                int64_t time,
                                std::vector <int64_t> &dims,
                                BaseFileInterface::DataType *type){
        
        dataSets->getVariableInfo(variableName,time,dims,type);
}




void HDF5_FQ::buildSpecificTimeIndex(int64_t time){
        fastQuery[time]->buildIndexes();
}

void HDF5_FQ::buildAllIndexes(){
        for(int64_t i=0;i<numtimeSlice;i++){
                fastQuery[i]->buildIndexes();
        }
}


void HDF5_FQ::buildIndexes(const char* variableName, const char *binning) {

        for(int64_t i=0;i<numtimeSlice;i++){
                fastQuery[i]->buildIndex(variableName, binning);
        }
}

void HDF5_FQ::buildSpecificTimeIndex(int64_t time, const char* variableName, const char *binning) {

    fastQuery[time]->buildIndex(variableName, binning);
}



int64_t HDF5_FQ::getHitCount(char * query,int64_t time){
        tok = fastQuery[time]->createQuery(query);
        numHits = fastQuery[time]->submitQuery(tok);
        fastQuery[time]->destroyQuery(tok);
        return numHits;
}

void HDF5_FQ::executeQuery(char * query, int64_t time, std::vector<int32_t>& offset){
        tok = fastQuery[time]->createQuery(query);
        numHits = fastQuery[time]->submitQuery(tok);
        if (numHits>0)
          fastQuery[time]->getHitLocations((const char*)tok,offset);

        fastQuery[time]->destroyQuery(tok);
}

int64_t HDF5_FQ::executeEqualitySelectionQuery(const char * varname, 
                                               int64_t time, 
                                               std::vector<double>& identifiers, 
                                               std::vector<int32_t>& offset){
  
  ibis::gVerbose = 0;
  //std::cerr<<"HDF_FQ:: executeEqualityQuery, input identifiers size="<< identifiers.size() << std::endl;
  tok = fastQuery[time]->createEqualitySelectionQuery(varname, identifiers);

  numHits = fastQuery[time]->submitQuery(tok);

  //std::cerr<<"HDF_FQ:: after executing equalitySelection query, #hits = "<< numHits << std::endl;
  if (numHits>0)
    fastQuery[time]->getHitLocations((const char*)tok,offset);
  
  fastQuery[time]->destroyQuery(tok);
  return numHits;
}

void HDF5_FQ::getVariableNames(std::vector<std::string> &names){
        dataSets->getVariableNames(names);
}

int64_t HDF5_FQ::getStepCount(){
        return dataSets->getNumSteps();
}

bool HDF5_FQ::checkForVariable(const std::string & q_string){
        return dataSets->queryVariableNames(q_string);
}

void HDF5_FQ::getDataMinMax(const std::string& variableName,
                            int64_t timestep,
                            const BaseFileInterface::DataType type,
                            void *min,
                            void *max){
        dataSets->getTimeStepDataMinMax(variableName,timestep,type,min,max);
}

void HDF5_FQ::getDataMinMax_Double(const std::string& variableName,
                                    int64_t timestep,
                                     double& min,
                                    double& max){
  //  getDataMinMax(variableName, timestep, (BaseFileInterface::DataType)1, min, max);
  double range[2];
  bool result = false;

  // timestep has to be zero, since there's only 1 dataset open at a time..
  result = dataSets->getActualRange(variableName, 0, (void*)range);
  if (!result) {
    //std::cerr<<"ERROR:: HDF_FQ:: getDataMinMax call failed" << std::endl << std::endl;
  }

  min = range[0];
  max = range[1];
  
}



// Convert FastBit histogram to a hist where
// #bounds = #counts + 1
// count[i]<-->[bounds[i],bounds[i+1])
//
// FastBit histograms have a format wherein there are 2 extra bins
// 
long HDF5_FQ::convert_fastbit_to_hdf5_fq_hist(double* bound, uint32_t *count,
                                              int num_read,
                                              std::vector<double> &bounds,
                                              std::vector<uint32_t> &counts){
  
  int err = num_read;
  int i;

  if (err>0) {
    
    for (i=0;i<err-1;i++) { 
      // std::cout<<"bound "<<i<<" is "<<bound[i]<<std::endl;
      bounds.push_back(bound[i]);
    }

    for (i=1;i<err-1;i++) { // skip the first and last buckets
      // std::cout<<"count "<<i<<" is "<<count[i]<<std::endl;
      counts.push_back(count[i]);
    }
    
    //std::cout << "Pushed "<< bounds.size() << " elements into the bounds array..."  
    //          << "and    "<< counts.size() << " elements into counts array..." 
    //          << std::endl;
    err = counts.size();
  }
  else {
    //std::cout << "convert_fastbit_to_hdf5_fq_hist called with invalid num_read" << std::endl;
  }
  
  return err;
}

void HDF5_FQ::convert_fastbit_to_hdf5_fq_hist(std::vector<uint32_t> &counts){
  counts.pop_back();
  counts.erase(counts.begin());
}

// increase size of histogram to accomodate overflow and fill in some known junk values for debugging
void HDF5_FQ::prepare_empty_fastbit_hist(uint32_t &num_bins, double* &bound, uint32_t* &count) {
  num_bins = num_bins + 2;
  bound = new double[num_bins];
  count = new uint32_t[num_bins];
  for (unsigned int i=0; i<num_bins; i++){
    bound[i]=-9999.0;
    count[i]=9999;
  }
}


long HDF5_FQ::get1DHistogram(int64_t timestep,
                             const char* variableName,
                             std::vector<double> &bounds,
                             std::vector<uint32_t> &counts) {
  long err;
  err = fastQuery[timestep]->getDistribution(variableName, 
                                             bounds, 
                                             counts);  
  return err;
}

long HDF5_FQ::get1DHistogram(int64_t timestep,
                             const char* condition,
                             const char* variableName,
                             std::vector<double> &bounds,
                             std::vector<uint32_t> &counts) {
  long err;
  err = fastQuery[timestep]->getDistribution(condition,
                                             variableName, 
                                             bounds, 
                                             counts);  
  return err;
}

long HDF5_FQ::get1DHistogram(int64_t timestep,
                             const char* variableName,
                             uint32_t num_bins,
                             std::vector<double> &bounds,
                             std::vector<uint32_t> &counts) {
  long err;
  //  double* bound;
  //  double* count;
  counts.resize(num_bins);
  bounds.resize(num_bins-1);

  err = fastQuery[timestep]->getDistribution(variableName,  
                                             num_bins,
                                             &bounds[0], 
                                             &counts[0]);  
  return err;
}

long HDF5_FQ::get1DHistogram(int64_t timestep,
                             const char* condition,
                             const char* variableName,
                             uint32_t num_bins,
                             std::vector<double> &bounds,
                             std::vector<uint32_t> &counts) {
  long err;
  counts.resize(num_bins);
  bounds.resize(num_bins-1);

  err = fastQuery[timestep]->getDistribution(condition,
                                             variableName,
                                             num_bins,
                                             &bounds[0], 
                                             &counts[0]);  
  return err;
}


long HDF5_FQ::get1DHistogram(int64_t timestep,
                             const char* condition,
                             const char* variableName,
                             double begin, double end, double stride,
                             std::vector<uint32_t> &counts) {
  long err = -1;
  std::vector<size_t> count;

  assert(0);

/*
  err = fastQuery[timestep]->get1DDistribution(condition,
                                             variableName,
                                             begin, end, stride,
                                             count);
*/
  //TODO:: transfer result from count to counts;
  // Check this with John Wu
  //std::cout<<"get1DDistribution w/ strides returned " << err 
  //         << "count size is " << count.size() << std::endl;

  for (unsigned int i=0;i<count.size();i++)
    counts.push_back((uint32_t)count[i]);
  
  err = counts.size();
  count.clear();  
  return err;
}


long HDF5_FQ::get2DHistogram(int64_t timestep,
                             const char *condition, 
                             const char *variableName1,
                             double begin1, double end1, uint32_t num_bins1,
                             const char *variableName2,
                             double begin2, double end2, uint32_t num_bins2,
                             std::vector<double> &bounds1,
                             std::vector<double> &bounds2,
                             std::vector<uint32_t> &counts) {
    long err;
    
    double stride1; 
    stride1 = (end1 - begin1)/num_bins1;
    stride1 = ibis::util::incrDouble(stride1); // As per John Wu, this will make sure that we have exactly num_bins

    double stride2;
    stride2 = (end2 - begin2)/num_bins2;
    stride2 = ibis::util::incrDouble(stride2);

    ibis::gVerbose = 0;
    // there's only 1 timestep active at a time..
    timestep = 0;
    err = fastQuery[timestep]->get2DDistribution(condition, 
                                                 variableName1,
                                                 begin1, end1, stride1,
                                                 variableName2,
                                                 begin2, end2, stride2,
                                                 counts);
    
    ibis::gVerbose = 0;

    // Fill in the bounds array..
    /*
      bounds1.clear();
      double counter = begin1;
      while (counter<=end1) {
      bounds1.push_back(counter);
      counter = counter + stride1;
      }                    
      
      bounds2.clear();
      counter = begin2;
      while(counter<=end2) {
      bounds2.push_back(counter);
      counter = counter + stride2;
      }
    */
    bounds1.clear();
    for (int i=0; i<=num_bins1; i++)
      bounds1.push_back(begin1 + i*stride1);
    
    bounds2.clear();
    for (int i=0; i<=num_bins2; i++)
      bounds2.push_back(begin2 + i*stride2);
    
    //std::cout << "HDF_FQ:: get2DHistogram created "
    //          << " bounds1 [size= " << bounds1.size() << "], " 
    //         << " bounds2 [size= " << bounds2.size() << "]"
    //          << std::endl;
    return err;
}


long HDF5_FQ::get2DHistogram(int64_t timestep,
                             const char *condition,
                             const char *variableName1, 
                             const char *variableName2,
                             std::vector<double> &bounds1,
                             std::vector<double> &bounds2,
                             std::vector<uint32_t> &counts) {
  long err;  
  err = fastQuery[timestep]->getJointDistribution(condition,
                                                  variableName1, 
                                                  variableName2,
                                                  bounds1,
                                                  bounds2,
                                                  counts);
  return err;
}

long HDF5_FQ::get2DHistogram(int64_t timestep,
                             const char *condition, 
                             const char *variableName1,
                             double begin1, double end1, double stride1,
                             const char *variableName2,
                             double begin2, double end2, double stride2,
                             std::vector<uint32_t> &counts) {
    long err;
    std::vector<size_t> count;

    ibis::gVerbose = 0;
    //std::cout << "HDF_FQ:: starting get2DDistribution call with following info" <<
    //  variableName1 << "[" << begin1 << "," << end1 << "," << stride1 << "] " <<
    //  variableName2 << "[" << begin2 << "," << end2 << "," << stride2 << "] " << std::endl;
      
      
    err = fastQuery[timestep]->get2DDistribution(condition, 
                                                 variableName1,
                                                 begin1, end1, stride1,
                                                 variableName2,
                                                 begin2, end2, stride2,
                                                 counts);

    //std::cout << "HDF_FQ:: returned from 2DDistribution call with err=" << err << std::endl;
    ibis::gVerbose = 0;
    return err;
 }



long HDF5_FQ::get2DAdaptiveHistogram(int64_t timestep,
                                     const char *variableName1,
                                     const char *variableName2,
                                     uint32_t num_bins1,
                                     uint32_t num_bins2,
                                     std::vector<double> &bounds1,
                                     std::vector<double> &bounds2,
                                     std::vector<uint32_t> &counts) {
  
  long err;
  ibis::gVerbose = 0;
  
  err = fastQuery[timestep]->get2DDistribution(variableName1, variableName2,
                                               num_bins1, num_bins2,
                                               bounds1, bounds2,
                                               counts);
                                               
  //std::cout << "HDF_FQ:: returned from First New 2DAdaptiveDistribution call with err=" 
            << err << std::endl;
  ibis::gVerbose = 0;
  return err;
}


long HDF5_FQ::get2DAdaptiveHistogram(int64_t timestep,
                                     const char *condition, 
                                     const char *variableName1,
                                     const char *variableName2,
                                     uint32_t num_bins1, 
                                     uint32_t num_bins2,
                                     std::vector<double> &bounds1,
                                     std::vector<double> &bounds2,
                                     std::vector<uint32_t> &counts) {
  
  long err;
  ibis::gVerbose = 0;
  
  err = fastQuery[timestep]->get2DDistribution(condition,
                                               variableName1, variableName2,
                                               num_bins1, num_bins2,
                                               bounds1, 
                                               bounds2,
                                               counts);
                                               
  //std::cout << "HDF_FQ:: returned from Second New 2DAdaptiveDistribution call with err=" 
  //          << err << std::endl;
  ibis::gVerbose = 0;
  return err;
}
