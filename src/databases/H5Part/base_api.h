#ifndef _BASE_INTERFACE_H
#define _BASE_INTERFACE_H

#include <stdint.h>
#include "hdf5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

/**
   Base class for the FlexIO API. 
*/

#ifdef true
#undef true
#endif
 
#ifdef false
#undef false
#endif

#define true 1
#define false 0

struct BaseFileInterface {
    //basic data types
    enum DataType {H5_Float=0,H5_Double=1,H5_Int32=2,H5_Int64=3,H5_Byte=4,H5_Error=-1};
    enum DataClass {Real, Integer, String_DC, Enum};

    //procedures
    //get the list of all variable names in the dataset
    virtual void getVariableNames(std::vector<std::string> &names) const=0;

    //query the current data set for the existence of a particular variable
    virtual bool queryVariableNames(const std::string& q_string) const=0;

    //Get the number of timesteps of data stored in the hdf5 file
    //Each step of data contains all of the variabless
    //Return value:
    //The total number of time steps in the HDF5 file
    virtual int64_t getNumSteps() const=0;
        
    //get the min valid time step (the earliest time for all samples)
    //Return value:
    //The smallest value of time associated with any time step.
    virtual double getMinStep() const=0;
        
    //get the max valid step (the latest time for all samples)
    //Return value:
    //The largest value of time associated with any time step
    virtual double getMaxStep() const=0;
        
    //calculate the size of any group of arrays based upon type and dimms
    //dims: a vector containing the dimensions for a given dataset.
    //DataType: The enumerated type of data for the dataset referenced by dims
    //Return value:
    //The size (in bytes) of the specified dataset.
    virtual int64_t getDatasetSize(const std::vector<int64_t> dims,
                                   const DataType type) const=0;
        
    //getVariableInfo retrieves information about a specific data set. The variablename MUST
    //be an explicit full pathname to a data set in the HDF5 file.  
    //Example: variablename = /Time/Step#2/density
    //Return value:
    //getVariableInfo will return the variable descriptor group number order of the requested 
    //dataset (if found -1 otherwise).
    //Example: given the datasets ordering {X,Y,Z,density, Pressure}, a getVariableInfo(/Time/Step#2/density)
    //will return 3 (where X would return 0, Y would return 1 and Z would return 4).
    virtual int64_t getVariableInfo(const std::string& variablename,
                                    int64_t time,
                                    std::vector <int64_t> &dims, 
                                    DataType *type)=0;
        
    //Read all of the data associated with the variable specified by the variable name.
    //Note that as with getVariableInfo(), variablename MUST 
    //be an explicit full pathname to a data set.  Example: variablename = /Time/Step#2/density
    virtual void getData(const std::string& variableName,
                         int64_t time, void *data) =0;
    virtual bool getSpecificData(const std::string& variablename,
                                 int64_t time,
                                 void *data,
                                 const std::vector<hsize_t>& indices) =0;
    virtual bool getPointData(const std::string& variablename,
                              int64_t time, void *data,
                              const std::vector<hsize_t>& indices)=0;

    //Creates a new initialized variable under EVERY Step#<n>
    //Must supply all information for given variables or pass NULL
    //declareVariable() will create a new descriptor variable using information from the input variables
    //as well as a new data sets (initialized) under every time step.
    //Return value:
    //declareVariable() will return the incremented total number of dataset variables currently 
    //in the HDF5 file
    virtual int64_t declareVariable(const std::string& variableName,
                                    const std::vector<int64_t> dims,
                                    const DataType type,
                                    const std::string& sData_center,
                                    const std::string& sCoordsys, 
                                    const std::string& sScheme,
                                    const std::string& sSchema_type)=0;

    //write variable data specified by variablename.
    //Note that as with getVariableInfo(), variablename MUST 
    //be an explicit full pathname to a data set.  Example: variablename = /Time/Step#2/density
    //It will overwrite existing data if data already exists 
    //in that position.
    virtual void insertVariableData(const std::string& variablename,
                                    int64_t time,
                                    const void* data,
                                    DataType type,
                                    const void* min,
                                    const void* max)=0;
        
    //add a new step to the file (returns the new step number)
    //The step is a monotonically increasing counter for the file
    //This function also creates new "initialized" datasets under this new time step
    //the timesteps will also all be updated so that the steps are in 
    //increasing chronological order.
    virtual void insertStep(double timeValue)=0;
};
#endif
