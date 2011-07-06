/**
 * @file VsRegistry.h
 * 
 * @class VsRegistry
 * @brief Tracks all objects created while reading a file.
 * 
 * Maintains lists of all objects that are
 * needed to represent an hdf5 file.  Provides functions
 * to retrieve objects by name, print debugging information,
 * and get lists of object names.  Also provides for the creation
 * of MDVariables and MDMeshes from the lists of variables and meshes.
 *
 *  Created on: Apr 30, 2010
 *      Author: mdurant
 */

#ifndef VSREGISTRY_H_
#define VSREGISTRY_H_

class VsH5Dataset;
class VsH5Group;
class VsMesh;
class VsVariableWithMesh;
class VsVariable;
class VsMDMesh;
class VsMDVariable;

#include <map>
#include <string>
#include <vector>

typedef std::pair<std::string, int> NamePair;

class VsRegistry {
public:
  VsRegistry();
  virtual ~VsRegistry();
  void deleteAllObjects();
  
  //DATASETS
  void add(VsH5Dataset* obj);
  void remove(VsH5Dataset* obj);
  VsH5Dataset* getDataset(std::string name);
  void deleteAllDatasets();
  void writeAllDatasets();
  int numDatasets();
  void buildDatasetObjects();
 
  //GROUPS
  void add(VsH5Group* obj);
  void remove(VsH5Group* obj);
  VsH5Group* getGroup(std::string name);
  void deleteAllGroups();
  void writeAllGroups();
  int numGroups();
  void buildGroupObjects();
  void loadTime(VsH5Group* group);
  void loadRunInfo(VsH5Group* group);
  
  //MESHES
  void add(VsMesh* mesh);
  void remove(VsMesh* mesh);
  VsMesh* getMesh(std::string name);
  void getAllMeshNames(std::vector<std::string>& names);
  void deleteAllMeshes();
  void writeAllMeshes();  
  int numMeshes();
  
  //MD MESHES
  void add(VsMDMesh* var);
  void remove(VsMDMesh* var);
  void buildMDMeshes();
  void writeAllMDMeshes();
  VsMDMesh* getMDMesh(std::string name);
  VsMDMesh* getMDParentForMesh(const std::string& name);
  VsMesh* findSubordinateMDMesh(const std::string& name);
  void getAllMDMeshNames(std::vector<std::string>& names);
  int numMDMeshes();
  
  //VARIABLES
  void add(VsVariable* mesh);
  void remove(VsVariable* mesh);
  VsVariable* getVariable(std::string name);
  void getAllVariableNames(std::vector<std::string>& names);
  void deleteAllVariables();
  void writeAllVariables();  
  int numVariables();

  //VARIABLES WITH MESH
  void add(VsVariableWithMesh* mesh);
  void remove(VsVariableWithMesh* mesh);
  VsVariableWithMesh* getVariableWithMesh(std::string name);
  void getAllVariableWithMeshNames(std::vector<std::string>& names);
  void deleteAllVariablesWithMesh();
  void writeAllVariablesWithMesh();  
  int numVariablesWithMesh();
  
  //MD VARIABLES
  void add(VsMDVariable* var);
  void remove(VsMDVariable* var);
  void buildMDVars();
  void writeAllMDVariables();
  VsMDVariable* getMDVariable(std::string name);
  void getAllMDVariableNames(std::vector<std::string>& names);
  int numMDVariables();
  VsVariable* findSubordinateMDVar(const std::string& name);
  
  //VsVars
  void buildExpressions(VsH5Group* group);
  void addExpression(std::string name, std::string value);
  void writeAllExpressions();
  std::map<std::string, std::string>* getAllExpressions();
  int numExpressions();

  //Variable components
  void getComponentInfo(std::string componentName, NamePair* namePair);
  void getComponentInfo(std::string varName, int componentNumber, NamePair* namePair);
  std::string getComponentName(std::string varName, int componentNumber);
  std::string getOldComponentName(std::string varName, int componentIndex);
  void registerComponent(std::string varName, int componentNumber, std::string userSuppliedName);
  void createComponents();

  //Time
  bool hasTime() { return (timeValue != -1); }
  double getTime() { return timeValue; }
  bool hasCycle() { return (cycle != -1); }
  double getCycle() { return cycle; }
    
private:
  /**
   * A boolean flag for when objects are being deleted.
   */
  bool deletingObjects;
  
  std::map<std::string, VsH5Dataset*> allDatasets;
  std::map<std::string, VsH5Dataset*> allDatasetsShort;

  std::map<std::string, VsH5Group*> allGroups;
  std::map<std::string, VsH5Group*> allGroupsShort;

  std::map<std::string, VsMesh*> allMeshes;
  std::map<std::string, VsMesh*> allMeshesShort;

  std::map<std::string, VsMDMesh*> allMDMeshes;

  std::map<std::string, VsVariable*> allVariables;
  std::map<std::string, VsVariable*> allVariablesShort;

  std::map<std::string, VsMDVariable*> allMDVariables;
  
  std::map<std::string, VsVariableWithMesh*> allVariablesWithMesh;
  std::map<std::string, VsVariableWithMesh*> allVariablesWithMeshShort;

  std::map<std::string, std::string> allExpressions;
  
  //tracks whether time data has been read for this database
  // a value of -1 indicates no time data is available
  //Named timeValue because "time" is a typedef
  double timeValue;
  int cycle;
  
  // a registry of user-specified component names and their mappings
  // first element of pair is the user-specified name
  // second element of pair is "true" component name
  std::vector< std::pair<std::string, NamePair > > componentNames;
  bool registerComponentInfo(std::string componentName, std::string varName, int componentNumber);

};

#endif /* VSREGISTRY_H_ */
