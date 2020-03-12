# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

"""
 file: pyocl_env.py
 author: Cyrus Harrison <cyrush@llnl.gov>
 created: 4/21/2012
 description:
    Use lazy created singleton pyopencl context, to get around
    issues with the NVIDIA driver on edge.

"""

# Guarded import of pyopencl
found_pyopencl = False
try:
    import numpy as npy
    import pyopencl as cl
    found_pyopencl = True
except ImportError:
    pass

__all__ = ["Manager",
           "Pool"]

from ..core import WallTimer, log


def info(msg):
    log.info(msg,"pyocl_env")

def err(msg):
    log.error(msg,"pyocl_env")

def calc_nbytes(shape,dtype):
    res = npy.dtype(dtype).itemsize
    for s in shape:
        res*=s
    return res

def nbytes_mb_gb(nbytes):
    mbytes = float(nbytes) * 9.53674e-7
    gbytes = mbytes * 0.000976562
    return mbytes, gbytes

def nbytes_str(nbytes):
    mbytes, gbytes = nbytes_mb_gb(nbytes)
    return "%d (MB: %s GB: %s)"  % (nbytes,repr(mbytes),repr(gbytes))

class PyOpenCLBuffer(object):
    def __init__(self,id,shape,dtype):
        self.id        = id
        self.shape     = shape
        self.out_shape = shape
        self.dtype     = dtype
        self.nbytes    = calc_nbytes(shape,dtype)
        ctx = PyOpenCLContextManager.context()
        balloc = PyOpenCLHostTimer("balloc",self.nbytes)
        balloc.start()
        self.cl_obj   = cl.Buffer(ctx, cl.mem_flags.READ_WRITE, self.nbytes)
        balloc.stop()
        PyOpenCLContextManager.add_host_event(balloc)
        self.__active = 2
        info("PyOpenCLBuffer create: " + str(self))
    def write(self,data):
        nbytes = calc_nbytes(data.shape,data.dtype)
        info("PyOpenCLBuffer write %s bytes to %s"  % (nbytes,str(self)))
        evnt = cl.enqueue_copy(PyOpenCLContextManager.queue(),self.cl_obj,data)
        PyOpenCLContextManager.add_event("win",evnt,nbytes)
        return evnt
    def read(self):
        nbytes = calc_nbytes(self.out_shape,self.dtype)
        info("PyOpenCLBuffer read %d bytes from %s " % (nbytes,str(self)))
        htimer = PyOpenCLHostTimer("ralloc",self.nbytes)
        htimer.start()
        res = npy.zeros(self.out_shape,dtype=self.dtype)
        htimer.stop()
        PyOpenCLContextManager.add_host_event(htimer)
        # this is blocking ...
        evnt = cl.enqueue_copy(PyOpenCLContextManager.queue(),res,self.cl_obj)
        PyOpenCLContextManager.add_event("rout",evnt,nbytes)
        evnt.wait()
        return res
    def active(self):
        return self.__active == 2
    def reactivate(self,out_shape,dtype):
        self.out_shape = out_shape
        self.dtype     = dtype
        self.__active  = 2
    def released(self):
        return self.__active == 1
    def release(self):
        info("PyOpenCLBuffer release: " + str(self))
        self.__active = 1
    def reclaim(self):
        self.__active = 0
    def available(self):
        return self.__active == 0
    def __str__(self):
        res = "(%d) dtype: %s, nbytes: %s, alloc_shape: %s, out_shape: %s status:%d"
        res = res % (self.id,self.dtype,self.nbytes,self.shape,self.out_shape,self.__active)
        return res


class PyOpenCLBufferPool(object):
    buffers     = []
    total_alloc = 0
    @classmethod
    def reset(cls):
        rset = PyOpenCLHostTimer("pool_reset",0)
        rset.start()
        # this should trigger pyopencl cleanup of buffers
        cls.buffers     = []
        cls.total_alloc = 0
        cls.max_alloc   = 0
        rset.stop()
        PyOpenCLContextManager.add_host_event(rset)
    @classmethod
    def available_device_memory(cls,percentage=False):
        devm = PyOpenCLContextManager.device_memory()
        res  = devm - cls.total_alloc
        if percentage:
            res = round(100.0 * (float(res) / float(devm)),2)
        return res
    @classmethod
    def device_memory_high_water(cls):
        return cls.max_alloc
    @classmethod
    def request_buffer(cls,shape,dtype):
        avail   = [b for b in cls.buffers if b.available()]
        rbytes  = calc_nbytes(shape,dtype)
        res_buf = None
        for b in avail:
            # first check for exact buffer size match
            if b.nbytes == rbytes:
                # we can reuse
                dreuse = PyOpenCLHostTimer("dreuse",b.nbytes)
                dreuse.start()
                info("PyOpenCLBufferPool reuse: " + str(b))
                b.reactivate(shape,dtype)
                res_buf = b
                dreuse.stop()
                PyOpenCLContextManager.add_host_event(dreuse)
                break
        if res_buf is None:
            res_buf = cls.__create_buffer(shape,dtype)
        return res_buf
    @classmethod
    def buffer_info(cls):
        res  = "Total Device Memory: %s\n" % nbytes_str(PyOpenCLContextManager.device_memory())
        res += "Available Memory:   %s "  % nbytes_str(cls.available_device_memory())
        res += "(" + repr(cls.available_device_memory(True)) + " %)\n"
        res += "Buffers:\n"
        for b in cls.buffers:
            res += " " + str(b) + "\n"
        return res
    @classmethod
    def reclaim(cls):
        #if released(), the buffer is avail for the next request
        for b in cls.buffers:
            if b.released():
                b.reclaim()
    @classmethod
    def release_buffer(cls,buff):
        drel = PyOpenCLHostTimer("drelease",buff.nbytes)
        drel.start()
        cls.total_alloc -= buff.nbytes
        cls.buffers.remove(buff)
        drel.stop()
        PyOpenCLContextManager.add_host_event(drel)
    @classmethod
    def __create_buffer(cls,shape,dtype):
        # no suitable buffer, we need to create a new one
        rbytes = calc_nbytes(shape,dtype)
        # see if we have enough bytes left on the device
        # if not, try to  reclaim some memory from released buffers
        # if rbytes > cls.available_device_memory():
        cls.__reap(rbytes)
        if rbytes > cls.available_device_memory():
            msg  = "Reap failed\n"
            msg += " Free Request:       %s\n" % nbytes_str(rbytes)
            msg += PyOpenCLContextManager.events_summary()[0] + "\n"
            msg += cls.buffer_info() + "\n"
            err(msg)
            raise MemoryError
        res = PyOpenCLBuffer(len(cls.buffers),shape,dtype)
        cls.total_alloc += res.nbytes
        if cls.total_alloc > cls.max_alloc:
            cls.max_alloc = cls.total_alloc
        cls.buffers.append(res)
        info(cls.buffer_info())
        return res
    @classmethod
    def __reap(cls,nbytes):
        rbytes = 0
        avail  = [b for b in cls.buffers if b.available()]
        for b in avail:
            rbytes += b.nbytes
            cls.release_buffer(b)
            if cls.available_device_memory() >= nbytes:
                # we have enough mem, so break
                break
        del avail
        msg  = "PyOpenCLBufferPool reap reclaimed: "
        msg += nbytes_str(rbytes)
        info(msg)


class PyOpenCLContextEvent(object):
    def __init__(self,tag,cl_evnt,nbytes):
        self.etype   = "device"
        self.tag     = tag
        self.cl_evnt = cl_evnt
        self.nbytes  = nbytes
    def summary(self):
        qts = 1e-9*(self.cl_evnt.profile.submit - self.cl_evnt.profile.queued)
        sts = 1e-9*(self.cl_evnt.profile.start  - self.cl_evnt.profile.submit)
        ste = 1e-9*(self.cl_evnt.profile.end    - self.cl_evnt.profile.start)
        qte = 1e-9*(self.cl_evnt.profile.end    - self.cl_evnt.profile.queued)
        res = "Device Event: %s (nbytes=%d)\n" % (self.tag,self.nbytes)
        res += "  Queued to Submit: %s\n" % repr(qts)
        res += "  Submit to Start:  %s\n" % repr(sts)
        res += "  Start  to End:    %s\n" % repr(ste)
        res += " Queued to End:     %s\n" % repr(qte)
        return res
    def queued_to_end(self):
        return 1e-9*(self.cl_evnt.profile.end - self.cl_evnt.profile.queued)
    def start_to_end(self):
        return 1e-9*(self.cl_evnt.profile.end - self.cl_evnt.profile.start)


class PyOpenCLHostTimer(WallTimer):
    def __init__(self,tag,nbytes):
        super(PyOpenCLHostTimer,self).__init__(tag)
        self.etype   = "host"
        self.nbytes  = nbytes
    def summary(self):
        res = "Host Event: %s (nbytes=%d)\n" % (self.tag,self.nbytes)
        res += "  Start  to End:    %s\n" % repr(self.start_to_end())
        return res
    def queued_to_end(self):
        return self.get_elapsed()
    def start_to_end(self):
        return self.get_elapsed()


class PyOpenCLContextManager(object):
    plat_id  = 0
    dev_id   = 0
    ctx      = None
    ctx_info = None
    device   = None
    cmdq     = None
    events   = []
    @classmethod
    def select_device(cls,platform_id,device_id):
        cls.plat_id = platform_id
        cls.dev_id  = device_id
    @classmethod
    def queue(cls):
        if cls.cmdq is None:
            ctx = cls.context()
            prof = cl.command_queue_properties.PROFILING_ENABLE
            cls.cmdq = cl.CommandQueue(ctx,properties=prof)
        return cls.cmdq
    @classmethod
    def instance(cls):
        return cls.context()
    @classmethod
    def context(cls):
        if not found_pyopencl:
            return None
        if cls.ctx is None:
            csetup = PyOpenCLHostTimer("ctx_setup",0)
            csetup .start()
            platform = cl.get_platforms()[cls.plat_id]
            device = platform.get_devices()[cls.dev_id]
            cinfo  = "OpenCL Context Info\n"
            cinfo += " Using platform id = %d\n" % cls.plat_id
            cinfo += "  Platform name: %s\n" % platform.name
            cinfo += "  Platform profile: %s\n" % platform.profile
            cinfo += "  Platform vendor: %s\n" % platform.vendor
            cinfo += "  Platform version: %s\n" % platform.version
            cinfo += " Using device id = %d\n" % cls.dev_id
            cinfo += "  Device name: %s\n" % device.name
            cinfo += "  Device type: %s\n" % cl.device_type.to_string(device.type)
            cinfo += "  Device memory: %s\n" % device.global_mem_size
            cinfo += "  Device max clock speed: %s MHz\n" % device.max_clock_frequency
            cinfo += "  Device compute units: %s\n" % device.max_compute_units
            info(cinfo)
            cls.device = device
            cls.ctx = cl.Context([device])
            cls.ctx_info = cinfo
            csetup.stop()
            PyOpenCLContextManager.add_host_event(csetup)
        return cls.ctx
    @classmethod
    def dispatch_kernel(cls,kernel_source,out_shape,buffers):
        kdisp = PyOpenCLHostTimer("kdispatch",0)
        kdisp.start()
        ibuffs = [ b.cl_obj for b in buffers]
        prg    = cl.Program(cls.context(),kernel_source).build()
        evnt   = prg.kmain(cls.queue(), out_shape, None, *ibuffs)
        cls.add_event("kexec",evnt)
        kdisp.stop()
        PyOpenCLContextManager.add_host_event(kdisp)
        return evnt
    @classmethod
    def device_memory(cls):
        cls.context()
        return cls.device.global_mem_size
    @classmethod
    def clear_events(cls):
        cls.events = []
    @classmethod
    def add_event(cls,tag,cl_evnt,nbytes=0):
        cls.events.append(PyOpenCLContextEvent(tag,cl_evnt,nbytes))
    @classmethod
    def add_host_event(cls,host_timer):
        cls.events.append(host_timer)
    @classmethod
    def events_summary(cls):
        res      = ""
        tbytes   = 0
        ttag     = {}
        tqte     = 0.0
        tste     = 0.0
        tnevents = len(cls.events)
        maxalloc = PyOpenCLBufferPool.device_memory_high_water()
        for e in cls.events:
            tbytes += e.nbytes
            tqte   += e.queued_to_end()
            tste   += e.start_to_end()
            if e.tag in list(ttag.keys()):
                t = ttag[e.tag]
                t["nevents"] += 1
                t["nbytes"] += e.nbytes
                t["qte"]    += e.queued_to_end()
                t["ste"]    += e.start_to_end()
            else:
                ttag[e.tag] = {"tag": e.tag,
                               "etype": e.etype,
                               "nevents":1,
                               "nbytes":e.nbytes,
                               "qte":e.queued_to_end(),
                               "ste":e.start_to_end()}
        tmbytes, tgbytes = nbytes_mb_gb(tbytes)
        res += cls.ctx_info
        res += "\nTag Totals:\n"
        for k,v in list(ttag.items()):
            nevents = v["nevents"]
            etype   = v["etype"]
            nbytes  = v["nbytes"]
            qte     = v["qte"]
            ste     = v["ste"]
            nmbytes, ngbytes = nbytes_mb_gb(nbytes)
            avg_bytes = nbytes / float(nevents)
            avg_mbytes, avg_gbytes =  nbytes_mb_gb(avg_bytes)
            gbps    = ngbytes / ste
            v["avg_bytes"]  = avg_bytes
            v["gbps"]       = gbps
            res += " Tag: %s (%s)\n" % (k ,etype)
            res += "  Total # of events: %d\n" % nevents
            res += "  Total queued to end: %s (s)\n" % repr(qte)
            res += "  Total start  to end: %s (s)\n" % repr(ste)
            res += "  Total nbytes: %s\n" % nbytes_str(nbytes)
            res += "  Total gb/s: %s [ngbytes / ste]\n" % repr(gbps)
            res += "  Average nbytes: %s\n" % nbytes_str(avg_bytes)
            res += "%s\n" % v
        res += "Total # of events: %d\n" % tnevents
        res += "Total nbytes: %s\n" % nbytes_str(tbytes)
        res += "Total start to end:  %s (s)\n" % repr(tqte)
        res += "Total queued to end: %s (s)\n" % repr(tste)
        res += "Dev max alloc: %s \n" % nbytes_str(maxalloc)
        ttag["total"] = {"tag":"total",
                         "etype":"total",
                         "nevents": tnevents,
                         "nbytes":  tbytes,
                         "qte":     tqte,
                         "ste":     tste,
                         "dev_max_alloc": maxalloc}
        res += "%s\n" % ttag["total"]
        return res, ttag


Manager = PyOpenCLContextManager
Pool    = PyOpenCLBufferPool
