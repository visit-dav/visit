/**
 *
 * @file    avtVsFileFormat.h
 *
 * @brief   Base class for VizSchema visit plugins
 *
 * @version $Id: avtVsFileFormat.h 27 2008-03-26 22:04:41Z sveta $
 *
 * Copyright &copy; 2007-2019, Tech-X Corporation
 * See LICENSE file for conditions of use.
 *
 */

// ************************************************************************* //
//                            avtVsFileFormat.h                              //
// ************************************************************************* //

#ifndef VS_FILE_FORMAT_H
#define VS_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>
#include "HighOrderUnstructuredData.h"

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
class VsReader;

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
    virtual void RegisterDataSelections(
        const std::vector<avtDataSelection_p> &sels,
        std::vector<bool> *selectionsApplied);

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
 * @return a pointer to the mesh. Caller assumes ownership
 *         (must delete when done)
 */
    virtual vtkDataSet* GetMesh(int domain, const char* meshname);

/**
 * get a scalar variable by name
 *
 * @param domain the number of the domain in this mesh
 * @param varname the name of the variable
 *
 * @return a pointer to the variable. Caller assumes ownership
 *         (must delete when done)
 */
    virtual vtkDataArray* GetVar(int domain, const char* varname);

/**
 * Get variable in the case when it is stored on a node by node basis per cell.
 * This is needed for discontinuous Galerkin data.
 * @param meta is the variable of interest
 * @param component is the component of interest
 */
    virtual vtkDataArray* NodalVar(VsVariable* meta,
        std::string requestedName, int component);

/**
 * Get a variable that is "standard".  A standard variable is a variable that
 * is stored on cell centers, cell vertices or edges.  It does not include
 * discontinuous Galerkin type data (or likely other high order data).
 */
    virtual vtkDataArray* StandardVar(int domain, const char* requestedName);

/**
 * Determine if the name is a component of a larger vector
 * @param name is the name of the variable
 * @param componentIndex is the index of that variable (returned).
 */
    virtual bool nameIsComponent(std::string& name, int& componentIndex);

/**
 * Check to see if the name has been transformed to a different name for
 * display
 * @param name is the name of the variable
 */
    virtual bool isTransformedName(std::string& name);

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

  protected:

/**
 * Get the cycle for the associated file
 * @return the cycle, or INVALID_CYCLE if none is available
 */
    virtual int GetCycle();

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
    VsReader* reader;

/** Ensure data has been read */
    void LoadData();

/**
 * This is not the best way to do this.  In fact every type of mesh should
 * have a separate class and then there would be a pointer to the type selected.
 * Since I only have one class that uses this approach I'm doing it this way.
 */
    HighOrderUnstructuredData thisData;

/**
 * Change the default behavior so that the plugin does not populate
 * spatial extent information.
 */
    bool HasInvariantMetaData(void) const {
      return false;
    }

  private:

/**
 * Get the meta data for a variable given a name.
 * @domain is the domain of interest
 * @requestedName is the full name of the variable
 * @componentIndex is the component we are requesting
 */
    VsVariable* getVariableMeta(int domain, std::string requestedName,
        int &componentIndex);

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
    void setAxisLabels(avtMeshMetaData* mmd, bool transform = false);

/**
 * Set the global extents for a mesh.
 *
 * @param mmd a pointer to the object that needs the axis labels.
 */
    void setGlobalExtents(avtMeshMetaData* mmd);

/**
 * Create various meshes.
 */
    vtkDataSet* getUniformMesh(VsUniformMesh*, bool, int*, int*, int*);
    vtkDataSet* getRectilinearMesh(VsRectilinearMesh*, bool, int*, int*, int*,
        bool);
    vtkDataSet* getStructuredMesh(VsStructuredMesh*, bool, int*, int*, int*);
    vtkDataSet* getUnstructuredMesh(VsUnstructuredMesh*, bool, int*, int*,
        int*);
    vtkDataSet* getPointMesh(VsVariableWithMesh*, bool, int*, int*, int*, bool);
    vtkDataSet* getHighOrderUnstructuredMesh(VsUnstructuredMesh*, bool, int*,
        int*, int*);
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

    void GetSelectionBounds(size_t numTopologicalDims,
        std::vector<int> &numCells,
        std::vector<int> &gdims,
        int *mins,
        int *maxs,
        int *strides,
        bool haveDataSelections,
        bool isNodal = true);

    bool GetParallelDecomp(int numTopologicalDims,
        std::vector<int> &dims,
        int *mins,
        int *maxs,
        int *strides,
        bool isNodal = true);

    template <typename TYPE>
    void fillInMaskNodeArray(const std::vector<int>& gdims,
        VsDataset *mask, bool maskIsFortranOrder,
        vtkUnsignedCharArray *maskedNodes);

    template <typename TYPE>
    void setStructuredMeshCoords(const std::vector<int>& gdims,
        const TYPE* dataPtr,
        bool isFortranOrder,
        vtkPoints* vpoints);

#else
    avtVsFileFormat(const char* dfnm) : avtSTMDFileFormat(&dfnm, 1) {};
#endif

};

#endif

