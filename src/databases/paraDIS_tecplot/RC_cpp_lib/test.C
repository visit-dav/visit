#include "Prefs.h"
#include "RangeList.h"
#include "stringutil.h"
#include <iostream>
#include <vector> 
#include <map>
#include <string>

using namespace std; 

//====================================================
int TestKey(Preferences &P, string key, string value){
  if (P.GetValue(key) != value){
    cerr << "Tests failed because P.GetValue("<<key<<") returned \"" <<P.GetValue(key)<<"\" instead of the expected value \""<<value<<"\""<<endl;
    return 4; 
  }
  return 0;
}

//====================================================
int TestPrefs(Preferences &P) {
  
  cerr << "save to file..."<<endl;
  string label = P.GetLabel();
  string key1("key1"), key2("key2");
  string value1(P.GetValue(key1)), value2(P.GetValue(key2));    
  string filename = P.GetFilename();
  try {
    P.SaveToFile();
  } 
  catch (string err){
    cerr << err <<endl;
    return 1;
  }
  cerr << "reset..."<<endl;
  P.Reset();
  cerr << "ReadFromFile..."<<endl;
  P.SetFile(filename);
  P.SetLabel(label);
  try {
    P.ReadFromFile();
  } 
  catch (string err){
    cerr << err <<endl;
    return 2;
  }
  int err = TestKey(P, key1, value1);
  if (!err)
    err = TestKey(P, key2, value2);
  return err; 
}

//====================================================
void TestPrefs(void){
  int err = 0; 
  try {
    string key1("key1"), key2("key2");
    
    //test prefrerences
    Preferences P("testprefs.txt");  
    P.SetLabel("This is the label");
    P.SetValue(key1, "Blah");
    P.SetValue(key2, "Lots of words");
    err = TestPrefs(P);
    if (!err){
      cout << "Round 2..." << endl;
      P.Reset();
      P.SetFile("testprefs.txt");
      P.SetLabel("New label");
      P.SetValue(key1, "Blah2");
      P.SetValue(key2, "True love lasts forever");
      err = TestPrefs(P);
    }
    P.SetLabel("new label");
    
    cout << "Testing RangeList" << endl; 
    RangeList rlist("testrange.txt", 1);
    rlist.Sort();
    rlist.Unique();
    cout << "unsorted:" << endl; 
    rlist.Print();
    cout << "sorted: " << endl; 
    rlist.Print();

    if (!rlist.InRange(43))
      throw string("43 should be in range but isn't!");
    if (rlist.InRange(7))
      throw string("7 is not supposed to be in range!");

    cout << " testing environment capture... " << endl; 
    P.ReadFromEnvironment(); 
    std::map<std::string, std::string> thePrefs = P.GetPrefsMap(); 
    std::map<std::string, std::string>::iterator currentpos = thePrefs.begin(), endpos = thePrefs.end(); 
    while (currentpos != endpos) {
      cout << currentpos->first << ": " << currentpos->second << endl; 
      ++currentpos; 
    }
  }
  catch (string errs) {
    cerr << "Error while testing: " << endl;
    cerr << errs << endl; 
    err = 1;
  }

  if (!err) 
    cout << "Tests succeeded" << endl;   
  else
    cout << "Test failed." << endl;
  
  return ; 

}


//====================================================
void RunTest(int testnum, vector<string> &tests, vector<string> &expectedResults) {
  string resultString; 
  vector<string> result = Split(tests[testnum]); 
  vector<string>::iterator pos = result.begin(), endpos = result.end(); 
  while (pos != endpos) {
    resultString = "("; 
    vector<string>::iterator respos = result.begin(), resendpos = result.end(); 
    while (respos != resendpos) {
      if (respos != result.begin()) {
        resultString += ", "; 
      }
      resultString += *respos; 
      respos++;
    }
    resultString += ")"; 
    if (resultString == expectedResults[testnum]) {
      cerr << "SUCCESS: "; 
    } else {
      cerr << "FAILED: "; 
    }
    cerr << "result of test: expected "<<expectedResults[testnum] << " and got " <<resultString << endl; 
    ++pos;
  }
}
  
//====================================================
void TestStrings(void) {

  //-------------------
#define ADDTEST(test, expected) \
  tests.push_back(test); \
  expectedResults.push_back(expected) 
  //-------------------

  vector<string> tests, expectedResults; 
  ADDTEST("", "()"); 
  ADDTEST(" ", "()"); 
  ADDTEST(" test", "(test)"); 
  ADDTEST("test ", "(test)"); 
  ADDTEST("test1 test2", "(test1, test2)"); 
  ADDTEST(" test1 test2", "(test1, test2)"); 
  ADDTEST(" test1 test2 ", "(test1, test2)"); 
  ADDTEST("test1 test2 ", "(test1, test2)"); 
  ADDTEST("test1  test2 ", "(test1, test2)"); 
  ADDTEST("test1  test2 test3", "(test1, test2, test3)"); 
  ADDTEST("   ", "()"); 
  vector<string>::iterator pos = tests.begin(), endpos = tests.end(); 
  uint32_t testnum = 0; 
  while (testnum < tests.size()) {
    RunTest(testnum, tests, expectedResults); 
    ++testnum; 
  }
  return; 
}

//====================================================
int main(int argc, char *argv[]) {
  TestPrefs(); 
  TestStrings(); 
}
