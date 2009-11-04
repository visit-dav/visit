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

#include <VsH5Reader.h>
#include <avtSTMDFileFormat.h>
#include <hdf5.h>
#include <visit-hdf5.h>

// Forward references to minimize compilation
class vtkDataSet;
class vtkDataArray;
class avtDatabaseMetaData;
class avtMeshMetaData;

/**
 * avtSTMDFileFormat is a base class for multi-domain, single-time
 * VisIt plugins.  It provides mesh, scalar, and vector data.  It
 * assumes an ordering of component minor in the data file.  This is
 * reversed to get Fortran ordering.
 *
 * No ability to account for tensors at the present time.
 */
class avtVsFileFormat : public avtSTMDFileFormat {
#if HDF5_VERSION_GE(1,8,1)
  public:

/**
 * Construct a file reader from a data file.
 *
 * @param dfnm the name of the data file
 * @param newStride The stride to use when loading data
 */
    avtVsFileFormat(const char* dfnm, std::vector<int> settings);

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

  protected:
/** Populate the meta data */
    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData* md);

/** The file containing the data */
    std::string dataFileName;

/** Reference to our stream for debugging information */
    std::ostream& debugStrmRef;

/** Pointer to the reader */
    VsH5Reader* reader;

  private:
       std::vector<int> stride;
/**
 * Set the axis labels for a mesh.
 *
 * @param mmd a pointer to the object that needs the axis labels.
 */
    void setAxisLabels(avtMeshMetaData* mmd);

/**
 * Create various meshes.
 */
    vtkDataSet* getUniformMesh(const std::string& nm, const VsMeshMeta&);
    vtkDataSet* getUnstructuredMesh(const std::string& nm, const VsMeshMeta&);
    vtkDataSet* getStructuredMesh(const std::string& nm, const VsMeshMeta&);
    vtkDataSet* getPointMesh(const std::string& name,
                              const VsVariableWithMeshMeta& meta);
    vtkDataSet* getSplitPointMesh(const std::string& name,
                              const VsUnstructuredMesh& meta);
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
#else
    avtVsFileFormat(const char* dfnm) : avtSTMDFileFormat(&dfnm, 1) {;};
#endif
};

#endif

