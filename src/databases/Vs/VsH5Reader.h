#include <hdf5.h>
#include <visit-hdf5.h>
#if HDF5_VERSION_GE(1,8,1)
/**
 * @file  VsH5Reader.h
 *
 * @class VsH5Reader
 *
 * @brief Interface for getting metadata of h5 arrays and
 * extracting them given their path.
 *
 * Copyright &copy; 2008 by Tech-X Corporation
 */

#ifndef VS_H5_READER
#define VS_H5_READER

#include <VsFilter.h>
#include <VsMeta.h>

typedef class std::pair<std::string, int> NamePair;

/**
 * VsH5Reader is a class for getting the vizschema metadata of
 * objects in and HDF5 file and also being able to return such objects.
 */
class VsH5Reader {

  public:
/**
 * Constructor.
 * @param filename the name of the HDF5 file
 * @param dbgstrm The output stream to use for all debugging output
 * @param newStride The stride to use when loading data
 */
    VsH5Reader(const std::string& filename, std::ostream& dbgstrm, std::vector<int> settings);

/**
 * Destructor.
 */
    ~VsH5Reader();

/**
 * Set file.
 */
    void setFile(const std::string& filename);

/**
 * Get the values of the whole variable (dataset).
 * Caller owns returned array and must allocate memory and delete it.
 * @param varname the variable name
 * @param data the returned filled array
 */
    herr_t getVariable(const std::string& varname, void* data) const;
    herr_t getVariableComponent(const std::string& varname, size_t indx, void* data);
    herr_t getVariableWithMesh(const std::string& varname, void* data) const;

/**
 * Get the values of the whole variable (dataset).
 * Caller owns returned array and must allocate memory and delete it.
 * @param varname the variable name
 * @param indx index of the component
 * @param partnumber number of mesh part (along axis with leargest extent to load) for partial loading
 * @param numparts into how many parts should mesh be split along axis of longest extent
 * @param splitDims optional... unknown purpose
 */
    void* getVariableComponent(const std::string& varname, size_t indx,
            size_t partnumber=0, size_t numparts=1, size_t* splitDims=0);

/**
 * Get all expressions.
 */
    const std::map<std::string, std::string>& getVsVars() const;

/**
 * Get var metadata by name.
 * @param name the name of the variable
 */
    const VsVariableMeta* getVariableMeta(const std::string& name) const;

/**
 * Get varWithMesh metadata by name.
 * @param varname the name of the variable
 */
    const VsVariableWithMeshMeta* getVariableWithMeshMeta(
      const std::string& varname) const;

/**
 * Get mesh metadata by name.
 * @param name the name of the variable
 */
    const VsMeshMeta* getMeshMeta (const std::string& name) const;

/**
 * Get MD mesh metadata by name.
 * @param name the name of the variable
 */
        const VsMDMeshMeta* getMDMeshMeta(const std::string& name) const;
        int getDomainNumberForMesh(const std::string& name) const;
        const VsMDVariableMeta* getMDVariableMeta(const std::string& name) const;
        const VsMDMeshMeta* getMDParentForMesh(const std::string& name) const;
        VsMeshMeta* findSubordinateMDMesh(const std::string& name) const;
        const VsVariableMeta* findSubordinateMDVar(const std::string& name) const;


/**
 * Get number of components of a variable.
 * @param name The name of the variable to retrieve
 */
    size_t getNumComps (const std::string& name) const;
    size_t getMDNumComps(const std::string& mdVarName) const;

/**
 * Get mesh's dimensions.
 * @param name The name of the mesh to retrieve
 * @param dims The return value - a vector of integer dimensions
 */
    size_t getMeshDims (const std::string& name, std::vector<int>* dims) const;

/**
 * Get dataset component of a mesh.
 * @param cname The name of the mesh component to retrieve
 * @param meta The metadata for the mesh
 * @param data The return value containing the requested dataset - ownership passes
 */
    herr_t getDatasetMeshComponent (const std::string& cname, const VsMeshMeta& meta,
      void* data) const;
    herr_t getSplitMeshData(std::string points0, std::string points1, std::string points2,
            const VsMeshMeta& mmeta, void* data) const;

/**
 * Get attribute component of a mesh.
 * @param cname The name of the mesh component to retrieve
 * @param meta The metadata of the mesh
 * @param data The return value containing the requested attribute - ownership passes
 */
    herr_t getAttMeshComponent (const std::string& cname, const VsMeshMeta& meta,
      void* data) const;

/**
 * Get mesh of a var with mesh.
 * @param meta metadata for the var with mesh
 * @param data the array to be filled with mesh points
 */
    herr_t getVarWithMeshMesh(const VsVariableWithMeshMeta& meta,
      void* data) const;

    herr_t getVarWithMeshMesh(const VsVariableWithMeshMeta& meta,
      void* data, size_t partStart, size_t partCount) const;

/**
 * Get variable component of a var with mesh
 * Caller owns data and must allocate and delete it.
 * @param varname the variable name
 * @param idx index of component to read
 * @param data the returned filled array
 * @return 0 on success
 */
    herr_t getVarWithMeshComponent(const std::string& varname, size_t idx,
      void* data) const;

    herr_t getVarWithMeshComponent(const std::string& varname, size_t idx,
      void* data, size_t partStart, size_t partCount) const;
/**
 * Get kind of mesh.
 */
 //   herr_t getMeshKind (const std::string& name, std::string& kind) const;

/**
 * Get meshes names.
 */
        void getMeshesNames(std::vector<std::string>& names) const;

/**
 * Get meshes names.
 */
        void getMDMeshNames(std::vector<std::string>& names) const;

/**
 * Get variables names.
 */
        void getVarsNames(std::vector<std::string>& names) const;

/**
 * Get variables names.
 */
        void getMDVarsNames(std::vector<std::string>& names) const;

/**
 * Get varsWithMesh names.
 */
    void getVarsWithMeshNames(std::vector<std::string>& names) const;

/**
 * Is it a var or var with mesh?
 */
    bool isVariable(const std::string& name) const;

/**
 * Should the reader use stride values to reduce the amount of data loaded?
 */
          bool useStride;

/**
 * Adds a component to the registry list
 * Each component name is mapped to the underlying var and index
 */
                bool registerComponentInfo(std::string componentName, std::string varName, int componentNumber);

/**
* Retrieve the registered component using its name
* Returns NULL if not found
*/
         void getComponentInfo(std::string componentName, NamePair* namePair);

/**
 * Find a unique name for the given component info.
 */
                std::string getUniqueComponentName(std::string userName, std::string varName, int componentIndex);

/**
 * Generate the old-style name from the given component info.
 * name will be of the form "varName_componentIndex"
 */
                std::string getOldComponentName(std::string varName, int componentIndex);


  private:
// a registry of user-specified component names and their mappings
// first element of pair is the user-specified name
// second element of pair is "true" component name
    std::vector< std::pair<std::string, NamePair > > componentNames;

// The stride to use when loading data
        std::vector<int> stride;

/** Reference to our stream for debugging information */
    std::ostream& debugStrmRef;

// Metareader
    VsFilter* filter;

// Copy of the metareader data
    const VsH5Meta* h5meta;

// Metadata
    VsMeta* meta;

// File id -opened once
    hid_t fileId;

// Make all metadata
    void makeVsMeta();

// Add all derived variables (vsVars)
    void makeVsVars();

/**
 * Create VSH5 metadata for var
 * @param dm The H5 metadata of the variable
 * @param meta the returned VSH5 variable metadata
 */
    herr_t makeVariableMeta(VsDMeta* dm, VsVariableMeta& meta) const;

/**
 * Create varWithMesh VSH5 metadata
 * @param dm The H5 metadata of the variable
 * @param meta the returned VSH5 variable metadata
 */
    herr_t makeVariableWithMeshMeta(VsDMeta* dm,
        VsVariableWithMeshMeta& meta) const;

/**
 * Create mesh VSH5 metadata
 * @param dm The H5 metadata of the mesh
 * @param meta The returned VSH5 metadata of the mesh
 */
    herr_t makeDMeshMeta(VsDMeta* dm, VsMeshMeta& meta ) const;

/**
 * Create mesh VSH5 metadata
 * @param gm The H5 metadata of the mesh
 * @param meta The returned VSH5 metadata of the mesh
 */
    herr_t makeGMeshMeta(VsGMeta* gm, VsMeshMeta& meta ) const;

};

#endif
#endif
