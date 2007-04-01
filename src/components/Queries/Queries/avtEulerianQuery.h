// ************************************************************************* //
//                            avtEulerianQuery.h                             //
// ************************************************************************* //

#ifndef AVT_EULERIAN_QUERY_H
#define AVT_EULERIAN_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <string>
#include <map>


class vtkDataSet;
class vtkGeometryFilter;



// ****************************************************************************
//  Class: avtEulerianQuery
//
//  Purpose:
//    A query that calculates the Eulerian number for the mesh.
//
//  Programmer: Akira Haddox
//  Creation:   June 28, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 12:37:11 PST 2002  
//    Moved from avtEulerianFilter, made to fit query model. 
//
//    Brad Whitlock, Fri Nov 22 11:00:01 PDT 2002
//    Introduced a typedef to make the Windows compiler happier. Added API.
//
//    Kathleen Bonnell, Fri Sep  3 10:10:28 PDT 2004 
//    Added VerifyInput. 
//
// ****************************************************************************

class QUERY_API avtEulerianQuery : public avtDatasetQuery
{
  public:
                              avtEulerianQuery();
    virtual                  ~avtEulerianQuery();


    virtual const char       *GetType(void)   { return "avtEulerianQuery"; };
    virtual const char       *GetDescription(void)
                                 { return "Calculating Eulerian of mesh."; };

  protected:

    // Structure to represent an edge
    // Stores two node ids: order is not important.
    // However, we store them sorted, so that edge(0,1) == edge(1,0)
    // Note that < is a strict weak ordering to work with set, such that
    // Equiv(a,b) is defined as !(a<b || b<a)
    struct edgepair
    {
        int myGreater, myLesser;
        edgepair(int _a, int _b)
        {
            if (_a < _b)
            {
                myLesser = _a;
                myGreater = _b;
            }
            else
            {
                 myLesser = _b;
                 myGreater = _a;
            }
         }
 
        bool
        operator < (const edgepair &e) const
        {
             return myGreater == e.myGreater ?
                    myLesser < e.myLesser : myGreater < e.myGreater;
        }
    };

    typedef std::map<int, int> DomainToEulerMap;

    vtkGeometryFilter              *gFilter;
    DomainToEulerMap                domToEulerMap;

    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    VerifyInput(void);
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
};


#endif
