#include "RangeList.h"
#include <iostream> 
#include <fstream>
#include <algorithm>
#define RANGE_LIST_SOURCE_ID "$Id: RangeList.C,v 1.5 2005/03/07 18:58:08 rcook Exp $"
void PrintVersion(void) {
  cout << "Range List header is " << RANGE_LIST_HEADER_ID << " and source is " << RANGE_LIST_SOURCE_ID << endl; 
  return; 
}

using namespace std; 
// for templated searches:
bool operator < (long num, const range &r)   { return r > num;}
bool operator > (long num, const range &r)   { return r < num;}
bool operator ==(long num, const range &r)   { return r.Contains(num);}
bool operator ==(const range &r, long num)   { return r.Contains(num);}

void RangeList::Print(void) {
  cout << "printing range list: " << endl; 
  vector<range>::iterator pos = ranges.begin(), end = ranges.end();
  while (pos != end){
    pos->Print();
    ++pos;
  }
  cout << "end of range list." << endl; 
  return; 
}


void RangeList::AddRange(long start, long end){  
  if (end < start)
    throw string("AddRange: start is greater than end!");

  ranges.push_back(range(start, end)); 
  if (verbose)
    cout << "RangeList:AddRange(" << start << ", " << end <<")" << endl; 
  return; 
}

void RangeList::GetOneRange(ifstream &filterfile) {
  long start, end; 
  char token; 
  while (filterfile >> start) {
    //if we immediately see a number, then it is just a number; treat it like a range
    AddRange(start, start); 
    return; 
  }

  // did not see a number 
  filterfile.clear();
  filterfile >> token; 
  if (token == ',') { 
    // a comma is optional; just ignore it and move on
    return; 
  }

  // if we don't see a number or a comma, then it's a range, starting with '['
  if (filterfile.eof())
    return; 

  if (token != '[' ) 
      throw string("Expected  '[' in range but got \"")+token+string("\"");
    
  if (!(filterfile >> start))
    throw string("Did not find a number after opening bracket in range.");

  filterfile >> token; 
  if (token != '-') 
    throw string("Expected dash; found \"")+token+"\" in range after number";

  if (!(filterfile >> end))
    throw string("Did not find a number after dash in range.");
  
  filterfile >> token; 
  if (token != ']' ) 
      throw string("Expected ']' in range but got \"")+token+string("\"");

  AddRange(start, end);
  return; 
}

void RangeList::Parse(char *filename) {

  ifstream filterfile(filename);
  if (!filterfile)
    throw string("Could not open filter file ")+string(filename); 

  while (!filterfile.eof() && !filterfile.bad()) 
    GetOneRange(filterfile);

  return; 
}
