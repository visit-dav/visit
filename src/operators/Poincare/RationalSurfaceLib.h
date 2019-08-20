// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            RationalSurfaceLib.h                           //
// ************************************************************************* //

#ifndef RATIONAL_SURFACE_LIB_H
#define RATIONAL_SURFACE_LIB_H



#include "FieldlineAnalyzerLib.h"

#include <math.h>
#include <map>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <avtVector.h>
#include <avtPoincareIC.h>

#include <vector>
#include <iomanip>

// Notes
// The rational surfaces parameter in the GUI is currently nonfunctional. Just using largest angle.


// Rational Search Settings
//--------------------------

// To enable logging
#define RATIONAL_DEBUG                 2

// if 1, just draw the first seeds (approximation to rational surface based on circle)
#define NO_MINIMIZATION                0

// hard set seed z offset from the plane (helps with consistent point finding)
#define Z_OFFSET                       -0.000001;

// The distance used to calculate how many new seeds will be placed
#define MAX_SEED_SPACING               0.001

// Sometimes the rational search gets caught in a loop. This is a hard limit on that.
#define MAX_ITERATIONS                35

// How close the minimizing points need to be before it's considered minimized
#define MAX_SPACING                    0.00041

// Should be the width of the initial bracket
#define BRACKET_WIDTH                  0.00075



// This is a constant used in the minimization algorithm. The golden ratio is essentially used to guess
// a new point if a minimum has not been bracketed
const double golden_R = 0.61803398875;
const double golden_C = 1.0 - golden_R;
const double GOLD = 1.61803398875;




bool NeedToMinimize(avtPoincareIC *poincare_ic);

bool PrepareToBracket(avtPoincareIC *seed,avtVector &newA,avtVector &newB,avtVector &newC);
bool UpdateBracket(avtPoincareIC *poincare_ic, bool &swapped, avtVector &C);
bool BracketIsValid(avtPoincareIC *poincare_ic);
bool MinimumIsBracketed(avtPoincareIC *poincare_ic);
bool SetupNewBracketA(avtPoincareIC *bracketA, avtPoincareIC *parent_seed, avtVector aPt);
bool SetupNewBracketB(avtPoincareIC *bracketB, avtPoincareIC *parent_seed, avtVector bPt);
bool SetupNewBracketC(avtPoincareIC *bracketC, avtPoincareIC *parent_seed, avtVector cPt);

bool SetupRational(avtPoincareIC *rational);
bool SetupNewSeed(avtPoincareIC *seed,
                  avtPoincareIC *rational,
                  avtVector seedPt,
                   avtVector point1,
                   avtVector point2);
bool PrepareToMinimize(avtPoincareIC *poincare_ic, avtVector &newPt, bool &cbGTba);


// HELPER METHODS //
double PythDist( avtVector p1, avtVector p2 );
double GetAngle( avtVector a, avtVector b, avtVector c );
std::vector<avtVector> GetSeeds( avtPoincareIC *poincare_ic,
                                 avtVector &point1,
                                 avtVector &point2,
                                 double maxDistance = MAX_SEED_SPACING
                                 ); 
double MaxRationalDistance( avtPoincareIC *ic,
                        unsigned int toroidalWinding,
                        unsigned int &index );
double MinRationalDistance( avtPoincareIC *ic,
                        unsigned int toroidalWinding,
                        unsigned int &index );
int FindMinimizationIndex( avtPoincareIC* ic);
double FindMinimizationDistance( avtPoincareIC* ic);

void PickBestAndSetupToDraw(avtPoincareIC *x0, avtPoincareIC *x1, avtPoincareIC *x2, avtPoincareIC *x3, std::vector<int> &ids_to_delete); 
void SwapWithSeed(avtPoincareIC* swap, avtPoincareIC *poincare_ic);
std::string VectorToString(avtVector &vec);
#endif
