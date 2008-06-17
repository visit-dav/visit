c-----------------------------------------------------------------------------
c
c Copyright (c) 2000 - 2006, The Regents of the University of California
c Produced at the Lawrence Livermore National Laboratory
c LLNL-CODE-400142
c All rights reserved.
c
c This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
c full copyright notice is contained in the file COPYRIGHT located at the root
c of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
c
c Redistribution  and  use  in  source  and  binary  forms,  with  or  without
c modification, are permitted provided that the following conditions are met:
c
c  - Redistributions of  source code must  retain the above  copyright notice,
c    this list of conditions and the disclaimer below.
c  - Redistributions in binary form must reproduce the above copyright notice,
c    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
c    documentation and/or other materials provided with the distribution.
c  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
c    be used to endorse or promote products derived from this software without
c    specific prior written permission.
c
c THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
c AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
c IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
c ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
c LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
c DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
c DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
c SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
c CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
c LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
c OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
c DAMAGE.
c
c-----------------------------------------------------------------------------

c-----------------------------------------------------------------
c Program: main
c
c Programmer: Brad Whitlock
c Date:       Fri Jan 26 15:24:35 PST 2007
c
c Modifications:
c
c-----------------------------------------------------------------
      program main
      implicit none
      include "visitfortransiminterface.inc"
ccc   local variables
      integer err
      err = visitsetupenv()
      err = visitinitializesim("fupdateplots", 12,
     . "Demonstrates visitupdateplots function", 39,
     . "/no/useful/path", 15,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     . VISIT_F77NULLSTRING,VISIT_F77NULLSTRINGLEN)
      call mainloop()
      stop
      end

c-----------------------------------------------------------------
c mainloop
c-----------------------------------------------------------------
      subroutine mainloop()
      implicit none
      include "visitfortransiminterface.inc"
ccc   local variables
      integer visitstate, result, blocking
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime, simUpdate
      save /SIMSTATE/

c     main loop
      runflag = 1
      simcycle = 0
      simtime = 0.
      simUpdate = 0
      do 10
          if(runflag.eq.1) then
              blocking = 0 
          else
              blocking = 1
          endif

          visitstate = visitdetectinput(blocking, -1)

          if (visitstate.lt.0) then
              goto 1234
          elseif (visitstate.eq.0) then
              call simulate_one_timestep()
          elseif (visitstate.eq.1) then
              result = visitattemptconnection()
              if (result.eq.1) then
                  write (6,*) 'VisIt connected!'
                  simUpdate = 1
              else
                  write (6,*) 'VisIt did not connect!'
              endif
          elseif (visitstate.eq.2) then
              if (visitprocessenginecommand().eq.0) then
                  result = visitdisconnect()
                  runflag = 1
              endif
          endif
10    continue
1234  end

      subroutine simulate_one_timestep()
      implicit none
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime, simUpdate
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 50)
      parameter (NY = 50)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal
      save /RECTMESH/
c Rectilinear mesh data
      data rmndims /2/
      data rmdims /NX, NY, 1/
ccc   Local vars
      integer err

c     Simulate one time step
      simcycle = simcycle + 1
      simtime = simtime + 3.14159 / 10.
      write (6,*) 'Simulating time step: cycle=',simcycle, 
     .' time=', simtime

      if(simUpdate.eq.1) then
c         Tell VisIt that the timestep changed
          err = visittimestepchanged()
c         Tell VisIt to update its plots
          err = visitupdateplots()
      endif
      end

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c These functions must be defined to satisfy the visitfortransiminterface lib.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

c---------------------------------------------------------------------------
c visitcommandcallback
c---------------------------------------------------------------------------
      subroutine visitcommandcallback (cmd, lcmd, intdata, 
     .                                 floatdata, stringdata, 
     .                                 lstringdata)
      implicit none
      character*8 cmd, stringdata
      integer     lcmd, lstringdata, intdata
      real        floatdata
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime, simUpdate
c     Handle the commands that we define in visitgetmetadata.
      if(visitstrcmp(cmd, lcmd, "halt", 4).eq.0) then
          runflag = 0
      elseif(visitstrcmp(cmd, lcmd, "step", 4).eq.0) then
          call simulate_one_timestep()
      elseif(visitstrcmp(cmd, lcmd, "run", 3).eq.0) then
          runflag = 1
      endif
      end

c---------------------------------------------------------------------------
c visitbroadcastintfunction
c---------------------------------------------------------------------------
      integer function visitbroadcastintfunction(value, sender)
      implicit none
      integer value, sender
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      visitbroadcastintfunction = 0
      end

c---------------------------------------------------------------------------
c visitbroadcaststringfunction
c---------------------------------------------------------------------------
      integer function visitbroadcaststringfunction(str, lstr, sender)
      implicit none
      character*8 str
      integer     lstr, sender
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      visitbroadcaststringfunction = 0
      end

c---------------------------------------------------------------------------
c visitslaveprocesscallback
c---------------------------------------------------------------------------
      subroutine visitslaveprocesscallback ()
      implicit none
c     REPLACE WITH MPI COMMUNICATION IF SIMULATION IS PARALLEL
      end

c---------------------------------------------------------------------------
c visitgetmetadata
c---------------------------------------------------------------------------
      integer function visitgetmetadata(handle)
      implicit none
      integer handle
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime, simUpdate
      integer err, tdim, sdim, mesh, mt, scalar, curve, mat, e

      err = visitmdsetcycletime(handle, simcycle, simtime)
      if(runflag.eq.1) then
          err = visitmdsetrunning(handle, VISIT_SIMMODE_RUNNING)
      else
          err = visitmdsetrunning(handle, VISIT_SIMMODE_STOPPED)
      endif

c     Add a 2D rectilinear mesh
      mt = VISIT_MESHTYPE_RECTILINEAR
      tdim = 2
      sdim = 2
      mesh = visitmdmeshcreate(handle, "mesh2d", 6, mt, tdim, sdim, 1)
      if(mesh.ne.VISIT_INVALID_HANDLE) then
          err = visitmdmeshsetunits(handle, mesh, "cm", 2)
          err = visitmdmeshsetlabels(handle, mesh, "Width", 5,
     .    "Height", 6, "Depth", 5)
          err = visitmdmeshsetblocktitle(handle, mesh, "Domains", 7)
          err = visitmdmeshsetblockpiecename(handle, mesh, "domain", 6)
      endif

c     Add a zonal variable on mesh2d.
      scalar = visitmdscalarcreate(handle, "zonal", 5, "mesh2d", 6,
     . VISIT_VARCENTERING_ZONE)

c     Add a curve variable
      curve = visitmdcurvecreate(handle, "sine", 4)
      if(curve.ne.VISIT_INVALID_HANDLE) then
          err = visitmdcurvesetlabels(handle, curve, "angle", 5,
     .                                "amplitude", 9)
          err = visitmdcurvesetunits(handle, curve, "radians", 7,
     .                               VISIT_F77NULLSTRING,
     .                               VISIT_F77NULLSTRINGLEN)
      endif

c     Add some expressions
      e = visitmdexpressioncreate(handle, "zvec", 4,
     . "{zonal, zonal}", 14, VISIT_VARTYPE_VECTOR)

c     Add simulation commands
      err = visitmdaddsimcommand(handle, "halt", 4, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "step", 4, VISIT_CMDARG_NONE,
     .                           1)
      err = visitmdaddsimcommand(handle, "run", 3, VISIT_CMDARG_NONE,
     .                           1)

      visitgetmetadata = VISIT_OKAY
      end

c---------------------------------------------------------------------------
c visitgetmesh
c---------------------------------------------------------------------------
      integer function visitgetmesh(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc" 
ccc   RECTMESH common block (shared with simulate_one_timestep)
      integer NX, NY
      parameter (NX = 50)
      parameter (NY = 50)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal

ccc   local variables
      integer m, I, baseindex(3), minrealindex(3), maxrealindex(3)
      real rmz

      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "mesh2d", 6).eq.0) then
          baseindex(1) = 1
          baseindex(2) = 1
          baseindex(3) = 1
          minrealindex(1) = 0
          minrealindex(2) = 0
          minrealindex(3) = 0
          maxrealindex(1) = rmdims(1)-1
          maxrealindex(2) = rmdims(2)-1
          maxrealindex(3) = rmdims(3)-1

c Create mesh coordinates
          do 300 I=1,NX
              rmx(I) = (float(I-1)/float(NX-1)) * 5. - 2.5
300       continue
          do 310 I=1,NY
              rmy(I) = (float(I-1)/float(NY-1)) * 5. - 2.5
310       continue

c Create a rectilinear rmesh here
          m = visitmeshrectilinear(handle, baseindex, minrealindex,
     .        maxrealindex, rmdims, rmndims, rmx, rmy, rmz)
      endif
      visitgetmesh = m
      end

c---------------------------------------------------------------------------
c visitgetscalar
c---------------------------------------------------------------------------
      integer function visitgetscalar(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc"
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime, simUpdate
ccc   RECTMESH common block
      integer NX, NY
      parameter (NX = 50)
      parameter (NY = 50)
      real rmx(NX), rmy(NY), zonal(NX-1,NY-1)
      integer rmdims(3), rmndims
      common /RECTMESH/ rmdims, rmndims, rmx, rmy, zonal
ccc   local vars
      integer m, sdims(3), I, J
      real angle, xpos, ypos, cellX, cellY, dX, dY, tx, ty
      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "zonal", 5).eq.0) then
c A zonal variable has 1 less value in each dimension as there
c are nodes. Send back REAL data.
          sdims(1) = rmdims(1)-1
          sdims(2) = rmdims(2)-1
          sdims(3) = rmdims(3)-1

c Calculate a zonal variable that depends on the simulation time.
          angle = simtime
          xpos = 2.5 * cos(angle)
          ypos = 2.5 * sin(angle)
          do 5010 J=1,NY-1
              ty = float(J-1) / float(NY-2)
              cellY = (1.-ty)*(-2.5) + 2.5*ty
              dY = cellY - ypos
              do 5000 I=1,NX-1
                  tx = float(I-1) / float(NX-2)
                  cellX = (1.-tx)*(-2.5) + 2.5*tx
                  dX = cellX - xpos
                  zonal(I,J) = sqrt(dX * dX + dY * dY)
5000          continue
5010      continue

          m = visitscalarsetdataf(handle, zonal, sdims, rmndims)
      endif

      visitgetscalar = m
      end


c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(handle, name, lname)
      implicit none
      character*8 name
      integer     handle, lname
ccc   SIMSTATE common block
      integer runFlag, simcycle, simUpdate
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime, simUpdate
ccc   Local variables
      integer     m, NPTS, i
      parameter   (NPTS = 200)
      real        x(NPTS), y(NPTS), t, angle
      include "visitfortransiminterface.inc"
      m = VISIT_ERROR
      if(visitstrcmp(name, lname, "sine", 4).eq.0) then
          do 10000 i=1,NPTS
              t = float(i-1) / float(NPTS-1)
              angle = simtime + t * 4. * 3.14159
              x(i) = angle
              y(i) = sin(x(i))
10000     continue          
          m = visitcurvesetdataf(handle, x, y, NPTS)
      endif
      visitgetcurve = m
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist(handle)
      implicit none
      integer handle
      include "visitfortransiminterface.inc"
ccc   local variables
      integer totaldomains, domainids(1), ndomids

      totaldomains = 1
      domainids(1) = 0
      ndomids = 1
      visitgetdomainlist = visitsetdomainlist(handle, totaldomains,
     .                                        domainids, ndomids)
      end

c---------------------------------------------------------------------------
c visitgetmaterial
c---------------------------------------------------------------------------
      integer function visitgetmaterial(handle, domain, name, lname)
      implicit none
      character*8 name
      integer     handle, domain, lname
      include "visitfortransiminterface.inc"
      visitgetmaterial = VISIT_ERROR
      end





























