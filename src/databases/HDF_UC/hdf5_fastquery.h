#ifndef _HDF5_FASTQUERY_H
#define _HDF5_FASTQUERY_H

#include "indexfile.h"
#include <vector>
#include <map>
#include <string>
#include <assert.h>
#include "timestep.h"   // H5_FQ_Timestep
#include "ibis.h"       // H5_FQ_Timestep


/*! \mainpage HDF5-FastQuery

  HDF5-FastQuery provides a simple API for indexing and querying HDF5
  datasets using the FastBit bitmap index technology. Datasets can be
  retrieved using complex compound range queries such as "(energy > 100)
  && (70 < pressure < 90)". The bitmap index technology only retrieves
  the data elements that satisfy the query condition and shows
  significant speedup compared with reading the entire datasets.   

*/



/// Main HDF5 class with functionality for users to add, modify and perform
/// queries on HDF5_UC structured HDF5-files.
class HDF5_FQ {

public:
    //constructor and destructor
    HDF5_FQ(const int v=0, const char *rcfile=0, const char *logfile=0);
    ~HDF5_FQ();

    /*!
      \brief Open or create a new HDF5_UC file.  
      Files that are not found will be created using that name.

      \param name IN: name of the file
      \param useH5PartFile IN: If set to true, H5Part format is assumed.
    */
    void openFile(const char* name,                 
                  const bool useH5PartFile = false  
                  );

    /*!
      \brief Open or create a new HDF5_UC file.  
      Files that are not found will be created using that name.
      
      \param name IN: name of the file.
      \param useH5PartFile IN: If set to true, H5Part format is assumed.
    */
    void openFile(const std::string & name,         
                  const bool useH5PartFile = false  
                  );

    
    /*!
      \brief Close the current file.
    */
    void closeFile();


    /*!
      \brief Create a new initialized variable for each time step

      \param variableName IN: Name of the variable to be created.
      \param dims IN: Vector containing the dimensions for a given variable.
      \param type IN: Enumerated type of variable 
      H5_Float=0,H5_Double=1,H5_Int32=2,H5_Int64=3,H5_Byte=4.
      \param sDataCenter IN: meta data value about the variable.
      \param sCoordsys  IN: meta data value about the variable.
      \param sSchema  IN: meta data value about the variable.
      \param sSchemaType IN: meta data value about the variable.
    */
    void createNewVariable(char* variableName, 
                           std::vector<int64_t> dims, 
                           BaseFileInterface::DataType type,
                           char* sDataCenter,
                           char* sCoordsys,
                           char* sSchema,
                           char* sSchemaType);

    /*!
      \brief Create a new initialized variable for each time step

      \param variableName IN: Name of the variable to be created.
      \param dims IN: Vector containing the dimensions for a given dataset.
      \param type IN: Enumerated type of variable 
      H5_Float=0,H5_Double=1,H5_Int32=2,H5_Int64=3,H5_Byte=4
      \param sDataCenter IN: meta data value about the variable.
      \param sCoordsys  IN: meta data value about the variable.
      \param sSchema  IN: meta data value about the variable.
      \param sSchemaType IN: meta data value about the variable.
    */
    void createNewVariable(const std::string & variableName,
                           std::vector<int64_t> dims,
                           BaseFileInterface::DataType type,
                           const std::string & sDataCenter,
                           const std::string & sCoordsys,
                           const std::string & sSchema,
                           const std::string & sSchemaType);

    /*!
      \brief Insert data values for a particular timestep.

      \param variableName IN: name of the variable
      \param timestep IN: timestep.
      \param data IN: data values.
      \param type IN:  Enumerated type of variable 
      H5_Float=0,H5_Double=1,H5_Int32=2,H5_Int64=3,H5_Byte=4
      \param min IN: min data value.
      \param max IN: max data value.
    */
    void insertData(const std::string & variableName,
                    int64_t timestep,
                    const void* data,
                    const BaseFileInterface::DataType type,
                    const void* min,
                    const void* max);
    
    /*!
      \brief Insert data values for a particular timestep.

      \param variableName IN: name of the variable.
      \param timestep IN: timestep.
      \param data IN: data values.
      \param type IN:  Enumerated type of variable 
      H5_Float=0,H5_Double=1,H5_Int32=2,H5_Int64=3,H5_Byte=4
      \param min IN: min data value.
      \param max IN: max data value.
    */
    void insertData(const char* variableName,
                    int64_t timestep,
                    const void* data,
                    const BaseFileInterface::DataType type,
                    const void* min,
                    const void* max);

    /*!
      \brief Get the data of a variable for a particular timestep.

      \param variableName IN: name of the variable.
      \param timestep IN: timestep.
      \param data OUT: data values.

    */
    void getData(const std::string & variableName,
                 int64_t timestep,
                 void *data);


    /*!
      \brief Get the data of a variable for a particular timestep.

      \param variableName IN: name of the variable.
      \param timestep IN: timestep.
      \param data OUT: data values.

    */

    void getData(const char* variableName,
                 int64_t timestep,
                 void *data);
        
    /*!
      \brief Get particular values of a variable specified by the vector
      indices.

      \param variableName IN: name of the variable.
      \param timestep IN: timestep.
      \param data OUT: data values.
      \param indices IN: indices of the data values to be retrieved.
    */
    void getPointData(const std::string & variableName,
                      int64_t timestep,
                      void *data,
                      const std::vector<hsize_t>& indices);

    /*!
      \brief Get particular values of a variable specified by the vector
      indices.

      \param variableName IN: name of the variable.
      \param timestep IN: timestep.
      \param data OUT: data values.
      \param indices IN: indices of the data values to be retrieved.
    */
    
    void getPointData(const char* variableName,
                      int64_t timestep,
                      void *data,
                      const std::vector<hsize_t>& indices);

 
    /*!
      \brief Calculate the size of any group of arrays based upon type and dims.
        
      \param dims IN: vector containing the dimensions for a given variable.
      \param type IN: The enumerated type of data for the dataset
      referenced by dims
      \return The size (in bytes) of the specified variable.
    */
    int64_t getDatasetSize(const std::vector<int64_t> dims,
                           const BaseFileInterface::DataType type);

    /*!
      \brief Retrieve information about a specific variable.

      \param variableName IN: name of the variable.
      \param timestep IN: timestep.
      \param dims OUT: Vector containing the dimensions for a given variable.
      \param type OUT: Enumerated type of variable 
      H5_Float=0,H5_Double=1,H5_Int32=2,H5_Int64=3,H5_Byte=4

    */

    void getVariableInformation(const std::string& variableName,
                                int64_t timestep,
                                std::vector <int64_t> &dims,
                                BaseFileInterface::DataType *type);


    /*!
      \brief Retrieve information about a specific variable.

      \param variableName IN: name of the variable
      \param timestep IN: timestep.
      \param dims OUT: Vector containing the dimensions for a given variable.
      \param type OUT: Enumerated type of variable 
      H5_Float=0,H5_Double=1,H5_Int32=2,H5_Int64=3,H5_Byte=4

    */
    void getVariableInformation(const char* variableName,
                                int64_t timestep,
                                std::vector <int64_t> &dims,
                                BaseFileInterface::DataType *type);


    /*!
      \brief The following sequence of calls return histograms
      FastBit histograms are open on both ends, so for a histogram of size n
      count[0] and count[n-1] can be ignored.
      count[i] <--> [ bound[i-1],bound[i] )
      counts.size() = bounds.size()+1

      Histograms can be made conditional depending on whether 'condition'
      is specified.
             
      \param variableName IN: name of the variable
      \param timestep IN: timestep.
      \param condition IN: conditions under which histogram is evaluated
      \param num_bins IN: number of bins to create 
      \param bounds OUT: Vector containing the bounds for the bins
      \param counts OUT: Vector containing the counts for the bins

      Return values for calls indicates number of bins creted. 0 and -ve
      values are error conditions.

    */
    long get1DHistogram(int64_t timestep,
                        const char* variableName,
                        std::vector<double> &bounds,
                        std::vector<uint32_t> &counts);
        
    long get1DHistogram(int64_t timestep,
                        const char* condition,
                        const char* variableName,
                        std::vector<double> &bounds,
                        std::vector<uint32_t> &counts);
        
    long get1DHistogram(int64_t timestep,
                        const char* variableName,
                        uint32_t num_bins,
                        std::vector<double> &bounds,
                        std::vector<uint32_t> &counts);

    long get1DHistogram(int64_t timestep,
                        const char* condition,
                        const char* variableName,
                        uint32_t num_bins,
                        std::vector<double> &bounds,
                        std::vector<uint32_t> &counts);

    long get1DHistogram(int64_t timestep,
                        const char* condition,
                        const char* variableName,
                        double begin, double end, double stride,
                        std::vector<uint32_t> &counts); 


    long get2DHistogram(int64_t timestep,
                        const char *condition,
                        const char *variableName1, 
                        double begin1, double end1, uint32_t num_bins1,
                        const char *variableName2,
                        double begin2, double end2, uint32_t num_bins2,
                        std::vector<double> &bounds1,
                        std::vector<double> &bounds2,
                        std::vector<uint32_t> &counts);
        

    long get2DHistogram(int64_t timestep,
                        const char *condition,
                        const char *variableName1, 
                        const char *variableName2,
                        std::vector<double> &bounds1,
                        std::vector<double> &bounds2,
                        std::vector<uint32_t> &counts);
        
    long get2DHistogram(int64_t timestep,
                        const char *condition, 
                        const char *variableName1,
                        double begin1, double end1, double stride1,
                        const char *variableName2,
                        double begin2, double end2, double stride2,
                        std::vector<uint32_t> &counts);

    // New call from John (1/2)
    long get2DAdaptiveHistogram(int64_t timestep,
                                const char *variableName1,
                                const char *variableName2,
                                uint32_t num_bins1,
                                uint32_t num_bins2,
                                std::vector<double> &bounds1,
                                std::vector<double> &bounds2,
                                std::vector<uint32_t> &counts);

    // New call from John (2/2)
    long get2DAdaptiveHistogram(int64_t timestep,
                                const char *condition, 
                                const char *variableName1,
                                const char *variableName2,
                                uint32_t num_bins1, 
                                uint32_t num_bins2,
                                std::vector<double> &bounds1,
                                std::vector<double> &bounds2,
                                std::vector<uint32_t> &counts);
        
    // New set of calls from John Wu. These set of calls return a
    // 3D histogram, with a bitmap vector on a per-bin basis
    // we can check the length of the bitmap vector to get counts
    long get3DBins(int64_t timestep,
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
                   std::vector<ibis::bitvector*> &bitmaps);

    long get3DBins(int64_t timestep,
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
                   std::map<int , ibis::bitvector*> &bitmaps);
        
    int get_Bitmap_Data(int64_t timestep,
                        const char* name,
                        const std::vector<ibis::bitvector*> &bitmaps,
                        void*& result);
        
    /*!
      \brief Initialize data structures for indexing and querying over all
      timesteps.
    */
    void prepareIndexes();


    /*!
      \brief Build basic bitmap indices for all variables over all timesteps.
    */
    void buildAllIndexes();


    /*!
      \brief Build basic bitmap indices for all variables for a specific
      timestep.

      \param timestep IN: timestep
    */
    void buildSpecificTimeIndex(int64_t timestep);

    /*!
      \brief Build bitmap indices for a specific variable over all timesteps.

      \param variableName IN: name of the variable.
      \param binning IN: binning option (optional) 
      The format for the binning option is '\<binning nbins=xxxx /\>'
      where xxxx is the number of bins.
      If no binning is specified, a bitmap index is built for
      each distinct data value.
      Details on the binning option can be found at:
      http://sdm.lbl.gov/fastbit/doc/indexSpec.html

    */
    void buildIndexes(const char* variableName, 
                      const char *binning=0);


    /*!
      \brief Build bitmap indices for a specific variable and timestep.

      \param timestep IN: timestep.
      \param variableName IN: name of the variable.
      \param binning IN: binning option (optional) 
      The format for the binning option is '\<binning nbins=xxxx /\>'
      where xxxx is the number of bins.
      If no binning is specified, a bitmap index is built for
      each distinct data value.
      Details on the binning option can be found at:
      http://sdm.lbl.gov/fastbit/doc/indexSpec.html

    */
    void buildSpecificTimeIndex(int64_t timestep, 
                                const char* variableName, 
                                const char *binning=0);


    /*!
      \brief Execute a query over a specific timestep.

      \param query IN: SQL-like query string, e.g. "a1 < 5 && b2 > 10'
      \param timestep IN: timestep
      \param offset OUT: offset locations of the hit values that satisfy
      the query condition.

    */
    void executeQuery(const char * query,
                      int64_t timestep,
                      std::vector<hsize_t>& offset);

    // Query for Equality selection. vector of double values is specified
    // which will avoid string creation/parsing overhead.. 
    int64_t executeEqualitySelectionQuery
    (const char * varname, int64_t time, 
     const std::vector<double>& identifiers, 
     std::vector<hsize_t>& offset);

    /*!
      \brief Get the number of hits that satisfy a query condition.

      \param query IN: SQL-like query string, e.g. "a1 < 5 && b2 > 10'
      \param timestep IN: timestep
      \return Number of hits.

    */
    int64_t getHitCount(const char * query,
                        int64_t timestep);


    /*!
      \brief Add a timestep with a specific time value.

      \param timeValue IN: value of the timestep.
    
    */

    void addNewTimeStep(double timeValue);


    /*!
      \brief Get the names of all variables of open file.

      \param names OUT: names of the variables.
    */

    void getVariableNames(std::vector<std::string> &names) const;


    /*!
      \brief Get number of timesteps.

      \return Number of timesteps.
    */
    int64_t getStepCount() const;


    /*!  \brief Query the current dataset for the existence of a particular
      variable
    */
    bool checkForVariable(const std::string& q_string) const;

    /*!
      \brief Get the min and max value of variable for a particular timestep.

      \param variableName IN: name of the variable
      \param timestep IN: timestep.
      \param type IN: data type of the variable.
      \param min OUT: min value.
      \param max OUT: max value.
    */
    bool getDataMinMax(const std::string& variableName,
                       int64_t timestep,
                       const BaseFileInterface::DataType type,
                       void *min,
                       void *max);

    /*!
      \brief Retrieve the min and max value of variable as double.

      \param variableName IN: name of the variable
      \param timestep IN: timestep.
      \param type IN: data type of the variable.
      \param min OUT: min value.
      \param max OUT: max value.
    */
    bool getDataMinMax_Double(const std::string& variableName,
                              int64_t timestep,
                              double &min,
                              double &max);

        void getDataMinMax_Float(const std::string& variableName,
                                 int64_t timestep,
                                 float &min,
                                 float &max);

    /*!
      \brief Get min time value.

      \return Min time value.
    */
    double getMinTimeValue(){return dataFile->getMinStep();}

    /*!
      \brief Get max time value.

      \return Max time value.
    */
    double getMaxTimeValue(){return dataFile->getMaxStep();}
    /// Retreive the object representing the data file.
    const H5_Index& getDataFile() const {return *dataFile;}

private:
    //private methods and members

    /// Create the data structures to represent the time slices.
    void createSlices();
    /// Remove the data structures representing the time slices.
    void deleteSlices();
        
   
    /// Number of time slices (time steps) in the HDF5 file.
    size_t numTimeSlices() const {return timeSlices.size();}

    /// std::vector<H5_FQ_Timestep*> timeSlices
    ibis::partList timeSlices;
    H5_Index *dataFile;
};
#endif
