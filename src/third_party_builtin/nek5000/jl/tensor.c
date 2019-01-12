#include "c99.h"
#include "name.h"
#include "types.h"

#if !defined(USE_CBLAS)

#define tensor_dot  PREFIXED_NAME(tensor_dot )
#define tensor_mtxm PREFIXED_NAME(tensor_mtxm)

/* Matrices are always column-major (FORTRAN style) */

double tensor_dot(const double *a, const double *b, uint n)
{
  double sum = 0;
  for(;n;--n) sum += *a++ * *b++;
  return sum;
}

#  if defined(USE_NAIVE_BLAS)
#    define tensor_mxv  PREFIXED_NAME(tensor_mxv )
#    define tensor_mtxv PREFIXED_NAME(tensor_mtxv)
#    define tensor_mxm  PREFIXED_NAME(tensor_mxm )

/* y = A x */
void tensor_mxv(
  double *restrict y, uint ny,
  const double *restrict A, const double *restrict x, uint nx)
{
  uint i;
  for(i=0;i<ny;++i) y[i]=0;
  for(;nx;--nx) {
    const double xk = *x++;
    for(i=0;i<ny;++i) y[i] += (*A++)*xk;
  }
}

/* y = A^T x */
void tensor_mtxv(
  double *restrict y, uint ny,
  const double *restrict A, const double *restrict x, uint nx)
{
  for(;ny;--ny) {
    const double *restrict xp = x;
    uint n = nx;
    double sum = *A++ * *xp++;
    for(--n;n;--n) sum += *A++ * *xp++;
    *y++ = sum;
  }
}

/* C = A * B */
void tensor_mxm(
  double *restrict C, uint nc,
  const double *restrict A, uint na, const double *restrict B, uint nb)
{
  uint i,j,k;
  for(i=0;i<nc*nb;++i) C[i]=0;
  for(j=0;j<nb;++j,C+=nc) {
    const double *restrict A_ = A;
    for(k=0;k<na;++k) {
      const double b = *B++;
      for(i=0;i<nc;++i) C[i] += (*A_++) * b;
    }
  }
}

#  endif

/* C = A^T * B */
void tensor_mtxm(
  double *restrict C, uint nc,
  const double *restrict A, uint na, const double *restrict B, uint nb)
{
  uint i,j;
  for(j=0;j<nb;++j,B+=na) {
    const double *restrict A_ = A;
    for(i=0;i<nc;++i,A_+=na) *C++ = tensor_dot(A_,B,na);
  }
}

#endif

double tensor_i1(const double *Jr, uint nr, const double *u)
{
  return tensor_dot(Jr,u,nr);
}

/* work holds ns doubles */
double tensor_i2(const double *Jr, uint nr,
                        const double *Js, uint ns,
                        const double *u, double *work)
{
  tensor_mtxv(work,ns, u, Jr,nr);
  return tensor_dot(Js,work,ns);
}

/* work holds ns*nt + nt doubles */
double tensor_i3(const double *Jr, uint nr,
                        const double *Js, uint ns,
                        const double *Jt, uint nt,
                        const double *u, double *work)
{
  double *work2 = work+nt;
  tensor_mtxv(work2,ns*nt,   u,     Jr,nr);
  tensor_mtxv(work ,nt   ,   work2, Js,ns);
  return tensor_dot(Jt,work,nt);
}



double tensor_ig1(double g[1],
                         const double *wtr, uint nr,
                         const double *u)
{
  g[0] = tensor_dot(wtr+nr,u,nr);
  return tensor_dot(wtr   ,u,nr);
}

/* work holds 2*nr doubles */
double tensor_ig2(double g[2],
                         const double *wtr, uint nr,
                         const double *wts, uint ns,
                         const double *u, double *work)
{
  tensor_mxm(work,nr, u,ns, wts,2);
  g[0] = tensor_dot(wtr+nr,work   ,nr);
  g[1] = tensor_dot(wtr   ,work+nr,nr);
  return tensor_dot(wtr   ,work   ,nr);
}

/* work holds 2*nr*ns + 3*nr doubles */
double tensor_ig3(double g[3],
                         const double *wtr, uint nr,
                         const double *wts, uint ns,
                         const double *wtt, uint nt,
                         const double *u, double *work)
{
  const uint nrs = nr*ns;
  double *a = work, *b = work+2*nrs, *c=b+2*nr;
  tensor_mxm(a,nrs, u,nt, wtt,2);
  tensor_mxm(b,nr,  a,ns, wts,2);
  tensor_mxv(c,nr, a+nrs, wts,ns);
  g[0] = tensor_dot(b   , wtr+nr, nr);
  g[1] = tensor_dot(b+nr, wtr   , nr);
  g[2] = tensor_dot(c   , wtr   , nr);
  return tensor_dot(b   , wtr   , nr);
}


/*
  out - nr x ns
  u   - mr x ms
  Jrt - mr x nr, Jst - ms x ns
  work - nr x ms
*/
void tensor_2t(double *out,
                      const double *Jrt, uint nr, uint mr,
                      const double *Jst, uint ns, uint ms,
                      const double *u, double *work)
{
  tensor_mtxm(work,nr, Jrt,mr, u,ms);
  tensor_mxm(out,nr, work,ms, Jst,ns);
}

/*
  out - nr x ns x nt
  u   - mr x ms x mt
  Jrt - mr x nr, Jst - ms x ns, Jtt - mt x nt
  work - nr*ms*mt + nr*ns*mt = nr*(ms+ns)*mt
*/
void tensor_3t(double *out,
                      const double *Jrt, uint nr, uint mr,
                      const double *Jst, uint ns, uint ms,
                      const double *Jtt, uint nt, uint mt,
                      const double *u, double *work)
{
  const uint nrs=nr*ns, mst=ms*mt, nrms=nr*ms;
  uint k;
  double *work2 = work+nr*mst;
  double *p; const double *q;
  tensor_mtxm(work,nr, Jrt,mr, u,mst);
  for(k=0,p=work2,q=work;k<mt;++k,p+=nrs,q+=nrms)
    tensor_mxm(p,nr, q,ms, Jst,ns);
  tensor_mxm(out,nrs, work2,mt, Jtt,nt);
}

