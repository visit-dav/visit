/****************************************************************************
* Copyright (c) 2024, CEA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#ifndef ReaderFORT21_HeaderFile
#define ReaderFORT21_HeaderFile

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>

#ifdef WNT
#define file_pos_t __int64
#define open_file fopen
#define get_file_pos _ftelli64
#define set_file_pos _fseeki64
#elif defined( __APPLE__)
#define set_file_pos fseeko
#define open_file fopen
#define file_pos_t __int64_t
#define get_file_pos ftello
#else
#define open_file fopen64
#define file_pos_t __off64_t
#define get_file_pos ftello64
#define set_file_pos fseeko64
#endif

/*
static inline int findFromEnd( const std::string& theStr, const std::string& theWhat )
{
  size_t pos = theStr.find_last_of( theWhat );
  if ( pos == std::string::npos )
    pos = -1;
  return int(pos);
}
 */

static inline int find(const std::string& theStr, const std::string& theWhat)
{
  size_t pos = theStr.find(theWhat);
  if (pos == std::string::npos)
    return -1;
  return int(pos);
}

//! Platform where FORT21 file has been generated. It is similar to GUTHAREplatforms.
enum Platform
{
  Windows_Linux_32 = 0,  // 32-bits Windows/Linux (Intel/AMD architecture)         r = 4 l = 4 d = 4
  Linux_64f90 = 1,       // 64-bits Linux (f90, f95)                               r = 4 l = 8 d = 4
  Linux_64gcc = 2,       // 64-bits Linux (gcc, g77, gnu)                          r = 8 l = 4 d = 4
  Tru64_64f90 = 3,       // Tru64 UNIX (f90)                                       r = 4 l = 8 d = 4
  Unix_64f77 = 4,        // Tru64 UNIX (f77)                                       r = 4 l = 8 d = 8
  SGI_32 = 5,            // 32-bits SGI (big ebdian)                               r = 4 l = 4 d = 4
  IBM_Tech = 6,          // 32-bits IBM RS6000 (OS: AIX)                           r = 4 l = 4 d = 4
  Linux_Tech = 7         // 32-bits IBM Blade server (OS: Red Hat Enterprise AS4)  r = 4 l = 4 d = 8
};


/*! @brief Mesh data stored internally as a BasicMesh object.
 *
 *  This structure contains all the necessary information
 *  for constructing MEDCouplingFieldDouble (with the addition of the MPI
 *  communicator).
 */

//! This class provides services for parsing FORT21 file
class ReaderFORT21
{
public:
  //! CATHARE data types
  enum CathareType
  {
    T_PTR = 0x00,
    T_INT = 0x01,
    T_REAL = 0x02,
    T_CHAR = 0x03,
    T_UNDEF = 0x08
  };
  enum LocalisationField
  {
    L_UNDEF,
    L_ELEM,
    L_GLOBAL,
    L_TWALL,
    L_TRACE,
    L_ALTERNATE,
    L_DUALX,
    L_DUALY,
    L_DUALZ
  };


  //! Sets information about platform where file was written
  enum Status
  {
    NotDone = -1,
    Done,
    Fail
  };

public:
  // ---------- PUBLIC METHODS ----------

  //! Empty constructor
  explicit ReaderFORT21(int verbosity = 0);

  //! Destructor
  ~ReaderFORT21();
  //! Set verbosity
  void setVerbosity(int v)
  {
    verbosity_=v;
  }
  //! Sets the file to parse
  void setFile(const std::string& theName);

  //! Gets the file parsed
  const std::string getFileName();

  //! parses the file
  void parse();
  void fix_bad_times();

  //! Returns the status of parsing, see below enum
  ReaderFORT21::Status getStatus() const;

  //! Set platform where FORT21 file has been generated
  void setPlatform(const Platform thePlatform);

protected:
  //! Represents a field in the file; it is written as:
  //!   size - integer, the size of field
  //!   char[size] - field data
  //!   size - once more the same size
  //! The instance of this class is resized as needed to store
  //! the next portion of the file
  class Field
  {
  public:
    Field()
      : myFile(0)
      , myData(0)
      , myCapacity(0)
      , mySize(0)
    {
      setPlatform(Windows_Linux_32);
    }

    ~Field()
    {
      if (myData != 0)
        delete[] myData;
      setFile(0);
    }

    void setFile(FILE* theFile)
    {
      if (myFile != 0)
        fclose(myFile);
      myFile = theFile;

      myFPos = myFile ? get_file_pos(myFile) : 0;
    }

    void setMaxFileInt(int theMaxSizeInt)
    {
      myMaxSizeInt = theMaxSizeInt;
    }

    FILE* getFile() const
    {
      return myFile;
    }

    file_pos_t getFilePos() const
    {
      return myFPos;
    }

    bool read(int theSize = 0);

    int size() const
    {
      return mySize;
    }

    char* getData() const
    {
      return myData;
    }

    void clear()
    {
      mySize = 0;
    }

    void setPlatform(const Platform thePlatform)
    {
      myPlatform = thePlatform;
    }

    Platform getPlatform() const
    {
      return myPlatform;
    }

  private:
    FILE* myFile;
    file_pos_t myFPos;
    char* myData;
    int myCapacity;
    int mySize;
    int myMaxSizeInt;

    Platform myPlatform;
  };

  bool readRecord();
  bool readHeader();
  bool readDescr();
  bool readVaria();
  bool readDesStack(bool theSkip);

public:
  //!  Codes for meaning of variables in FORT21 file.
  enum MeshType
  {
    MESH_Unknown = 0,  //!< Fictive mesh (port, reactor, circuit)
    MESH_Polygone = 1,   //!< for volum, axial
    MESH_Hexa   = 2,  //!< for threed
    MESH_Polyedre = 3   //!< for volum, axial
  };

  //! Get meaning of CATHARE variable.
  const std::string& getTitle() const { return title_;};
  const std::string& getVersion() const { return version_;};
private:
  Status myStatus;
  int myRealSize;
  int myMaxSizeInt;
  Field myField;
  file_pos_t size_file;
  std::string myInFile;

  bool myIsC3;
  bool myIsSimpleReal;

  std::vector<double> Times_glob_;
  std::vector<file_pos_t> offset_a_time_;
  std::string title_,version_;

public:
  class FieldInfo
  {
    friend ReaderFORT21;

  public:
    int size() const
    {
      return size_;
    }
    int nb_parts() const
    {
      return nb_parts_;
    }
    // mettre un enum
    CathareType type_of_field() const
    {
      return type_of_field_;
    }
    LocalisationField localisation_of_field() const
    {
      return localisation_of_field_;
    }

  private:
    int size_;
    CathareType type_of_field_;
    LocalisationField localisation_of_field_;
    file_pos_t offset_rel_stack_;
    std::vector<file_pos_t> offsets_;
    std::vector<float> times_;
    int nb_parts_;
  };

private:
  class ElementInfo
  {
  public:
    std::string type_;
    //file_pos_t offset_const_;
    std::vector<int> index_times_;
    std::vector<file_pos_t> offset_rel_time_;
    std::map<std::string, FieldInfo> ConstFields_;
    std::map<std::string, FieldInfo> VarFields_;
    file_pos_t sizeblock_ = -1;
    bool stack_var_;
  };
  std::map<std::string, ElementInfo> elements_list_;
  int verbosity_;

  const ElementInfo& getElementInfo(const std::string& name_stack) const;
  const FieldInfo& getVarFieldInfo(const ElementInfo& eleminfo, const std::string& name_field) const;
  const FieldInfo& getConstFieldInfo(const ElementInfo& eleminfo, const std::string& name_field) const;

public:
  const FieldInfo& getVarFieldInfo(const std::string& name_stack, const std::string& name_field) const;
  const FieldInfo& getConstFieldInfo(const std::string& name_stack, const std::string& name_field) const;
  std::vector<double> getTimes() const
  {
    return Times_glob_;
  }
  int getNbCells(const std::string& name_stack) const;
  std::vector<double> getTimesStack(const std::string& name_stack) const;
  std::vector<float> getTimesStack_float(const std::string& name_stack, const std::string& name_field) const;
  // std::vector<int> getTimesIndexStack(const std::string& name_stack) const;
  std::vector<std::string> getElementNames() const;
  std::string getElementType(const std::string& name_stack) const;
  std::vector<std::string> getVarFieldNames(const std::string& name_stack) const;
  template <typename _TYPE_> void getValuesVarField(const std::string& name_stack, const std::string& name_field, std::vector<_TYPE_>& data, const int& id_time_field) const;
  template <typename _TYPE_> void getValuesVarFieldOnIndex(const std::string& name_stack, const std::string& name_field, std::vector<_TYPE_>& data, const int& id_index) const;
  template <typename _TYPE_> void getInterpolatedValuesVarField(const std::string& name_stack, const std::string& name_field, std::vector<_TYPE_>& data, const int& global_id_time) const;
  void getInterpolatedValuesVarPos(const std::string& name_stack, const std::string& name_field, std::vector<float>& data) const;
  void getInterpolatedValuesVarPos(const std::string& name_stack, const std::string& name_field, std::vector<double>& data) const;
  template <typename _TYPE1_, typename _TYPE2_> void getPosAndValuesLine3d(const std::string& elem, const std::string& field, int dir, float x, float y,float z, const float& time,  std::vector<_TYPE1_>& pos, std::vector<_TYPE2_>& data) const;
  int getIndexFromPos(const std::string& name_stack, const std::string& name_field, const double& x, const double& y, const double& z) const;
  std::vector<std::vector<float>>  getXYZS(const std::string& name_stack, const std::string& name_field) const;
  int getIndexFromPos(const std::string& name_stack, const std::string& name_field, const double& x, const double& y, const double& z, const  std::vector<std::vector<float>>& xyzs, int& i,int& j,int& k, int control =1) const;
  int getIndexFromPos(const std::string& name_stack, const std::string& name_field, const double& pos) const ;
  std::vector<int> getIJKFromIndex(const std::string& name_stack, const std::string& name_field, int index) const;
  std::vector<int> getIJKFromPos(const std::string& name_stack, const std::string& name_field, const double& x, const double& y, const double& z ) const;
  std::vector<float> getPosFromIndex(const std::string& name_stack, const std::string& name_field, int index) const;
  int getIndexFromTime(const std::string& name_stack, const double& t, const std::string& method="") const;


  file_pos_t getOffsetVarField(const std::string& name_stack, const std::string& name_field, const int& id_time_field) const;
  std::vector<std::string> getConstFieldNames(const std::string& name_stack) const;
  std::vector<std::string> getValuesConstNames(const std::string& name_stack, const std::string& name_field) const;
  template <typename _TYPE_> void getValuesConstField(const std::string& name_stack, const std::string& name_field, std::vector<_TYPE_>& data) const;
  class BasicMesh
  {
  public:
    /*! @brief Builds an empty mesh.
     */
    BasicMesh();

    /*! @brief Destructor.
     */
    ~BasicMesh();

    /*! @brief clear and reset all internal data structures.
     *
     * After the call to clear(), all pointers are null
     * Arrays are deleted if necessary
     */
    void clear();

    /*! @brief Used to simulate a 0D geometry (Cathare/Trio for example).
     */
    void dummy_geom();

    /*! @brief Save mesh to a .mesh file
     */
    void save(std::ostream& os) const;

    /*! @brief Restore mesh from a .mesh file
     */
    void restore(std::istream& in);

  public:
    MeshType type_mesh_;                  ///< 0 polygone 1 quadri ....
    int mesh_dim_;                   ///< Mesh dimension
    int space_dim_;                  ///< Space dimension
    int nbnodes_;                    ///< Number of nodes
    int nodes_per_elem_;             ///< Number of nodes for a single element
    int nb_elems_;                   ///< Number of elements
    std::vector<int> connectivity_;  ///< Nodal connectivity array
    std::vector<double> coords_;     ///< Coordinate array
  };
  BasicMesh getMeshStack(const std::string& name_stack) const;
  bool is3D(const std::string& name_stack) const;
protected:
  BasicMesh getMeshStack_(const std::string& name_stack, bool incomplet) const;
};
#endif
