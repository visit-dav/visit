import os

cases = (
(1,16,     1,  8, 2, "pdebug"),
(16,256,   1, 16, 16, "pdebug"),
(32,512,   2, 16, 16, "pdebug"),
(64,1024,  2, 32, 16, "pdebug"),
(128,2048, 2, 32, 32, "psmall"),
(256,4096, 2, 64, 32, "psmall"),
(512,8192, 2, 64, 64, "psmall"),
(2048, 32768, 4, 128, 64, "pbatch"),
(4096, 65536, 4, 128, 128, "pbatch"),
(6144, 98304, 6, 128, 128, "pbatch")
)

for c in cases:
    dname = "/p/lscratchv/%s/%d_%d" % (os.getenv("USER"), c[0], c[1])
    os.mkdir(dname)
    srun = "srun -N %d -n %d -A sspwork -p %s --chdir=%s mandelbulb_par -nr 100 -ntheta 100 -nphi 100 -rdoms %d -thetadoms %d -phidoms %d" % (c[0], c[1], c[5], dname, c[2], c[3], c[4])
    print(srun)
    s =os.system(srun)
