#include <stddef.h> /* for size_t */
#include <stdlib.h> /* for exit */
#include <string.h> /* memcpy */
#include <limits.h> /* for gs identities */
#include <float.h>  /* for gs identities */
#include "name.h"
#include "fail.h"
#include "types.h"
#include "tensor.h"
#include "gs_defs.h"
#include "gs_local.h"
#include "comm.h"

uint comm_gbl_id=0, comm_gbl_np=1;

GS_DEFINE_IDENTITIES()
GS_DEFINE_DOM_SIZES()

void comm_init(struct comm *c, comm_ext ce)
{
#ifdef MPI
  int i;
  MPI_Comm_dup(ce, &c->c);
  MPI_Comm_rank(c->c,&i), comm_gbl_id=c->id=i;
  MPI_Comm_size(c->c,&i), comm_gbl_np=c->np=i;
#else
  c->id = 0, c->np = 1;
#endif
}

void comm_init_check_(struct comm *c, MPI_Fint ce, uint np,
                             const char *file, unsigned line)
{
#ifdef MPI
  comm_init(c,MPI_Comm_f2c(ce));
  if(c->np != np)
    fail(1,file,line,"comm_init_check: passed P=%u, "
                     "but MPI_Comm_size gives P=%u",
                     (unsigned)np,(unsigned)c->np);
#else
  comm_init(c,0);
  if(np != 1)
    fail(1,file,line,"comm_init_check: passed P=%u, "
                     "but not compiled with -DMPI",(unsigned)np);
#endif
}

#define comm_init_check(c,ce,np) comm_init_check_(c,ce,np,__FILE__,__LINE__)

void comm_dup_(struct comm *d, const struct comm *s,
                      const char *file, unsigned line)
{
  d->id = s->id, d->np = s->np;
#ifdef MPI
  MPI_Comm_dup(s->c,&d->c);
#else
  if(s->np!=1) fail(1,file,line,"%s not compiled with -DMPI\n",file);
#endif
}
#define comm_dup(d,s) comm_dup_(d,s,__FILE__,__LINE__)

void comm_free(struct comm *c)
{
#ifdef MPI
  MPI_Comm_free(&c->c);
#endif
}

double comm_time(void)
{
#ifdef MPI
  return MPI_Wtime();
#else
  return 0;
#endif
}

void comm_barrier(const struct comm *c)
{
#ifdef MPI
  MPI_Barrier(c->c);
#endif
}

void comm_recv(const struct comm *c, void *p, size_t n,
                      uint src, int tag)
{
#ifdef MPI
# ifndef MPI_STATUS_IGNORE
  MPI_Status stat;
  MPI_Recv(p,n,MPI_UNSIGNED_CHAR,src,tag,c->c,&stat);
# else
  MPI_Recv(p,n,MPI_UNSIGNED_CHAR,src,tag,c->c,MPI_STATUS_IGNORE);
# endif
#endif
}

void comm_send(const struct comm *c, void *p, size_t n,
                      uint dst, int tag)
{
#ifdef MPI
  MPI_Send(p,n,MPI_UNSIGNED_CHAR,dst,tag,c->c);
#endif
}

void comm_irecv(comm_req *req, const struct comm *c,
                       void *p, size_t n, uint src, int tag)
{
#ifdef MPI
  MPI_Irecv(p,n,MPI_UNSIGNED_CHAR,src,tag,c->c,req);
#endif
}

void comm_isend(comm_req *req, const struct comm *c,
                       void *p, size_t n, uint dst, int tag)
{
#ifdef MPI
  MPI_Isend(p,n,MPI_UNSIGNED_CHAR,dst,tag,c->c,req);
#endif
}

void comm_wait(comm_req *req, int n)
{
#ifdef MPI
# ifndef MPI_STATUSES_IGNORE
  MPI_Status status[8];
  while(n>=8) MPI_Waitall(8,req,status), req+=8, n-=8;
  if(n>0) MPI_Waitall(n,req,status);
# else
  MPI_Waitall(n,req,MPI_STATUSES_IGNORE);
# endif
#endif
}

void comm_bcast(const struct comm *c, void *p, size_t n, uint root)
{
#ifdef MPI
  MPI_Bcast(p,n,MPI_UNSIGNED_CHAR,root,c->c);
#endif
}

static void scan_imp(void *scan, const struct comm *com, gs_dom dom, gs_op op,
                     const void *v, uint vn, void *buffer)
{
  comm_req req[2];
  size_t vsize = vn*gs_dom_size[dom];
  const uint id=com->id, np=com->np;
  uint n = np, c=1, odd=0, base=0;
  void *buf[2];
  void *red = (char*)scan+vsize;
  buf[0]=buffer,buf[1]=(char*)buffer+vsize;
  while(n>1) {
    odd=(odd<<1)|(n&1);
    c<<=1, n>>=1;
    if(id>=base+n) c|=1, base+=n, n+=(odd&1);
  }
  gs_init_array(scan,vn,dom,op);
  memcpy(red,v,vsize);
  while(n<np) {
    if(c&1) n-=(odd&1), base-=n;
    c>>=1, n<<=1, n+=(odd&1);
    odd>>=1;
    if(base==id) {
      comm_irecv(&req[0],com, buf[0],vsize, id+n/2,id+n/2);
      comm_isend(&req[1],com, red   ,vsize, id+n/2,id);
      comm_wait(req,2);
      gs_gather_array(red,buf[0],vn,dom,op);
    } else {
      comm_irecv(&req[0],com, scan,vsize, base,base);
      comm_isend(&req[1],com, red ,vsize, base,id);
      comm_wait(req,2);
      break;
    }
  }
  while(n>1) {
    if(base==id) {
      comm_send(com, scan  ,2*vsize, id+n/2,id);
    } else {
      comm_recv(com, buffer,2*vsize, base,base);
      gs_gather_array(scan,buf[0],vn,dom,op);
      memcpy(red,buf[1],vsize);
    }
    odd=(odd<<1)|(n&1);
    c<<=1, n>>=1;
    if(id>=base+n) c|=1, base+=n, n+=(odd&1);
  }
}


void allreduce_imp(const struct comm *com, gs_dom dom, gs_op op,
                          void *v, uint vn, void *buf)
{
  size_t total_size = vn*gs_dom_size[dom];
  const uint id=com->id, np=com->np;
  uint n = np, c=1, odd=0, base=0;
  while(n>1) {
    odd=(odd<<1)|(n&1);
    c<<=1, n>>=1;
    if(id>=base+n) c|=1, base+=n, n+=(odd&1);
  }
  while(n<np) {
    if(c&1) n-=(odd&1), base-=n;
    c>>=1, n<<=1, n+=(odd&1);
    odd>>=1;
    if(base==id) {
      comm_recv(com, buf,total_size, id+n/2,id+n/2);
      gs_gather_array(v,buf,vn, dom,op);
    } else {
      comm_send(com, v,total_size, base,id);
      break;
    }
  }
  while(n>1) {
    if(base==id)
      comm_send(com, v,total_size, id+n/2,id);
    else
      comm_recv(com, v,total_size, base,base);
    odd=(odd<<1)|(n&1);
    c<<=1, n>>=1;
    if(id>=base+n) c|=1, base+=n, n+=(odd&1);
  }
}

void comm_scan(void *scan, const struct comm *com, gs_dom dom, gs_op op,
               const void *v, uint vn, void *buffer)
{
  scan_imp(scan, com,dom,op, v,vn, buffer);
}

void comm_allreduce(const struct comm *com, gs_dom dom, gs_op op,
                          void *v, uint vn, void *buf)
{
  if(vn==0) return;
#ifdef MPI
  {
    MPI_Datatype mpitype;
    MPI_Op mpiop;
    #define DOMAIN_SWITCH() do { \
      switch(dom) { case gs_double:    mpitype=MPI_DOUBLE;    break; \
                    case gs_float:     mpitype=MPI_FLOAT;     break; \
                    case gs_int:       mpitype=MPI_INT;       break; \
                    case gs_long:      mpitype=MPI_LONG;      break; \
     WHEN_LONG_LONG(case gs_long_long: mpitype=MPI_LONG_LONG; break;) \
                  default:        goto comm_allreduce_byhand; \
      } \
    } while(0)
    DOMAIN_SWITCH();
    #undef DOMAIN_SWITCH
    switch(op) { case gs_add: mpiop=MPI_SUM;  break;
                 case gs_mul: mpiop=MPI_PROD; break;
                 case gs_min: mpiop=MPI_MIN;  break;
                 case gs_max: mpiop=MPI_MAX;  break;
                 default:        goto comm_allreduce_byhand;
    }
    MPI_Allreduce(v,buf,vn,mpitype,mpiop,com->c);
    memcpy(v,buf,vn*gs_dom_size[dom]);
    return;
  }
#endif
#ifdef MPI
comm_allreduce_byhand:
  allreduce_imp(com,dom,op, v,vn, buf);
#endif
}

double comm_dot(const struct comm *comm, double *v, double *w, uint n)
{
  double s=tensor_dot(v,w,n),b;
  comm_allreduce(comm,gs_double,gs_add, &s,1, &b);
  return s;
}

/* T comm_reduce__T(const struct comm *comm, gs_op op, const T *in, uint n) */

#define SWITCH_OP_CASE(T,OP) case gs_##OP: WITH_OP(T,OP); break;
#define SWITCH_OP(T,op) do switch(op) { \
    GS_FOR_EACH_OP(T,SWITCH_OP_CASE) case gs_op_n: break; } while(0)

#define WITH_OP(T,OP) \
  do { T v = *in++; GS_DO_##OP(accum,v); } while(--n)

#define DEFINE_REDUCE(T) \
T PREFIXED_NAME(comm_reduce__##T)( \
    const struct comm *comm, gs_op op, const T *in, uint n) \
{                                                           \
  T accum = gs_identity_##T[op], buf;                       \
  if(n!=0) SWITCH_OP(T,op);                                 \
  comm_allreduce(comm,gs_##T,op, &accum,1, &buf);           \
  return accum;                                             \
}

GS_FOR_EACH_DOMAIN(DEFINE_REDUCE)

#undef DEFINE_REDUCE
#undef WITH_OP
#undef SWITCH_OP
#undef SWITCH_OP_CASE

