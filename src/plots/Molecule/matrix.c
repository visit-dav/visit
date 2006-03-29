/* Local prototypes. */
static int l_u_decomp4(float M[4][4], int indx[4]);
static void l_u_backsub4(float M[4][4], int indx[4], float v[4]);

/**************************************************************************
 *
 * Calculate the inversion of a n by m matrix via lower-upper decomposition
 * and backsubstitution.  Efficient and effective.
 *
 **************************************************************************/

/**************************************************************************
 * Function:   l_u_decomp4()
 *
 * Programmer: Jeremy Meredith
 *             March 18, 1998
 *
 * Purpose:
 *  Perform L-U decomposition on the 4x4 matrix M, return result in M
 *  Return the permutation vector in indx[]
 *
 * Modifications:
 *
 **************************************************************************/
static int
l_u_decomp4(float M[4][4], int indx[4])
{
    int i,j,k;
    float vv[4];

    for (i=0; i<4; i++)
    {
        float aamax=0;
        for (j=0; j<4; j++)
        {
            if (fabs(M[i][j]) > aamax)
                aamax = fabs(M[i][j]);
        }
        if (aamax == 0)
            return -1; /* singular */
        vv[i] = 1./aamax;
    }

    for (j=0; j<4; j++)
    {
         float aamax;
         int   imax;

         if (j>0)
         {
             for (i=0; i<j; i++)
             {
                 float sum = M[i][j];
                 if (i>0)
                 {
                     for (k=0; k<i; k++)
                     {
                         sum -= M[i][k]*M[k][j];
                     }
                     M[i][j] = sum;
                 }
             }
         }

         aamax = 0.;
         imax  = -1;
         for (i=j; i<4; i++)
         {
             float sum = M[i][j];
             float t;
             if (j>0)
             {
                 for (k=0; k<j; k++)
                 {
                     sum -= M[i][k]*M[k][j];
                 }
                 M[i][j] = sum;
             }
             t = vv[i]*fabs(sum);
             if (t >= aamax)
             {
                 imax  = i;
                 aamax = t;
             }
         }

         if (j != imax)
         {
             for (k=0; k<4; k++)
             {
                 float t    = M[imax][k];
                 M[imax][k] = M[j][k];
                 M[j][k]    = t;
             }
             vv[imax] = vv[j];
         }

         indx[j] = imax;

         if (j < 3)
         {
             float t;

             if (M[j][j]==0)
                 return -1; /* singular */

             t = 1. / M[j][j];
             for (i=j+1; i<4; i++)
             {
                 M[i][j] *= t;
             }
         }
    }
    if (M[3][3] == 0)
        return -1; /* singular */

    return 0;
}

/**************************************************************************
 * Function:   l_u_backsub4()
 *
 * Programmer: Jeremy Meredith
 *             March 18, 1998
 *
 * Purpose:
 *  Perform back-substitution on matrix M[][] and vector v[] with
 *  permutation vector indx[].
 *  Return the result in v[]
 *
 * Modifications:
 *   Lisa J. Roberts, Fri Nov 19 10:04:49 PST 1999
 *   Removed k, which was unused.
 *
 **************************************************************************/
static void
l_u_backsub4(float M[4][4], int indx[4], float v[4])
{
    int ii = -1;
    int i,j;

    for (i=0; i<4; i++)
    {
        int ll = indx[i];
        float sum = v[ll];
        v[ll] = v[i];

        if (ii >= 0)
        {
            for (j=ii; j<i; j++)
            {
                sum -= M[i][j]*v[j];
            }
        }
        else if (sum != 0)
        {
            ii = i;
        }
        v[i] = sum;
    }

    for (i=3; i>=0; i--)
    {
        float sum = v[i];
        if (i < 3)
        {
            for (j=i+1; j<4; j++)
            {
                sum -= M[i][j]*v[j];
            }
        }
        v[i] = sum / M[i][i];
    }
}

/**************************************************************************
 * Function:   matrix_invert()
 *
 * Programmer: Jeremy Meredith
 *             March 18, 1998
 *
 * Purpose:
 *   Invert matrix M.
 *   If M is invertible:  return a 0, and return M^(-1) in I.
 *   If M is singular:    return nonzero, return identity matrix in I.
 *
 * Notes:
 *   This makes a copy of M and transposes it.  This makes the back-
 *   substitution straightforward since we are working on rows instead
 *   of columns.  It also prevents modification of M.
 *   Also, 'I' need NOT be the identity matrix before calling this function.
 *
 * Modifications:
 *
 **************************************************************************/
static int
matrix_invert(float M[4][4], float I[4][4])
{
    int i,j;
    int indx[4];
    float T[4][4];
    int err;

    for (i=0; i<4; i++)
    {
        for (j=0; j<4; j++)
        {
            T[i][j] = M[j][i];        /* copy and transpose M */
            I[i][j] = (i==j ? 1 : 0); /* set I to identity    */
        }
    }

    err = l_u_decomp4(T, indx);

    if (! err)
    {
        for (i=0; i<4; i++)
        {
            l_u_backsub4(T, indx, I[i]);
        }
    }

    /* No need to transpose I since we transposed M */

    return err;
}

static void matrix_mul_point(float out[3], float in[3], float M[4][4])
{
    float tmp[3];
    tmp[0] = in[0]*M[0][0] + in[1]*M[1][0] + in[2]*M[2][0] + M[3][0];
    tmp[1] = in[0]*M[0][1] + in[1]*M[1][1] + in[2]*M[2][1] + M[3][1];
    tmp[2] = in[0]*M[0][2] + in[1]*M[1][2] + in[2]*M[2][2] + M[3][2];
    out[0] = tmp[0];
    out[1] = tmp[1];
    out[2] = tmp[2];
}
