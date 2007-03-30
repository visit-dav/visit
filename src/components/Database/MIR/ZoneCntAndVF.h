#ifndef ZONE_CNT_AND_VF_H
#define ZONE_CNT_AND_VF_H

// ****************************************************************************
//  Class:  ZoneCntAndVF
//
//  Purpose:
//    Store a volume fraction for a each mat for a face/node/edge/etc., and
//    keep track of how many zones share this face/node/edge/etc.
//
//  Note:   
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
struct ZoneCntAndVF
{
    int           zonecnt;
    vector<float> vf;
    ZoneCntAndVF();
    ZoneCntAndVF(int c, int n);
    ~ZoneCntAndVF();
    void operator=(const ZoneCntAndVF &rhs);
};


// ----------------------------------------------------------------------------
//                             Inline Methods
// ----------------------------------------------------------------------------


// ****************************************************************************
//  Default Constructor:  ZoneCntAndVF::ZoneCntAndVF
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
ZoneCntAndVF::ZoneCntAndVF()
{
    zonecnt = 0;
}

// ****************************************************************************
//  Constructor:  ZoneCntAndVF::ZoneCntAndVF
//
//  Arguments:
//    c    the number of zones attached to this object
//    n    the number of materials in the problem
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
ZoneCntAndVF::ZoneCntAndVF(int c, int n)
{
    zonecnt = c;
    vf.resize(n, 0.0);
}

// ****************************************************************************
//  Destructor:  ZoneCntAndVF::~ZoneCntAndVF
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline
ZoneCntAndVF::~ZoneCntAndVF()
{
    vf.clear();
}

// ****************************************************************************
//  Method:  ZoneCntAndVF::operator=
//
//  Purpose:
//    assignment operator
//
//  Arguments:
//    rhs        the source ZoneCntAndVF
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 12, 2000
//
// ****************************************************************************
inline void
ZoneCntAndVF::operator=(const ZoneCntAndVF &rhs)
{
    zonecnt = rhs.zonecnt;
    vf      = rhs.vf;
}


#endif
