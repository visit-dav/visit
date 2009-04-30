/* This file is an attempt to allow any application to read its preferences into what Randy Frank would call a "mapobj".  I have stolen his idea and hopefully improved it to be more general and more robust, because it no longer relies on pointers to store its information.  In fact, it no longer allows pointers to be stored at all.  The presumption is that this is non-volatile information which can be written to disk.  I don't know of a way yet to pickle C items.  Maybe later I'll change to a binary output format and then allow any data to be captured to disk.  Not today. 
   All values are stored as C++ strings.  Functions which set or get values as other types are merely converting a string to the desired type or vice-versa.

   To do:  multiple Preferences should be able to be written to a single file. 
 */
// $Revision: 1.20 $
#ifndef RCPREFS_H
#define RCPREFS_H

#include <map>
#include <iostream>
#include <vector> 
#include <string>
#include <fstream>
#include "stringutil.h"

//======================================================
struct argType {
  argType(string flag, string key, string preftype) {
    mKey = key; 
    mType = preftype; 
    mFlag = flag; 
  }
  string mFlag, 
    mKey, 
    mType; // "bool", "long", "double", "string"
};
    
//======================================================
class Preferences {
 public:
  //========================
  //Initialization
  Preferences(){Reset();}
  Preferences(std::string filename) { Reset(); SetFile(filename);}
  
  /* Merge:  combine this with other.  
     Iff addnew is true, then keys in other but not in this will be added to this
     Iff overwrite is true, then keys in other overwrite any keys in this by the same name.
     Note that if both are false, then nothing will be done! 
  */
  void Merge(const Preferences &other, bool addnew=true, bool overwrite=true); 
  // conveniences:
  void MergeNoOverwrite(const Preferences &other) { Merge(other, true, false); }
  void MergeNoAdd(const Preferences &other) {Merge(other, false, true); }

  void Reset(void); 
  void ClearPrefs(void) { mPrefs.clear();}
  std::map<std::string, std::string> GetPrefsMap(void) { return mPrefs; }

  //========================
  operator string() const {
    string tmp = string("Preferences: \n")
      + "mFilename: : " + mFilename + "\n" 
      +  "mPrefs:\n"; 
    map<string, string>::const_iterator pos = mPrefs.begin(), end = mPrefs.end();
    while(pos!=end) {
      tmp +=  " (" + pos->first + ", " + pos->second + ") \n";
      ++pos;
    }
    tmp +=  string("_dirty: ") + (int)_dirty  +  "\n"
      +  "_writtenToDisk: "  +  (int)_writtenToDisk  +  "\n"; 
    return tmp; 
  }

  //========================
  // meta-attributes
  void SetFile(std::string name){ mFilename = name;}
  std::string GetFilename(void) { return mFilename;}

  // disambiguate self from other prefs in a file:
  void SetLabel(std::string label){ SetValue("prefs_label", label);}
  std::string GetLabel(void) { return GetValue ("prefs_label");}

  //========================
  //saving and restoring from disk
  void SaveToFile(bool createDir=true, bool clobber=true); //open the file, read and remember previous sections, read and discard my section, read and remember sections after me, then write previous section, my section, and trailing sections, then close the file.  Optionally create the needed directory for the file. 
  void SaveToFile(vector<string> saveKeys, bool createDir=false, bool clobber=false) {
    //same as SaveToFile() above, but only save keys listed in saveKeys
    mSaveKeys = saveKeys; 
    SaveToFile(createDir, clobber); 
    mSaveKeys.clear(); 
  }
  void SaveToFile(string filename, bool createDir=false) {
    //convenience function
    SetFile(filename); 
    SaveToFile(createDir); 
  }

  void ReadFromFile(bool throw_exceptions=false); //open the file, seek past label, read my section, close file
  void ReadFromFile(string filename, bool throw_exceptions=true) {
    SetFile(filename); 
    ReadFromFile(throw_exceptions); 
  }
  //=============================
  /* Glean the prefs from the command line.  The "types" argument is a vector of argTypes, which tell the flag, the key to assign, and the type, e.g., 
     vector<argType> types; 
     types.push_back("-keep", "keep", "bool"); 
     types.push_back("-longflag", "longkey", "long"); 
     types.push_back("-doubleflag", "doublekey", "double"); 
     types.push_back("-d", "doublekey", "double"); 
     GetFromArgs(argc, argv, types); 

     When done, the options and their arguments will be stripped from argv and argc will be adjusted appropriately.  
  */ 

  void GetFromArgs(int &argc, char *argv[], vector<argType> &types);

  //=============================
  // Copy the entire environment variable list into prefs, e.g., if $verbose is 5, then set Prefs["verbose"] to "5"
  void ReadFromEnvironment(void); 

  bool hasKey(string key) {
    return mPrefs.find(key) != mPrefs.end();
  }

  //=============================
  // AddEquivalentArgs: (not implemented yet)
  /* All keys in inEquivalents are considered to be the same as inOption when found on the command line.  inEquivalents are appended to any previous equivalents.  For example, if inOption = "keep" and inEquivalents is { "keep", "dontdiscard" }, then all the following set the value of variable "keep", when found on the command line: {--keep, -keep, --dontdiscard, -dontdiscard }.  Equivalencies can be chained together.  That is, 
     AddEquivalentArgs("arg", {"blah"});
     and
     AddEquivalentArgs("blah", {"blather"}); 
     makes "blather" an equivalent to "arg"!
  */
  // void AddEquivalentArgs(const string &inOption, const vector<string> &inEquivalents); 
  /* append the inEquivalent to the list of equivalent options for inOption */
  // void AddEquivalentArg(const string &inOption, const vector<string> &inEquivalents);


  //========================
  /* values: getting and setting (remember to set dirty bit!) 
     Note that all values are actually saved as strings and are converted, so this is not exactly a high-performance library.  :-) */
  void SetValue(const std::string &key, const std::string &value) { mPrefs[key]= value;}
  void SetValue(const std::string &key, double value);
  void SetBoolValue(const std::string &key, bool value) {
    SetValue(key, value?string("true"):string("false")); 
  }

  // The following return false if no such key exists or bad values or an empty string are generated, 
  // They return true if it does and store the value: 
  bool TryGetValue(const std::string &key, std::string &outValue) const;
  bool TryGetLongValue(const std::string &key, long &outValue) const;
  bool TryGetDoubleValue(const std::string &key, double &outValue) const;
  // see below for GetBoolValue()

  /*!
    The following return 0, or "" if no such key exists or bad vals are generated. If dothrow is true, then  throw an exception for bad vals.
  */ 
  std::string GetValue(const  std::string &key, bool dothrow=false) const;
  long GetLongValue(const  std::string &key, bool dothrow=false)  const;
  double GetDoubleValue(const  std::string &key, bool dothrow=false) const;
  std::string operator [] (const std::string &key) const { return GetValue(key); }

  // special case: never throws;  not being defined is the same as being false
  bool GetBoolValue(const std::string &key) { 
    string value; 
    if (!TryGetValue(key, value)) return false; 
    return value == "true"; 
  }
  
 protected:
  std::string NextKey(std::ifstream&theFile);
  std::map<std::string, std::string> ReadNextSection(std::ifstream &theFile);
  void SaveSectionToFile(std::ofstream &outfile, std::map<std::string, std::string> &section);

  // there is no copy constructor, so be sure these shallow copy:
  vector< pair< string, vector<string> > > mEquivalents; 
  std::map<std::string, std::string>  mPrefs; 
  std::vector<std::string> mSaveKeys; // only save these keys if given
  std::string mFilename;
  char _dirty; 
  char _writtenToDisk;
};

#endif
