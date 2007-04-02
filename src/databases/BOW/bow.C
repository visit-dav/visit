

/********** bow.c created by lib **********/



/*
 * lib_init for bow
 */

void lib_init_bow()
{

}



/*
 * lib_quit for bow
 */

void lib_quit_bow()
{

}



/* lib_public include: */
#include "bow.h"



extern int libargc;
extern char **libargv;
extern char **libenvp;




/* lib_application "Bof2bow/bof2bow.l" */

/* lib_application "Bowcat/bowcat.l" */

/* lib_application "Bowcrop/bowcrop.l" */

/* lib_application "Bowinfo/bowinfo.l" */

/* lib_application "Bowtest/bowtest.l" */




#define HEADSIZE (4+4+4+2+4+4+2+2+2+2+2+2)

#define FULLHEADSIZE (4+HEADSIZE+1+4*2*bigcnt+4*ntimax)


#define I ii

#define K kio

#define KS ksio

#define COUT(X) { buf[I++]=(X); }

#define SOUT(X) { K=(X); COUT((K>>8)&0xff) COUT(K&0xff) }

#define IOUT(X) {                                                           \
    K=(X); COUT((K>>24)&0xff) COUT((K>>16)&0xff)                            \
    COUT((K>>8)&0xff) COUT(K&0xff)                                          \
}

#define FOUT(X) { int fout_i; f=(X); fout_i= *((int *)(&f)); IOUT(fout_i) }

#define HEADOUT {                                                           \
    IOUT(n) IOUT(ti) IOUT(bi) SOUT(vi) FOUT(v0) FOUT(v1)                    \
    SOUT(xs) SOUT(ys) SOUT(zs) SOUT(x0) SOUT(y0) SOUT(z0)                   \
}

#define CIN(X) { (X)= *ub++; }

#define SIN(X) \
    { KS= *ub++; KS=(KS<<8)|(*ub++); (X)=((KS&0x8000)?KS|0xffff0000:KS); }

#define IIN(X) { K= *ub++; K=(K<<8)|(*ub++); K=(K<<8)|(*ub++); \
    K=(K<<8)|(*ub++); (X)=K; }

#define FIN(X) { int fin_i; IIN(fin_i) (X)= *((float *)(&fin_i)); }

#define HEADIN {                                                            \
    IIN(n) IIN(ti) IIN(bi) SIN(vi) FIN(v0) FIN(v1)                          \
    SIN(xs) SIN(ys) SIN(zs) SIN(x0) SIN(y0) SIN(z0)                         \
}


static void find_tmpdir(char *tmpdir);
static char *fioX_read(bowglobal bg,char *pathsrc);
static int fioX_write(char *pathdst,char *buf,int size);
static int fioX_size(char *pathsrc);
static int fioX_isdir(char *pathsrc);
static int my_floor(float f);
static void bowglobal_adderror(bowglobal bg);


bowglobal bowglobal_create(gzlib_alloctype my_alloc,gzlib_freetype my_free,
    void *opaque)
{
    bowglobal bg;

    if (!my_alloc || !my_free) { my_alloc=gzlib_alloc; my_free=gzlib_free; }
    bg=(bowglobal)(*my_alloc)(opaque,sizeof(bowglobal_struct));
    if (!bg) return bg;
    bg->alloc=my_alloc;
    bg->free=my_free;
    bg->opaque=opaque;
    bg->error_count=0;
    { int i; for (i=0;i<BOWGLOBAL_ERROR_MAX;i++) bg->error_msgs[i][0]='\0'; }
    bg->error_msg=bg->error_msgs[bg->error_count];
    bg->no_copy=0;
    return bg;
}


void bowglobal_destroy(bowglobal bg)
{
    (*bg->free)(bg->opaque,(void *)bg);
}


int bof2bow(bowglobal bg,int ti,int bi,int vi,int xs,int ys,int zs,
    int x0,int y0,int z0,float *vtab,float v0,float v1,float q,
    char **rbow,int *rbowsize)
{
    int i,j,k,n,ibig[256],*ip,bigcnt,s,nti,ntimax,ntab[40],ii,kio,ksio;
    short *vshrt;
    float td,thresh[40],a,f,fbig[256],*vtab1,vmaxtab[40],vmax;
    char *outbuf,*buf;
    int outsize;

    /* disallow ti<0 (we need that bit for the notran flag) */
    if (ti<0 || ti>0x7ffffffe) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"ti=%d out of range 0..%d\n",ti,0x7ffffffe);
        return -1;
    }

    n=xs*ys*zs;
    if (bg->no_copy) vtab1=vtab;
    else{
        vtab1=(float *)(*bg->alloc)(bg->opaque,n*sizeof(float));
        if (!vtab1) {
            bowglobal_adderror(bg);
            sprintf(bg->error_msg,"out of memory");
            return -1;
        }
    }
    a=0.5/(v1-v0);
    for (i=0;i<n;i++) vtab1[i]=(vtab[i]-v0)*a;
    vtab=vtab1;

    vshrt=(short *)(*bg->alloc)(bg->opaque,xs*ys*zs*sizeof(short));
    if (!vshrt) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"out of memory");
        return -1;
    }
    j=0;
    if (q>=0.0) {
        td=0.20*q; a=1.4142136f;
        for (i=0;i<40;i++) { thresh[i]=td; if (zs>1 || i%3!=2) td/=a; }
        {
            fwav fw;

            fw=fwav_create(xs,ys,zs,thresh,bg->alloc,bg->free,bg->opaque);
            fwav_atran(fw,vtab);
            fwav_destroy(fw);
        }
    
        vmax=2.5/q;
        bow_ntitab(bg,xs,ys,zs,ntab,&ntimax);
        for (nti=ntimax-1;nti>=0;nti--)
            { if (vmax>32767.0) vmax=32767.0; vmaxtab[nti]=vmax; vmax*=a; }
        for (nti=0,i=0;nti<ntimax;nti++) {
            n=ntab[nti]; vmax=vmaxtab[nti];
            for (s=0;s<n;s++,i++) {
                f=vtab[i]; k=my_floor(vmax*f+0.5);
                if (k<-32767 || k>32767) {
                    if (j<255) { k=0x8000; fbig[j]=f; ibig[j]=i; j++; }
                    else{
                        bowglobal_adderror(bg);
                        sprintf(bg->error_msg,
                            "bof2bow: !!!!! k=%d !!!!!",k);
                        if (k<-32767) k= -32767; if (k>32767) k=32767;
                    }
                }
                vshrt[i]=k;
            }
        }
    }else{
        vmax=1.0/(-q); if (vmax>32767.0) vmax=32767.0;
        ntab[0]=n; ntimax=1; vmaxtab[0]=vmax;
        for (i=0;i<n;i++) {
            f=vtab[i]; k=my_floor(vmax*f+0.5);
            if (k<-32767 || k>32767) {
                if (j<255) { k=0x8000; fbig[j]=f; ibig[j]=i; }
                else{
                    if (j==255) {
                        bowglobal_adderror(bg);
                        sprintf(bg->error_msg,
                            "*** WARNING bof2bow: bigcnt overflow");
                    }
                    if (k<-32767) k= -32767; if (k>32767) k=32767;
                }
                j++;
            }
            vshrt[i]=k;
        }
    }

    /* fix endian if HI LO machine */
    {
        short s;
        char *cp;

        s=0x0011; cp=(char *)(&s);
        if (cp[1]) {
            n=xs*ys*zs;
            for (i=0;i<n;i++) {
                s=vshrt[i];
                j=cp[0]; cp[0]=cp[1]; cp[1]=j;
                vshrt[i]=s;
            }
        }
    }

    /*...more: this is bad -- we should deal with excess large coefs (e.g. */
    /* revert to raw quantization) */
    if (j>255) j=255;
    bigcnt=j;
    if (!bg->no_copy) (*bg->free)(bg->opaque,(void *)vtab);

    /*...more: THIS SECTION NEEDS TO BE REWRITTEN (use fast custom coder) */

    {
        gzlib gz;

        gz=gzlib_create(bg->alloc,bg->free,bg->opaque);
        if (!gz) {
            (*bg->free)(bg->opaque,(void *)vshrt);
            bowglobal_adderror(bg);
            sprintf(bg->error_msg,"gzlib_create failed");
            return -1;
        }
        outbuf=(char *)0; outsize=0;
        n=gzlib_deflate(gz,(char *)vshrt,xs*ys*zs*sizeof(short),
            &outbuf,&outsize);
        (*bg->free)(bg->opaque,(void *)vshrt);
        if (n) {
            bowglobal_adderror(bg);
            sprintf(bg->error_msg,"gzlib_deflate failed");
            return -1;
        }
        gzlib_destroy(gz);
    }

    /* store: fullhead+outbuf */
    n=FULLHEADSIZE+outsize;
    buf=(char *)(*bg->alloc)(bg->opaque,n);
    if (!buf) {
        (*bg->free)(bg->opaque,(void *)outbuf);
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"out of memory");
        return -1;
    }
    ii=0;
    COUT('b') COUT('o') COUT('w') COUT('C')
    ti=(q<0.0?-(ti+1):ti);
    HEADOUT
    COUT(bigcnt)
    for (j=0;j<bigcnt;j++) { IOUT(ibig[j]) FOUT(fbig[j]) }
    for (nti=0;nti<ntimax;nti++) FOUT(vmaxtab[nti])
    for (j=0;j<outsize;j++) buf[ii++]=outbuf[j];
    (*bg->free)(bg->opaque,(void *)outbuf);
    *rbow=buf; *rbowsize=n;
    return 0;
}


float *bow2bof(bowglobal bg,char *bow,int bowi)
{
    int size,offset,notran,ti,bi,vi,xs,ys,zs,x0,y0,z0;
    int i,j,k,n,bigcnt,ibig[256],ii,kio,ksio,*ip,ks;
    int ntab[40],nti,ntimax,s;
    short *vshrt;
    float v0,v1,*vtab,a,fbig[256],*fp,vmaxtab[40],vmax,f;
    unsigned char *ub;
    bowinfo binf;

    if (!(binf=bow_getbowinfo(bg,bow))) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"*** bow2bof: bad magic number");
        return (float *)0;
    }
    if (bowi<0 || bowi>=binf->numbow) {
        bow_freebowinfo(bg,binf);
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"*** bow2bof: bowi out of range");
        return (float *)0;
    }

    
#define XCOP(X) { X=binf->X[bowi]; }
    XCOP(size) XCOP(offset) XCOP(notran) XCOP(ti) XCOP(bi) XCOP(vi)
    XCOP(xs) XCOP(ys) XCOP(zs) XCOP(x0) XCOP(y0) XCOP(z0) XCOP(v0) XCOP(v1)

    ub=(unsigned char *)(bow+(offset+4+HEADSIZE));
    bigcnt= *ub++;
    for (j=0;j<bigcnt;j++) { IIN(ibig[j]) FIN(fbig[j]) }
    if (notran) { ntimax=1; ntab[0]=xs*ys*zs; }
    else bow_ntitab(bg,xs,ys,zs,ntab,&ntimax);
    for (nti=0;nti<ntimax;nti++) FIN(vmaxtab[nti])

    if (binf->version<='B') {
        char tmpdir[1024],tmpname[1024],tmpnamegz[1024],cmd[1024];

        find_tmpdir(tmpdir);
        sprintf(tmpname,"%s/bof2bow_tmp%d_%d_%d_%d",tmpdir,
            binf->ti[bowi],binf->bi[bowi],binf->vi[bowi],getpid());
        sprintf(tmpnamegz,"%s.gz",tmpname);
        fioX_write(tmpnamegz,(char *)ub,size-FULLHEADSIZE);
        sprintf(cmd,"gunzip %s",tmpnamegz);
        system(cmd);
        unlink(tmpnamegz);
        vshrt=(short *)fioX_read(bg,tmpname);
        unlink(tmpname);
        if (!vshrt) {
            bow_freebowinfo(bg,binf);
            bowglobal_adderror(bg);
            sprintf(bg->error_msg,"*** bow2bof: gunzip failed");
            return (float *)0;
        }
    }else{
        gzlib gz;
        char *inbuf,*outbuf;
        int insize,outsize;

        gz=gzlib_create(bg->alloc,bg->free,bg->opaque);
        if (!gz) {
            bow_freebowinfo(bg,binf);
            bowglobal_adderror(bg);
            sprintf(bg->error_msg,"gzlib_create failed");
            return (float *)0;
        }
        inbuf=(char *)ub; insize=size-FULLHEADSIZE;
        outbuf=(char *)0; outsize=xs*ys*zs*sizeof(short);
        if (gzlib_inflate(gz,inbuf,insize,&outbuf,&outsize)) {
            bow_freebowinfo(bg,binf);
            bowglobal_adderror(bg);
            sprintf(bg->error_msg,"gzlib_inflate failed");
            return (float *)0;
        }
        vshrt=(short *)outbuf;
        gzlib_destroy(gz);
    }

    n=xs*ys*zs;

    /* fix endian if HI LO machine */
    if (binf->version>'A') {
        short s;
        char *cp;

        s=0x0011; cp=(char *)(&s);
        if (cp[1]) {
            for (i=0;i<n;i++) {
                s=vshrt[i];
                j=cp[0]; cp[0]=cp[1]; cp[1]=j;
                vshrt[i]=s;
            }
        }
    }

    vtab=(float *)(*bg->alloc)(bg->opaque,n*sizeof(float));
    if (!vtab) {
        bow_freebowinfo(bg,binf);
        (*bg->free)(bg->opaque,(void *)vshrt);
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"out of memory");
        return (float *)0;
    }
    j=0;
    for (nti=0,i=0;nti<ntimax;nti++) {
        n=ntab[nti]; vmax=vmaxtab[nti];
        for (s=0;s<n;s++,i++) {
            k=vshrt[i];
            if ((k&0xffff)==0x8000) {
                k=(j>=bigcnt?-1:ibig[j]);
                if (i!=k) {
                    bowglobal_adderror(bg);
                    sprintf(bg->error_msg,"*** bow2bof: corrupted data");
                    bow_freebowinfo(bg,binf);
                    (*bg->free)(bg->opaque,(void *)vshrt);
                    (*bg->free)(bg->opaque,(void *)vtab);
                    return (float *)0;
                }
                vtab[i]=fbig[j]; j++;
            }else vtab[i]=(float)k/vmax;
        }
    }
    (*bg->free)(bg->opaque,(void *)vshrt);
    n=xs*ys*zs;

    if (!notran) {
        fwav fw;

        fw=fwav_create(xs,ys,zs,(float *)0,bg->alloc,bg->free,bg->opaque);
        fwav_stran(fw,vtab);
        fwav_destroy(fw);
    }

    a=(v1-v0)/0.5;
    for (i=0;i<n;i++) vtab[i]=a*vtab[i]+v0;

    bow_freebowinfo(bg,binf);

    return vtab;
}


/* NOTE: use bow_freebowinfo() when done with bowinfo structure */

bowinfo bow_getbowinfo(bowglobal bg,char *bow)
{
    int i,n,ti,bi,vi,xs,ys,zs,x0,y0,z0,iscat,nb,size,offset,notran;
    int ii,kio,ksio,*ip,ks;
    float v0,v1,f;
    bowinfo binf;
    unsigned char *ub;

    if (bow[0]!='b' || bow[1]!='o' || bow[2]!='w' || bow[3]>'C') {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"bad magic number");
        return (bowinfo)0;
    }
    binf=(bowinfo)(*bg->alloc)(bg->opaque,sizeof(bowinfo_struct));
    if (!binf) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"out of memory");
        return binf;
    }
    binf->version=bow[3];
    ub=(unsigned char *)(bow+4);
    HEADIN
    if (n<0)
        { iscat=1; nb= -n; ub=(unsigned char *)(bow+8); offset=8+nb*HEADSIZE; }
    else { iscat=0; nb=1; ub=(unsigned char *)(bow+4); offset=0; }


    /*...more: test all these alloc calls for failure, deal with it */
    
#define IGET(X) { binf->X=(int *)(*bg->alloc)(bg->opaque,nb*sizeof(int)); }
    
#define FGET(X) \
        { binf->X=(float *)(*bg->alloc)(bg->opaque,nb*sizeof(float)); }
    binf->numbow=nb;
    IGET(size) IGET(offset) IGET(notran) IGET(ti) IGET(bi) IGET(vi)
    IGET(xs) IGET(ys) IGET(zs) IGET(x0) IGET(y0) IGET(z0) FGET(v0) FGET(v1)
    for (i=0;i<nb;i++) {
        HEADIN
        if (ti<0) { notran=1; ti= -ti-1; } else notran=0;
        size=n;
        binf->size[i]=size; binf->offset[i]=offset; binf->notran[i]=notran;
        binf->ti[i]=ti; binf->bi[i]=bi; binf->vi[i]=vi;
        binf->xs[i]=xs; binf->ys[i]=ys; binf->zs[i]=zs;
        binf->x0[i]=x0; binf->y0[i]=y0; binf->z0[i]=z0;
        binf->v0[i]=v0; binf->v1[i]=v1;
        offset+=size;
    }

    return binf;
}


void bow_freebowinfo(bowglobal bg,bowinfo binf)
{
    
#define XPUT(X) { (*bg->free)(bg->opaque,(void *)(binf->X)); }
    XPUT(size) XPUT(offset) XPUT(notran) XPUT(ti) XPUT(bi) XPUT(vi)
    XPUT(xs) XPUT(ys) XPUT(zs) XPUT(x0) XPUT(y0) XPUT(z0) XPUT(v0) XPUT(v1)
    (*bg->free)(bg->opaque,(void *)binf);
}


char *bow_cat(bowglobal bg,int numbow,char **bowtab)
{
    int bowi,nbow,size,k,ks,js,ii,kio,ksio;
    int i,b,n,nb,ti,bi,vi,xs,ys,zs,x0,y0,z0,*ip;
    float v0,v1,f,*fp;
    char *bow,*buf;
    unsigned char *ub;

    /* get total number of simple bows, size sum */
    nbow=0; size=0;
    for (bowi=0;bowi<numbow;bowi++) {
        if (bowtab[bowi][3]!=bowtab[0][3]) {
            bowglobal_adderror(bg);
            sprintf(bg->error_msg,
                "bow_cat: bows not all the same version of format");
            return (char *)0;
        }
        bow=bowtab[bowi]; ub=(unsigned char *)(bow+4);
        HEADIN
        if (n<0) {
            nb= -n; ub=(unsigned char *)(bow+8);
            for (b=0;b<nb;b++) {
                HEADIN
                nbow++;
                size+=n;
            }
        }else{ nbow++; size+=n; }
    }
    ks=8+HEADSIZE*nbow; size+=ks;
    buf=(char *)(*bg->alloc)(bg->opaque,size); ii=0;
    if (!buf) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"out of memory");
        return (char *)0;
    }
    COUT('b') COUT('o') COUT('w') COUT((bowtab[0][3]))
    IOUT(-nbow)
    for (bowi=0;bowi<numbow;bowi++) {
        bow=bowtab[bowi]; ub=(unsigned char *)(bow+4);
        HEADIN
        if (n<0) {
            nb= -n; ub=(unsigned char *)(bow+8);
            js=8+HEADSIZE*nb;
            for (b=0;b<nb;b++) {
                HEADIN
                HEADOUT
                for (k=0;k<n;k++) buf[ks+k]=bow[js+k];
                ks+=n; js+=n;
            }
        }else{
            HEADOUT
            for (k=0;k<n;k++) buf[ks+k]=bow[k];
            ks+=n;
        }
    }
    return buf;
}


int bow_test(bowglobal bg,int ti,int bi,int vi,int xs,int ys,int zs,
    int x0,int y0,int z0,float *vsum,float v0,float v1,char *bow)
{
    int i,n,xi,yi,zi,xa,ya,za,ta,ba,va;
    float *vcop,f,f1,fsum,fmed;
    bowinfo binf;

    n=xs*ys*zs;

    printf("Testing bow against original bof\n"); fflush(stdout);
    printf("  args: ti=%d bi=%d vi=%d\n",ti,bi,vi);
    printf("    xs=%d ys=%d zs=%d x0=%d y0=%d z0=%d\n",xs,ys,zs,x0,y0,z0);
    if (!(binf=bow_getbowinfo(bg,bow))) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"*** bow_test: bad magic number");
        return -1;
    }
    if (binf->numbow!=1) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"*** bow_test: can't test a catenated bow");
        bow_freebowinfo(bg,binf);
        return -1;
    }
    printf("  doing bow2bof\n"); fflush(stdout);
    if (!(vcop=bow2bof(bg,bow,0))) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"bow2bof failed");
        bow_freebowinfo(bg,binf);
        return -1;
    }
    ta=binf->ti[0]; ba=binf->bi[0]; va=binf->vi[0];
    xi=binf->xs[0]; yi=binf->ys[0]; zi=binf->zs[0];
    xa=binf->x0[0]; ya=binf->y0[0]; za=binf->z0[0];
    printf("  comparing\n");
    printf("  decode: ti=%d bi=%d vi=%d\n",ta,ba,va);
    printf("    xs=%d ys=%d zs=%d x0=%d y0=%d z0=%d\n",xi,yi,zi,xa,ya,za);
    fflush(stdout);
    if (ta!=ti || ba!=bi || va!=vi || xi!=xs || yi!=ys || zi!=zs ||
        xa!=x0 || ya!=y0 || za!=z0) {
        bowglobal_adderror(bg);
        sprintf(bg->error_msg,"bow2bof decode mismatch");
        (*bg->free)(bg->opaque,(void *)vcop);
        bow_freebowinfo(bg,binf);
        return -1;
    }
    f1=0.0;
    for (i=0;i<n;i++)
        { f=vsum[i]-vcop[i]; if (f<0.0) f= -f; fsum+=f; if (f>f1) f1=f; }
    fsum/=(float)n;
    {
        int h,htot,hist[255];

        for (h=0;h<255;h++) hist[h]=0;
        for (i=0;i<n;i++) {
            f=vsum[i]-vcop[i]; if (f<0.0) f= -f;
            h=my_floor(255.9*f/f1+0.05);
            hist[h]++;
        }
        htot=0; for (h=0;h<255 && htot+hist[h]<n/2;h++) htot+=hist[h];
        fmed=f1*(float)h/255.0;
    }

    printf("  measured error:\n");
    printf("                 max            med            ave    \n");
    printf("             -----------    -----------    -----------\n");
    printf("             %11.7f    %11.7f    %11.7f\n",f1,fmed,fsum);
    printf("             %11.7f %%  %11.7f %%  %11.7f %%\n",
        100.0*f1/(v1-v0),100.0*fmed/(v1-v0),100.0*fsum/(v1-v0));

    (*bg->free)(bg->opaque,(void *)vcop);
    bow_freebowinfo(bg,binf);

    return 0;
}


void bow_ntitab(bowglobal bg,int xs,int ys,int zs,int *ntab,int *rntimax)
{
    int i,j,n,n0,dx,dy,dz,nt[40];

    
#define HALF(X) (((X)+1)>>1)
    
#define NTOUT { n0=n; n=dx*dy*dz; nt[i++]=n0-n; }

    dx=xs; dy=ys; dz=zs;
    i=0;
    n=dx*dy*dz;
    while (dx>1 || dy>1 || dz>1) {
        if (dx>1) { dx=HALF(dx); NTOUT }
        if (dy>1) { dy=HALF(dy); NTOUT }
        if (dz>1) { dz=HALF(dz); NTOUT }
    }

    *rntimax=i+1; ntab[0]=1;
    for (j=i-1,n=1;j>=0;j--) ntab[n++]=nt[j];
}


static void find_tmpdir(char *tmpdir)
{
    char *s;

    if (s=getenv("BOW_TMPDIR")) {
        sprintf(tmpdir,"%s",s);
        return;
    }

    if (fioX_isdir("/var/tmp")) {
        sprintf(tmpdir,"/var/tmp");
        return;
    }

    sprintf(tmpdir,".");
}



#define RET(X) { return (X); }


/* read a file into new buffer */
/* */
/* returns newly created buffer on success, 0 on failure */
/* (mem_size() and mem_put() may be applied to return buffer) */

static char *fioX_read(bowglobal bg,char *pathsrc)
{
    int id,n,m;
    static struct stat st_store;
    static struct stat *st = &st_store;
    char *buf;

    /* open for read, get size stat */
    if ((id=open(pathsrc,O_RDONLY))<0) RET((char *)0)
    if (fstat(id,st)) { close(id); RET((char *)0) }
    n=st->st_size;
    if (n<=0) { close(id); RET((char *)0) }
    buf=(char *)(*bg->alloc)(bg->opaque,n);
    if (read(id,buf,n)!=n)
        { close(id); (*bg->free)(bg->opaque,(void *)buf); RET((char *)0) }
    close(id);
    RET(buf)
}


/* write a buffer to a file */
/* */
/* returns 0 on success, -1 on failure */

static int fioX_write(char *pathdst,char *buf,int size)
{
    int id;

    id=open(pathdst,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if (id<0) RET(-1)
    if (write(id,buf,size)!=size) { close(id); RET(-1) }
    close(id);
    RET(0)
}


/* return size of file or -1 on error */

static int fioX_size(char *pathsrc)
{
    static struct stat st_store;
    static struct stat *st = &st_store;

    if (stat(pathsrc,st)) return -1;
    return st->st_size;
}


/* return 1 if dir and exists, 0 otherwise */

static int fioX_isdir(char *pathsrc)
{
    static struct stat st_store;
    static struct stat *st = &st_store;

    if (stat(pathsrc,st)) return 0;
    return ((st->st_mode&S_IFDIR)?1:0);
}


static int my_floor(float f)
{
    f=(f-0.25f)+(float)0x00600000;
    return ((*((int *)(&f)))>>1)-0x25600000;
}


static void bowglobal_adderror(bowglobal bg)
{
    bg->error_msg=bg->error_msgs[bg->error_count%BOWGLOBAL_ERROR_MAX];
    bg->error_count++;
}


