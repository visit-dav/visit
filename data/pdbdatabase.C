/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <visitstream.h>
#include <math.h>
#include <stdio.h>

//
// Include classes that help write PDB files.
//
#include <pdbhelper.C>

// ****************************************************************************
// Function: MatForRadius
//
// Purpose:
//   Determines a material number from a radius.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 24 11:53:13 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

inline int MatForRadius(double rad)
{
    int retval;

    if(rad > 7.)
        retval = 4;
    else if(rad > 5.)
        retval = 3;
    else if(rad > 1.5)
        retval = 2;
    else
        retval = 1;

    return retval;
}

// ****************************************************************************
// Function: MixVar
//
// Purpose: 
//   Determines the value for a mixvar.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 7 17:11:36 PST 2004
//
// Modifications:
//   
// ****************************************************************************

float MixVar(int matno, int ts, int timeStates, float cX, float cY)
{
    float t = float(ts) / float(timeStates-1);

    float p0x = -5., p0y = 22.;
    float dX = cX - p0x; 
    float dY = cY - p0y; 
    float val1 = float(matno) + float(sqrt(dX * dX + dY * dY));
    
    float p1x = 8., p1y = 5.;
    dX = cX - p1x; 
    dY = cY - p1y; 
    float val2 = float(matno) + float(log10(sqrt(dX * dX + dY * dY) + 1.));

    float ct = 0.5 * cos(t * 3.14159) + 0.5;
    float val = ct * val1 + (1. - ct) * val2;

    return val;
}

// ****************************************************************************
// Function: main
//
// Purpose:
//   This is the main function for the program.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 24 11:53:54 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 23 14:51:33 PST 2004
//   Changed WriteData to match the new interface.
//
//   Brad Whitlock, Fri Jul 23 14:32:30 PST 2004
//   Added a creation comment to the file.
//
//   Brad Whitlock, Thu Sep 9 10:18:31 PDT 2004
//   I changed the code so it works with the modified PDB reader.
//
//   Brad Whitlock, Tue Dec 7 17:17:30 PST 2004
//   I added a float mixed variable.
//
//   Mark C. Miller, Sat Feb  3 00:42:05 PST 2007
//   Added stuff to test test array variables
// ****************************************************************************

int
main(int, char *[])
{
    int timeStates = 23;
    int kmax = 101, lmax = 101;

    FieldWithData<double> rt(timeStates, 1, kmax, lmax, 0., true);
    FieldWithData<double> zt(timeStates, 1, kmax, lmax, 0., true);
    FieldWithData<double> layerTAtIJ(timeStates, 1, kmax, lmax, 0., true);
    FieldWithData<double> marray(timeStates, 13, kmax, lmax, 0., false);
    FieldWithData<int>    order(timeStates, 1, kmax, lmax, 0, true);
    FieldWithData<int>    ireg(timeStates, 1, kmax, lmax, 1, false);

    // Mixed material information.
    FieldWithData<int>    nummm(timeStates, 1, kmax, lmax, 0, false);
    FieldWithData<int>    ilamm(timeStates, 1, kmax*2, lmax, 0, false);

    const int mixSizePerTimeState = int(float(kmax*lmax) / 3.f);
    FieldWithData1D<int>    iregmm(timeStates, mixSizePerTimeState, 0, false);
    FieldWithData1D<double> volfmm(timeStates, mixSizePerTimeState, 0., false);

    // Mixvar
    FieldWithData<float>   mixvar(timeStates, 1, kmax, lmax, 0.f);
    FieldWithData1D<float> mixvarmm(timeStates, mixSizePerTimeState, 0.f);

    //
    // Populate the rt and zt arrays at timestate 0.
    //
    double xmin = -10., xmax = 10.;
    double ymin = 0., ymax = 30.;
    int i, j;
    for(j = 0; j < lmax; ++j)
    {
        double ty = double(j) / double(lmax-1);
        double Y = ty * ymax + (1. - ty) * ymin;
        for(i = 0; i < kmax; ++i)
        {
            double tx = double(i) / double(kmax-1);
            double X = tx * xmax + (1. - tx) * xmin;

            rt.SetValue(0, 0, i, j, Y);
            zt.SetValue(0, 0, i, j, X);
        }
    }

    //
    // Populate the rt and zt arrays at the last timestate.
    //
    int nLayers = (kmax-1) / 2;
    const double twopi = 2. * 3.14159;
    int ic = (kmax-1) / 2;
    int jc = (lmax-1) / 2;
    int ts = timeStates - 1;

    for(int layer = 0; layer <= nLayers; ++layer)
    {
        i = ic + layer;
        j = jc;
        int ii, index = 0;

        int nPointsInLayer = 8 * layer;
        double R = zt.GetValue(0,0,i,j) - zt.GetValue(0,0,ic,j);
        double layerT = double(layer) / double(nLayers-1);

#define SetCircularPoints() \
        double t = double(index) / double(nPointsInLayer); \
        double angle = t * twopi; \
        rt.SetValue(ts, 0, i, j, 10. + R * sin(angle)); \
        zt.SetValue(ts, 0, i, j, R * cos(angle)); \
        layerTAtIJ.SetValue(0, 0, i, j, layerT);

        if(layer == 0)
        {
            rt.SetValue(ts, 0, i, j, 10.);
            zt.SetValue(ts, 0, i, j, 0.);
            continue;
        }

        for(ii = 0; ii < layer; ++ii)
        {
            SetCircularPoints();
            order.SetValue(0, 0, i, j, index);
            ++j; ++index;
        }
        for(ii = 0; ii < 2*layer; ++ii)
        {
            SetCircularPoints();
            order.SetValue(0, 0, i, j, index);
            --i; ++index;
        }
        for(ii = 0; ii < 2*layer; ++ii)
        {
            SetCircularPoints();
            order.SetValue(0, 0, i, j, index);
            --j; ++index;
        }
        for(ii = 0; ii < 2*layer; ++ii)
        {
            SetCircularPoints();
            order.SetValue(0, 0, i, j, index);
            ++i; ++index;
        }
        for(ii = 0; ii < layer; ++ii)
        {
            SetCircularPoints();
            order.SetValue(0, 0, i, j, index);
            ++j; ++index;
        }
    }

    //
    // Interpolate the first and last timestates for rt, zt.
    //
    for(ts = 1; ts < timeStates - 1; ++ts)
    {
        double t = double(ts) / double(timeStates-1);
        rt.LinearInterpolate(t, ts, 0, timeStates-1);
        zt.LinearInterpolate(t, ts, 0, timeStates-1);
    }

    //
    // Populate marray data
    //
    for(ts = 0; ts < timeStates; ++ts)
    {
        for(int c = 0; c < 13; ++c)
        {
            for(j = 0; j < lmax; ++j)
            {
                for(i = 0; i < kmax; ++i)
                {
                    marray.SetValue(ts, c, i, j, (double) (i+j)*c*ts);
                }
            }
        }
    }

    //
    // Copy the order array data to all of its timesteps.
    //
    for(ts = 1; ts < timeStates; ++ts)
    {
        order.CopyData(ts, 0);
        layerTAtIJ.CopyData(ts, 0);
    }

    //
    // Figure out the materials.
    //
    for(ts = 0; ts < timeStates; ++ts)
    {
        int array_index = 1;
        double tst = double(ts) / double(timeStates - 1);
        double timeShift = tst * 4. * twopi;
        double cX = zt.GetValue(ts, 0, ic, jc);
        double cY = rt.GetValue(ts, 0, ic, jc);

        for(j = 1; j < lmax; ++j)
        {
            for(i = 1; i < kmax; ++i)
            {
                double X[4], Y[4];

#define GetValues(DEST, FIELD) \
                DEST[0] = FIELD.GetValue(ts, 0, i, j);\
                DEST[1] = FIELD.GetValue(ts, 0, i-1, j);\
                DEST[2] = FIELD.GetValue(ts, 0, i-1, j-1);\
                DEST[3] = FIELD.GetValue(ts, 0, i, j-1);

                GetValues(X, zt);
                GetValues(Y, rt);

                float cellCenterX = float(X[0] + X[1] + X[2] + X[3]) / 4.f;
                float cellCenterY = float(Y[0] + Y[1] + Y[2] + Y[3]) / 4.f;

#define MAXMATS 10
                const int xSamples = 10;
                const int ySamples = 10;
                int matsInZone[MAXMATS];
                for(int n = 0; n < MAXMATS; ++n)
                    matsInZone[n] = 0;
                for(int ys = 0; ys < ySamples; ++ys)
                {
                    double yt = double(ys) / double(ySamples - 1);
                    double one_minus_yt = 1. - yt;
                    for(int xs = 0; xs < xSamples; ++xs)
                    {
                        double xt = double(xs) / double(xSamples - 1);
                        double one_minus_xt = 1. - xt;

                        double TempLeft, TempRight;
                        double pX, pY, layerShift;

#define BiLinearInterpolate(DEST, VALS) \
                        TempLeft  = one_minus_yt * VALS[2] + yt * VALS[1]; \
                        TempRight = one_minus_yt * VALS[3] + yt * VALS[0]; \
                        DEST = one_minus_xt * TempLeft + xt * TempRight;

                        BiLinearInterpolate(pX, X);
                        BiLinearInterpolate(pY, Y);
                        layerShift = layerTAtIJ.GetValue(ts,0,i,j);
                        layerShift *= (twopi * 0.125);
                        double dX = pX - cX;
                        double dY = pY - cY;
                        double R = sqrt(dX*dX + dY*dY);
                        double angle;

                        if(dX == 0)
                        {
                            if(dY > 0.)
                                angle = 3.14159 / 2.;
                            else
                                angle = 3. * 3.14159 / 2.;
                        }
                        else
                            angle = atan(dY / dX);

                        double A = 8. * (angle - layerShift) - timeShift;
                        double rad = 0.7*R + 0.3*R*sin(A);
                        ++matsInZone[MatForRadius(rad)];
                    }
                }

                // Store the number of materials in each zone.
                int nmats = 0; int maxContribution = 0; int matno=0;
                for(int n = 0; n < MAXMATS; ++n)
                {
                    nmats += ((matsInZone[n] > 0) ? 1 : 0);
                    if(maxContribution < matsInZone[n])
                    {
                        maxContribution = matsInZone[n];
                        matno = n;
                    }
                }

                if(nmats > 1)
                {
                    // Save the indices into the iregmm and volfmm arrays.
                    ilamm.SetValue(ts, 0, 2*i, j, array_index);
                    ilamm.SetValue(ts, 0, 2*i+1, j, 0);
                    nummm.SetValue(ts, 0, i, j, nmats);

                    const double nTotalSamples = double(ySamples * xSamples);
                    for(int n = 0; n < MAXMATS; ++n)
                    {
                        if(matsInZone[n] > 0)
                        {
                            volfmm.SetValue(ts, array_index-1,
                                double(matsInZone[n]) / nTotalSamples);
                            iregmm.SetValue(ts, array_index-1, n);
                            mixvarmm.SetValue(ts, array_index-1,
                                MixVar(n,ts,timeStates, cellCenterX,
                                cellCenterY));
                            ++array_index;
                        }
                    }
                }
                else
                {
                    // Save the indices into the iregmm and volfmm arrays.
                    ilamm.SetValue(ts, 0, 2*i, j, 0);
                    ilamm.SetValue(ts, 0, 2*i+1, j, 0);
                    nummm.SetValue(ts, 0, i, j, 0);
                }

                ireg.SetValue(ts, 0, i, j, matno);
                mixvar.SetValue(ts, 0, i, j, MixVar(matno,ts,timeStates,
                    cellCenterX, cellCenterY));
            }
        }
    }

    //
    // Create the PDB file and set up the fields that we want to write to it.
    //
    FieldWriter pdb;

    const int startCycle = 0;
    const int endCycle = 1000;
    const double startTime = 1.02e-02;
    const double endTime = 11.02e-02;

    for(ts = 0; ts < timeStates; ++ts)
    {
         double t = double(ts) / double(timeStates - 1);
         double omt = 1. - t;

         // Figure out the cycle and the time.
         int   cycle = int(omt * startCycle) + int(t * endCycle);
         double time = double(omt * startTime) + double(t * endTime);

         // Add a new time state.
         pdb.AddTimeStep(cycle, time);
    }

    //
    // Add fields to the pdb file.
    //
    pdb.AddField("rt", "rt@value", &rt);
    pdb.AddField("zt", "zt@value", &zt);
    pdb.AddField("marray", "marray@value", &marray);
    pdb.AddField("ireg", "ireg@value", &ireg);
    pdb.AddField("a", "a@value", &order);
    pdb.AddField("lt", "lt@value", &layerTAtIJ);
    pdb.AddField("mixvar", "mixvar@value", &mixvar);

    pdb.AddField("nummm", "nummm@las", &nummm);
    pdb.AddField("ilamm", "ilamm@las", &ilamm);
    pdb.AddField("iregmm", "iregmm@las", &iregmm);
    pdb.AddField("volfmm", "volfmm@las", &volfmm);
    pdb.AddField("mixvarmm", "mixvarmm@value", &mixvarmm);

    //
    // Write the data to a set of PDB files that each have multiple times.
    //
    pdb.SetDatabaseComment("Flash database: multiple time states per file.");
    pdb.WriteData("multi", kmax, lmax, 10, true);

    //
    // Write the data to a set of individual PDB files.
    //
    pdb.SetDatabaseComment("Flash database: 1 time state per file.");
    pdb.WriteData("family", kmax, lmax, 1, true);

    //
    // Write the data to a single PDB file.
    //
    pdb.SetDatabaseComment("Flash database: all time states in one file.");
    pdb.WriteData("allinone", kmax, lmax, timeStates, true);

    return 0;
}
