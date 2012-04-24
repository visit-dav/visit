/**
 *
 * @file        avtVsFileFormat.h
 *
 * @brief       Base class for VizSchema visit plugins
 *
 * @version $Id: avtVsFileFormat.h 27 2008-03-26 22:04:41Z sveta $
 *
 * Copyright &copy; 2007, Tech-X Corporation
 * See LICENSE file for conditions of use.
 *
 */

// ************************************************************************* //
//                            avtVsFileFormat.h                                 //
// ************************************************************************* //

#ifndef VS_FILE_FORMAT_H
#define VS_FILE_FORMAT_H

#include <vtkUnsignedCharArray.h>
#include <vtkPoints.h>
#include <VsH5Dataset.h>
//#include <VsH5Reader.h>
#include <avtSTMDFileFormat.h>
#include <hdf5.h>
#include <visit-hdf5.h>

#include <string>
#include <vector>

// Forward references to minimize compilation
class vtkPoints;
class vtkUnsignedCharArray;
class vtkDataSet;
class vtkDataArray;
class avtDatabaseMetaData;
class avtMeshMetaData;
class VsVariableWithMesh;
class VsUnstructuredMesh;
class VsUniformMesh;
class VsStructuredMesh;
class VsRectilinearMesh;
class VsRegistry;
class VsH5Reader;

/**
 * avtSTMDFileFormat is a base class for multi-domain, single-time
 * VisIt plugins.  It provides mesh, scalar, and vector data.  It
 * assumes an ordering of component minor in the data file.  This is
 * reversed to get Fortran ordering.
 *
 * No ability to account for tensors at the present time.
 */
class avtVsFileFormat: public avtSTMDFileFormat {
#if HDF5_VERSION_GE(1, 8, 1)
  public:

  /**
   * Construct a file reader from a data file.
   *
   * @param dfnm the name of the data file
   */
  avtVsFileFormat(const char*, DBOptionsAttributes *);

  /**
   * Destructor
   */
  virtual ~avtVsFileFormat();

  /**
   * Get plugin type
   *
   * @return plugin type name
   */
  virtual const char* GetType(void) {
    return "Vs";
  };

  virtual std::string CreateCacheNameIncludingSelections(std::string s);

  /**
   * Get the data selections
   *
   */
  virtual void RegisterDataSelections( const std::vector<avtDataSelection_p> &sels,
                                       std::vector<bool> *selectionsApplied );

  /**
   * Process the data selections
   *
   */
  bool ProcessDataSelections(int *mins, int *maxs, int *strides);

  /**
   * Get a mesh by name
   *
   * @param domain the number of the domain in this mesh
   * @param meshname the name of the mesh
   *
   * @return a pointer to the mesh. Caller assumes ownership (must delete when done)
   */
  virtual vtkDataSet* GetMesh(int domain, const char* meshname);

  /**
   * get a scalar variable by name
   *
   * @param domain the number of the domain in this mesh
   * @param varname the name of the variable
   *
   * @return a pointer to the variable. Caller assumes ownership (must delete when done)
   */
  virtual vtkDataArray* GetVar(int domain, const char* varname);

  /**
   * Free up any resources created by this object.
   */
  virtual void FreeUpResources(void);

  /**
   * Called to alert the database reader that VisIt is about to ask for data
   * at this timestep.  Since this reader is single-time, this is 
   * only provided for reference and does nothing.
   */
  virtual void ActivateTimestep(void);
  
  /**
   * Updates cycles and times in the given database metadata object
   * Deprecated 06.02.2011 in favor of GetCycle and GetTime
   * Marc Durant
   */
  virtual void UpdateCyclesAndTimes(avtDatabaseMetaData* md);
  
  protected:
  /**
   * Determines if the associated file has a valid cycle number
   * @return true if cycle is valid, otherwise false
   */
  virtual bool ReturnsValidCycle();

  /**
   * Get the cycle for the associated file
   * @return the cycle, or INVALID_CYCLE if none is available
   */
  virtual int GetCycle();

  /**
   * Determines if the associated file has a valid time
   * @return true if time is valid, otherwise false
   */
  virtual bool ReturnsValidTime();

  /**
   * Get the time for the associated file
   * @return the time, or INVALID_TIME if none is available
   */
  virtual double GetTime();

  /** Populate the meta data */
  virtual void PopulateDatabaseMetaData(avtDatabaseMetaData* md);

  /** The file containing the data */
  std::string dataFileName;

  /** Pointer to the reader */
  VsH5Reader* reader;

  /** Ensure data has been read **/
  void LoadData();

  private:
  /**
   * A counter to track the number of avtVsFileFormat objects in existence
   */
  static int instanceCounter;

  /**
   * A registry of all objects found in the data file
   */
  VsRegistry* registry;

  /** Some stuff to keep track of data selections */
  std::vector<avtDataSelection_p> selList;
  std::vector<bool>              *selsApplied;

  bool processDataSelections;

  /**
   * Maintain a list of curve names so we can classify expressions better
   */
  std::vector<std::string> curveNames;

  /**
   * Set the axis labels for a mesh.
   *
   * @param mmd a pointer to the object that needs the axis labels.
   */
  void setAxisLabels(avtMeshMetaData* mmd);

  /**
   * Create various meshes.
   */
  vtkDataSet* getUniformMesh(VsUniformMesh*, bool, int*, int*, int*);
  vtkDataSet* getRectilinearMesh(VsRectilinearMesh*, bool, int*, int*, int*, bool);
  vtkDataSet* getStructuredMesh(VsStructuredMesh*, bool, int*, int*, int*);
  vtkDataSet* getUnstructuredMesh(VsUnstructuredMesh*, bool, int*, int*, int*);
  vtkDataSet* getPointMesh(VsVariableWithMesh*, bool, int*, int*, int*);
  vtkDataSet* getCurve(int domain, const std::string& name);

  /**
   * Each type of object is added to the database with a separate method
   * for neatness.
   */
  void RegisterMeshes(avtDatabaseMetaData* md);
  void RegisterMdMeshes(avtDatabaseMetaData* md);
  void RegisterVarsWithMesh(avtDatabaseMetaData* md);
  void RegisterVars(avtDatabaseMetaData* md);
  void RegisterMdVars(avtDatabaseMetaData* md);
  void RegisterExpressions(avtDatabaseMetaData* md);

  void GetSelectionBounds( int numTopologicalDims,
                           std::vector<int> &numCells,
                           std::vector<int> &gdims,
                           int *mins,
                           int *maxs,
                           int *strides,
                           bool haveDataSelections,
                           bool isNodal = true );

  bool GetParallelDecomp( int numTopologicalDims,
                          std::vector<int> &dims,
                          int *mins,
                          int *maxs,
                          int *strides,
                          bool isNodal = true );
  
  template <typename TYPE>
  void fillInMaskNodeArray(const std::vector<int>& gdims,
                           VsH5Dataset *mask, bool maskIsFortranOrder,
                           vtkUnsignedCharArray *maskedNodes);

  template <typename TYPE>
    void setStructuredMeshCoords(const std::vector<int>& gdims,
                                 const TYPE* dataPtr,
                                 bool isFortranOrder,
                                 vtkPoints* vpoints);


#else
  avtVsFileFormat(const char* dfnm) : avtSTMDFileFormat(&dfnm, 1) {;};
#endif
};

#endif
