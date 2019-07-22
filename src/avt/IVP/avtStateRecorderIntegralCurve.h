// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStateRecorderIntegralCurve.h                     //
// ************************************************************************* //

#ifndef AVT_STATE_RECORDER_INTEGRAL_CURVE_H
#define AVT_STATE_RECORDER_INTEGRAL_CURVE_H

#include <avtIntegralCurve.h>

// ****************************************************************************
//  Class: avtStateRecorderIntegralCurve
//
//  Purpose:
//      A derived type of avtIntegralCurve.  This class records the state for
//      every step of the integral curve.  As the steps may occur on separate
//      processors, it also contains code to collect the steps and reconstitute 
//      them on the originating processor.
//
//  Programmer: Hank Childs (refactoring of work from others)
//  Creation:   June 4, 2010
//
//  Modifications:
//
//    Hank Childs, Tue Jun  8 09:30:45 CDT 2010
//    Place sequence tracking code from base class into this class.
//
//    Christoph Garth, Tue July 10 17:34:33 PDT 2010
//    Major rewrite around removing avtIVPStep storage.
//
//    Hank Childs, Fri Oct  8 23:30:27 PDT 2010
//    Refactor into an abstract type.  Remove all data members specific to
//    integral curves or Poincare.
//
//    Dave Pugmire, Fri Nov  5 15:36:31 EDT 2010
//    Make historyMask public.
//
//    Hank Childs, Sun Dec  5 10:18:13 PST 2010
//    Add an avtIVPField as an argument to CheckForTermination.
//
//    Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//    Add scalar2
//
//    Hank Childs, Tue Dec  6 19:01:30 PST 2011
//    Add methods LessThan and PrepareForFinalCommunication.
//
//   David Camp, Wed Mar  7 10:43:07 PST 2012
//   Added a Serialize flag to the arguments. This is to support the restore
//   ICs code.
//
// ****************************************************************************

class IVP_API avtStateRecorderIntegralCurve : public avtIntegralCurve
{
public:

    // Caution: If you modify these flags, also check 
    // historyMask and the *Sample* member functions.
    enum Attribute
    {
        SAMPLE_TIME       = 0x0001,
        SAMPLE_POSITION   = 0x0002,
        SAMPLE_VELOCITY   = 0x0004,
        SAMPLE_VORTICITY  = 0x0008,
        SAMPLE_ARCLENGTH  = 0x0010,
        SAMPLE_DOM_VISIT  = 0x0020,
        SAMPLE_VARIABLE   = 0x0040,
        SAMPLE_SECONDARY0 = 0x0080,
        SAMPLE_SECONDARY1 = 0x0100,
        SAMPLE_SECONDARY2 = 0x0200,
        SAMPLE_SECONDARY3 = 0x0400,
        SAMPLE_SECONDARY4 = 0x0800,
        SAMPLE_SECONDARY5 = 0x1000
    };

    struct Sample
    {
        double    time;
        avtVector position;
        avtVector velocity;
        double    vorticity;
        double    arclength;
        double    numDomainsVisited;
        double    variable;
        double    secondarys[6];
    };

    // ----

    avtStateRecorderIntegralCurve( unsigned int mask,
                                   const avtIVPSolver* model, 
                                   Direction dir,
                                   const double& t_start, 
                                   const avtVector &p_start,
                                   const avtVector &v_start,
                                   int ID );

    avtStateRecorderIntegralCurve();
    virtual ~avtStateRecorderIntegralCurve();

  protected:
    avtStateRecorderIntegralCurve( const avtStateRecorderIntegralCurve& );
    avtStateRecorderIntegralCurve& operator=( const avtStateRecorderIntegralCurve& );

 public:
    virtual void  Finalize();
    virtual void  Serialize(MemStream::Mode mode, MemStream &buff, 
                            avtIVPSolver *solver, SerializeFlags serializeFlags);
    virtual void  PrepareForSend(void)
                           { _serializeFlags = (SerializeFlags)(_serializeFlags | avtIntegralCurve::SERIALIZE_INC_SEQ); };
    virtual void      ResetAfterSend(void) { _serializeFlags = SERIALIZE_NO_OPT;}
    virtual bool  SameCurve(avtIntegralCurve *ic);

    virtual avtIntegralCurve* MergeIntegralCurveSequence(
                              std::vector<avtIntegralCurve *> &v);

    virtual void MergeIntegralCurve(avtIntegralCurve *) = 0;

    static bool IdSeqCompare(const avtIntegralCurve *slA,
                             const avtIntegralCurve *slB);
    static bool IdRevSeqCompare(const avtIntegralCurve *slA,
                                const avtIntegralCurve *slB);
    virtual bool LessThan(const avtIntegralCurve *ic) const;
    virtual void PrepareForFinalCommunication(void)
                     { _serializeFlags = avtIntegralCurve::SERIALIZE_STEPS; };

    size_t  GetNumberOfSamples() const;
    Sample  GetSample( size_t n ) const;
    
    virtual bool CheckForTermination(avtIVPStep &step, avtIVPField *) = 0;

    virtual void SetHistoryMask (unsigned int mask) { historyMask = mask; };

    virtual double GetTime() { return time; };
    virtual double GetDistance() { return distance; };
    virtual avtVector GetEndPoint() { return ivp->GetCurrentY(); };

  protected:
    virtual void AnalyzeStep( avtIVPStep& step,
                              avtIVPField* field,
                              bool firstStep=false);

    void RecordStep( const avtIVPField* field, 
                     const avtIVPStep& step, 
                     bool firstStep);

    size_t    GetSampleStride() const;
    size_t    GetSampleIndex(const Attribute &attr) const;

  protected:
    SerializeFlags       _serializeFlags;
    long                 sequenceCnt;

    double               time;
    double               distance;

    unsigned int         variableIndex;

    std::vector<double>  history;
    unsigned int         historyMask;
};

#endif 
