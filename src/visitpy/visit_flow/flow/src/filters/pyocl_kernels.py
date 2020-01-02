# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: pyocl_kernels.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 9/6/2012
 description:

"""

# Guarded import of pyopencl
found_pyopencl = False
try:
    import numpy as npy
    import pyopencl as cl
    found_pyopencl = True
except ImportError:
    pass


Kernels = {
### add
"kadd" : {"name":  "add",
          "input_ports": ["in_a","in_b"],
          "src":"""
            float kadd(const float a,const float b)
            {return a + b;}
            """},
### sub
"ksub" : {"name":  "sub",
          "input_ports": ["in_a","in_b"],
          "src":"""
            float ksub(const float a,const float b)
            {return a - b;}
            """},

### mult
"kmult": {"name":  "mult",
          "input_ports": ["in_a","in_b"],
          "src":"""
            float kmult(const float a,const float b)
            {return a * b;}
            """},

### div
"kdiv": {"name":  "div",
          "input_ports": ["in_a","in_b"],
          "src":"""
            float kdiv(const float a,const float b)
            {return a / b;}
            """},
### mod
"kmod": {"name":  "mod",
          "input_ports": ["in_a","in_b"],
          "src":"""
            float kmod(const float a, const float b)
            {return a % b;}
            """},
### cos
"kcos" : {"name":  "cos",
          "input_ports": ["in"],
          "src":"""
            float kcos(const float a)
            {return cos(a);}
            """},
### sin
"ksin": {"name":  "sin",
          "input_ports": ["in"],
          "src":"""
            float ksin(const float a)
            {return sin(a);}
            """},
### sin
"ktan": {"name":  "tan",
         "input_ports": ["in"],
         "src": """
            float ktan(const float a)
            {return tan(a);}
            """},
### ciel
"kciel": {"name":  "ciel",
         "input_ports": ["in"],
         "src": """
            float kceil(const float a)
            {return ceil(a);}
            """},
### floor
"kfloor": {"name":  "floor",
           "input_ports": ["in"],
           "src":"""
            float kfloor(const float a)
            {return floor(a);}
            """},
### abs
"kabs": {"name":  "abs",
          "input_ports": ["in"],
          "src":"""
            float kabs(const float a)
            {return abs(a);}
            """},
### log10
"klog10": {"name":  "log10",
          "input_ports": ["in"],
          "src":"""
            float klog10(const float a)
            {return log10(a);}
            """},
### log
"klog": {"name":  "log",
         "input_ports": ["in"],
         "src":"""
            float klog10(const float a)
            {return log10(a);}
            """},
### exp
"kexp": {"name":  "exp",
         "input_ports": ["in"],
         "src":"""
            float kexp(const float a)
            {return exp(a);}
            """},
### pow
"kpow": {"name":  "pow",
         "input_ports": ["in"],
         "src":"""
            float kpow(const float a, const float b)
            {return kpow(a, b);}
            """},
### id
"kid":  {"name":  "id",
         "input_ports": ["in"],
         "src":"""
            float kid(const float a)
            {return a;}
            """},
### square
"ksquare":  {"name":  "square",
             "input_ports": ["in"],
             "src": """
            float ksquare(const float a)
            {return a*a;}
            """},
### sqrt
"ksqrt": {"name":  "sqrt",
          "input_ports": ["in"],
          "src":"""
            float ksqrt(const float a)
            {return sqrt(a);}
            """},
### curl3d
"curl3d":  {"name":  "sqrt",
            "input_ports": ["dfx","dfy","dfx"],
            "in_types": ["direct",
                         "direct",
                         "direct"],
            "out_type": "float4",
            "src":"""
            float4 kcurl3d(__global const float *dfx,
                           __global const float *dfy,
                           __global const float *dfz
            {
              int gid = get_global_id(0);

              float dfzdy = dfz[gid*3+1];
              float dfydz = dfy[gid*3+2];

              float dfxdz = dfx[gid*3+2];
              float dfzdx = dfz[gid*3];

              float dfydx = dfy[gid*3];
              float dfxdy = dfx[gid*3+1];
              float4 res;
              res.x  = dfzdy - dfydz;
              res.y  = dfxdz - dfzdx;
              res.z  = dfydx - dfxdy;
            }
            """},
### grad3d
"kgrad3d": {"name":  "grad3d",
            "input_ports": ["in","dims","x","y","z"],
            "in_types": ["direct",
                         "direct",
                         "direct",
                         "direct",
                         "direct"],
            "out_type": "float4",
            "src":"""
            float4 kgrad3d(__global const float *v,
                           __global const int   *d,
                           __global const float *x,
                           __global const float *y,
                           __global const float *z)
            {
                int gid = get_global_id(0);

                int di = d[0]-1;
                int dj = d[1]-1;
                int dk = d[2]-1;

                int zi = gid % di;
                int zj = (gid / di) % dj;
                int zk = (gid / di) / dj;

                // for rectilinear, we only need 2 points to get dx,dy,dz
                int pi0 = zi + zj*(di+1) + zk*(di+1)*(dj+1);
                int pi1 = zi + 1 + (zj+1)*(di+1) + (zk+1)*(di+1)*(dj+1);

                float vv = v[gid];
                float4 p_0 = (float4)(x[pi0],y[pi0],z[pi0],1.0);
                float4 p_1 = (float4)(x[pi1],y[pi1],z[pi1],1.0);
                float4 dg  = p_1 - p_0;

                // value
                float4 f_0 = (float4)(vv,vv,vv,1.0);
                float4 f_1 = (float4)(vv,vv,vv,1.0);

                // i bounds
                if(zi > 0)
                {
                    f_0.x = v[gid-1];
                }

                if(zi < (di-1))
                {
                    f_1.x = v[gid+1];
                }

                // j bounds
                if(zj > 0)
                {
                    f_0.y = v[gid-di];
                }

                if(zj < (dj-1))
                {
                    f_1.y = v[gid+di];
                }

                // k bounds
                if(zk > 0)
                {
                    f_0.z = v[gid-(di*dj)];
                }

                if(zk < (dk-1))
                {
                    f_1.z = v[gid+(di*dj)];
                }

                float4 df = (f_1 - f_0) / dg;

                // central diff if we aren't on the edges
                if( (zi != 0) && (zi != (di-1)))
                {
                    df.x *= .5;
                }

                // central diff if we aren't on the edges
                if( (zj != 0) && (zj != (dj-1)))
                {
                    df.y *= .5;
                }

                // central diff if we aren't on the edges
                if( (zk != 0) && (zk != (dk-1)))
                {
                    df.z *= .5;
                }
                //return (float4)(1.0,2.0,3.0,0.0);
                return df;
            }
            """}
}

# fill in set defaults
for k,v in list(Kernels.items()):
    if "out_type" not in v:
        v["out_type"] = "float"
    if "in_types" not in v:
        v["in_types"] = [ "fetch" for ipt in v["input_ports"]]

def create_stub(filter,inputs):
    # gen stub glue & execute
    ident = "            "
    args_ident = "                               "
    res  = filter.kernel_source
    res += "\n%s__kernel void kmain(" % ident
    ninputs = len(inputs)
    for idx in range(ninputs):
        if isinstance(inputs[idx],float):
            itype = "float"
        elif inputs[idx].dtype == npy.int32:
            itype = "int  "
        else:
            itype = "float"
        iname = "in_%04d" % idx
        res  += "__global const %s *%s,\n%s " % (itype,iname,args_ident)
    res += "__global float *out)\n"
    res += "%s{\n" % ident
    res += "%s int gid = get_global_id(0);\n" % ident
    call_names = []
    for idx in range(ninputs):
        iname = "in_%04d" % idx
        if filter.in_types[idx] == "fetch":
            if isinstance(inputs[idx],float):
                itype = "float"
            elif inputs[idx].dtype == npy.int32:
                itype = "int  "
            else:
                itype = "float"
            cname = "%s_fetch" % iname
            res += "%s %s %s = %s[gid];\n" % (ident,itype,cname,iname)
        else:
            cname = iname
        call_names.append(cname)
    call = "k%s(" % filter.filter_type
    for cn in call_names:
        call += "%s," % cn
    call = call[:-1] + ")"
    out_dim = None
    if filter.out_type == "float":
        res += "%s out[gid] = %s;\n" % (ident,call)
    elif filter.out_type == "float4":
        res += "%s float4 res = %s;\n" % (ident,call)
        res += "%s out[gid*3]   = res.x;\n" % (ident)
        res += "%s out[gid*3+1] = res.y;\n" % (ident)
        res += "%s out[gid*3+2] = res.z;\n" % (ident)
        out_dim = 3
    res += "%s}\n" % ident
    return res, out_dim

