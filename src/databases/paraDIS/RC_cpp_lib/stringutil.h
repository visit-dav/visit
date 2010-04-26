#ifndef TSB_STRING_UTIL_H
#define TSB_STRING_UTIL_H
#include <string>
#include <string>
#include <stdio.h>
#include <algorithm>
//#include <inttypes.h>
//#include "RCDebugStream.h"

using namespace std; 

#define errout if (0) cerr
#define debugout if (0) cerr

  
//===============================================================
inline vector<string> Split(const string &s, char delimchar = ' ') {
  vector <string> sv; 
  vector<string::size_type> delims; 
  string sub; 
  errout << "Split(\""<<s<<"\", '"<<delimchar<<"')"<<endl;
  // build a list of where the delims occur in the string
  string::size_type found = s.find(delimchar), previous=string::npos; 
  errout << "initially found " << found<<endl;
  if (found == string::npos) {
    errout << "No delimchar found in string" << endl; 
    if (s.size()) {
      sv.push_back(s); 
    }
    return sv; 
  }

  while (1) {
    errout << "Loop: found is "<< found << endl;
    if (found == string::npos) {
      if (previous < s.size()-1) {
        sub=s.substr(previous+1); 
        errout << "1. Pushing back \""<<sub<<"\""<<endl;
        sv.push_back(sub);
      }
      return sv; 
    } 
    if (previous == string::npos) {
      if (found > 0) {
        sub=s.substr(0, found);
        errout << "2. Pushing back \""<<sub<<"\""<<endl;
        sv.push_back(sub); 
      }
    }
    else if (found - previous > 1) {
      sub=s.substr(previous+1, found-(previous+1));
      errout << "2. Pushing back \""<<sub<<"\""<<endl;
      sv.push_back(sub); 
    }
    previous=found;
    found = s.find(delimchar, found+1); 
  }

  return sv;             
}

//===============================================================
// Don't use references, so caller can construct args on the fly
// The rule is that after each pattern is replaced,  a new search is performed 
// from just after the old pattern.  So, for example, 
// Replace("xxxxx", "xxx", "yyy") = "yyyxx", NOT "yyyyy" -- since after the first replacement, the remaining "xx" != "xxx", so no second replacement is done.
inline string Replace(string input, string origPattern, string replPattern) {
  string output; 
  string::size_type found = input.find(origPattern), previous=string::npos; 
  // now loop and find and incrementall build new string.  
  while (1) {
    errout << "Loop: found is "<< found << endl;
    if (previous == string::npos) {
      if (found == string::npos) {
        return input; 
      }
      output = input.substr(0, found) + replPattern; 
    }
    else {
      if (found == string::npos) {
        output += input.substr(previous, input.size()-previous); 
        return output; 
      }
      if (found != previous) {
        output += input.substr(previous, found-previous); 
      }
      output += replPattern; 
    }
    previous = found + origPattern.size(); // just past the last pattern in input
    found = input.find(origPattern, previous); 
  }
}

//===============================================================
/*!
  Strip the indicated pattern from the tail of the value string, 
  as many repetitions as needed. May result in empty string. 
*/ 
inline string StripBack(const string &value, string pattern=" ") {
  string result(value);
  int loc = result.size() - pattern.size(), patlen = pattern.length(); 
  
  while(loc >= 0 && result.substr(loc,patlen) == pattern) {
    result.erase(loc, patlen); 
    loc = result.size() - pattern.size();
  }
  return result; 
}
//===============================================================
/*!
  Strip the indicated pattern from the head of the value string, 
  as many repetitions as needed. May result in empty string. 
*/ 
inline string StripFront(const string &value, string pattern=" ") {
  string result(value); 
  int reslen = result.length(), patlen = pattern.length(); 

  while (reslen >= patlen && result.substr(0, patlen) == pattern) {
    result.erase(0, patlen); 
    reslen = result.length();
  }
  return result; 
}
//===============================================================
/*!
  Strip the indicated pattern from the head and tail of the value string, 
  as many repetitions as needed. May result in empty string. 
*/ 
inline string Strip(const string &value, string pattern=" ") {
  return StripFront(StripBack(value, pattern), pattern); 
}
/* given a format string with a %0xd type format in it, apply the num to it and return the result.  I.e., applyPatternToInt("hello %04d world", 6) returns "hello 0006 world"
 */
inline string applyPatternToInt(string pattern, int num) {
  int len = pattern.size()+10; 
  char *tmp = new char[len];
  if (!tmp) throw string("Cannot allocate memory in applyPatternToInt"); 

  sprintf(tmp, pattern.c_str(), num); 
  string out(tmp);
  delete[] tmp; 
  return out; 
}

/* given a format string with a %s type format in it, apply the string s to it and return the result.  I.e., applyPatternToInt("hello %s", "world) returns "hello world"
 */
inline string applyPatternToString(string pattern, string s) {
  int len = pattern.size()+s.size()+5; 
  char *tmp = new char[len];
  if (!tmp) throw string("Cannot allocate memory in applyPatternToString"); 

  sprintf(tmp, pattern.c_str(), s.c_str()); 
  string out(tmp);
  delete[] tmp; 
  return out; 
}


// operator string() cannot be overloaded for doubles, so:
inline std::string doubleToString(double d, int precision=-1){
  char buf[128] = "", fmt[1024] = "%f";
  if (precision != -1) 
    sprintf(fmt, "%%.%df", precision); 
  
  sprintf(buf, fmt, d);    
  std::string s(buf);
  return s; 
}

// operator string() cannot be overloaded for ints
inline std::string intToString(double i) {
  return doubleToString(i, 0); 

  /*  char buf[128] = "";
  sprintf(buf, "%d", i);    
  std::string s(buf);
  return s; 
  */
}

inline std::string pointerToString(const void *ptr) {
  char buf[128] = "";
  sprintf(buf, "%p", ptr);
  return std::string(buf);
}

inline std::string operator +(std::string s , int d){
  return s+doubleToString(d);
}

inline std::string operator +(int d, std::string s ){
  return s+doubleToString(d);
}

inline std::string operator +(std::string s , double d){
  return std::string(s+doubleToString(d));
}

inline std::string operator +(double d, std::string s){
  return std::string(doubleToString(d)+s);
}

//=====================================================
template <class T> 
string doubleArrayToString(const vector<T>  &array) {
  if (!array.size()) return string("[(empty vector)]"); 

  string value("<");
  typename vector<T>::const_iterator pos = array.begin(), endpos = array.end();
  while (pos != endpos) {
    value += (doubleToString(*pos++));
    if (pos != endpos) value += ", "; 
    else value += ">"; 
  }
  return value; 
}

//=====================================================
template <class T> 
string arrayToString(const vector<T>  &array) {
  if (!array.size()) return string("[(empty vector)]"); 

  string value("<");
  typename vector<T>::const_iterator pos = array.begin(), endpos = array.end(); 
  while (pos != endpos) {
    value += (string(*pos++));
    if (pos == endpos) value += ", "; 
    else value += ">"; 
  }
  return value; 
}

//=====================================================
template <class T> 
string arrayToString(T *array, int length) {
  if (length < 1) return string("[(empty array)]"); 

  string value("[");
  T *ptr = array; 
  while (--length) {
    value += (doubleToString(*ptr++)+", "); 
  }
  value += doubleToString(*ptr) +"]"; 
  return value; 
}
//==========================================================================


#endif
