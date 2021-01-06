/***************************************************
 ** ViSUS Visualization Project                    **
 ** Copyright (c) 2010 University of Utah          **
 ** Scientific Computing and Imaging Institute     **
 ** 72 S Central Campus Drive, Room 3750           **
 ** Salt Lake City, UT 84112                       **
 **                                                **
 ** For information about this project see:        **
 ** http://www.pascucci.org/visus/                 **
 **                                                **
 **      or contact: pascucci@sci.utah.edu         **
 **                                                **
 ****************************************************/

#ifndef _visit_idx_utils_h
#define _visit_idx_utils_h

#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include <functional>
#include <cctype>

static inline int        cint   (std::string s) {int    value;std::istringstream iss(s);iss>>value;return value;}
static inline float      cfloat (std::string s) {float  value;std::istringstream iss(s);iss>>value;return value;}
static inline double     cdouble(std::string s) {double value;std::istringstream iss(s);iss>>value;return value;}
static inline int        cround (double x) { x = x + 0.5 - (x<0); return (int)x; }

// trim from start (in place)
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    return s;
}

// trim from end (in place)
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}

inline int 
int16_Reverse_Endian(short val, unsigned char *output)
{
  unsigned char *input  = ((unsigned char *)&val);
  
  output[0] = input[1];
  output[1] = input[0];
  
  return 2;
}

inline int
int32_Reverse_Endian(int val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;
    
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;
    
    return 4;
}

inline int
float32_Reverse_Endian(float val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 3;
    unsigned char *out = outbuf;
    
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;
    
    return 4;
}

inline int
double64_Reverse_Endian(double val, unsigned char *outbuf)
{
    unsigned char *data = ((unsigned char *)&val) + 7;
    unsigned char *out = outbuf;
    
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out++ = *data--;
    *out = *data;
    
    return 8;
}

template <typename Type>
inline Type* convertTo3Components(const unsigned char* src, int src_ncomponents, int dst_ncomponents, long long totsamples){
    int n=src_ncomponents;
    int m=dst_ncomponents;
    int ncomponents=std::min(m,n);
    
    Type* dst = (Type*)calloc(totsamples*m, sizeof(Type));
    
    //for each component...
    for (int C=0;C<ncomponents;C++)
    {
        Type* src_p=((Type*)src)+C;
        Type* dst_p=((Type*)dst)+C;
        for (long long I=0; I<totsamples; I++,src_p+=n,dst_p+=m)
        {
            *dst_p=*src_p;
            
            //std::cout << "c " << C << " I " <<I << std::endl;
        }
        
    }
    //std::cout << "data converted " << std::endl;
    
    return dst;
}


#endif
