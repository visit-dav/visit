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

#ifndef _visit_idx_io_types_h
#define _visit_idx_io_types_h

#include <cmath>
#include <sstream>
#include <cassert>
#include <algorithm>

namespace VisitIDXIO{
enum DTypes{
    IDX_INT8, IDX_UINT8,
    IDX_INT16, IDX_UINT16,
    IDX_INT32, IDX_UINT32,
    IDX_INT64, IDX_UINT64,
    IDX_FLOAT32, IDX_FLOAT64,
    UNKNOWN
};
    
struct Field{
    std::string name;
    DTypes type;
    bool isVector;
    int ncomponents;
};

class Point3d{
public:
    double x,y,z;
    
    //default constructor
    inline Point3d() :x(0.0),y(0.0),z(0.0)
    {}
    
    //constructor
    inline explicit Point3d(double x_,double y_,double z_=0) : x(x_), y(y_), z(z_)
    {}
    
    //constructor from string
    inline explicit Point3d(std::string value)
    {std::stringstream parser(value);parser>>x>>y>>z;}
    
    //convert to std::string
    inline std::string toString() const
    {
        std::stringstream out;
        out<<this->x<<" "<<this->y<<" "<<this->z;
        return out.str();
    }
    
    //module*module
    inline double module2() const
    {return x*x+y*y+z*z;}
    
    //module
    inline double module() const
    {return (double)sqrt(module2());}
    
    //normalize a vector
    inline Point3d normalize() const
    {
        double len=module();
        if (!len) len=1.0;
        return Point3d(x/len,y/len,z/len);
    }
    
    //abs
    inline Point3d abs() const
    {return Point3d(x>=0?+x:-x , y>=0?+y:-y , z>=0?+z:-z);}
    
    //inverse
    inline Point3d inv() const
    {return Point3d(1.0/x,1.0/y,1.0/z);}
    
    //+a
    inline const Point3d& operator+()  const
    {return *this;}
    
    //-a
    inline Point3d operator-()  const
    {return Point3d(-this->x,-this->y,-this->z);}
    
    //a+b
    inline Point3d operator+(const Point3d&  b)  const
    {return Point3d(this->x+b.x,this->y+b.y,this->z+b.z);}
    
    //a+=b
    inline Point3d& operator+=(const Point3d&  b)
    {this->x+=b.x;this->y+=b.y;this->z+=b.z;return *this;}
    
    //a-b
    inline Point3d operator-(const Point3d&  b)  const
    {return Point3d(this->x-b.x,this->y-b.y,this->z-b.z);}
    
    //a-=b
    inline Point3d& operator-=(const Point3d&  b)
    {this->x-=b.x;this->y-=b.y;this->z-=b.z;return *this;}
    
    //a*f
    inline Point3d operator*(double s) const
    {return Point3d(this->x*s,this->y*s,this->z*s);}
    
    //a*=f
    inline Point3d& operator*=(double s)
    {this->x=this->x*s;this->y=this->y*s;this->z=this->z*s;return *this;}
    
    //a/f
    inline Point3d operator/(double s) const
    {return Point3d(this->x/s,this->y/s,this->z/s);}
    
    //a/f
    inline Point3d& operator/=(double s)
    {this->x=this->x/s;this->y=this->y/s;this->z=this->z/s;return *this;}
    
    //a==b
    inline bool operator==(const Point3d& b) const
    {return  x==b.x && y==b.y && z==b.z;}
    
    //a!=b
    inline bool operator!=(const Point3d& b) const
    {return  x!=b.x || y!=b.y || z!=b.z;}
    
    //dot product
    inline double dot(const Point3d&  b) const
    {return this->x*b.x+this->y*b.y+this->z*b.z;}
    
    //dot product
    inline double operator*(const Point3d& b) const
    {return this->dot(b);}
    
    //distance between two points
    inline double distance(const Point3d& p) const
    {return (p-*this).module();}
    
    //access an item using an index
    inline double& operator[](int i)
    {
        assert(i>=0 && i<3);
        if (i==0) return x;
        else if (i==1) return y;
        else           return z;
    }
    
    //access an item using an index
    inline const double& operator[](int i) const
    {
        assert(i>=0 && i<3);
        if (i==0) return x;
        else if (i==1) return y;
        else           return z;
    }
    
    //set
    inline Point3d& set(int index,double value)
    {(*this)[index]=value;return *this;}
    
    //cross product
    inline Point3d cross(const Point3d& v) const
    {
        return Point3d
        (
         y * v.z - v.y * z,
         z * v.x - v.z * x,
         x * v.y - v.x * y
         );
    }
    
    // return index of smallest/largest value
    inline int biggest () const {return (x >  y) ? (x > z  ? 0 : 2) : (y > z  ? 1 : 2);}
    inline int smallest() const {return (x <= y) ? (x <= z ? 0 : 2) : (y <= z ? 1 : 2);}
    
};

// TODO add other useful operators
inline std::ostream& operator<<(std::ostream &out,const Point3d &p)
{out <<"<"<<p.x <<","<<p.y<<","<<p.z<<">";return out;}

class Box{
public:
    
    Point3d p1;
    Point3d p2;
    
    inline Box(){};
    
    //construct from string
    inline Box(std::string value)
    {
        std::istringstream parser(value);
        parser>>this->p1.x>>this->p1.y>>this->p1.z;
        parser>>this->p2.x>>this->p2.y>>this->p2.z;
    }
    
    //constructor
    inline Box(Point3d p1,Point3d p2)
    {
        this->p1=Point3d(std::min(p1.x,p2.x),std::min(p1.y,p2.y),std::min(p1.z,p2.z));
        this->p2=Point3d(std::max(p1.x,p2.x),std::max(p1.y,p2.y),std::max(p1.z,p2.z));
    }
    
    //size
    inline Point3d size() const
    {return p2-p1;}
    
    //addPoint
    inline void addPoint(double x,double y,double z)
    {
        this->p1.x = std::min(x,p1.x); this->p2.x = std::max(x,p2.x);
        this->p1.y = std::min(y,p1.y); this->p2.y = std::max(y,p2.y);
        this->p1.z = std::min(z,p1.z); this->p2.z = std::max(z,p2.z);
    }
    
    //add a point to the bounding box
    inline void addPoint(Point3d p)
    {addPoint(p.x,p.y,p.z);}
    
    //get point
    inline Point3d getPoint(int idx) const
    {
        switch (idx)
        {
            case 0:return Point3d(p1.x,p1.y,p1.z);
            case 1:return Point3d(p2.x,p1.y,p1.z);
            case 2:return Point3d(p2.x,p2.y,p1.z);
            case 3:return Point3d(p1.x,p2.y,p1.z);
            case 4:return Point3d(p1.x,p1.y,p2.z);
            case 5:return Point3d(p2.x,p1.y,p2.z);
            case 6:return Point3d(p2.x,p2.y,p2.z);
            case 7:return Point3d(p1.x,p2.y,p2.z);
        }
        
        return Point3d();
    }
    
    //test if a point is inside the box
    inline bool containsPoint(Point3d p) const
    {
        return p.x>=this->p1.x && p.x<=this->p2.x
        && p.y>=this->p1.y && p.y<=this->p2.y
        && p.z>=this->p1.z && p.z<=this->p2.z;
    }
    
    //test if two box are equal
    inline bool operator==(const Box& b) const
    {return p1==b.p1 && p2==b.p2;}
    
    //test equality
    inline bool operator!=(const Box& b) const
    {return !(this->operator==(b));}
    
    //get intersection of two boxes
    inline Box getIntersection(const Box& b) const
    {
        Box ret;
        ret.p1.x=std::max(this->p1.x,b.p1.x); ret.p2.x=std::min(this->p2.x,b.p2.x);
        ret.p1.y=std::max(this->p1.y,b.p1.y); ret.p2.y=std::min(this->p2.y,b.p2.y);
        ret.p1.z=std::max(this->p1.z,b.p1.z); ret.p2.z=std::min(this->p2.z,b.p2.z);
        return ret;
    }
    
    //get union of two boxes
    inline Box getUnion(const Box& b) const
    {
        Box ret;
        ret.p1.x=std::min(this->p1.x,b.p1.x);ret.p2.x=std::max(this->p2.x,b.p2.x);
        ret.p1.y=std::min(this->p1.y,b.p1.y);ret.p2.y=std::max(this->p2.y,b.p2.y);
        ret.p1.z=std::min(this->p1.z,b.p1.z);ret.p2.z=std::max(this->p2.z,b.p2.z);
        return ret;
    }
    
    //construct to string
    inline std::string toString()
    {return p1.toString() + " " + p2.toString();}
    
};



}

#endif
