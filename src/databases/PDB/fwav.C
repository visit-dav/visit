

/*
 * fwav.{c,h}         linear wavelet transform library for bricks of floats
 * by Mark Duchaineau (free but copyrighted, see LibGen/COPYING)
 *
 * 09-21-00: based on 12-11-99 Wav/wav.l library
 * 09-24-00: allow non power-of-two bricks; added fwavtest.l application
 * 09-25-00: fixed stran() failure to init _zero
 * 12-08-02: new API to be thread safe, use app-supplied mem allocation
 * 12-13-02: API change adds opaque arg to fwav_create()
 * 2003-10-09: took snapshot of LibGen fwav.l compile and converted to
 *             stand-alone fwav.c, fwav.h
 */


#include "fwav.h"


/*
 * lib_struct fwav
 */

struct fwav_structdef {
    gzlib_alloctype alloc;
    gzlib_freetype free;
    void *opaque;
    int xs,ys,zs;
    float *v;
    float *thresh;
    float *bt;
    float *zero;
    float *burow;
    float *bvrow;
    float td0,td1;
};



/* macro that "tosses out" (i.e. sets to zero) small wavelet coefficients */


#define THRESH(V) { if (V>td0 && V<td1) V=0.0; }


/* reduced scaling coef array size after one filter step */


#define HALF(X) ((X+1)>>1)


/* brick size to reduce to */


#define DX 1

#define DY 1

#define DZ 1


/* prototypes of internal routines */

void fwav_atran_x(fwav fw,float *ba0,int dx,int dy,int dz);
void fwav_atran_y(fwav fw,float *b0,int dx,int dy,int dz);
void fwav_atran_z(fwav fw,float *b0,int dx,int dy,int dz);
void fwav_stran_xyz(fwav fw,float *b1,int dx,int dy,int dz);
void fwav_stran_x(fwav fw,float *b1,int dx,int dy,int dz);
void fwav_stran_y(fwav fw,float *b1,int dx,int dy,int dz);
void fwav_stran_z(fwav fw,float *b1,int dx,int dy,int dz);


fwav fwav_create(int xs,int ys,int zs,float *thresh,
    gzlib_alloctype my_alloc,gzlib_freetype my_free,void *opaque)
{
    int i;
    fwav fw;
    
    if (!my_alloc || !my_free) { my_alloc=gzlib_alloc; my_free=gzlib_free; }
    fw=(fwav)(*my_alloc)(opaque,sizeof(fwav_struct));
    fw->alloc=my_alloc;
    fw->free=my_free;
    fw->opaque=opaque;
    fw->xs=xs; fw->ys=ys; fw->zs=zs;
    fw->v=(float *)0;
    fw->thresh=thresh;
    fw->bt=(float *)(*fw->alloc)(fw->opaque,xs*ys*zs*sizeof(float));
    fw->zero=(float *)(*fw->alloc)(fw->opaque,(xs+40)*sizeof(float));
    for (i=0;i<xs+40;i++) fw->zero[i]=0.0f;
    fw->burow=(float *)(*fw->alloc)(fw->opaque,xs*sizeof(float));
    fw->bvrow=(float *)(*fw->alloc)(fw->opaque,xs*sizeof(float));
    fw->td0=fw->td1=0.0f;
    return fw;
}


void fwav_destroy(fwav fw)
{
    (*fw->free)(fw->opaque,(void *)fw->bvrow);
    (*fw->free)(fw->opaque,(void *)fw->burow);
    (*fw->free)(fw->opaque,(void *)fw->zero);
    (*fw->free)(fw->opaque,(void *)fw->bt);
    (*fw->free)(fw->opaque,(void *)fw);
}


void fwav_atran(fwav fw,float *v)
{
    int dx,dy,dz;
    float *b1,*td;

    fw->v=v;
    dx=fw->xs; dy=fw->ys; dz=fw->zs; b1=fw->v;

    
#define TD { fw->td0= -td[0]; fw->td1=td[0]; td++; }

    td=(fw->thresh?fw->thresh:fw->zero);
    while (dx>DX || dy>DY || dz>DZ) {
        TD if (dx>DX) { fwav_atran_x(fw,b1,dx,dy,dz); dx=HALF(dx); }
        TD if (dy>DY) { fwav_atran_y(fw,b1,dx,dy,dz); dy=HALF(dy); }
        TD if (dz>DZ) { fwav_atran_z(fw,b1,dx,dy,dz); dz=HALF(dz); }
    }
}


void fwav_stran(fwav fw,float *v)
{
    fw->v=v;
    fwav_stran_xyz(fw,v,fw->xs,fw->ys,fw->zs);
}



/* ========== internal routines ========== */


void fwav_atran_x(fwav fw,float *ba0,int dx,int dy,int dz)
{
    int j,dx2,y,z;
    float a0,a1,a2,v0,v1;
    float *ba,*bu,*bv,td0,td1;

    td0=fw->td0; td1=fw->td1;
    dx2=HALF(dx); bu=ba0; ba=ba0; bv=fw->bt;
    for (z=0;z<dz;z++) {
        for (y=0;y<dy;y++) {
            v1=0.0; a2= *ba++;
            for (j=dx2-1;j--;) {
                a0=a2; v0=v1; a1= *ba++; a2= *ba++;
                v1=a1-0.5*(a0+a2); THRESH(v1) *bv++ =v1;
                *bu++ =a0+0.25*(v0+v1);
            }
            v0=v1;
            if (dx&1) v1=0.0;
            else { a1= *ba++; v1=a1-0.5*(a2+a2); THRESH(v1) *bv++ =v1; }
            *bu++ =a2+0.25*(v0+v1);
        }
    }
    j=bv-fw->bt; bv=fw->bt; while (j--) *bu++ = *bv++;
}


void fwav_atran_y(fwav fw,float *b0,int dx,int dy,int dz)
{
    int j,y,z;
    float *ba0,*ba1,*ba2,*bb,v1,td0,td1;

    td0=fw->td0; td1=fw->td1;
    bb=fw->bt;
    for (z=0;z<dz;z++) {
        for (y=0;y+1<dy;y+=2) {
            ba0=b0+(z*dy+y)*dx;
            ba1=ba0+dx;
            ba2=(y+2<dy?ba1+dx:ba0);
            for (j=dx;j--;)
                { v1=(*ba1++)-0.5*((*ba0++)+(*ba2++)); THRESH(v1) *bb++ =v1; }
        }
    }
    bb=b0;
    for (z=0;z<dz;z++) {
        for (y=0;y<dy;y+=2) {
            ba1=b0+(z*dy+y)*dx;
            ba2=(y+1<dy?fw->bt+(z*(dy>>1)+(y>>1))*dx:fw->zero);
            ba0=(y?fw->bt+(z*(dy>>1)+(y>>1)-1)*dx:fw->zero);
            for (j=dx;j--;) *bb++ =(*ba1++)+0.25*((*ba0++)+(*ba2++));
        }
    }
    j=dx*dy*dz-(bb-b0); ba0=fw->bt; while (j--) *bb++ = *ba0++;
}


void fwav_atran_z(fwav fw,float *b0,int dx,int dy,int dz)
{
    int j,y,z,dd;
    float *ba0,*ba1,*ba2,*bb,v1,td0,td1;

    td0=fw->td0; td1=fw->td1;
    dd=dx*dy;
    bb=fw->bt;
    for (z=0;z+1<dz;z+=2) {
        for (y=0;y<dy;y++) {
            ba0=b0+(z*dy+y)*dx;
            ba1=ba0+dd;
            ba2=(z+2<dz?ba1+dd:ba0);
            for (j=dx;j--;)
                { v1=(*ba1++)-0.5*((*ba0++)+(*ba2++)); THRESH(v1) *bb++ =v1; }
        }
    }
    bb=b0;
    for (z=0;z<dz;z+=2) {
        for (y=0;y<dy;y++) {
            ba1=b0+(z*dy+y)*dx;
            ba2=(z+1<dz?fw->bt+((z>>1)*dy+y)*dx:fw->zero);
            ba0=(z?fw->bt+(((z>>1)-1)*dy+y)*dx:fw->zero);
            for (j=dx;j--;) *bb++ =(*ba1++)+0.25*((*ba0++)+(*ba2++));
        }
    }
    j=dx*dy*dz-(bb-b0); ba0=fw->bt; while (j--) *bb++ = *ba0++;
}


void fwav_stran_xyz(fwav fw,float *b1,int dx,int dy,int dz)
{
    int dx0,dy0,dz0;

    if (dx<=DX && dy<=DY && dz<=DZ) return;

    dx0=dx; dy0=dy; dz0=dz;
    if (dx>DX) dx=HALF(dx); if (dy>DY) dy=HALF(dy); if (dz>DZ) dz=HALF(dz);
    fwav_stran_xyz(fw,b1,dx,dy,dz);
    if (dz0>DZ) fwav_stran_z(fw,b1,dx ,dy ,dz0);
    if (dy0>DY) fwav_stran_y(fw,b1,dx ,dy0,dz0);
    if (dx0>DX) fwav_stran_x(fw,b1,dx0,dy0,dz0);
}


void fwav_stran_x(fwav fw,float *b1,int dx,int dy,int dz)
{
    int j,dx2,dxv,y,z,d;
    float a0,a2,v0,v1;
    float *ba,*bu,*bv;

    dx2=HALF(dx); d=dz*dy*dx2; dxv=dx>>1;
    bu=fw->bt; ba=b1; for (j=d;j--;) *bu++ = *ba++;
    for (z=0;z<dz;z++) {
        for (y=0;y<dy;y++) {
            ba=fw->bt+(z*dy+y)*dx2; bu=fw->burow;
            for (j=dx2;j--;) *bu++ = *ba++;
            ba=b1+(d+(z*dy+y)*dxv); bv=fw->bvrow;
            for (j=dxv;j--;) *bv++ = *ba++;

            ba=b1+(z*dy+y)*dx; bu=fw->burow; bv=fw->bvrow;
            v0=0.0; v1= *bv++;
            *ba++ =a2=(*bu++)-0.25*(v0+v1);
            for (j=dxv-1;j--;) {
                a0=a2; v0=v1;
                v1= *bv++; a2=(*bu++)-0.25*(v0+v1);
                *ba++ =0.5*(a0+a2)+v0;
                *ba++ =a2;
            }
            if (dx&1) {
                a0=a2; v0=v1;
                v1=0.0; a2=(*bu++)-0.25*(v0+v1);
                *ba++ =0.5*(a0+a2)+v0;
                *ba++ =a2;
            }else { *ba++ =0.5*(a2+a2)+v1; }
        }
    }
}


void fwav_stran_y(fwav fw,float *b1,int dx,int dy,int dz)
{
    int j,dy2,y,z,d,dyv,y2;
    float *ba0,*ba1,*ba2,*bb;

    dy2=HALF(dy); d=dx*dy2*dz; dyv=dy>>1;
    bb=fw->bt; ba1=b1; for (j=dz*dy*dx;j--;) *bb++ = *ba1++;
    for (z=0;z<dz;z++) {
        for (y=0;y<dy;y+=2) {
            bb=b1+(z*dy+y)*dx;
            y2=y>>1;
            ba1=fw->bt+(z*dy2+y2)*dx;
            ba2=(y2>=dyv?fw->zero:fw->bt+(d+(z*dyv+y2)*dx));
            ba0=(y?fw->bt+(d+(z*dyv+y2-1)*dx):fw->zero);
            for (j=dx;j--;) *bb++ =(*ba1++)-0.25*((*ba0++)+(*ba2++));
        }
    }
    for (z=0;z<dz;z++) {
        for (y=0;y+1<dy;y+=2) {
            bb=fw->bt+(d+(z*dyv+(y>>1))*dx);
            ba0=b1+(z*dy+y)*dx;
            ba1=ba0+dx;
            ba2=(y+2<dy?ba1+dx:ba0);
            for (j=dx;j--;) *ba1++ =(*bb++)+0.5*((*ba0++)+(*ba2++));
        }
    }
}


void fwav_stran_z(fwav fw,float *b1,int dx,int dy,int dz)
{
    int j,dz2,y,z,dd,d,dzv,z2;
    float *ba0,*ba1,*ba2,*bb;

    dz2=HALF(dz); dd=dx*dy; d=dx*dy*dz2; dzv=dz>>1;
    bb=fw->bt; ba1=b1; for (j=dz*dy*dx;j--;) *bb++ = *ba1++;
    for (z=0;z<dz;z+=2) {
        for (y=0;y<dy;y++) {
            z2=z>>1;
            bb=b1+(z*dy+y)*dx;
            ba1=fw->bt+(z2*dy+y)*dx;
            ba2=(z2>=dzv?fw->zero:fw->bt+(d+(z2*dy+y)*dx));
            ba0=(z?fw->bt+(d+((z2-1)*dy+y)*dx):fw->zero);
            for (j=dx;j--;) *bb++ =(*ba1++)-0.25*((*ba0++)+(*ba2++));
        }
    }
    for (z=0;z+1<dz;z+=2) {
        for (y=0;y<dy;y++) {
            bb=fw->bt+(d+((z>>1)*dy+y)*dx);
            ba0=b1+(z*dy+y)*dx;
            ba1=ba0+dd;
            ba2=(z+2<dz?ba1+dd:ba0);
            for (j=dx;j--;) *ba1++ =(*bb++)+0.5*((*ba0++)+(*ba2++));
        }
    }
}


