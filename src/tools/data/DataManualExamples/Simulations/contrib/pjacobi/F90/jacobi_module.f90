module jacobi_module
  implicit none
  INTEGER, PARAMETER :: real4 = selected_real_kind(6,37)
  INTEGER, PARAMETER :: real8 = selected_real_kind(15,307)

  REAL(real8), DIMENSION(:,:), ALLOCATABLE, TARGET :: Temp     ! solution array
  REAL(real8), DIMENSION(:,:), ALLOCATABLE, TARGET :: oldTemp  ! previous iteration
  REAL(real8) :: del, gdel=1.0d0
  REAL(real4) :: start_time, end_time
  INTEGER :: par_size, ierr, below, above, par_rank, m, mp, iter=0
  INTEGER, PARAMETER :: MAXSTEPS = 80000, INCREMENT = 100
  REAL(real8), PARAMETER :: TOL = 1.d-6
#if ( defined _VISIT_ )
  LOGICAL :: visRunFlag, visUpdateFlag
  INTEGER simUpdate, visitstate, blocking, visResult
  REAL(real8)  simtime
#endif
  include "mpif.h"
  public

contains

  subroutine set_initial_bc()
! PDE: Laplacian u = 0;      0<=x<=1;  0<=y<=1
! B.C.: u(x,0)=sin(pi*x); u(x,1)=sin(pi*x)*exp(-pi); u(0,y)=u(1,y)=0
! SOLUTION: u(x,y)=sin(pi*x)*exp(-pi*y)
    implicit none
    INTEGER j
    REAL(real8), DIMENSION(0:m+1) :: y0

    y0 = sin(3.141593*(/(j,j=0,m+1)/)/(m+1))
    oldTemp = 0.0d0
    if(par_size > 1) then
      Temp = 0.0d0
      if (par_rank == 0  ) Temp(:,   0) = y0
      if (par_rank == par_size-1) Temp(:,mp+1) = y0*exp(-3.141593)
    else
      Temp = 0.0d0
      Temp(:,0) = y0
      Temp(:,m+1) = y0*exp(-3.141593)
    end if
    return
  end subroutine set_initial_bc

  subroutine simulate_one_timestep()
#if ( defined _VISIT_ )
    external visittimestepchanged, visitupdateplots
    integer visittimestepchanged, visitupdateplots
#endif

    REAL(real8), DIMENSION(:,:), POINTER :: cold, c, n, e, w, s

    oldTemp = Temp                            ! save current solution array to buffer

    c    =>    Temp(1:m  ,1:mp  )             ! i  ,j    for 1<=i<=m; 1<=j<=mp
    cold => oldTemp(1:m  ,1:mp  )             ! i  ,j    for 1<=i<=m; 1<=j<=mp
    n =>    oldTemp(1:m  ,2:mp+1)             ! i  ,j+1
    s =>    oldTemp(1:m  ,0:mp-1)             ! i,  j-1
    e =>    oldTemp(2:m+1,1:mp  )             ! i+1,j  
    w =>    oldTemp(0:m-1,1:mp  )             ! i-1,j

    c = ( n + e + w + s ) * 0.25   ! new solution
    del = SUM(DABS(c-cold))           ! local error sum
!    if(MOD(iter,INCREMENT)==0) then
!      if( par_rank == 0) WRITE(*,"('par_rank,iter,del:',i4,i6,e12.4)")par_rank,iter,del
!    endif

    call MPI_Allreduce( del, gdel, 1, MPI_DOUBLE_PRECISION, MPI_MAX,   &
         MPI_COMM_WORLD, ierr )     ! find global max error

    call exchange_ghost_lines()     ! update lowest and uppermost grid lines

    iter = iter + 1
#if ( defined _VISIT_ )
    ierr = visittimestepchanged()
    ierr = visitupdateplots()
#endif
  end subroutine simulate_one_timestep

  subroutine exchange_ghost_lines()
    INTEGER :: ierr
    INTEGER status(MPI_STATUS_SIZE)
! send my last computed row above and receive from below my south boundary wall
    call MPI_SendRECV(                                       &
              Temp(1,mp  ), m, MPI_DOUBLE_PRECISION, above, 0,  &
              Temp(1,   0), m, MPI_DOUBLE_PRECISION, below, 0,  &
              MPI_COMM_WORLD, status, ierr )
! send my first computed row below and receive from above my north boundary wall
    call MPI_SendRECV(                                       &
              Temp(1,   1), m, MPI_DOUBLE_PRECISION, below, 1,  &
              Temp(1,mp+1), m, MPI_DOUBLE_PRECISION, above, 1,  &
              MPI_COMM_WORLD, status, ierr )
    return
  end subroutine exchange_ghost_lines

  subroutine neighbors()
    implicit none

    if(par_rank == 0) then
      below = MPI_PROC_NULL        ! tells MPI not to perform send/recv
      if(par_size > 1) then
         above = par_rank+1
      else
         above = MPI_PROC_NULL
      endif
    else if(par_rank == par_size-1) then
      below = par_rank-1
      above = MPI_PROC_NULL        ! tells MPI not to perform send/recv
    else
      below = par_rank-1
      above = par_rank+1
    endif
    return
  end subroutine neighbors

  subroutine InitMPI()
    INTEGER :: ierr, rc

    call MPI_Init( ierr )

    if (ierr .ne. MPI_SUCCESS) then
      print *,'Error in MPI_Init. Terminating.'
      call MPI_ABORT(MPI_COMM_WORLD, rc, ierr)
    end if

    call MPI_Comm_Size( MPI_COMM_WORLD, par_size, ierr )
    
    if (ierr .ne. MPI_SUCCESS) then
      print *,'Error in MPI_Comm_Size. Terminating.'
      call MPI_ABORT(MPI_COMM_WORLD, rc, ierr)
    end if
    
    call MPI_Comm_Rank( MPI_COMM_WORLD, par_rank, ierr )

    if (ierr .ne. MPI_SUCCESS) then
      print *,'Error in MPI_Comm_Rank. Terminating.'
      call MPI_ABORT(MPI_COMM_WORLD, rc, ierr)
    end if

    call neighbors()  ! determines MPI-domain border flags    
  end subroutine InitMPI

  subroutine MPIIOWriteData(Filename)
    INTEGER :: dimuids(2), ustart(2), ucount(2)
    CHARACTER(len=*) :: Filename
    INTEGER(KIND=MPI_OFFSET_KIND) :: disp
    INTEGER :: ierr, filehandle, filetype
    ! global size of array on disk
    dimuids(1) = m + 2
    dimuids(2) = m + 2  
    disp = 0
    call MPI_File_open(MPI_COMM_WORLD, Filename,             &
                          IOR(MPI_MODE_CREATE, MPI_MODE_WRONLY),  &
                          MPI_INFO_NULL, filehandle, ierr)

    if(par_rank .eq. 0) then
      call MPI_File_write(filehandle, dimuids, 2, MPI_INTEGER, MPI_STATUS_IGNORE, ierr)
    endif
    disp = SIZEOF(dimuids) !offset because we just wrote 2 integers

    ustart(2) = (mp * par_rank)
    ustart(1) = 0 ! in the X direction, always start at 0

    ucount(1) = m+2;
    ucount(2) = mp;
! all tasks write mp lines except last rank which writes (mp+2)
! in total, we have  the (m+2)*(m+2) grid written, including the b.c.
    if(par_rank .eq. (par_size-1)) then
      ucount(2) = mp + 2
    endif

! Create the subarray representing the local block
    call MPI_Type_create_subarray(2, dimuids, ucount, ustart, &
                                       MPI_ORDER_FORTRAN, MPI_REAL8, filetype, ierr)
    call MPI_Type_commit(filetype, ierr)

    call MPI_File_set_view(filehandle, disp, MPI_REAL8, &
                                filetype, "native", MPI_INFO_NULL, ierr)

    call MPI_File_write_all(filehandle, Temp, PRODUCT(ucount), MPI_REAL8, MPI_STATUS_IGNORE, ierr)
    call MPI_Type_free(filetype, ierr)
    call MPI_File_close(filehandle, ierr)

  end subroutine MPIIOWriteData

end module jacobi_module
