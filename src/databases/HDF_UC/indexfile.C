#include "indexfile.h"

H5_Index::H5_Index(const bool useH5PartFile){

  useH5Part = useH5PartFile;
  file_open = false;

  if (useH5Part) {
    strcpy(timestepPath, "/__H5PartIndex__/Step#%d");
  }
  else {
    strcpy(timestepPath, "/HDF5_UC/TimeStep%d");
  }
}


H5_Index::~H5_Index(){

  if (!useH5Part) {
    if(file_open){
      closeHDF5File();
    }
  }
}

void H5_Index::closeHDF5File(){

  if (useH5Part) {
    closeH5PartFile();
  }
  else {
    //write out the total number of timesteps
    group_id.open(file_id.getID(),"/HDF5_UC");
    attribute_id.open(group_id.getID(),"TimeCount");
    attribute_id.write(H5T_NATIVE_INT64,&timeStepNum);

    //write out the min_max values
    attribute_id.open(group_id.getID(),"TimeInterval");
    attribute_id.write(H5T_NATIVE_DOUBLE,min_max_time);        
        
    //close the group and file
    delete [] timevalues;
    file_open = false;
    //now close all of the sub-class groups
    dataspace_id.close();
    dataset_id.close();
    attribute_id.close();
    group_id.close();
    file_id.close();
        
    variable_names.erase(variable_names.begin(),variable_names.end());
  }
}



void H5_Index::openHDF5File(const char* file_location){
  
  if (useH5Part == true) {
    openH5PartFile(file_location);
  }
  else {
    openHDF5FileLocal(file_location);
  }
}

void H5_Index::openHDF5File(std::string file_location){

  if (useH5Part == true) {
    openH5PartFile(file_location);
  }
  else {
    openHDF5FileLocal(file_location.c_str());
  }
}

void H5_Index::openHDF5FileLocal(const char* file_location){
        char path1[200],path2[200],string_type[200];
        std::string data_type;
        
        if(file_open){
                closeHDF5File();
        }
         
        file_open = true;
        if(file_id.open(file_location)){
                //open and read the attributes
                group_id.open(file_id.getID(),"/HDF5_UC");
                hsize_t j;
                //get the total groupcount -1 = # of timesteps
                j = group_id.getGroupCount();
                timeStepNum = j-1;                
                timevalues = new double [timeStepNum];
        
                for(int64_t i=0;i<timeStepNum;i++){
                        double x;
                        sprintf(path2,"/HDF5_UC/TimeStep%d",(int)i);
                        strncpy(path1,path2,200);
                        group_id.open(file_id.getID(),path1);
                        attribute_id.open(group_id.getID(),"TimeValue");
                        strncpy(string_type,"",200);
                        attribute_id.read(H5T_NATIVE_DOUBLE,&x);
                        timevalues[i] = x;
                        
                        //find the existing min and max
                        if(i == 0){
                                min_max_time[0]=x;
                                min_max_time[1]=x;
                        }
                        if(min_max_time[0] > x){
                                min_max_time[0]=x;
                        }
                        if(min_max_time[1] < x){
                                min_max_time[1]=x;
                        }
                        //close the file, group and attribute
                }                
                
                //get the attributes for all Step<n>
                group_id.open(file_id.getID(),"/HDF5_UC/TOC");
                attribute_id.open(group_id.getID(),"GroupCount");
                attribute_id.read(H5T_NATIVE_INT64,&variableStepNum);
                //now read all of the data names into the vector passed in
                for(int64_t i=0;i<variableStepNum;i++){
                        sprintf(path2,"/HDF5_UC/TOC/VariableDescriptor%d",(int)i);
                        strncpy(path1,path2,200);
                        group_id.open(file_id.getID(),path1);
                        attribute_id.open(group_id.getID(),"VariableName");
                        strncpy(string_type,"",200);
                        attribute_id.read(H5T_C_S1,string_type);
                        data_type = string_type;
                        variable_names.push_back(data_type);
                }
                //QSort(0,timeStepNum-1);                
        }
        //if it does not...we need to make a new file formatted to the HDF5_UC format
        else{
                group_id.create(file_id.getID(),"/HDF5_UC");
                
                char attr_data_c[200];
                
                //create the groupcount attribute
                timeStepNum = 0;
                min_max_time[0] = min_max_time[1] = 0;

                hsize_t dimms = 1;
                dataspace_id.create(1,&dimms);
                //create the attribute
                attribute_id.create(group_id.getID(),"TimeCount",H5T_NATIVE_INT64,dataspace_id.getID());
                //write the attribute data
                attribute_id.write(H5T_NATIVE_INT64,&timeStepNum);
        
                dimms = 2;
                //write the min_max attributes
                dataspace_id.create(1,&dimms);
                //create the attribute
                attribute_id.create(group_id.getID(),"TimeInterval",H5T_NATIVE_DOUBLE,dataspace_id.getID());
                //write the attribute data
                attribute_id.write(H5T_NATIVE_DOUBLE,&min_max_time);

                dimms = 4;
                //write the TimeUnits attributes
                strncpy(attr_data_c,"None",200);
                dataspace_id.create(1,&dimms);
                //create the attribute
                attribute_id.create(group_id.getID(),"TimeUnits",H5T_C_S1,dataspace_id.getID());
                //write the attribute data
                attribute_id.write(H5T_C_S1,attr_data_c);

                dimms = 1;
                double time_var = 0.0;
                group_id.create(file_id.getID(),"/HDF5_UC/TimeStep0");
                //create the attribute
                dataspace_id.create(1,&dimms);
                attribute_id.create(group_id.getID(),"TimeValue",H5T_NATIVE_DOUBLE,dataspace_id.getID());
                //write the attribute data
                attribute_id.write(H5T_NATIVE_DOUBLE,&time_var);
                        
                //create the TOC group
                group_id.create(file_id.getID(),"/HDF5_UC/TOC");
                //create the groupcount attribute
                dataspace_id.create(1,&dimms);
                //create the attribute
                attribute_id.create(group_id.getID(),"GroupCount",H5T_NATIVE_INT64,dataspace_id.getID());
                //write the attribute data
                attribute_id.write(H5T_NATIVE_INT64,&timeStepNum);
        
                //add the FastBit group
                group_id.create(file_id.getID(),"/HDF5_UC/TOC/FastBit");
        
                timevalues = new double [1];
                timevalues[0] = 0;
                variableStepNum = 0;
        }
}

void H5_Index::openH5PartFile(std::string file_location) {

  //std::cout << "OpenH5PartFile..." << std::endl;

  // open in HDF5_UC file mode
  if (!file_open) { 
    if(!file_id.open(file_location.c_str())) {
      perror("File open failed:  exiting!");
    }
  }
  file_open = true;

  // open in H5Part file mode
  h5partFileOpen = true;
  strcpy(fileName, file_location.c_str());
  h5part_int64_t status = H5PART_SUCCESS; 
  h5partFile = H5PartOpenFile(file_location.c_str(), H5PART_READ);
  if(!h5partFile) {
    perror("File open failed:  exiting!");
    exit(0);
  }
  H5PartSetStep(h5partFile,0);

  nParticles = H5PartGetNumParticles(h5partFile);
  nTimeSteps = H5PartGetNumSteps(h5partFile);
  nDatasets = H5PartGetNumDatasets(h5partFile);

  //std::cout << "nParticles: " << nParticles << std::endl;
  //std::cout << "nDatasets: " << nDatasets << std::endl;

  const h5part_int64_t lenName = 64;
  char datasetName[lenName];
  h5part_int64_t datasetType;
  h5part_int64_t datasetNElems;

  for (h5part_int64_t i=0; i < nDatasets; i++) {
    status = H5PartGetDatasetInfo(h5partFile, i, datasetName, lenName,
                                  &datasetType, &datasetNElems);
    if (status != H5PART_SUCCESS) {
      perror("Could not retrieve dataset names!");
    }
    else {
      //std::cout << "datasetName: " << datasetName << "." << std::endl;
      variable_names.push_back(datasetName);
      variable_indices.push_back(i);
      variable_types.push_back(datasetType);
    }
  }
  
}


void H5_Index::closeH5PartFile(){

  if (h5partFileOpen) {
    H5PartCloseFile(h5partFile);
  }
  h5partFileOpen = false;

  if (file_open) {
    file_id.close();
  }
}


int64_t H5_Index::getNumSteps(){

  if (useH5Part) {
    return getH5PartNumSteps();
  }
  else {
    return timeStepNum;
  }
}

int64_t H5_Index::getH5PartNumSteps(){
        return nTimeSteps;
}



double H5_Index::getMinStep(){
        return min_max_time[0];
}

double H5_Index::getMaxStep(){
        return min_max_time[1];
}

void H5_Index::getVariableNames(std::vector<std::string> &names){

  if (useH5Part) {
    getH5PartVariableNames(names);
  }
  else {
    std::string temp_str;

    //copy all of the names into the vector passed in
    for(int64_t i=0;i<variableStepNum;i++){
      temp_str = variable_names[i];
      names.push_back(temp_str);
    }
  }
}

void H5_Index::getH5PartVariableNames(std::vector<std::string> &names){

  std::string temp_str;

  //copy all of the names into the vector passed in
  for (h5part_int64_t i=0; i < nDatasets; i++){
    temp_str = variable_names[i];
    names.push_back(temp_str);
  }
}


bool H5_Index::queryVariableNames(std::string q_string){
        bool answer = false;
        for(int64_t i=0;i< (int64_t) variable_names.size();i++){
                if(variable_names[i] == q_string){
                        answer = true;
                        i=variable_names.size();
                }
        }
        return answer;
}


int64_t H5_Index::getDatasetSize(const std::vector<int64_t>        dims,
                                 const BaseFileInterface::DataType type){

        int64_t value,result_immediate;

        //precure the DataType
        //H5_Float,H5_Double,H5_Int32,H5_Int64,H5_Byte
        switch(type){
                case H5_Float:
                        value = sizeof(float);
                break;
                case H5_Double:
                        value = sizeof(double);
                break;
                case H5_Int32:
                        value = sizeof(int32_t);
                break;
                case H5_Int64:
                        value = sizeof(int64_t);
                break;
                case H5_Byte:
                        value = sizeof(char);
                break;
                default:
                        value = 0;
                break;
        }        

        result_immediate=1;
        for(int64_t i=0;i<(int64_t)dims.size();i++){
                result_immediate *= dims[i];
        }
        return result_immediate*value;
}


//returns the dataset number of the group (if H5Part is not used)
int64_t H5_Index::getVariableInfo(const std::string            variableName,
                                  int64_t                      time,
                                  std::vector <int64_t>       &dims,
                                  BaseFileInterface::DataType *type){

  if (useH5Part) {
    return getH5PartVariableInfo(variableName, time, dims, type);
  }
  else {
        //open the group specified
        std::string path = stringPath(variableName,time);
        dataset_id.open(file_id.getID(),path.c_str());
        hsize_t *stddims,*maxdims;
        
        //get the type of data in the data set
        *type = dataset_id.getDataType();

        int64_t int_dim,num_dim;
        //get the # of dims for this data set (attribute Nspace)
        
        int_dim = dataspace_id.getNDim(dataset_id.getDataSpace());
        stddims = new hsize_t[int_dim];
        maxdims = new hsize_t[int_dim];
        
        num_dim = dataspace_id.getDim(dataset_id.getDataSpace(),stddims,maxdims);
        
        //now read the attributes for this data set to get the number of dim
        for(int64_t i=0;i<num_dim;i++){
                dims.push_back(stddims[i]);
        }
        delete[] stddims;
        delete[] maxdims;
        int64_t answer;
        attribute_id.open(dataset_id.getID(),"DataKey");
        attribute_id.read(H5T_NATIVE_INT64,&answer);        
        return answer;
  }
}


int64_t H5_Index::getH5PartVariableInfo(const std::string             variableName,
                                        int64_t                       time,
                                        std::vector <int64_t>        &dims,
                                        BaseFileInterface::DataType  *type) {
  

  // get variable name and type from cached data members
  h5part_int64_t h5PartType = -1;
  bool foundName = false;
  h5part_int64_t datasetIdx = 0;
  int64_t answer = 0;

  for (unsigned int i=0; i < variable_names.size(); i++) {
    if (variable_names[i] == variableName) {
      h5PartType = variable_types[i];
      foundName = true;
    }
  }

  if (!foundName) {
    answer = -1;
  }

  if (h5PartType == H5PART_INT64) {
    *type = H5_Int64;
  }
  else {
    if (h5PartType == H5PART_FLOAT64) {
      *type = H5_Double;
    }
    else {
      perror("getH5PartDatasetSize: Data type not defined in H5Part.");
      answer = -1;
    }
  }

  const h5part_int64_t lenName = 64;
  char datasetName[lenName];
  h5part_int64_t datasetType;
  h5part_int64_t datasetNElems;

  // retrieve datasets size of a specific time step
  if (foundName) {
    H5PartSetStep(h5partFile, time);
    status = H5PartGetDatasetInfo(h5partFile, datasetIdx, datasetName, lenName,
                                  &datasetType, &datasetNElems);
    if (status != H5PART_SUCCESS) {
      perror("Could not retrieve H5PartDatasetInfo!");
      answer = -1;
    }
    else {
      dims.push_back(datasetNElems);
    }
  }

  return answer;
}



void H5_Index::getData(const std::string variablename,int64_t time, void *data){

  if (useH5Part) {
    getH5PartData(variablename, time, data);
  }
  else {
    BaseFileInterface::DataType var_type;
    std::string path = stringPath(variablename,time);

    //open an existing data set...
    dataset_id.open(file_id.getID(),path.c_str());
    //first get the type information...
    var_type = dataset_id.getDataType();
        
    //read the data set according to datatype
    switch(var_type){
    case BaseFileInterface::DataType(0):
      dataset_id.read(H5T_NATIVE_FLOAT,data);
      break;
    case BaseFileInterface::DataType(1):
      dataset_id.read(H5T_NATIVE_DOUBLE,data);
      break;
    case BaseFileInterface::DataType(2):
      dataset_id.read(H5T_NATIVE_INT32,data);
      break;
    case BaseFileInterface::DataType(3):
      dataset_id.read(H5T_NATIVE_INT64,data);
      break;
    case BaseFileInterface::DataType(4):
      dataset_id.read(H5T_NATIVE_CHAR,data);
      break;
    default:
      break;
    }
  }
}



void H5_Index::getH5PartData(const std::string variablename, int64_t time, void *data) {

  H5PartSetStep(h5partFile,time);
  h5part_int64_t datasetIdx = 0;

  // get variable information
  for (unsigned int i=0; i < variable_names.size(); i++) {
    if (variable_names[i] == variablename)
      datasetIdx = variable_indices[i];
  }

  // retrieve data
  if (variable_types[datasetIdx] == H5PART_INT64) {
    H5PartReadDataInt64(h5partFile, variablename.c_str(), (h5part_int64_t*) data);
  }
  else {
    if (variable_types[datasetIdx] == H5PART_FLOAT64) {
      H5PartReadDataFloat64(h5partFile, variablename.c_str(), (h5part_float64_t*) data);
    }
    else {
      perror("getH5PartData: Data type not defined in H5Part.");
    }
  }
}


void H5_Index::string_attribute(std::string string_title,const char *label){
        char attr_data_c[200];
        hsize_t temp_size;

        temp_size = string_title.length();
        strncpy(attr_data_c,"",200);
        sprintf(attr_data_c,"%s",string_title.c_str());
        dataspace_id.create(1,&temp_size);
        //create the attribute
        attribute_id.create(group_id.getID(),label,H5T_C_S1,dataspace_id.getID());
        //write the attribute data
        attribute_id.write(H5T_C_S1,attr_data_c);
}

void H5_Index::write_int_attribute(int64_t size,const char *label){
        int64_t spacial_d;
        hsize_t temp_size = 1;

        spacial_d = size;
        dataspace_id.create(1,&temp_size);
        //create the attribute
        attribute_id.open(group_id.getID(),label);
        //write the attribute data
        attribute_id.write(H5T_NATIVE_INT64,&spacial_d);
}

void H5_Index::write_double_attribute(double size,const char *label){
        double spacial_d;
        hsize_t temp_size = 1;

        spacial_d = size;
        dataspace_id.create(1,&temp_size);
        //create the attribute
        attribute_id.open(group_id.getID(),label);
        //write the attribute data
        attribute_id.write(H5T_NATIVE_DOUBLE,&spacial_d);
}


void H5_Index::create_int_attribute(int64_t size,const char *label){
        int64_t spacial_d;
        hsize_t temp_size = 1;

        spacial_d = size;
        dataspace_id.create(1,&temp_size);
        //create the attribute
        attribute_id.create(group_id.getID(),label,H5T_NATIVE_INT64,dataspace_id.getID());
        //write the attribute data
        attribute_id.write(H5T_NATIVE_INT64,&spacial_d);
}

void H5_Index::create_double_attribute(double size,const char *label){
        double spacial_d;
        hsize_t temp_size = 1;

        spacial_d = size;
        dataspace_id.create(1,&temp_size);
        //create the attribute
        attribute_id.create(group_id.getID(),label,H5T_NATIVE_DOUBLE,dataspace_id.getID());
        //write the attribute data
        attribute_id.write(H5T_NATIVE_DOUBLE,&spacial_d);
}

void H5_Index::create_dataset(const std::vector<int64_t> dims,
                                char* variablename,
                                const BaseFileInterface::DataType type,
                                const char *file_path,
                                int64_t new_dataset){
        //now create "initialized" data sets into each /Time/Step<n> group
        int64_t i,j,k;
        hsize_t *size;

        k=dims.size();
        size = new hsize_t[k];
        j=1;
        for(i=0;i<k;i++){
                j*=dims[i];
                size[i] = dims[i];
        }

        //the attribute for the dataset
        hsize_t temp_size = 1;        


        //create a new data space and dataset for this group...
        group_id.open(file_id.getID(),file_path);
        
        dataspace_id.create(k,size);
        switch(type){
                case H5_Float:
                        float *temp_float;
                        float tfloat;
                        temp_float = new float[j];
                        for(i=0;i<j;i++){
                                temp_float[i]=0.0;
                        }
                        dataset_id.create(group_id.getID(),variablename,H5T_NATIVE_FLOAT,dataspace_id.getID());
                        //read out the data to the new file 
                        dataset_id.write(H5T_NATIVE_FLOAT,temp_float);
                        //release the memory
                        delete temp_float;
                        //create the attribute
                        dataspace_id.create(1,&temp_size);
                        attribute_id.create(dataset_id.getID(),"DataSetMinValue",H5T_NATIVE_FLOAT,dataspace_id.getID());
                        //write the attribute data
                        tfloat = 0.0;
                        attribute_id.write(H5T_NATIVE_FLOAT,&tfloat);
                        //the max value for the data set
                        dataspace_id.create(1,&temp_size);
                        //create the attribute
                        attribute_id.create(dataset_id.getID(),"DataSetMaxValue",H5T_NATIVE_FLOAT,dataspace_id.getID());
                        //write the attribute data
                        attribute_id.write(H5T_NATIVE_FLOAT,&tfloat);
                break;
                case H5_Double:
                        double *temp_double;
                        double tdouble;
                        temp_double = new double[j];
                        for(i=0;i<j;i++){
                                temp_double[i]=0.0;
                        }
                        dataset_id.create(group_id.getID(),variablename,H5T_NATIVE_DOUBLE,dataspace_id.getID());
                        //read out the data to the new file 
                        dataset_id.write(H5T_NATIVE_DOUBLE,temp_double);
                        //release the memory
                        delete temp_double;
                        //create the attribute
                        dataspace_id.create(1,&temp_size);
                        attribute_id.create(dataset_id.getID(),"DataSetMinValue",H5T_NATIVE_DOUBLE,dataspace_id.getID());
                        //write the attribute data
                        tdouble = 0.0;
                        attribute_id.write(H5T_NATIVE_DOUBLE,&tdouble);
                        //the max value for the data set
                        dataspace_id.create(1,&temp_size);
                        //create the attribute
                        attribute_id.create(dataset_id.getID(),"DataSetMaxValue",H5T_NATIVE_DOUBLE,dataspace_id.getID());
                        //write the attribute data
                        attribute_id.write(H5T_NATIVE_DOUBLE,&tdouble);

                break;
                case H5_Int32:
                        int32_t *temp_int;
                        int32_t tint32;
                        temp_int = new int32_t[j];
                        for(i=0;i<j;i++){
                                temp_int[i]=0;
                        }
                        dataset_id.create(group_id.getID(),variablename,H5T_NATIVE_INT32,dataspace_id.getID());
                        //read out the data to the new file 
                        dataset_id.write(H5T_NATIVE_INT32,temp_int);
                        //release the memory
                        delete temp_int;
                        //create the attribute
                        dataspace_id.create(1,&temp_size);
                        attribute_id.create(dataset_id.getID(),"DataSetMinValue",H5T_NATIVE_INT32,dataspace_id.getID());
                        //write the attribute data
                        tint32 = 0;
                        attribute_id.write(H5T_NATIVE_INT32,&tint32);
                        //the max value for the data set
                        dataspace_id.create(1,&temp_size);
                        //create the attribute
                        attribute_id.create(dataset_id.getID(),"DataSetMaxValue",H5T_NATIVE_INT32,dataspace_id.getID());
                        //write the attribute data
                        attribute_id.write(H5T_NATIVE_INT32,&tint32);

                break;
                case H5_Int64:
                        int64_t *temp_lng;
                        int64_t tint64;
                        temp_lng = new int64_t[j];
                        for(i=0;i<j;i++){
                                temp_lng[i]=0;
                        }
                        dataset_id.create(group_id.getID(),variablename,H5T_NATIVE_INT64,dataspace_id.getID());
                        //read out the data to the new file 
                        dataset_id.write(H5T_NATIVE_INT64,temp_lng);
                        //release the memory
                        delete temp_lng;
                        //create the attribute
                        dataspace_id.create(1,&temp_size);
                        attribute_id.create(dataset_id.getID(),"DataSetMinValue",H5T_NATIVE_INT64,dataspace_id.getID());
                        //write the attribute data
                        tint64 = 0;
                        attribute_id.write(H5T_NATIVE_INT64,&tint64);
                        //the max value for the data set
                        dataspace_id.create(1,&temp_size);
                        //create the attribute
                        attribute_id.create(dataset_id.getID(),"DataSetMaxValue",H5T_NATIVE_INT64,dataspace_id.getID());
                        //write the attribute data
                        attribute_id.write(H5T_NATIVE_INT64,&tint64);

                break;
                case H5_Byte:
                        char *temp_chr;
                        char tchar;
                        temp_chr = new char[j];
                        for(i=0;i<j;i++){
                                temp_chr[i]=0;
                        }
                        dataset_id.create(group_id.getID(),variablename,H5T_NATIVE_CHAR,dataspace_id.getID());
                        //read out the data to the new file 
                        dataset_id.write(H5T_NATIVE_CHAR,temp_chr);
                        //release the memory
                        delete temp_chr;
                        //create the attribute
                        dataspace_id.create(1,&temp_size);
                        attribute_id.create(dataset_id.getID(),"DataSetMinValue",H5T_NATIVE_CHAR,dataspace_id.getID());
                        //write the attribute data
                        tchar = 0;
                        attribute_id.write(H5T_NATIVE_CHAR,&tchar);
                        //the max value for the data set
                        dataspace_id.create(1,&temp_size);
                        //create the attribute
                        attribute_id.create(dataset_id.getID(),"DataSetMaxValue",H5T_NATIVE_CHAR,dataspace_id.getID());
                        //write the attribute data
                        attribute_id.write(H5T_NATIVE_CHAR,&tchar);

                break;
                default:
                break;
        }
        

        dataspace_id.create(1,&temp_size);
        //create the attribute
        attribute_id.create(dataset_id.getID(),"DataKey",H5T_NATIVE_INT64,dataspace_id.getID());
        //write the attribute data
        attribute_id.write(H5T_NATIVE_INT64,&new_dataset);
        //close the attribute data
        
        delete size;
}

// create a uint32_t array with [nelements] elements and fill them with 0.
bool H5_Index::createBitmap(std::string variableName, 
                            uint64_t timestep,
                            uint64_t nelements){
        bool answer = true;        
        hsize_t temp_size = nelements;
        std::string file_path;
        char p2[200];
        strncpy(p2,"",(int)200);
        
        file_path = variableName + ".bitmap";

        if (useH5Part) {
          // create index group 
          group_id.create(file_id.getID(),"/__H5PartIndex__");
          sprintf(p2,"/__H5PartIndex__/Step#%d",(int)timestep);
          
          // create time step group
          group_id.create(file_id.getID(), p2);
        }
        else {
          sprintf(p2,"/HDF5_UC/TimeStep%d",(int)timestep);
        }

        //create a new data space and .bitmap dataset for this group...
        answer = group_id.open(file_id.getID(),p2);
        if(answer == false) return answer;
        //initialize data to be read to the group
        answer = dataspace_id.create(1,&temp_size);
        if(answer == false) return answer;
        uint32_t *temp_int;
        temp_int = new uint32_t[nelements];
        for(int64_t i=0;i<(int64_t)nelements;i++){
                temp_int[i]= 0;
        }

        answer = dataset_id.create(group_id.getID(),file_path.c_str(),H5T_NATIVE_UINT32,dataspace_id.getID());
        if(answer == false) return answer;
     
        //read out the data to the new file 
        answer = dataset_id.write(H5T_NATIVE_UINT32,temp_int);
        if(answer == false) return answer;

        //release the memory from the three datasets
        delete temp_int;

        //determine the type of the original data...
        BaseFileInterface::DataType type;
        answer = getAttribute(variableName,timestep,&type,".bitmap");
        if(answer == false) return answer;

        return answer;
}
bool H5_Index::getPointData(const std::string variablename,int64_t time,void *data, const std::vector<int32_t>& indices){
        bool answer = true;
        
        //here we get specific values from the datasets indicated by the indices vector
        BaseFileInterface::DataType var_type;
        std::string path = stringPath(variablename,time);

        //open an existing data set...
        answer = dataset_id.open(file_id.getID(),path.c_str());
        if(answer == false) {
          //std::cout << "Dataset of file " << file_id.getID() << " could not be opened " << std::endl;
          return answer;
        }
        //first get the type information...
        var_type = dataset_id.getDataType();

        //pass the space handle for the dataset
        answer = dataspace_id.assignSpace(dataset_id.getDataSpace());
        if(answer == false) return answer;

        //next determine the dimmensionality of the dataset...
        hsize_t num_dimms = dataspace_id.getNDim(dataset_id.getDataSpace());

        hsize_t count[255];
        int32_t max = indices.size();
        max /= num_dimms;

        /// the function selectElements should not modify indices array!
        answer = dataspace_id.selectElements
            (max, num_dimms, const_cast<std::vector<int32_t>*>(&indices));
        if(answer == false) return answer;

        H5S dataspace_id2;
        count[0] = max;
        answer = dataspace_id2.create(1,count);
        if(answer == false) return answer;
        
        switch(var_type){
                case BaseFileInterface::DataType(0):
                        float *f_ptr;
                        f_ptr = (float*)data;
                        answer = dataset_id.slabRead(H5T_NATIVE_FLOAT,dataspace_id2.getID(),dataspace_id.getID(),&f_ptr[0]);
                break;
                case BaseFileInterface::DataType(1):
                        double *d_ptr;
                        d_ptr = (double*)data;
                        answer = dataset_id.slabRead(H5T_NATIVE_DOUBLE,dataspace_id2.getID(),dataspace_id.getID(),&d_ptr[0]);
                break;
                case BaseFileInterface::DataType(2):
                        int32_t *i3_ptr;
                        i3_ptr = (int32_t*)data;
                        answer = dataset_id.slabRead(H5T_NATIVE_INT32,dataspace_id2.getID(),dataspace_id.getID(),&i3_ptr[0]);
                break;
                case BaseFileInterface::DataType(3):
                        int64_t *i6_ptr;
                        i6_ptr = (int64_t*)data;
                        answer = dataset_id.slabRead(H5T_NATIVE_INT64,dataspace_id2.getID(),dataspace_id.getID(),&i6_ptr[0]);
                break;
                case BaseFileInterface::DataType(4):
                        char *c_ptr;
                        c_ptr = (char*)data;
                        answer = dataset_id.slabRead(H5T_NATIVE_CHAR,dataspace_id2.getID(),dataspace_id.getID(),&c_ptr[0]);
                break;
                default:
                break;
        }
        if(answer == false) return answer;
        return answer;
}

bool H5_Index::getSpecificData(const std::string variablename,int64_t time,void *data, const std::vector<int32_t>& indices){
        bool answer = true;
        
        //here we get specific values from the datasets indicated by the indices vector
        BaseFileInterface::DataType var_type;
        std::string path = stringPath(variablename,time);

        //open an existing data set...
        answer = dataset_id.open(file_id.getID(),path.c_str());
        if(answer == false) return answer;

        //first get the type information...
        var_type = dataset_id.getDataType();

        //pass the space handle for the dataset
        answer = dataspace_id.assignSpace(dataset_id.getDataSpace());
        if(answer == false) return answer;

        //next determine the dimmensionality of the dataset...
        hsize_t num_dimms = dataspace_id.getNDim(dataset_id.getDataSpace());


        int32_t counter=0;
        int32_t max = indices.size();
        max /= num_dimms;
        //loop through them all and get all of the values...store them in *data
        for(int32_t i=0;i<max;i++){
                        
                        //define the hyperslab in the dataset
                        hsize_t offset[255],count[255];
                        if(num_dimms>= 255){
                                printf("Error too many dimmensions per element");
                        }
                        for(int32_t i2=0;i2<255;i2++){
                                count[i2]=1;
                        }
                        for(int32_t j=0;j<(int32_t)num_dimms;j++){
                                offset[j]=indices.operator[] (num_dimms*i + j);
                        }
                        answer = dataspace_id.selectHSlab(offset,count);
                        if(answer == false) return answer;

                        //define a new memory dataspace of EQUAL size to the portion being read
                        H5S dataspace_id2;
                        answer = dataspace_id2.create(num_dimms,count);
                        if(answer == false) return answer;
        
                        //now define this memory hyperslab
                        hsize_t offset2[255] = {0};
                        answer = dataspace_id2.selectHSlab(&offset2[0],count);
                        if(answer == false) return answer;
        
                        //read into memory
                        switch(var_type){
                                case BaseFileInterface::DataType(0):
                                        float *f_ptr;
                                        f_ptr = (float*)data;
                                        answer = dataset_id.slabRead(H5T_NATIVE_FLOAT,dataspace_id2.getID(),dataspace_id.getID(),&f_ptr[counter]);
                                break;
                                case BaseFileInterface::DataType(1):
                                        double *d_ptr;
                                        d_ptr = (double*)data;
                                        answer = dataset_id.slabRead(H5T_NATIVE_DOUBLE,dataspace_id2.getID(),dataspace_id.getID(),&d_ptr[counter]);
                                break;
                                case BaseFileInterface::DataType(2):
                                        int32_t *i3_ptr;
                                        i3_ptr = (int32_t*)data;
                                        answer = dataset_id.slabRead(H5T_NATIVE_INT32,dataspace_id2.getID(),dataspace_id.getID(),&i3_ptr[counter]);
                                break;
                                case BaseFileInterface::DataType(3):
                                        int64_t *i6_ptr;
                                        i6_ptr = (int64_t*)data;
                                        answer = dataset_id.slabRead(H5T_NATIVE_INT64,dataspace_id2.getID(),dataspace_id.getID(),&i6_ptr[counter]);
                                break;
                                case BaseFileInterface::DataType(4):
                                        char *c_ptr;
                                        c_ptr = (char*)data;
                                        answer = dataset_id.slabRead(H5T_NATIVE_CHAR,dataspace_id2.getID(),dataspace_id.getID(),&c_ptr[counter]);
                                break;
                                default:
                                break;
                        }
                        if(answer == false) return answer;
                        counter        ++;
        }
        
        return answer;
}

bool H5_Index::readBitmap(std::string variableName, uint64_t timestep, 
                                uint64_t startoffset, 
                                uint64_t endoffset,
                                uint32_t *data){


        bool answer = true;        

        std::string path = stringPathIdx(variableName,timestep);
        answer = dataset_id.open(file_id.getID(),path.c_str());
        if(answer == false) return answer;

        //pass the space handle for the dataset
        answer = dataspace_id.assignSpace(dataset_id.getDataSpace());
        if(answer == false) return answer;

        //define the hyperslab in the dataset
        hsize_t offset = startoffset,count = (endoffset - startoffset);
        answer = dataspace_id.selectHSlab(&offset,&count);
        if(answer == false) return answer;

        //define a new memory dataspace of EQUAL size to the portion being read
        H5S dataspace_id2;
        answer = dataspace_id2.create(1,&count);
        if(answer == false) return answer;
        
        //now define this memory hyperslab
        offset = 0;
        answer = dataspace_id2.selectHSlab(&offset,&count);
        if(answer == false) return answer;
        
        //read into memory
        answer = dataset_id.slabRead(H5T_NATIVE_UINT32,dataspace_id2.getID(),dataspace_id.getID(),data);

        return answer;
}

bool H5_Index::writeBitmap(std::string variableName, 
                                uint64_t timestep,
                                uint64_t startoffset,
                                uint64_t endoffset,
                                uint32_t *data){
        bool answer = true;        

        std::string path = stringPathIdx(variableName,timestep);
        answer = dataset_id.open(file_id.getID(),path.c_str());
        if(answer == false) return answer;

        //pass the space handle for the dataset
        answer = dataspace_id.assignSpace(dataset_id.getDataSpace());
        if(answer == false) return answer;

        //define the hyperslab in the dataset
        hsize_t offset = startoffset,count = (endoffset - startoffset);
        answer = dataspace_id.selectHSlab(&offset,&count);
        if(answer == false) return answer;
        
        //define a new memory dataspace of EQUAL size to the portion being read
        H5S dataspace_id2;
        answer = dataspace_id2.create(1,&count);
        if(answer == false) return answer;
        
        //now define this memory hyperslab
        offset = 0;
        answer = dataspace_id2.selectHSlab(&offset,&count);
        if(answer == false) return answer;
        
        // write into file
        answer = dataset_id.slabWrite(H5T_NATIVE_UINT32,dataspace_id2.getID(),dataspace_id.getID(),data);

        return answer;
}

bool H5_Index::getType(std::string variable, uint64_t time, BaseFileInterface::DataType *type){
        std::string variableName;
        char p2[200];
        bool answer = true;
        strncpy(p2,"",200);

        // Note: H5Part datasets are in the group /Step#x where x is the step number
        if (useH5Part)
          sprintf(p2,"/Step#%d",(int)time);
        else
          sprintf(p2,"/HDF5_UC/TimeStep%d",(int)time);

        variableName = p2;
        variableName += "/";
        variableName += variable;
        answer = dataset_id.open(file_id.getID(),variableName.c_str());
        if(answer != false){
                //get the type of data in the data set
                *type = dataset_id.getDataType();
        }
        return answer;
}

bool H5_Index::getAttribute(std::string variable, uint64_t time, BaseFileInterface::DataType *type,const char * attribute){
        std::string variableName;
        char p2[200];
        bool answer = true;
        strncpy(p2,"",200);
        //sprintf(p2,"/HDF5_UC/TimeStep%d",(int)time);
        sprintf(p2, timestepPath,(int)time);
        answer = getType(variable,time,type);
        if(answer == false) return answer;
        variableName = p2;
        variableName += "/";
        variableName += variable + attribute;
        //open the .bitmap version of this dataset
        answer = dataset_id.open(file_id.getID(),variableName.c_str());
        return answer;
}

//attribute functions for the idx datasets
bool H5_Index::getBitmapKeysLength(std::string variableName,uint64_t timestep,uint64_t *length){
        bool answer = true;
        BaseFileInterface::DataType type;
        std::string file_path;
        char p2[200];

        //determine the type of the original data...
        answer = getType(variableName,timestep,&type);
        if(answer == false) return answer;
        strncpy(p2,"",(int)200);
        sprintf(p2,timestepPath,(int)timestep);

        file_path = variableName + ".bitmapKeys";

        //open this group...
        answer = group_id.open(file_id.getID(),p2);
        if(answer == false) return answer;
        answer = dataset_id.open(group_id.getID(),file_path.c_str());
        if(answer == false) return answer;
        //read out the data to the new file 
        dataset_id.getSize(length);
        switch(type){
                case BaseFileInterface::DataType(0):
                        *length /= sizeof(float);
                break;
                case BaseFileInterface::DataType(1):
                        *length /= sizeof(double);
                break;
                case BaseFileInterface::DataType(2):
                        *length /= sizeof(int32_t);
                break;
                case BaseFileInterface::DataType(3):
                        *length /= sizeof(int64_t);
                break;
                case BaseFileInterface::DataType(4):
                        *length /= sizeof(char);
                break;
                default:
                break;
        }        
        return answer;
}

//attribute functions for the idx datasets
bool H5_Index::getBitmapSize(std::string variableName,uint64_t timestep,uint64_t *length){
  bool answer = true;
  BaseFileInterface::DataType type;
  std::string file_path;
  char p2[200];
  
  //determine the type of the original data...
  answer = getType(variableName,timestep,&type);
  if(answer == false) return answer;
  strncpy(p2,"",(int)200);
  sprintf(p2,timestepPath,(int)timestep);
  
  file_path = variableName + ".bitmap";
  
  //open this group...
  answer = group_id.open(file_id.getID(),p2);
  if(answer == false) return answer;

  answer = dataset_id.open(group_id.getID(),file_path.c_str());
  if(answer == false) return answer;

  //read out the data to the new file 
  dataset_id.getSize(length);

  return answer;
}

bool H5_Index::getBitmapKeys(std::string variableName,uint64_t timestep,void *keys){
        bool answer = true;
        BaseFileInterface::DataType type;
        std::string file_path;
        char p2[200];

        //determine the type of the original data...
        answer = getType(variableName,timestep,&type);
        if(answer == false) return answer;
        strncpy(p2,"",(int)200);
        //sprintf(p2,"/HDF5_UC/TimeStep%d",(int)timestep);
        sprintf(p2,timestepPath,(int)timestep);

        file_path = variableName + ".bitmapKeys";
        //open this group...
        answer = group_id.open(file_id.getID(),p2);
        if(answer == false) return answer;
        answer = dataset_id.open(group_id.getID(),file_path.c_str());
        if(answer == false) return answer;
        //read out the data to the new file 
        answer = dataset_id.read_type(type,keys);
        if(answer == false) return answer;
        return answer;
}

bool H5_Index::setBitmapKeys(std::string variableName,int64_t timestep,void *data,uint64_t numelements){
        bool answer = true;
        BaseFileInterface::DataType type;
        hsize_t temp_size = numelements;
        std::string file_path;
        char p2[200];

        //determine the type of the original data...
        answer = getType(variableName,timestep,&type);
        if(answer == false) return answer;
        strncpy(p2,"",(int)200);
        //sprintf(p2,"/HDF5_UC/TimeStep%d",(int)timestep);
        sprintf(p2,timestepPath,(int)timestep);

        file_path = variableName + ".bitmapKeys";
        //create a new data space for this group...
        answer = group_id.open(file_id.getID(),p2);
        if(answer == false) return answer;
        //initialize data to be read to the group
        answer = dataspace_id.create(1,&temp_size);
        if(answer == false) return answer;
        //by default make the first datatype UINT_32
        answer = dataset_id.create_type(group_id.getID(),file_path.c_str(),type,dataspace_id.getID());
        if(answer == false) return answer;
        //write out the data to the new file 
        answer = dataset_id.write_type(type,data);
        if(answer == false) return answer;

        //actualRange
        temp_size = 2;
        answer = dataspace_id.create(1,&temp_size);
        if(answer == false) return answer;
        //create the attribute
        answer = attribute_id.create_type(dataset_id.getID(),"actualRange",type,dataspace_id.getID());
        if(answer == false) return answer;

        //expectedRange
        temp_size = 2;
        answer = dataspace_id.create(1,&temp_size);
        if(answer == false) return answer;
        //create the attribute
        answer = attribute_id.create_type(dataset_id.getID(),"expectedRange",type,dataspace_id.getID());

        return answer;
}

bool H5_Index::getBitmapOffsetsLength(std::string variableName,uint64_t timestep,uint64_t *length){
        bool answer = true;
        std::string file_path;
        char p2[200];

        //bitmapOffsets
        strncpy(p2,"",(int)200);
        //sprintf(p2,"/HDF5_UC/TimeStep%d",(int)timestep);
        sprintf(p2, timestepPath,(int)timestep);

        file_path = variableName + ".bitmapOffsets";
        //create a new data space and .bitmap dataset for this group...
        answer = group_id.open(file_id.getID(),p2);
        if(answer == false) return answer;
        //by default make the first datatype UINT_32
        answer = dataset_id.open(group_id.getID(),file_path.c_str());
        if(answer == false) return answer;
        //read out the data to the new file 
        dataset_id.getSize(length);
        *length /= sizeof(uint32_t);
        return answer;
}

bool H5_Index::getBitmapOffsets(std::string variableName,uint64_t timestep,uint32_t *offsets){
        bool answer = true;
        std::string file_path;
        char p2[200];

        //bitmapOffsets
        strncpy(p2,"",(int)200);
        //sprintf(p2,"/HDF5_UC/TimeStep%d",(int)timestep);
        sprintf(p2,timestepPath,(int)timestep);
        file_path = variableName + ".bitmapOffsets";
        //create a new data space and .bitmap dataset for this group...
        answer = group_id.open(file_id.getID(),p2);
        if(answer == false) return answer;
        //by default make the first datatype UINT_32
        answer = dataset_id.open(group_id.getID(),file_path.c_str());
        if(answer == false) return answer;
        //read out the data to the new file 
        answer = dataset_id.read(H5T_NATIVE_UINT32,offsets);
        if(answer == false) return answer;
        return answer;
}

bool H5_Index::setBitmapOffsets(std::string variableName,uint64_t timestep,uint32_t *offsets,uint64_t nelements){
        bool answer = true;
        hsize_t temp_size = nelements;
        std::string file_path;
        char p2[200];

        //bitmapOffsets
        strncpy(p2,"",(int)200);
        sprintf(p2,timestepPath,(int)timestep);
        file_path = variableName + ".bitmapOffsets";
        //create a new data space and .bitmap dataset for this group...
        answer = group_id.open(file_id.getID(),p2);
        if(answer == false) return answer;
        //initialize data to be read to the group
        answer = dataspace_id.create(1,&temp_size);
        if(answer == false) return answer;
        //by default make the first datatype UINT_32
        answer = dataset_id.create(group_id.getID(),file_path.c_str(),H5T_NATIVE_UINT32,dataspace_id.getID());
        if(answer == false) return answer;
        //write out the data to the new file 
        answer = dataset_id.write(H5T_NATIVE_UINT32,offsets);
        if(answer == false) return answer;
        return answer;
}

bool H5_Index::getExpectedRange(std::string variableName,uint64_t timestep,void *range){
        bool answer = true;
        BaseFileInterface::DataType type;
        answer = getAttribute(variableName,timestep,&type,".bitmapKeys");
        if(answer == false) return answer;
        answer = attribute_id.open(dataset_id.getID(),"expectedRange");
        if(answer == false) return answer;
        answer = attribute_id.read_type(type,range);                
        return answer;
}

bool H5_Index::setExpectedRange(std::string variableName, uint64_t timestep,void *range){
        bool answer = true;
        BaseFileInterface::DataType type;
        answer = getAttribute(variableName,timestep,&type,".bitmapKeys");
        if(answer == false) return answer;
        answer = attribute_id.open(dataset_id.getID(),"expectedRange");
        if(answer == false) return answer;
        answer = attribute_id.write_type(type,range);                
        return answer;
}
 
bool H5_Index::getActualRange(std::string variableName,uint64_t timestep,void *range){
        bool answer = true;
        BaseFileInterface::DataType type;
        answer = getAttribute(variableName,timestep,&type,".bitmapKeys");
        if(answer == false) return answer;
        answer = attribute_id.open(dataset_id.getID(),"actualRange");
        if(answer == false) return answer;
        answer = attribute_id.read_type(type,range);                
        return answer;
}

bool H5_Index::setActualRange(std::string variableName,uint64_t timestep,void *range){
        bool answer = true;
        BaseFileInterface::DataType type;
        answer = getAttribute(variableName,timestep,&type,".bitmapKeys");
        if(answer == false) return answer;
        answer = attribute_id.open(dataset_id.getID(),"actualRange");
        if(answer == false) return answer;
        answer = attribute_id.write_type(type,range);                
        return answer;
}

//this function creates a new variable under /HFD5_UC/TOC/VariableDescriptor<n> as well as creates the new data set 
//under each /HDF5_UC/Step<n>/ the function returns the Data the new dataset is.
int64_t H5_Index::declareVariable(const std::string variableName,const std::vector<int64_t > dims,
        const BaseFileInterface::DataType type, std::string sData_center, 
        std::string sCoordsys, std::string sScheme, std::string sSchema_type){
        char temp_str[200],temp_str2[200];
        //first, add the name of the new dataset to the vector of known datasets...
        int64_t set_count = variable_names.size();
        variable_names.push_back(variableName.c_str());
        //next, create a new group under /Time/Dataset_count
        sprintf(temp_str,"/HDF5_UC/TOC/VariableDescriptor%d",(int)set_count);
        group_id.create(file_id.getID(),temp_str);

        //and enter the attributes...
        //Nspace
        create_int_attribute(dims.size(),"NSpace");
        //DataCentering
        string_attribute(sData_center,"DataCentering");
        //Coordsys
        string_attribute(sCoordsys,"Coordsys");
        //Schema
        string_attribute(sScheme,"Schema");
        //Schematype
        string_attribute(sSchema_type,"SchemaType");
        //variablename
        string_attribute(variableName,"VariableName");

        //update the TOC groupcount variable
        group_id.open(file_id.getID(),"HDF5_UC/TOC");
        attribute_id.open(group_id.getID(),"GroupCount");
        int64_t variableNum;
        attribute_id.read(H5T_NATIVE_INT64,&variableNum);
        variableNum++;
        attribute_id.write(H5T_NATIVE_INT64,&variableNum);

        strncpy(temp_str,"",200);
        sprintf(temp_str,"%s",variableName.c_str());
        //create and initialize a new data set in every time step
        variableStepNum++;
        if(timeStepNum == 0){
                strncpy(temp_str2,"",200);
                sprintf(temp_str2,"/HDF5_UC/TimeStep0");
                create_dataset(dims, temp_str,type,temp_str2,(variableStepNum-1));
        }
        else{
                for(int64_t i=0;i<timeStepNum;i++){
                        strncpy(temp_str2,"",200);
                        sprintf(temp_str2,"/HDF5_UC/TimeStep%d",(int)i);
                        create_dataset(dims, temp_str,type,temp_str2,(variableStepNum-1));
                }
        }
        
        return variableNum;
}

void H5_Index::getTimeStepDataMinMax(std::string variable_name,
                                        uint32_t timestep,
                                        const BaseFileInterface::DataType type,
                                        void *min,
                                        void *max){

        //given a path and data, open the old file and write 
        //the new data to it
        std::string path = stringPath(variable_name,timestep);
        dataset_id.open(file_id.getID(),path.c_str());

        switch(type){
                case H5_Float:
                        attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                        attribute_id.read(H5T_NATIVE_FLOAT,min);
                        attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                        attribute_id.read(H5T_NATIVE_FLOAT,max);        
                break;
                case H5_Double:
                        attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                        attribute_id.read(H5T_NATIVE_DOUBLE,min);
                        attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                        attribute_id.read(H5T_NATIVE_DOUBLE,max);        
                break;
                case H5_Int32:
                        attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                        attribute_id.read(H5T_NATIVE_INT32,min);
                        attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                        attribute_id.read(H5T_NATIVE_INT32,max);        
                break;
                case H5_Int64:
                        attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                        attribute_id.read(H5T_NATIVE_INT64,min);
                        attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                        attribute_id.read(H5T_NATIVE_INT64,max);        
                break;
                case H5_Byte:
                        attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                        attribute_id.read(H5T_NATIVE_CHAR,min);
                        attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                        attribute_id.read(H5T_NATIVE_CHAR,max);        
                break;
                default:
                break;
        }

}

void H5_Index::insertVariableData(const std::string variableName,int64_t time,
                                const void* data,
                                const BaseFileInterface::DataType type,
                                const void* min,
                                const void* max){
        //given a path and data, open the old file and write 
        //the new data to it
        std::string path = stringPath(variableName,time);
        dataset_id.open(file_id.getID(),path.c_str());

        switch(type){
                case H5_Float:
                        float *data_f;
                        data_f = (float *)data;
                        dataset_id.write(H5T_NATIVE_FLOAT,data_f);
                        //write the min max if we have them
                        if((min != NULL) && (max != NULL)){
                                attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                                data_f = (float *)min;
                                attribute_id.write(H5T_NATIVE_FLOAT,data_f);
                                attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                                data_f = (float *)max;
                                attribute_id.write(H5T_NATIVE_FLOAT,data_f);        
                        }
                break;
                case H5_Double:
                        double *data_d;
                        data_d = (double *)data;
                        dataset_id.write(H5T_NATIVE_DOUBLE,data_d);
                        if((min != NULL) && (max != NULL)){
                                attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                                data_d = (double *)min;
                                attribute_id.write(H5T_NATIVE_DOUBLE,data_d);
                                attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                                data_d = (double *)max;
                                attribute_id.write(H5T_NATIVE_DOUBLE,data_d);        
                        }
                break;
                case H5_Int32:
                        int32_t *data_i;
                        data_i = (int32_t *)data;
                        dataset_id.write(H5T_NATIVE_INT32,data_i);
                        if((min != NULL) && (max != NULL)){
                                attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                                data_i = (int32_t *)min;
                                attribute_id.write(H5T_NATIVE_INT32,data_i);
                                attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                                data_i = (int32_t *)max;
                                attribute_id.write(H5T_NATIVE_INT32,data_i);        
                        }
                break;
                case H5_Int64:
                        int64_t *data_l;
                        data_l = (int64_t *)data;
                        dataset_id.write(H5T_NATIVE_INT64,data_l);
                        if((min != NULL) && (max != NULL)){
                                attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                                data_l = (int64_t *)min;
                                attribute_id.write(H5T_NATIVE_INT64,data_l);
                                attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                                data_l = (int64_t *)max;
                                attribute_id.write(H5T_NATIVE_INT64,data_l);        
                        }
                break;
                case H5_Byte:
                        char *data_b;
                        data_b = (char *)data;
                        dataset_id.write(H5T_NATIVE_CHAR,data_b);        
                        if((min != NULL) && (max != NULL)){
                                attribute_id.open(dataset_id.getID(),"DataSetMinValue");
                                data_b = (char *)min;
                                attribute_id.write(H5T_NATIVE_CHAR,data_b);
                                attribute_id.open(dataset_id.getID(),"DataSetMaxValue");
                                data_b = (char *)max;
                                attribute_id.write(H5T_NATIVE_CHAR,data_b);        
                        }
                break;
                default:
                break;
        }
}

//function returns false if an identicle time value 
//is found at a previous time step
bool H5_Index::queryTimeValue(double timeValue){
        bool answer = true;
        double x;
        char path[200];
        for(int64_t i=0;i<timeStepNum;i++){
                strncpy(path,"",(int)200);
                sprintf(path,"/HDF5_UC/TimeStep%d",(int)i);
                group_id.open(file_id.getID(),path);
                attribute_id.open(group_id.getID(),"TimeValue");
                attribute_id.read(H5T_NATIVE_DOUBLE,&x);
                if(timeValue == x){
                        i=timeStepNum;
                        answer = false;
                }
        }
        return answer;
}

//inserts a new time step under the "/Time" group  and makes sure that the new time value is inserted in 
//a time coherent manner
void H5_Index::insertStep(double timeValue){
        char temp_str[200];

        //first verify the value is NOT a duplicate
        if(queryTimeValue(timeValue)){

                strncpy(temp_str,"",200);
                sprintf(temp_str,"/HDF5_UC/TimeStep%d",(int)timeStepNum);
        
                //update the appropriate /Time attributes
                timeStepNum++;

                //now create the two attributes for the /Time/Step<n>
                //create the dataspace for the attribute
                int64_t variableNum;
                group_id.open(file_id.getID(),"/HDF5_UC/TOC");
                attribute_id.open(group_id.getID(),"GroupCount");
                attribute_id.read(H5T_NATIVE_INT64,&variableNum);
                
                if(timeStepNum == 1){
                        //first create the new time step
                        group_id.open(file_id.getID(),temp_str);
                        min_max_time[0] = min_max_time[1] = timeValue;
                        //calls to write the values to the new group
                        write_double_attribute(timeValue,"TimeValue");
                }
                else{
                        //first create the new time step
                        group_id.create(file_id.getID(),temp_str);
                        if(min_max_time[0] > timeValue){
                                min_max_time[0] = timeValue;
                        }
                        else if (min_max_time[1] < timeValue){
                                min_max_time[1] = timeValue;
                        }
                        //calls to write the values to the new group
                        create_double_attribute(timeValue,"TimeValue");
                }
                
        
                //save the new value to the list of all time values
                double *timevalues2;
                timevalues2 = new double [timeStepNum];
                for(int64_t i=0;i<timeStepNum-1;i++){
                        timevalues2[i] = timevalues[i];
                }
                timevalues2[timeStepNum-1]=timeValue;
                delete [] timevalues;
                timevalues = new double [timeStepNum];
                for(int64_t i=0;i<timeStepNum;i++){
                        timevalues[i] = timevalues2[i];
                }
                delete []timevalues2;
        
                //Now create new datasets under this new time step group
                //by using the /Time/Step0 information
                std::vector<int64_t> dims;
                std::string variablename;
                BaseFileInterface::DataType type;
                char temp_str2[200];
                group_id.open(file_id.getID(),"/HDF5_UC/TOC");
                if(timeStepNum > 1){
                        for(int64_t i=0;i<variableNum;i++){
                                //start by getting the name of the dataset
                                strncpy(temp_str,"",200);
                                sprintf(temp_str,"/HDF5_UC/TOC/VariableDescriptor%d",(int)i);
                                group_id.open(file_id.getID(),temp_str);
                                attribute_id.open(group_id.getID(),"VariableName");
                                strncpy(temp_str2,"",200);
                                attribute_id.read(H5T_C_S1,temp_str2);
                                strncpy(temp_str,"",200);
                                sprintf(temp_str,"/HDF5_UC/TimeStep0/%s",temp_str2);
                                variablename = temp_str;
                                //now open and retreive the properties of the data set
                                dims.erase(dims.begin(),dims.end());
                                int64_t count = getVariableInfo(temp_str2,0,dims,&type);
                                strncpy(temp_str,"",200);
                                sprintf(temp_str,"/HDF5_UC/TimeStep%d",(int)(timeStepNum -1));
                                create_dataset(dims,temp_str2,type,temp_str,count);
                        }
                }
                //now sort the groups
                //QSort(0,timeStepNum-1);
        }
}
void H5_Index::QSort(int64_t p,int64_t r){
        int64_t q;
        if(p < r){
                q = Partition(p,r);
                QSort(p,q-1);
                QSort(q+1,r);
        }
}

int64_t H5_Index::Partition(int64_t p,int64_t r){
        double x;
        int64_t i;
        x = timevalues[r];
        i = p-1;
        for(int64_t j=p;j<=r-1;j++){
                if(timevalues[j] <= x){
                        i = i+1;
                        double temp_int = timevalues[i];
                        timevalues[i] = timevalues[j];
                        timevalues[j] = temp_int;
                        if(i != j)flipGroup(i,j);
                }
        }
        double temp_int = timevalues[i+1];
        timevalues[i+1] = timevalues[r];
        timevalues[r] = temp_int;
        if((i+1) != r)flipGroup(i+1,r);
        return i+1;
}

//function used to swap two groups
void H5_Index::flipGroup(std::string grp1,std::string grp2){
        char temp_str[200],temp_str1[200],temp_str2[200];
        strncpy(temp_str,"",200);
        strncpy(temp_str1,"",200);
        strncpy(temp_str2,"",200);
        sprintf(temp_str,"/HDF5_UC/temp_step");
        sprintf(temp_str1,"%s",grp1.c_str());
        sprintf(temp_str2,"%s",grp2.c_str());
        status = H5Gmove(file_id.getID(),temp_str2,temp_str);
        status = H5Gmove(file_id.getID(),temp_str1,temp_str2);
        status = H5Gmove(file_id.getID(),temp_str,temp_str1);
}

//function used to swap two groups
void H5_Index::flipGroup(int64_t grp1,int64_t grp2){
        char temp_str[200],temp_str1[200],temp_str2[200];
        strncpy(temp_str,"",200);
        strncpy(temp_str1,"",200);
        strncpy(temp_str2,"",200);
        sprintf(temp_str,"/HDF5_UC/temp_step");
        sprintf(temp_str1,"/HDF5_UC/TimeStep%d",(int)grp1);
        sprintf(temp_str2,"/HDF5_UC/TimeStep%d",(int)grp2);
        status = H5Gmove(file_id.getID(),temp_str2,temp_str);
        status = H5Gmove(file_id.getID(),temp_str1,temp_str2);
        status = H5Gmove(file_id.getID(),temp_str,temp_str1);
}

std::string H5_Index::stringPath(std::string s1,int64_t time){
        char temp[250];
        strncpy(temp,"",250);
        sprintf(temp,"%d",(int)time);
        std::string ret;
        if (useH5Part) 
          ret = "/Step#";
        else
          ret = "HDF5_UC/TimeStep";
        ret += temp;
        ret += "/" + s1;
        return ret;
}        

std::string H5_Index::stringPathIdx(std::string s1,int64_t time){
        char temp[250];
        strncpy(temp,"",250);
        sprintf(temp,"%d",(int)time);
        std::string ret;
        if (useH5Part) 
          ret = "__H5PartIndex__/Step#";
        else
          ret = "HDF5_UC/TimeStep";
        ret += temp;
        ret += "/" + s1;
        ret += ".bitmap";
        return ret;
}        
