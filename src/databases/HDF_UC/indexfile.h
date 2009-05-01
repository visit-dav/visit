#ifndef _INDEX_FILE_H
#define _INDEX_FILE_H
#include "base_api.h"
#include "h5_driver.h"
#include "H5Part.h"

/**
  HDF5 class with functionality for adding, modifying and performing queries
  on HDF5_UC structured *.H5 files.
*/


class H5_Index : public BaseFileInterface{
  public:
        H5_Index(const bool useH5PartFile = false);
        H5_Index(const H5_Index &);
        virtual ~H5_Index();

        //open or create a new HDF5_UC file.  Files that are not found
        //will be created using that name
        void openHDF5File(const char* file_location);
        void openHDF5File(std::string file_location);


        //close the current file.        
        void closeHDF5File();
    // Return the name of the HDF5 file.
        std::string getFileName() {return (file_open?file_id.getName():"");}

        void openH5PartFile(std::string file_location);
        void closeH5PartFile();

        //get the list of all variable names in the dataset
        void getVariableNames(std::vector<std::string> &names);
        void getH5PartVariableNames(std::vector<std::string> &names);

        
        //query the current data set for the existence of a particular variable
        bool queryVariableNames(std::string q_string);
        
        //Get the number of timesteps of data stored in the hdf5 file
        //Each step of data contains all of the variables
        //Return value:
        //The total number of time steps in the HDF5 file
        int64_t getNumSteps();
        int64_t getH5PartNumSteps();

        // void getTimeRange(double &min,double &max);
        // perhaps numsteps is always starting from 0?
        
        //get the min valid time step (the earliest time for all samples)
        //Return value:
        //The smallest value of time associated with any time step.
        double getMinStep();
        // double getMinTime();
        // int64_t getMinStep(); // perhaps always starts from zero?
        
        //get the max valid step (the latest time for all samples)
        //Return value:
        //The largest value of time associated with any time step
        double getMaxStep();
        // double getMaxTime();
        // int64_t getMaxStep();

        //calculate the size of any group of arrays based upon type and dimms
        //dims: a vector containing the dimensions for a given dataset.
        //DataType: The enumerated type of data for the dataset referenced by dims
        //Return value:
        //The size (in bytes) of the specified dataset.
        virtual int64_t getDatasetSize(const std::vector<int64_t>        dims,
                                   const BaseFileInterface::DataType type);

        //getVariableInfo retrieves information about a specific data set. The variablename MUST
        //be an explicit full pathname to a data set in the HDF5 file.  
        //Example: variablename = /Time/Step#2/density
        //Return value:
        //getVariableInfo will return the variable descriptor group number order of the requested 
        //dataset (if found -1 otherwise).
        //Example: given the datasets ordering {X,Y,Z,density, Pressure}, a getVariableInfo(/Time/Step#2/density)
        //will return 3 (where X would return 0, Y would return 1 and Z would return 2).
        int64_t getVariableInfo(const std::string            variableName,
                            int64_t                      time,
                            std::vector <int64_t>       &dims,
                            BaseFileInterface::DataType *type);

        int64_t getH5PartVariableInfo(const std::string            variableName,
                                  int64_t                      time,
                                  std::vector <int64_t>       &dims,
                                  BaseFileInterface::DataType *type);


        //Read all of the data associated with the variable specified by the variable name.
        //Note that as with getVariableInfo(), variablename MUST 
        //be an explicit full pathname to a data set.  Example: variablename = /Time/Step#2/density
        void getData(const std::string variablename,int64_t time,void *data);        
        
        void getH5PartData(const std::string variablename,int64_t time,void *data);

        //get specific data
        //Note that as with getVariableInfo(), variablename MUST 
        //be an explicit full pathname to a data set.  Example: variablename = /Time/Step#2/density
        bool getSpecificData(const std::string variablename,int64_t time,void *data, const std::vector<int32_t>& indices);
        //similar to getSpecificData but uses gather scattered technique to collect as apposed to individual hyper slabs
        bool getPointData(const std::string variablename,int64_t time,void *data, const std::vector<int32_t>& indices);

        //Creates a new initialized variable under EVERY Step#<n>
        //Must supply all information for given variables orparse_str_vec pass NULL
        //declareVariable() will create a new descriptor variable using information from the input variables
        //as well as a new data sets (initialized) under every time step.
        //Return value:
        //declareVariable() will return the incremented total number of dataset variables currently 
        //in the HDF5 file        
        int64_t declareVariable(const std::string variableName,
                            const std::vector<int64_t> dims,
                            const BaseFileInterface::DataType type,
                            std::string sData_center,
                            std::string sCoordsys,
                            std::string sScheme,
                            std::string sSchema_type);

        //write variable data specified by variablename.
        //Note that as with getVariableInfo(), variablename MUST 
        //be an explicit full pathname to a data set.  Example: variablename = /Time/Step#2/density
        //It will overwrite existing data if data already exists 
        //in that position.
        void insertVariableData(const std::string variablename,
                            int64_t timestep,
                            const void* data,
                            const BaseFileInterface::DataType type,
                            const void* min,
                            const void* max);

        //add a new step to the file (returns the new step number)
        //The step is a monotonically increasing counter for the file
        //This function also creates new "initialized" datasets under this new time step
        //the timesteps will also all be updated so that the steps are in 
        //increasing chronological order.        
        void insertStep(double timeValue);

        // we must create a dataset before we can write to it in HDF5.
        // createBitmap reserves a dataset in the HDF5 file named <variableName>.bitmap
        // (the .bitmap is automatically
        // appended to the variableName so we can hide our naming conventions from API users.
        // Later on we can use HDF5 ragged arrays to allow elements to be appended as needed
        bool createBitmap(std::string variableName, 
                      uint64_t timestep,
                      uint64_t nelements);

        // the readBitmap call will read a subsection of the bitmapindex dataset
        // into the "data" buffer.  The offsets are in terms of 32-bit elements.
        // The startoffset is always *inclusive* and the endoffset is  always *exclusive*
        // The numbering of elements for the offsets starts from zero  (as expected).
        bool readBitmap(std::string variableName, uint64_t timestep, 
                    uint64_t startoffset, 
                    uint64_t endoffset,
                    uint32_t *data);

        // this writes a completed bitmap index into the variableName.bitmap dataset.
        // this again uses HDF5 dataset "selections" to write subsections of the dataset
        // since the bitmap indices are built incrementally.
        bool writeBitmap(std::string variableName, 
                     uint64_t timestep,
                     uint64_t startoffset,
                     uint64_t endoffset,
                     uint32_t *data);


    // returns the number of *elements* in the bitmapKeys attribute for variableName at timestep.
    // the datatype for the elements is implicitly the same as that used for the dataset for variableName
    bool getBitmapKeysLength(std::string variableName,
                             uint64_t timestep,
                             uint64_t *length);

    // return the size of the bitmap
    bool getBitmapSize(std::string variableName,
                       uint64_t timestep,
                       uint64_t *length);

    // will read "bitmapKeys" attribute from timestep and put into pre-allocated "keys" array.
    bool getBitmapKeys(std::string variableName,
                       uint64_t timestep,
                       void *keys);

    // writes "bitmapKeys" array as a 1D dataset <variableName>.bitmapKeys to coinside with
        // <variableName>.bitmap at timestep. presumes the datatype is the same as that for
        // the dataset the keys are associated with
    bool setBitmapKeys(std::string variableName,
                       int64_t timestep,
                       void *data,
                       uint64_t numelements);

    bool getBitmapOffsetsLength(std::string variableName,
                                uint64_t timestep,
                                uint64_t *length);

    // this is the same as BitMapKeys except that the datatype is always a uint32_t rather than void*
        //and that the dataset name is <variableName>.bitmapOffset
    bool getBitmapOffsets(std::string variableName,
                          uint64_t timestep,
                          uint32_t *offsets);

    bool setBitmapOffsets(std::string variableName,
                          uint64_t timestep,
                          uint32_t *offsets,
                          uint64_t nelements);

    // we always know ExpectedRange and ActualRange are always two elements with
    // the datatype the same as the indexed dataset.  So we do not need to query the
    // length, nor do we need to supply the length when we write the Range data.
    bool getExpectedRange(std::string variableName,
                          uint64_t timestep,
                          void *range);

    bool setExpectedRange(std::string variableName,
                          uint64_t timestep,
                          void *range);
 
    bool getActualRange(std::string variableName,        
                        uint64_t timestep,
                        void *range);

    bool setActualRange(std::string variableName,
                        uint64_t timestep,
                        void *range);
        void getTimeStepDataMinMax(std::string variableName,
                               uint32_t timestep,
                               const BaseFileInterface::DataType type,
                               void *min,
                               void *max);

  private:
        //usefull functions for working with hdf5 files
        
        void openHDF5FileLocal(const char* file_location);
        void string_attribute(std::string string_title,const char *label);
        void write_int_attribute(int64_t size,const char *label);
        void write_double_attribute(double size,const char *label);
        void create_int_attribute(int64_t size,const char *label);
        void create_double_attribute(double size,const char *label);
        bool queryTimeValue(double timeValue);

        //helper function for determining the datatype of an idx
        //files counterpart.  Also opens the appropriate handles
        //to the .bitmap dataset before exiting.
        bool getAttribute(std::string variable, uint64_t time, BaseFileInterface::DataType *type,const char * attribute);
        //returns the data type in the data set
        bool getType(std::string variable, uint64_t time, BaseFileInterface::DataType *type);
        //helper function supporting insertstep() and declareVariable()
        void create_dataset(const std::vector<int64_t> dims,
                        char* variablename,
                        const BaseFileInterface::DataType type,
                        const char* file_path,
                        int64_t new_dataset);
        //helper function to create the .bitmap dataset
        void flipGroup(std::string grp1,std::string grp2);
        void flipGroup(int64_t grp1,int64_t grp2);

        std::string stringPath(std::string s1,int64_t time);        
        std::string stringPathIdx(std::string s1,int64_t time);        

        //QUICKSORT routine which helps to sort all timesteps
        //by increasing time value
        void QSort(int64_t first,int64_t last);
        //PARTITION routine assisting the QUICKSORT function
        int64_t Partition(int64_t first,int64_t last);

        //bool for checking the status of an open file
        bool file_open;
        
        //other useful "cache" variables
        std::vector< std::string > variable_names;
        std::vector< h5part_int64_t > variable_types;
        std::vector< h5part_int64_t > variable_indices;
        //universal attributes, to be initialized at construction
        
        //file class
        H5F file_id;
        //the group class
        H5G group_id;
        //the attribute class
        H5A attribute_id;
        //the dataset class
        H5D dataset_id;
        //the dataspace class
        H5S dataspace_id;

        herr_t status;
        //attributes of the /Time group
        int64_t timeStepNum;
        double min_max_time[2];
        //used to maintain an array of all time values
        //updated with insertstep and is of size timeStepNum;
        double *timevalues;
        //attributes of the Step#<n> group
        int64_t variableStepNum;
        //attributes of the Data#<n> group

        char timestepPath[100];

        // H5Part information
        H5PartFile *h5partFile;
        bool useH5Part;
        bool h5partFileOpen;
        char fileName[64];
        h5part_int64_t nParticles;
        h5part_int64_t nTimeSteps;
        h5part_int64_t nDatasets;
};

#endif
