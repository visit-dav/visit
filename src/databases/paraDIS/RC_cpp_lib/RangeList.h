#ifndef RANGE_LIST_H
#define RANGE_LIST_H

#define RANGE_LIST_HEADER_ID "$Id: RangeList.h,v 1.4 2005/03/07 18:58:08 rcook Exp $"

#include <vector>
#include <string>
#include <iostream> 
#include <algorithm>
using namespace std; 

void PrintVersion(void); 

struct range {
  range(long range_start, long range_end) { start = range_start; end = range_end; }
  long start, end; 
  void Print(void) { 
    cout << "range: (" << start << ", " << end << ")" << endl; 
  }
 
  bool operator ==(range &r) const  { return r.start == start && r.end == end;}
  bool Contains(long num) const  { return num >= start && num <= end;}
  bool operator < (const range &r) const { return start < r.start; }
  bool operator < (long num) const { return num > end; }
  bool operator > (long num) const { return num < start; }
  
};

// for templated searches:
bool operator < (long num, const range &r) ;
bool operator > (long num, const range &r)  ;
bool operator ==(long num, const range &r);
bool operator ==(const range &r, long num)  ;


struct RangeList { 
  RangeList(char *filterfile, char verbosity=0) {Init(verbose); Parse(filterfile);} 
  RangeList(){Init(0);} // ok to not give a file to start with
  void Init(int verbose=0) { verbose = verbose; ranges.clear(); }
  void Parse(char *filterfile) ;//open and parse the filter file; add ranges therein to ranges, throw exception if problem
  void AddRange(long start, long end);
  bool InRange(long num) { return binary_search(ranges.begin(), ranges.end(), num); }
  void Print(void); 
  void Sort(void){ sort(ranges.begin(), ranges.end());}
  void Unique(void) {unique(ranges.begin(), ranges.end());}
  void GetOneRange(ifstream &filterfile); //throw exception if parse error
  char verbose; 
  vector<range> ranges;
};
    
#endif 


