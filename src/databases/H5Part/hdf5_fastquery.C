#include "hdf5_fastquery.h"
#include "base_api.h"

#ifdef HAVE_LIBFASTBIT

/// Constructor.
///@arg v Verbose level.  Default to 0.
///@arg rcfile The name of runtime-control file.  The default value is nil,
///in which case it will examine the following values to find a rc file.
///If none is fine, the run-time parameter list is empty.
/// - Environment variable IBISRC.
/// - File named ibis.rc in the current working directory.
/// - File named .ibisrc in the current working directory.
/// - File named .ibisrc in the current user's home directory (only if 
///   the environmental variable HOME is defined).
///@arg loffile The name of the log file used for output error messages.
HDF5_FQ::HDF5_FQ(const int v, const char* rcfile, const char* logfile)
    : dataFile(0) {
    ibis::init(v, rcfile, logfile);
    //  ibis::gParameters().add("fileManager.maxBytes", "3GB");
    // ibis::util::setLogFileName("FastBit.Log");
}

HDF5_FQ::~HDF5_FQ() {
    ibis::util::clean(timeSlices);
    delete dataFile;
    ibis::fileManager::instance().clear();
    ibis::util::closeLogFile();
}

void HDF5_FQ::deleteSlices() {
    ibis::util::clean(timeSlices);
}

void HDF5_FQ::createSlices() {
    if (dataFile == 0) return;

    int64_t numSlices = dataFile->getNumSteps();
    size_t actualNum = static_cast<size_t>(numSlices);
    if (actualNum == numSlices) {
        timeSlices.resize(actualNum);
        for(size_t i = 0; i < actualNum; ++i)
            timeSlices[i] = new H5_FQ_Timestep(*dataFile, (const int64_t)i);
    }
    else {
        LOGGER(ibis::gVerbose > 0)
            << "Warning -- HDF5_FQ::createSlices is not able to continue "
            "because getNumSteps on " << dataFile->getFileName()
            << " returned " << numSlices;
    }
}

void HDF5_FQ::openFile(const char* name, const bool useH5PartFile) {
    std::string temp = name;
    openFile(temp, useH5PartFile);
}

void HDF5_FQ::closeFile() {
    deleteSlices();
    dataFile->closeHDF5File();
}

void HDF5_FQ::addNewTimeStep(double timeValue){
    dataFile->insertStep(timeValue);
}

void HDF5_FQ::openFile(const std::string &name, const bool useH5PartFile) {
    if (dataFile != 0) {
        std::string oldname = dataFile->getFileName();
        if (name.compare(oldname) == 0) {
            // same file name as the existing one, do nothing
            return;
        }
        LOGGER(ibis::gVerbose > 0 && ! oldname.empty())
            << "HDF5_FQ::openFile will close " << oldname << " and then open "
            << name;
    }

    // perform the clean up
    ibis::util::clean(timeSlices);
// would prefer to simply close the existing file rather than create a new
// dataFile object, however, it does not seem to work as of May 5, 2009
//     if (dataFile != 0)
//      dataFile->closeHDF5File();
//     else
//      dataFile = new H5_Index(useH5PartFile);
    delete dataFile;
    dataFile = new H5_Index(useH5PartFile);
    // open the new file
    dataFile->openHDF5File(name);
    createSlices();
}

void HDF5_FQ::prepareIndexes() {
    deleteSlices();
    createSlices();
}

void HDF5_FQ::createNewVariable(char* variableName,
                                std::vector<int64_t> dims,
                                BaseFileInterface::DataType type,
                                char* sData_center,
                                char* sCoordsys,
                                char* sScheme,
                                char* sSchema_type) {
    std::string temp = variableName;
    std::string temp2 = sData_center;
    std::string temp3 = sCoordsys;
    std::string temp4 = sScheme;
    std::string temp5 = sSchema_type;
    createNewVariable(temp, dims, type, temp2, temp3, temp4, temp5);
}

void HDF5_FQ::createNewVariable(const std::string& variableName,
                                std::vector<int64_t> dims,
                                BaseFileInterface::DataType type,
                                const std::string& sData_center,
                                const std::string& sCoordsys,
                                const std::string& sScheme,
                                const std::string& sSchema_type) {
    dataFile->declareVariable(variableName, dims, type, sData_center,
                              sCoordsys, sScheme, sSchema_type);
}

void HDF5_FQ::insertData(const char* variableName,
                         int64_t timestep,
                         const void* data,
                         const BaseFileInterface::DataType type,
                         const void* min,
                         const void* max) {
    std::string temp = variableName;
    insertData(temp,timestep,data,type,min,max);
}

void HDF5_FQ::insertData(const std::string & variableName,
                         int64_t timestep,
                         const void* data,
                         const BaseFileInterface::DataType type,
                         const void* min,
                         const void* max) {
    dataFile->insertVariableData(variableName, timestep, data, type, min, max);
}

void HDF5_FQ::getData(const char* variableName,
                      int64_t time,
                      void *data) {
    std::string temp = variableName;
    getData(temp,time,data);
}

void HDF5_FQ::getData(const std::string & variableName,
                      int64_t time,
                      void *data) {
    dataFile->getData(variableName, time, data);
}

void HDF5_FQ::getPointData(const char* variableName,
                           int64_t time,
                           void *data,
                           const std::vector<hsize_t>& indices) {
    std::string temp = variableName;
    getPointData(temp, time, data, indices);
}

void HDF5_FQ::getPointData(const std::string & variableName,
                           int64_t time,
                           void *data,
                           const std::vector<hsize_t>& indices) {
    dataFile->getPointData(variableName, time, data, indices);
}

int64_t HDF5_FQ::getDatasetSize(const std::vector<int64_t> dims,
                                const BaseFileInterface::DataType type){
    return dataFile->getDatasetSize(dims,type);
}

void HDF5_FQ::getVariableInformation(const char* variableName,
                                     int64_t time,
                                     std::vector <int64_t> &dims,
                                     BaseFileInterface::DataType *type){
        
    std::string temp = variableName;
    getVariableInformation(temp, time, dims, type);
}

void HDF5_FQ::getVariableInformation(const std::string &variableName,
                                     int64_t time,
                                     std::vector <int64_t> &dims,
                                     BaseFileInterface::DataType *type){
    dataFile->getVariableInfo(variableName, time, dims, type);
}

void HDF5_FQ::buildSpecificTimeIndex(int64_t time) {
    timeSlices[time]->buildIndexes();
}

void HDF5_FQ::buildAllIndexes() {
    for(int64_t i=0; i<numTimeSlices(); i++){
        timeSlices[i]->buildIndexes();
    }
}

void HDF5_FQ::buildIndexes(const char* variableName, const char *binning) {
    for(int64_t i=0; i<numTimeSlices(); i++){
        static_cast<H5_FQ_Timestep*>(timeSlices[i])->
            buildIndex(variableName, binning);
    }
}

void HDF5_FQ::buildSpecificTimeIndex(int64_t time, const char* variableName,
                                     const char *binning) {
    static_cast<H5_FQ_Timestep*>(timeSlices[time])->
        buildIndex(variableName, binning);
}

int64_t HDF5_FQ::getHitCount(const char * query, int64_t time){
    H5_FQ_Timestep *tstep = static_cast<H5_FQ_Timestep*>(timeSlices[time]);
    const char* tok = tstep->createQuery(query);
    int64_t numHits = tstep->submitQuery(tok);
    tstep->destroyQuery(tok);
    return numHits;
}

void HDF5_FQ::executeQuery(const char * query, int64_t time,
                           std::vector<hsize_t>& offset){
    H5_FQ_Timestep *tstep = static_cast<H5_FQ_Timestep*>(timeSlices[time]);
    const char* tok = tstep->createQuery(query);
    int64_t numHits = tstep->submitQuery(tok);
    if (numHits > 0)
        tstep->getHitLocations(tok, offset);

    tstep->destroyQuery(tok);
}

int64_t HDF5_FQ::executeEqualitySelectionQuery
(const char * varname,
 int64_t time,
 const std::vector<double>& identifiers,
 std::vector<hsize_t>& offset) {
    H5_FQ_Timestep *tstep = static_cast<H5_FQ_Timestep*>(timeSlices[time]);
    const char* tok =
        tstep->createEqualitySelectionQuery(varname, identifiers);
    int64_t numHits = tstep->submitQuery(tok);
    if (numHits > 0)
        tstep->getHitLocations(tok,offset);
  
    tstep->destroyQuery(tok);
    return numHits;
}

void HDF5_FQ::getVariableNames(std::vector<std::string> &names) const {
    dataFile->getVariableNames(names);
}

int64_t HDF5_FQ::getStepCount() const {
    return dataFile->getNumSteps();
}

bool HDF5_FQ::checkForVariable(const std::string & q_string) const{
    return dataFile->queryVariableNames(q_string);
}

/// @note Assumes min and max correctly matches type.
bool HDF5_FQ::getDataMinMax(const std::string& variableName,
                            int64_t timestep,
                            const BaseFileInterface::DataType type,
                            void *min,
                            void *max) {
    bool ret = false;
    switch (type) {
    case BaseFileInterface::H5_Float: {
        float buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        *(float*)min = buff[0];
        *(float*)max = buff[1];
        break;}
    case BaseFileInterface::H5_Double: {
        double buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        *(double*)min = buff[0];
        *(double*)max = buff[1];
        break;}
    case BaseFileInterface::H5_Int32: {
        int32_t buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        *(int32_t*)min = buff[0];
        *(int32_t*)max = buff[1];
        break;}
    case BaseFileInterface::H5_Int64: {
        int64_t buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        *(int64_t*)min = buff[0];
        *(int64_t*)max = buff[1];
        if (ret == true)
            ret = (*static_cast<int64_t*>(min) == buff[0] &&
                   *static_cast<int64_t*>(max) == buff[1]);
        break;}
    case BaseFileInterface::H5_Byte: {
        char buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        *(char*)min = buff[0];
        *(char*)max = buff[1];
        break;}
    default: {
        break;}
    }
    return ret;
}

bool HDF5_FQ::getDataMinMax_Double(const std::string& variableName,
                                   int64_t timestep,
                                   double& min,
                                   double& max){
    BaseFileInterface::DataType type;
    std::vector<int64_t> dims;
    bool ret =
        (0 <= dataFile->getVariableInfo(variableName, timestep, dims, &type));
    if (! ret) return ret;

    switch (type) {
    case BaseFileInterface::H5_Float: {
        float buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        min = buff[0];
        max = buff[1];
        break;}
    case BaseFileInterface::H5_Double: {
        double buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        min = buff[0];
        max = buff[1];
        break;}
    case BaseFileInterface::H5_Int32: {
        int32_t buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        min = buff[0];
        max = buff[1];
        break;}
    case BaseFileInterface::H5_Int64: {
        int64_t buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        min = buff[0];
        max = buff[1];
        if (ret == true)
            ret = (static_cast<int64_t>(min) == buff[0] &&
                   static_cast<int64_t>(max) == buff[1]);
        break;}
    case BaseFileInterface::H5_Byte: {
        char buff[2];
        ret = dataFile->getActualRange(variableName, 0, (void*)buff);
        min = buff[0];
        max = buff[1];
        break;}
    default: {
        ret = false;
        break;}
    }
    return ret;
}



long HDF5_FQ::get1DHistogram(int64_t timestep,
                             const char* variableName,
                             std::vector<double> &bounds,
                             std::vector<uint32_t> &counts) {
    long err;
    err = timeSlices[timestep]->getDistribution
        (variableName, bounds, counts);  
    return err;
}

long HDF5_FQ::get1DHistogram(int64_t timestep,
                             const char* condition,
                             const char* variableName,
                             std::vector<double> &bounds,
                             std::vector<uint32_t> &counts) {
    long err;
    err = timeSlices[timestep]->getDistribution
        (condition, variableName, bounds, counts);  
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

    err = timeSlices[timestep]->getDistribution
        (variableName, num_bins, &bounds[0], &counts[0]);  
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

    err = timeSlices[timestep]->getDistribution
        (condition, variableName, num_bins, &bounds[0], &counts[0]);  
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
      err = timeSlices[timestep]->get1DDistribution(condition,
      variableName,
      begin, end, stride,
      count);
    */
    //TODO:: transfer result from count to counts;
    // Check this with John Wu
    LOGGER(ibis::gVerbose > 0)
        <<"get1DDistribution w/ strides returned " << err 
        << "count size is " << count.size();

    for (unsigned int i=0; i<count.size(); i++)
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
    // make sure that begin1 + stride1*num_bins1 > end1
    stride1 = ibis::util::incrDouble(stride1);

    double stride2;
    stride2 = (end2 - begin2)/num_bins2;
    stride2 = ibis::util::incrDouble(stride2);

    // there's only 1 timestep active at a time..
    timestep = 0;
    err = timeSlices[timestep]->get2DDistribution(condition, 
                                                  variableName1,
                                                  begin1, end1, stride1,
                                                  variableName2,
                                                  begin2, end2, stride2,
                                                  counts);

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
    for (unsigned int i=0; i<=num_bins1; i++)
        bounds1.push_back(begin1 + i*stride1);
    
    bounds2.clear();
    for (unsigned int i=0; i<=num_bins2; i++)
        bounds2.push_back(begin2 + i*stride2);
    
    LOGGER(ibis::gVerbose > 0)
        << "HDF_FQ:: get2DHistogram created "
        << " bounds1 [size= " << bounds1.size() << "], " 
        << " bounds2 [size= " << bounds2.size() << "]";
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
    err = timeSlices[timestep]->getJointDistribution(condition,
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

    LOGGER(ibis::gVerbose > 0)
        << "HDF_FQ:: starting get2DDistribution call with following info"
        << variableName1 << "[" << begin1 << "," << end1 << "," << stride1
        << "] " << variableName2 << "[" << begin2 << "," << end2 << ","
        << stride2 << "] ";
    
    err = timeSlices[timestep]->get2DDistribution(condition, 
                                                  variableName1,
                                                  begin1, end1, stride1,
                                                  variableName2,
                                                  begin2, end2, stride2,
                                                  counts);
    
    LOGGER(ibis::gVerbose > 0)
        << "HDF_FQ:: returned from 2DDistribution call with err=" << err;
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
    const char* option = "d";
    LOGGER(ibis::gVerbose > 0)
        << "HDF_FQ:: starting get2DAdaptive call with following info" <<
        variableName1 << ", #bins1 = " << num_bins1 << ", " <<
        variableName2 << ", #bins2 = " << num_bins2 << "." <<    
        " Data/index option = " << option;

    err = timeSlices[timestep]->get2DDistribution(variableName1, variableName2,
                                                  num_bins1, num_bins2,
                                                  bounds1, bounds2,
                                                  counts, option);

    LOGGER(ibis::gVerbose > 0)
        << "HDF_FQ:: returned from First New 2DAdaptiveDistribution call "
        "with err=" << err 
        << ", bounds sizes = " << bounds1.size()<<"x"<<bounds2.size()
        << ", Counts size = " << counts.size();
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

    err = timeSlices[timestep]->get2DDistribution(condition,
                                                  variableName1, variableName2,
                                                  num_bins1, num_bins2,
                                                  bounds1, 
                                                  bounds2,
                                                  counts);
    LOGGER(ibis::gVerbose > 0)
        << "HDF_FQ:: returned from Second New 2DAdaptiveDistribution call "
        "with err=" << err;
    return err;
}

long HDF5_FQ::get3DBins(int64_t timestep,
                        const char *condition, 
                        const char *variableName1,
                        double begin1, double end1, uint32_t num_bins1, 
                        const char *variableName2,
                        double begin2, double end2, uint32_t num_bins2, 
                        const char *variableName3,
                        double begin3, double end3, uint32_t num_bins3,
                        std::vector<double> &bounds1,
                        std::vector<double> &bounds2,
                        std::vector<double> &bounds3,
                        std::vector<uint32_t> &counts,
                        std::vector<ibis::bitvector*> &bitmaps) {  
    long err;

    double stride1; 
    stride1 = (end1 - begin1)/num_bins1;
    // make sure that begin1 + stride1*num_bins1 > end1
    stride1 = ibis::util::incrDouble(stride1);
  
    double stride2;
    stride2 = (end2 - begin2)/num_bins2;
    stride2 = ibis::util::incrDouble(stride2);

    double stride3;
    stride3 = (end3 - begin3)/num_bins3;
    stride3 = ibis::util::incrDouble(stride3);
  
    err = timeSlices[timestep]->get3DBins(condition, 
                                          variableName1, begin1, end1, stride1,
                                          variableName2, begin2, end2, stride2,
                                          variableName3, begin3, end3, stride3,
                                          bitmaps);

    //  std::cout << "HDF_FQ:: returned from get3DBins call with err = "
    //  << err << std::endl;
    //  std::cout << "Now setting the counts array " << std::endl;

    //Compute the bounds
    bounds1.resize(num_bins1+1);
    bounds2.resize(num_bins2+1);
    bounds3.resize(num_bins3+1);
    for(uint32_t i=0; i<=num_bins1 ; ++i)
        bounds1[i] = begin1 + i*stride1;
    for(uint32_t i=0; i<=num_bins2 ; ++i)
        bounds2[i] = begin2 + i*stride2;
    for(uint32_t i=0; i<=num_bins3 ; ++i)
        bounds3[i] = begin3 + i*stride3;

    //Compute the counts
    counts.resize(bitmaps.size());
    for (unsigned int i=0; i<bitmaps.size(); i++) {
        // cnt() function on a bitvector gives the # elements
        if (bitmaps[i]!=NULL)
            counts[i] = bitmaps[i]->cnt();
        else 
            counts[i] = 0;      
    } 
    return err;
}

long HDF5_FQ::get3DBins(int64_t timestep,
                        const char *condition,
                        const char *variableName1,
                        double begin1, double end1, uint32_t num_bins1,
                        const char *variableName2,
                        double begin2, double end2, uint32_t num_bins2,
                        const char *variableName3,
                        double begin3, double end3, uint32_t num_bins3,
                        std::vector<double> &bounds1,
                        std::vector<double> &bounds2,
                        std::vector<double> &bounds3,
                        std::vector<uint32_t> &counts,
                        std::map<int, ibis::bitvector*> &bitmaps) {
    long err;
    double stride1;
    stride1 = (end1 - begin1)/num_bins1;
    // make sure that begin1 + stride1*num_bins1 > end1
    stride1 = ibis::util::incrDouble(stride1);

    double stride2;
    stride2 = (end2 - begin2)/num_bins2;
    stride2 = ibis::util::incrDouble(stride2);

    double stride3;
    stride3 = (end3 - begin3)/num_bins3;
    stride3 = ibis::util::incrDouble(stride3);

    std::vector<ibis::bitvector*> tempBitmaps;
    err = timeSlices[timestep]->get3DBins(condition,
                                          variableName1, begin1, end1, stride1,
                                          variableName2, begin2, end2, stride2,
                                          variableName3, begin3, end3, stride3,
                                          tempBitmaps);

    LOGGER(ibis::gVerbose > 0)
        << "HDF_FQ:: returned from get3DBins call with err = " << err
        << "\nNow setting the counts array ";

    //Compute the bounds
    bounds1.resize(num_bins1+1);
    bounds2.resize(num_bins2+1);
    bounds3.resize(num_bins3+1);
    for(uint32_t i=0; i<=num_bins1 ; ++i)
        bounds1[i] = begin1 + i*stride1;
    for(uint32_t i=0; i<=num_bins2 ; ++i)
        bounds2[i] = begin2 + i*stride2;
    for(uint32_t i=0; i<=num_bins3 ; ++i)
        bounds3[i] = begin3 + i*stride3;

    //Compute the counts and save the relevant bitmaps
    counts.resize(tempBitmaps.size());
    bitmaps.clear();

    for (unsigned int i=0; i<tempBitmaps.size(); i++) {
        // cnt() function on a bitvector gives the # elements
        if (tempBitmaps[i]!=NULL)
            counts[i] = tempBitmaps[i]->cnt();
        else
            counts[i] = 0; 

        if (counts[i] > 0) {
            bitmaps[i] = tempBitmaps[i];
        }
    }
    return err;
}

int HDF5_FQ::get_Bitmap_Data(int64_t timestep,
                             const char* name,
                             const std::vector<ibis::bitvector*> &bitmaps,
                             void*& result) {
    //  std::cout << "HDF5_FQ::get_Bitmap_Data got " << bitmaps.size()
    // << " entries in the bitmaps vector" << std::endl;

    // create a composit bitvector
    ibis::bitvector composit_bitvector;  
    for (unsigned int i=0; i<bitmaps.size(); i++) {
        if (bitmaps[i]!=NULL) {
            if ((*bitmaps[i]).size()>0) {
                composit_bitvector |= *bitmaps[i];
            }
            else { 
                // do nothing, this bitvector does not exist
            }
        }
    }

    // get variable information to determine variable type
    std::vector<int64_t> dims;
    BaseFileInterface::DataType dataType;
    getVariableInformation(name, timestep, dims, &dataType);
  
    int i;
    array_t<float>    *ftmp;  float    *fptr;
    array_t<double>   *dtmp;  double   *dptr;
    array_t<int32_t>  *itmp;  int32_t  *iptr;
    array_t<int64_t>  *litmp; int64_t  *liptr;

    switch(dataType) {

    case BaseFileInterface::H5_Float: {
        //std::cout<<"HDF5_FQ::get_Bitmap_Data load float data"<<std::endl;
        i = 0;
        ftmp = timeSlices[timestep]->selectFloats(name, composit_bitvector);
        fptr = (float*) malloc(ftmp->size()*sizeof(float));
        for (array_t<float>::const_iterator ii = ftmp->begin();
             ii!= ftmp->end(); ii++, i++)
            fptr[i] = *ii;

        int retVal = ftmp->size();
        delete ftmp;
        result = (void*)fptr;
        return retVal;
        break;    }

    case BaseFileInterface::H5_Double: {
        // std::cout<<"HDF5_FQ::get_Bitmap_Data load double data"<<std::endl;
        i = 0;
        dtmp = timeSlices[timestep]->selectDoubles(name, composit_bitvector);
        dptr = (double*) malloc(dtmp->size()*sizeof(double));
        for (array_t<double>::const_iterator ii = dtmp->begin();
             ii!= dtmp->end(); ii++, i++)
            dptr[i] = *ii;

        int retVal = dtmp->size();
        delete dtmp;
        result = (void*)dptr;
        return retVal;
        break;    }

    case BaseFileInterface::H5_Int32: {
        //std::cout<<"HDF5_FQ::get_Bitmap_Data load int32 data"<<std::endl;
        i = 0;
        itmp = timeSlices[timestep]->selectInts(name, composit_bitvector);
        iptr = (int32_t*) malloc(itmp->size()*sizeof(int32_t));
        for (array_t<int32_t>::const_iterator ii = itmp->begin();
             ii!= itmp->end(); ii++, i++)
            iptr[i] = *ii;

        int retVal = itmp->size();
        delete itmp;
        result = (void*)iptr;
        return retVal;
        break;    }

    case BaseFileInterface::H5_Int64: {
        //std::cout<<"HDF5_FQ::get_Bitmap_Data load int64 data"<<std::endl;
        i = 0;
        litmp = timeSlices[timestep]->selectLongs(name, composit_bitvector);
        liptr = (int64_t*) malloc(litmp->size()*sizeof(int64_t));
        for (array_t<int64_t>::const_iterator ii = litmp->begin();
             ii!= litmp->end(); ii++, i++)
            liptr[i] = *ii;

        int retVal = litmp->size();
        delete litmp;
        result = (void*)liptr;
        return retVal;
        break;    }

    default: {
        LOGGER(ibis::gVerbose >= 0)
            << "Warning:: HDF5_FQ::get_Bitmap_Data:: "
            "unsupported variable type, returning NULL";
        result = NULL;
        return 0;
        break;    }
    
    }
    return -1;
}

#endif
