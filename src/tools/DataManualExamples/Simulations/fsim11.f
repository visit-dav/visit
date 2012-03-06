c-----------------------------------------------------------------------------
c
c Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
c Produced at the Lawrence Livermore National Laboratory
c LLNL-CODE-442911
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
c Date:       Fri Jan 12 14:12:55 PST 2007
c
c Modifications:
c
c-----------------------------------------------------------------
      program main
      implicit none
      include "visitfortransimV2interface.inc"
ccc   local variables
      integer err

      call simulationarguments()
      err = visitsetupenv()
      err = visitinitializesim("fsim11", 6,
     . "Demonstrates creating expression metadata", 41,
     . "/no/useful/path", 15,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN,
     . VISIT_F77NULLSTRING, VISIT_F77NULLSTRINGLEN)
      call mainloop()
      stop
      end

c-----------------------------------------------------------------
c mainloop
c-----------------------------------------------------------------
      subroutine mainloop()
      implicit none
      include "visitfortransimV2interface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      save /SIMSTATE/
ccc   local variables
      integer visitstate, result, blocking

c     main loop
      runflag = 1
      simcycle = 0
      simtime = 0.
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
              runflag = 0
              result = visitattemptconnection()
              if (result.eq.1) then
                  write (6,*) 'VisIt connected!'
              else
                  write (6,*) 'VisIt did not connect!'
              endif
          elseif (visitstate.eq.2) then
              runflag = 0
              if (visitprocessenginecommand().eq.0) then
                  result = visitdisconnect()
                  runflag = 1
              endif
          endif
10    continue
1234  end

      subroutine simulate_one_timestep()
c Simulate one time step
ccc   SIMSTATE common block
      integer runFlag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
      simcycle = simcycle + 1
      simtime = simtime + 0.0134
      write (6,*) 'Simulating time step: cycle=',simcycle, ' time=', simtime
      call sleep(1)
      end

ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c These functions must be defined to satisfy the visitfortransimV2interface lib.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

c---------------------------------------------------------------------------
c visitcommandcallback
c---------------------------------------------------------------------------
      subroutine visitcommandcallback (cmd, lcmd, args, largs)
      implicit none
      character*8 cmd, args
      integer     lcmd, largs
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
c visitactivatetimestep
c---------------------------------------------------------------------------
      integer function visitactivatetimestep()
      implicit none
      include "visitfortransimV2interface.inc"
      visitactivatetimestep = VISIT_OKAY
      end

c---------------------------------------------------------------------------
c visitgetmetadata
c---------------------------------------------------------------------------
      integer function visitgetmetadata()
      implicit none
      include "visitfortransimV2interface.inc"
ccc   SIMSTATE common block
      integer runflag, simcycle
      real simtime
      common /SIMSTATE/ runflag, simcycle, simtime
ccc   local variables
      integer md, m1, m2, vmd, cmd, mat, emd, err

      if(visitmdsimalloc(md).eq.VISIT_OKAY) then
          err = visitmdsimsetcycletime(md, simcycle, simtime)
          if(runflag.eq.1) then
              err = visitmdsimsetmode(md, VISIT_SIMMODE_RUNNING)
          else
              err = visitmdsimsetmode(md, VISIT_SIMMODE_STOPPED)
          endif

c Set the first mesh's properties
          if(visitmdmeshalloc(m1).eq.VISIT_OKAY) then
              err = visitmdmeshsetname(m1, "mesh2d", 6)
              err = visitmdmeshsetmeshtype(m1, 
     .            VISIT_MESHTYPE_RECTILINEAR)
              err = visitmdmeshsettopologicaldim(m1, 2)
              err = visitmdmeshsetspatialdim(m1, 2)
              err = visitmdmeshsetxunits(m1, "cm", 2)
              err = visitmdmeshsetyunits(m1, "cm", 2)
              err = visitmdmeshsetxlabel(m1, "Width", 5)
              err = visitmdmeshsetylabel(m1, "Height", 6)
              err = visitmdmeshsetcellorigin(m1, 1)
              err = visitmdmeshsetnodeorigin(m1, 1)

              err = visitmdsimaddmesh(md, m1)
          endif

c Set the second mesh's properties
          if(visitmdmeshalloc(m2).eq.VISIT_OKAY) then
              err = visitmdmeshsetname(m2, "mesh3d", 6)
              err = visitmdmeshsetmeshtype(m2, 
     .            VISIT_MESHTYPE_CURVILINEAR)
              err = visitmdmeshsettopologicaldim(m2, 3)
              err = visitmdmeshsetspatialdim(m2, 3)
              err = visitmdmeshsetxunits(m2, "cm", 2)
              err = visitmdmeshsetyunits(m2, "cm", 2)
              err = visitmdmeshsetzunits(m2, "cm", 2)
              err = visitmdmeshsetxlabel(m2, "Width", 5)
              err = visitmdmeshsetylabel(m2, "Height", 6)
              err = visitmdmeshsetzlabel(m2, "Depth", 5)
              err = visitmdmeshsetcellorigin(m2, 1)
              err = visitmdmeshsetnodeorigin(m2, 1)

              err = visitmdsimaddmesh(md, m2)
          endif

c Add a zonal scalar variable on mesh2d. 
          if(visitmdvaralloc(vmd).eq.VISIT_OKAY) then
              err = visitmdvarsetname(vmd, "zonal", 5)
              err = visitmdvarsetmeshname(vmd, "mesh2d", 6)
              err = visitmdvarsettype(vmd, VISIT_VARTYPE_SCALAR)
              err = visitmdvarsetcentering(vmd, VISIT_VARCENTERING_ZONE)

              err = visitmdsimaddvariable(md, vmd)
          endif

c Add a nodal scalar variable on mesh3d. 
          if(visitmdvaralloc(vmd).eq.VISIT_OKAY) then
              err = visitmdvarsetname(vmd, "nodal", 5)
              err = visitmdvarsetmeshname(vmd, "mesh3d", 6)
              err = visitmdvarsettype(vmd, VISIT_VARTYPE_SCALAR)
              err = visitmdvarsetcentering(vmd, VISIT_VARCENTERING_NODE)

              err = visitmdsimaddvariable(md, vmd)
          endif

c     Add a curve variable
          if(visitmdcurvealloc(cmd).eq.VISIT_OKAY) then
              err = visitmdcurvesetname(cmd, "sine", 4)
              err = visitmdcurvesetxlabel(cmd, "angle", 5)
              err = visitmdcurvesetxunits(cmd, "radians", 7)
              err = visitmdcurvesetylabel(cmd, "amplitude", 9)

              err = visitmdsimaddcurve(md, cmd)
          endif

c     Add a material
          if(visitmdmatalloc(mat).eq.VISIT_OKAY) then
              err = visitmdmatsetname(mat, "mat", 3)
              err = visitmdmatsetmeshname(mat, "mesh2d", 6)
              err = visitmdmataddmaterialname(mat, "Iron", 4)
              err = visitmdmataddmaterialname(mat, "Copper", 6)
              err = visitmdmataddmaterialname(mat, "Nickel", 6)

              err = visitmdsimaddmaterial(md, mat)
          endif

c     Add some expressions
          if(visitmdexpralloc(emd).eq.VISIT_OKAY) then
              err = visitmdexprsetname(emd, "zvec", 4)
              err = visitmdexprsetdefinition(emd, "{zonal, zonal}", 14)
              err = visitmdexprsettype(emd, VISIT_VARTYPE_VECTOR)

              err = visitmdsimaddexpression(md, emd)
          endif
          if(visitmdexpralloc(emd).eq.VISIT_OKAY) then
              err = visitmdexprsetname(emd, "nid", 3)
              err = visitmdexprsetdefinition(emd, "nodeid(mesh3d)", 14)
              err = visitmdexprsettype(emd, VISIT_VARTYPE_SCALAR)

              err = visitmdsimaddexpression(md, emd)
          endif
      endif
      visitgetmetadata = md
      end

c---------------------------------------------------------------------------
c visitgetmesh
c---------------------------------------------------------------------------
      integer function visitgetmesh(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc" 
      visitgetmesh = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetvariable
c---------------------------------------------------------------------------
      integer function visitgetvariable(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"
      visitgetvariable = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetcurve
c---------------------------------------------------------------------------
      integer function visitgetcurve(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetcurve = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetdomainlist
c---------------------------------------------------------------------------
      integer function visitgetdomainlist(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetdomainlist = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetdomainbounds
c---------------------------------------------------------------------------
      integer function visitgetdomainbounds(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetdomainbounds = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetdomainnesting
c---------------------------------------------------------------------------
      integer function visitgetdomainnesting(name, lname)
      implicit none
      character*8 name
      integer     lname
      include "visitfortransimV2interface.inc"
      visitgetdomainnesting = VISIT_INVALID_HANDLE
      end

c---------------------------------------------------------------------------
c visitgetmaterial
c---------------------------------------------------------------------------
      integer function visitgetmaterial(domain, name, lname)
      implicit none
      character*8 name
      integer     domain, lname
      include "visitfortransimV2interface.inc"
      visitgetmaterial = VISIT_INVALID_HANDLE
      end
