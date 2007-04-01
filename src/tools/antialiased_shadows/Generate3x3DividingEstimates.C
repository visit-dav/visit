#include <visitstream.h>
#include <math.h>

//    Jeremy Meredith
//    October 29, 2004
//
//    This program estimates the angle and offset of a dividing line
//    (i.e. the coefficients A, B, and C in the equation Ax+By+C=0)
//    that would generate a 3x3 grid where the bits corresponding to
//    the index in 0...2^9 (i.e. index in 0...511) are on one side of
//    the line if set, and on the other side of the line if not set.
//
//    Specifically, it incorporates samples over over 360 degrees in
//    1-degree increments and 60 different offsets from -1.5 to +1.5,
//    averaging any that land on the same case.
//
//    For example, suppose we have the 3x3 grid:
//
//    XX.
//    X..
//    ... 
//
//    and we want to know what the equation of the line that would
//    split the 'X's and '.'s based on the center cell center lying at
//    0,0 and the other cells centers lying at +/- 1.0.  It assumes the
//    X/. categories are taken with samples at the cell center.  If you
//    convert the above grid into (1*2^0 + 1*2^1 + 1*2^3) and get the
//    resulting index 11, look it up in this output and see that it gives
//    you A=.707,B=.707,C=-.38 -- in other words, an angle of 45 degrees
//    and offset 0.38 units from the center of the center cell.
//
int main()
{
    int degrees[512];
    float offset[512];
    int count[512];

    for (int ctr = 0; ctr<512; ctr++)
    {
        count[ctr]=0;
        degrees[ctr]=0;
        offset[ctr]=0;
    }

    for (int deg = 0; deg < 360; deg+=1)
    {
        float rad = float(deg)*3.1415926536/180.;
        float _gx = cos(rad);
        float _gy = sin(rad);
        for (int ic = -30; ic< 30; ic+=1)
        {
            float intercept = 3.*(float(ic)+.5)/20.;
            int ctr = 0;
            for (int a=-1; a<=1; a++)
            {
                for (int b=-1; b<=1; b++)
                {
                    if (float(b)*_gx + float(a)*_gy + intercept > 0)
                    {
                        ctr++;
                    }
                    ctr *= 2;
                }
            }
            ctr /= 2;
            if (ctr == 0 || ctr == 511)
                continue;

            if (count[ctr] > 0)
            {
                if (float(degrees[ctr])/float(count[ctr]) - float(deg) >= 180)
                {
                    degrees[ctr] += (deg+180);
                }
                else if (float(degrees[ctr])/float(count[ctr]) - float(deg) <= -180)
                {
                    degrees[ctr] += (deg-180);
                }
                else
                {
                    degrees[ctr] += deg;
                }
            }
            else
            {
                degrees[ctr] += deg;
            }
            count[ctr]++;
            offset[ctr] += intercept;
        }
    }

    float gx[512];
    float gy[512];
    cout << "float estimates[512][3] = {\n";
    for (int ctr = 0; ctr<512; ctr++)
    {
        float rad = (float(degrees[ctr])/float(count[ctr]))*3.1415926536/180.;
        gx[ctr] = cos(rad);
        gy[ctr] = sin(rad);
        offset[ctr] /= float(count[ctr]);

        if (count[ctr]>0)
        {
            cout << "{"<<gx[ctr]<<", "<<gy[ctr]<<", "<<offset[ctr]<<"}, // "<<ctr<<endl;
        }
        else
        {
            cout << "{"<<0<<", "<<0<<", "<<0<<"},                         // "<<ctr<<endl;
        }
    }
    cout << "};\n";
   
    
    return 0;
}
