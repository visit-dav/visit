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

class VsDataset;
class VsGroup;
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
  void add(VsDataset* obj);
  void remove(VsDataset* obj);
  VsDataset* getDataset(const std::string& name);
  void deleteAllDatasets();
  void writeAllDatasets() const;
  int numDatasets();
  void buildDatasetObjects();
 
  //GROUPS
  void add(VsGroup* obj);
  void remove(VsGroup* obj);
  VsGroup* getGroup(const std::string& name);
  void deleteAllGroups();
  void writeAllGroups() const;
  int numGroups();
  void buildGroupObjects();
  void loadTime(VsGroup* group);
  void loadRunInfo(VsGroup* group);
  
  //MESHES
  void add(VsMesh* mesh);
  void remove(VsMesh* mesh);
  VsMesh* getMesh(const std::string& name);
  void getAllMeshNames(std::vector<std::string>& names);
  void deleteAllMeshes();
  void writeAllMeshes() const;  
  int numMeshes();
  
  //MD MESHES
  void add(VsMDMesh* var);
  void remove(VsMDMesh* var);
  void buildMDMeshes();
  void writeAllMDMeshes() const;
  VsMDMesh* getMDMesh(const std::string& name);
  VsMDMesh* getMDParentForMesh(const std::string& name);
  VsMesh* findSubordinateMDMesh(const std::string& name);
  void getAllMDMeshNames(std::vector<std::string>& names);
  int numMDMeshes();
  
  //VARIABLES
  void add(VsVariable* variable);
  void remove(VsVariable* variable);
  VsVariable* getVariable(const std::string& name);
  void getAllVariableNames(std::vector<std::string>& names);
  void deleteAllVariables();
  void writeAllVariables() const;  
  int numVariables();

  //VARIABLES WITH MESH
  void add(VsVariableWithMesh* variable);
  void remove(VsVariableWithMesh* variable);
  VsVariableWithMesh* getVariableWithMesh(const std::string& name);
  void getAllVariableWithMeshNames(std::vector<std::string>& names);
  void deleteAllVariablesWithMesh();
  void writeAllVariablesWithMesh() const;  
  int numVariablesWithMesh();

  // TRANSFORMED MESH NAMES
  bool registerTransformedMeshName(std::string transformedMeshName, std::string origMeshName);
  std::string getOriginalMeshName(std::string transformedMeshName);

  // TRANSFORMED VAR NAMES
  bool registerTransformedVarName(std::string transformedVarName, std::string origVarName);
  std::string getOriginalVarName(std::string transformedVarName);

  //MD VARIABLES
  void add(VsMDVariable* var);
  void remove(VsMDVariable* var);
  void buildMDVars();
  void writeAllMDVariables() const;
  VsMDVariable* getMDVariable(const std::string& name);
  void getAllMDVariableNames(std::vector<std::string>& names);
  int numMDVariables();
  VsVariable* findSubordinateMDVar(const std::string& name);
  
  //Transformed meshes
  void buildTransformedMeshes();
  void buildTransformedVariables();
  
  //VsVars
  void buildExpressions(VsGroup* group);
  void addExpression(const std::string& name, const std::string& value);
  void writeAllExpressions() const;
  std::map<std::string, std::string>* getAllExpressions();
  int numExpressions();

  //Variable components
  void getComponentInfo(const std::string& componentName, 
                        NamePair* namePair);
  void getComponentInfo(const std::string& varName, 
                        int componentNumber, NamePair* namePair);
  std::string getComponentName(const std::string& varName, 
                               int componentNumber);
  std::string getOldComponentName(const std::string& varName, 
                                  int componentIndex);
  void registerComponent(const std::string& varName, 
                         int componentNumber, 
                         const std::string& userSuppliedName);
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
  
  std::map<std::string, VsDataset*> allDatasets;
  std::map<std::string, VsDataset*> allDatasetsShort;

  std::map<std::string, VsGroup*> allGroups;
  std::map<std::string, VsGroup*> allGroupsShort;

  std::map<std::string, VsMesh*> allMeshes;
  std::map<std::string, VsMesh*> allMeshesShort;

  std::map<std::string, VsMDMesh*> allMDMeshes;

  std::map<std::string, VsVariable*> allVariables;
  std::map<std::string, VsVariable*> allVariablesShort;

  std::map<std::string, VsMDVariable*> allMDVariables;
  
  std::map<std::string, VsVariableWithMesh*> allVariablesWithMesh;
  std::map<std::string, VsVariableWithMesh*> allVariablesWithMeshShort;

  std::map<std::string, std::string> allExpressions;
  
  std::map<std::string, std::string> transformedMeshNames;
  std::map<std::string, std::string> transformedVarNames;

  //tracks whether time data has been read for this database
  // a value of -1 indicates no time data is available
  //Named timeValue because "time" is a typedef
  double timeValue;
  int cycle;
  
  // a registry of user-specified component names and their mappings
  // first element of pair is the user-specified name
  // second element of pair is "true" component name
  std::vector< std::pair<std::string, NamePair > > componentNames;

  bool registerComponentInfo(const std::string& componentName, 
                             const std::string& varName, 
                             int componentNumber);

};

#endif /* VSREGISTRY_H_ */
