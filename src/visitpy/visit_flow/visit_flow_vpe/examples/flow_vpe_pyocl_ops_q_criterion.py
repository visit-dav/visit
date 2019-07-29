# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: vpe_flow_pyocl_ops_q_criterion.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 5/8/2012
 description:

"""


from flow import *
from flow.filters import pyocl_ops


def setup_workspace():
    w = Workspace()
    w.register_filters(pyocl_ops)
    ctx = w.add_context("pyocl_ops","root")
    ctx.start()
    ctx.add_filter("grad","du")
    ctx.add_filter("grad","dv")
    ctx.add_filter("grad","dw")
    ctx.add_filter("const","half",{"value":.5})
    # u = x comp
    ctx.add_filter("decompose","dudx",{"index":0})
    ctx.add_filter("decompose","dudy",{"index":1})
    ctx.add_filter("decompose","dudz",{"index":2})
    # v = y comp
    ctx.add_filter("decompose","dvdx",{"index":0})
    ctx.add_filter("decompose","dvdy",{"index":1})
    ctx.add_filter("decompose","dvdz",{"index":2})
    # w = z comp
    ctx.add_filter("decompose","dwdx",{"index":0})
    ctx.add_filter("decompose","dwdy",{"index":1})
    ctx.add_filter("decompose","dwdz",{"index":2})
    #
    idx =0
    for i in [1,2,3,5,6,7]:
        ctx.add_filter("add","s_add_%d" % i)
        ctx.add_filter("mult","s_%d" % i)
        # #
        ctx.add_filter("sub","w_sub_%d" % i)
        ctx.add_filter("mult","w_%d" % i)
        #
        ctx.add_filter("mult","s_sq_%d" % i)
        #
        ctx.add_filter("mult","w_sq_%d" % i)
        ctx.add_filter("add","s_norm_%d" % i)
        ctx.add_filter("add","w_norm_%d" % idx)
        idx +=1
    ctx.add_filter("mult","s_sq_0")
    ctx.add_filter("add","s_norm_0")
    ctx.add_filter("mult","s_sq_4")
    ctx.add_filter("add","s_norm_4")
    ctx.add_filter("mult","s_sq_8")
    ctx.add_filter("sub","final_sub")
    ctx.add_filter("mult","final_mult")
    ctx.connect(":x","du:x")
    ctx.connect(":y","du:y")
    ctx.connect(":z","du:z")
    ctx.connect(":dims","du:dims")
    ctx.connect(":vx","du:in")
    ctx.connect(":x","dv:x")
    ctx.connect(":y","dv:y")
    ctx.connect(":z","dv:z")
    ctx.connect(":dims","dv:dims")
    ctx.connect(":vy","dv:in")
    ctx.connect(":x","dw:x")
    ctx.connect(":y","dw:y")
    ctx.connect(":z","dw:z")
    ctx.connect(":dims","dw:dims")
    ctx.connect(":vz","dw:in")
    ctx.connect("du","dudx:in")
    ctx.connect("du","dudy:in")
    ctx.connect("du","dudz:in")
    ctx.connect("dv","dvdx:in")
    ctx.connect("dv","dvdy:in")
    ctx.connect("dv","dvdz:in")
    ctx.connect("dw","dwdx:in")
    ctx.connect("dw","dwdy:in")
    ctx.connect("dw","dwdz:in")
    # float S_mat[9];
    # S_mat[0] = du.x;
    # S_mat[1] = 0.5 * (du.y + dv.x);
    # S_mat[2] = 0.5 * (du.z + dw.x);
    # S_mat[3] = 0.5 * (dv.x + du.y);
    # S_mat[4] = dv.y;
    # S_mat[5] = 0.5 * (dv.z + dw.y);
    # S_mat[6] = 0.5 * (dw.x + du.z);
    # S_mat[7] = 0.5 * (dw.y + dv.z);
    # S_mat[8] = dw.z;
    #
    #
    ctx.connect("dudx","s_sq_0:in_a")
    ctx.connect("dudx","s_sq_0:in_b")
    #
    ctx.connect("dudy","s_add_1:in_a")
    ctx.connect("dvdx","s_add_1:in_b")
    ctx.connect("s_add_1","s_1:in_a")
    ctx.connect("half","s_1:in_b")
    ctx.connect("s_1","s_sq_1:in_a")
    ctx.connect("s_1","s_sq_1:in_b")
    #
    ctx.connect("dudz","s_add_2:in_a")
    ctx.connect("dwdx","s_add_2:in_b")
    ctx.connect("s_add_2","s_2:in_a")
    ctx.connect("half","s_2:in_b")
    ctx.connect("s_2","s_sq_2:in_a")
    ctx.connect("s_2","s_sq_2:in_b")
    # #
    # #
    ctx.connect("dvdx","s_add_3:in_a")
    ctx.connect("dudy","s_add_3:in_b")
    ctx.connect("s_add_3","s_3:in_a")
    ctx.connect("half","s_3:in_b")
    ctx.connect("s_3","s_sq_3:in_a")
    ctx.connect("s_3","s_sq_3:in_b")
    #
    ctx.connect("dvdy","s_sq_4:in_a")
    ctx.connect("dvdy","s_sq_4:in_b")
    #
    ctx.connect("dvdz","s_add_5:in_a")
    ctx.connect("dwdy","s_add_5:in_b")
    ctx.connect("s_add_5","s_5:in_a")
    ctx.connect("half","s_5:in_b")
    ctx.connect("s_5","s_sq_5:in_a")
    ctx.connect("s_5","s_sq_5:in_b")
    #
    #
    ctx.connect("dwdx","s_add_6:in_a")
    ctx.connect("dudz","s_add_6:in_b")
    ctx.connect("s_add_6","s_6:in_a")
    ctx.connect("half","s_6:in_b")
    ctx.connect("s_6","s_sq_6:in_a")
    ctx.connect("s_6","s_sq_6:in_b")
    #
    ctx.connect("dwdy","s_add_7:in_a")
    ctx.connect("dvdz","s_add_7:in_b")
    ctx.connect("s_add_7","s_7:in_a")
    ctx.connect("half","s_7:in_b")
    ctx.connect("s_7","s_sq_7:in_a")
    ctx.connect("s_7","s_sq_7:in_b")
    #
    ctx.connect("dwdz","s_sq_8:in_a")
    ctx.connect("dwdz","s_sq_8:in_b")
    #
    # float W_mat[9];
    # W_mat[0] = 0;
    # W_mat[1] = 0.5 * (du.y - dv.x);
    # W_mat[2] = 0.5 * (du.z - dw.x);
    # W_mat[3] = 0.5 * (dv.x - du.y);
    # W_mat[4] = 0;
    # W_mat[5] = 0.5 * (dv.z - dw.y);
    # W_mat[6] = 0.5 * (dw.x - du.z);
    # W_mat[7] = 0.5 * (dw.y - dv.z);
    # W_mat[8] = 0;
    #
    # 0
    #
    ctx.connect("dudy","w_sub_1:in_a")
    ctx.connect("dvdx","w_sub_1:in_b")
    ctx.connect("w_sub_1","w_1:in_a")
    ctx.connect("half","w_1:in_b")
    ctx.connect("w_1","w_sq_1:in_a")
    ctx.connect("w_1","w_sq_1:in_b")
    #
    ctx.connect("dudz","w_sub_2:in_a")
    ctx.connect("dwdx","w_sub_2:in_b")
    ctx.connect("w_sub_2","w_2:in_a")
    ctx.connect("half","w_2:in_b")
    ctx.connect("w_2","w_sq_2:in_a")
    ctx.connect("w_2","w_sq_2:in_b")
    #
    #
    ctx.connect("dvdx","w_sub_3:in_a")
    ctx.connect("dudy","w_sub_3:in_b")
    ctx.connect("w_sub_3","w_3:in_a")
    ctx.connect("half","w_3:in_b")
    ctx.connect("w_3","w_sq_3:in_a")
    ctx.connect("w_3","w_sq_3:in_b")
    #
    # 0
    #
    ctx.connect("dvdz","w_sub_5:in_a")
    ctx.connect("dwdy","w_sub_5:in_b")
    ctx.connect("w_sub_5","w_5:in_a")
    ctx.connect("half","w_5:in_b")
    ctx.connect("w_5","w_sq_5:in_a")
    ctx.connect("w_5","w_sq_5:in_b")
    #
    #
    ctx.connect("dwdx","w_sub_6:in_a")
    ctx.connect("dudz","w_sub_6:in_b")
    ctx.connect("w_sub_6","w_6:in_a")
    ctx.connect("half","w_6:in_b")
    ctx.connect("w_6","w_sq_6:in_a")
    ctx.connect("w_6","w_sq_6:in_b")
    #
    ctx.connect("dwdy","w_sub_7:in_a")
    ctx.connect("dvdz","w_sub_7:in_b")
    ctx.connect("w_sub_7","w_7:in_a")
    ctx.connect("half","w_7:in_b")
    ctx.connect("w_7","w_sq_7:in_a")
    ctx.connect("w_7","w_sq_7:in_b")
    # #
    # 0
    #
    # // Frobenius norm
    # float S_sum = 0, W_sum = 0;
    # for (int i = 0; i < 9; ++i) {
    #     S_sum += S_mat[i] * S_mat[i];
    #     W_sum += W_mat[i] * W_mat[i];
    # }
    ctx.connect("s_sq_0","s_norm_0:in_a")
    ctx.connect("s_sq_1","s_norm_0:in_b")
    # #
    ctx.connect("s_norm_0","s_norm_1:in_a")
    ctx.connect("s_sq_2","s_norm_1:in_b")
    #
    ctx.connect("s_norm_1","s_norm_2:in_a")
    ctx.connect("s_sq_3","s_norm_2:in_b")
    #
    ctx.connect("s_norm_2","s_norm_3:in_a")
    ctx.connect("s_sq_4","s_norm_3:in_b")
    #
    ctx.connect("s_norm_3","s_norm_4:in_a")
    ctx.connect("s_sq_5","s_norm_4:in_b")
    #
    ctx.connect("s_norm_4","s_norm_5:in_a")
    ctx.connect("s_sq_6","s_norm_5:in_b")
    #
    ctx.connect("s_norm_5","s_norm_6:in_a")
    ctx.connect("s_sq_7","s_norm_6:in_b")
    #
    ctx.connect("s_norm_6","s_norm_7:in_a")
    ctx.connect("s_sq_8","s_norm_7:in_b")
    ##
    ##
    ##
    ctx.connect("w_sq_1","w_norm_0:in_a")
    ctx.connect("w_sq_2","w_norm_0:in_b")
    #
    ctx.connect("w_norm_0","w_norm_1:in_a")
    ctx.connect("w_sq_3","w_norm_1:in_b")
    #
    ctx.connect("w_norm_1","w_norm_2:in_a")
    ctx.connect("w_sq_5","w_norm_2:in_b")
    #
    ctx.connect("w_norm_2","w_norm_3:in_a")
    ctx.connect("w_sq_6","w_norm_3:in_b")
    #
    ctx.connect("w_norm_3","w_norm_4:in_a")
    ctx.connect("w_sq_7","w_norm_4:in_b")
    #o[gid] = 0.5 * (W_sum - S_sum);
    ctx.connect("w_norm_4","final_sub:in_a")
    ctx.connect("s_norm_7","final_sub:in_b")
    ctx.connect("final_sub","final_mult:in_a")
    ctx.connect("half","final_mult:in_b")
    #print w.graph.save_dot("qcrit.dot")
    #print w.execution_plan()
    return w

