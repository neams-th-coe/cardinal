C=======================================================================
      subroutine nek_init_step()
      include 'SIZE'
      include 'TSTEP'
      include 'INPUT'
      include 'CTIMER'
      real*4 papi_mflops
      integer*8 papi_flops
      integer icall, kstep, i, pstep
      common /cht_coupler/ pstep
      save icall
 
      if (icall.eq.0) then
      call nekgsync()
      if (instep.eq.0) then
        if(nid.eq.0) write(6,'(/,A,/,A,/)')
     &     ' nsteps=0 -> skip time loop',
     &     ' running solver in post processing mode'
      else
        if(nio.eq.0) write(6,'(/,A,/)') 'Starting time loop ...'
      endif
      isyc  = 0
      itime = 0
      if(ifsync) isyc=1
      itime = 1
      call nek_comm_settings(isyc,itime)
      call nek_comm_startstat()
      istep  = 0
      icall  = 1
      endif

      istep=istep+1

      if (lastep .eq. 1) then
        pstep=2
      else
        call nek_advance_moose
        pstep=2
      endif

      return
      end
C=======================================================================
      subroutine nek_step()

      include 'SIZE'
      include 'TSTEP'
      include 'INPUT'
      include 'CTIMER'
      common /cht_coupler/ pstep
      integer pstep

      pstep=pstep+1
      call heat_mod(pstep)

      return
      end
C=======================================================================
      subroutine nek_finalize_step()
      include 'SIZE'
      include 'TSTEP'
      include 'INPUT'
      include 'CTIMER'
      common /cht_coupler/ pstep
      integer pstep 
      logical ifdoin
      character*3 prefix 
      real*4 papi_mflops
      integer*8 papi_flops
      integer icall, kstep, knstep, i

      pstep=2
      if (param(103).gt.0)   call q_filter      (param(103))
      call setup_convect (pstep)  ! Save convective velocity _after_ filter

      call userchk

      prefix='his'
      ifdoin=.false. 
      call prepost (ifdoin,prefix)
      call in_situ_check()

      if (mod(istep,nsteps).eq.0) lastep=1
      call nek_comm_settings(isyc,0)
      call comment

      return
      end
C=======================================================================
      subroutine nek_expansion()

      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE' 
      integer e,f

      real*8 fmode(lx1,ly1,lz1,lelt), cache(lx1,ly1,lz1,lelt)
      real*8 sint, sint1, sarea, sarea1
      real*8 pi

      pi=4.0*atan(1.0)
      ntot=nx1*ny1*nz1*nelt

      call rzero(fmode,ntot)
      do i0=1,n_legendre
        do j0=1,m_fourier
          call nek_mode(fmode,i0,j0)
          sarea1=0.0
          sint1= 0.0
          do e=1,nelt
            do f=1,6
              sint=0.0
              sarea=0.0
              if (cbc(f,e,1).eq.'W  ') then
                call col3(cache,fmode,t,ntot)
                call surface_int(sint,sarea,cache,e,f)
                sint1=sint1+sint
                sarea1=sarea1+sarea
              endif
            enddo
          enddo
          call  gop(sint1,wtmp,'+  ',1)
          call  gop(sarea1,wtmp,'+  ',1)
!
          coeff_tij(i0,j0)=sint1*4.0*pi/sarea1
!
        enddo
      enddo

c     For Testing
c     call nek_testp()
      return
      end
C=======================================================================
      subroutine nek_diag()
      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE' 
      common/diags_coeff/diag_c(nl_max,nf_max)
      integer e,f

      real*8 fmode(lx1,ly1,lz1,lelt)
      real*8 cache(lx1,ly1,lz1,lelt)
      real*8 sint, sint1

      ntot=nx1*ny1*nz1*nelt

      zmin=glmin(zm1,ntot)
      zmax=glmax(zm1,ntot)

      call rzero(fmode,ntot)
         do i0=1,n_legendre
           do j0=1,m_fourier
             call nek_mode(fmode,i0,j0)  
             sint1=0.0
             do e=1,nelt
               do f=1,6
                 sint=0.0
                 if (cbc(f,e,1).eq.'W  ') then
                   call col3(cache,fmode,fmode,ntot)
                   call surface_int(sint,sarea,cache,e,f)
                   sint1=sint1+sint
                 endif
               enddo
             enddo
          call  gop(sint1,wtmp,'+  ',1)
          diag_c(i0,j0)=sint1*2.0/(0.5*(zmax-zmin))
          if (nid.eq.0) write(6,*)i0,j0,diag_c(i0,j0)
        enddo
      enddo

      return
      end
C=======================================================================
      subroutine nek_testp()
      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE'      
      integer e,f
      real*8 fmode(lx1,ly1,lz1,lelt), cache(lx1,ly1,lz1,lelt)
      real*8 fun(lx1,ly1,lz1,lelt) 
      ntot=nx1*ny1*nz1*nelt
      call rzero(fmode,ntot)
      call rzero(fun,ntot)   
         do i0=1,n_legendre
           do j0=1,m_fourier
             call nek_mode(fmode,i0,j0)
             do i=1,ntot
             fun(i,1,1,1)=fun(i,1,1,1)+fmode(i,1,1,1)*coeff_tij(i0,j0) 
             enddo  
          enddo
         enddo
      call rzero(cache,ntot) 
      call sub3(cache,fun,t,ntot)
      do i=1,ntot
        c1=cache(i,1,1,1)**2
        cache(i,1,1,1)=c1 
      enddo
 
      sint1=0.0
      sarea1=0.0
      do e=1,lelt
        do f=1,6
          call surface_int(sint,sarea,cache,e,f)
          if (cbc(f,e,1).eq.'W  ') then
           sint1=sint1+sint
           sarea1=sarea1+sarea
          endif
         enddo
      enddo
      call  gop(sint1,wtmp,'+  ',1)
      call  gop(sarea1,wtmp,'+  ',1)

      er_avg=sqrt(sint1/sarea1)
      if (nid.eq.0) write(6,*)"Error: ",er_avg
      return
      end
C=======================================================================
      subroutine nek_mode(fmode,im,jm)
      include 'SIZE'
      include 'TOTAL'
      integer e,f
      real*8 fmode(lx1,ly1,lz1,lelt) 
      ntot=nx1*ny1*nz1*nelt
      zmin=glmin(zm1,ntot)
      zmax=glmax(zm1,ntot)
             do e=1,nelt
               do f=1,6
                 if (cbc(f,e,1).eq.'W  ') then
                   call dsset(nx1,ny1,nz1)
                   iface  = eface1(f)
                   js1    = skpdat(1,iface)
                   jf1    = skpdat(2,iface)
                   jskip1 = skpdat(3,iface)
                   js2    = skpdat(4,iface)
                   jf2    = skpdat(5,iface)
                   jskip2 = skpdat(6,iface)
                   do j2=js2,jf2,jskip2
                     do j1=js1,jf1,jskip1
                       x=xm1(j1,j2,1,e)
                       y=ym1(j1,j2,1,e)
                       z=zm1(j1,j2,1,e)
                       z_leg=2*((z-zmin)/zmax)-1
                       theta=atan2(y,x)
                       fmode(j1,j2,1,e)=
     &                 pl_leg(z_leg,im-1)*fl_four(theta,jm-1)
                     enddo
                   enddo
                 endif
              enddo
            enddo
      return
      end 
c-----------------------------------------------------------------------
      subroutine flux_reconstruction()
      include 'SIZE'
      include 'TOTAL'
      include 'NEKMOOSE'  
      integer e,f
      real*8 coeff_base, flux_base, flux_moose 
      real*8 fmode(lx1,ly1,lz1,lelt)
      ntot=nx1*ny1*nz1*nelt

c     --------------------------------------
c     The flux from MOOSE must have proper sign
c     --------------------------------------
      coeff_base=-1.0 
      flux_base=0.25 ! from energy conservation in the problem
c     --------------------------------------

      call rzero(flux_recon,ntot)
         do i0=1,n_legendre
           do j0=1,m_fourier
             call rzero(fmode,ntot)
             call nek_mode(fmode,i0,j0)
             do i=1,ntot
             flux_recon(i,1,1,1)= flux_recon(i,1,1,1)
     &                  + coeff_base*fmode(i,1,1,1)*coeff_fij(i0,j0)
             enddo
          enddo
         enddo

c---- Below is for testing
c             do i=1,ntot
c             flux_recon(i,1,1,1)= 0.0
c             enddo
c--------------------------

c---- Renormalization
      sint1=0.0
      sarea1=0.0

      do e=1,lelt
        do f=1,6
          call surface_int(sint,sarea,flux_recon,e,f)
          if (cbc(f,e,1).eq.'W  ') then
           sint1=sint1+sint
           sarea1=sarea1+sarea
          endif
         enddo
      enddo

      call  gop(sint1,wtmp,'+  ',1)
      call  gop(sarea1,wtmp,'+  ',1)

      flux_moose=sint1/sarea1
      do i=1,ntot
         flux_recon(i,1,1,1)= flux_recon(i,1,1,1)*flux_base/flux_moose
      enddo
c-----------------------
      return
      end
c-----------------------------------------------------------------------
! calculates Legendre polynomials using a recurrence relationship. If
! n > the maximum Legendre order, the function returns 0.0.
      function pl_leg(x,n)
      parameter (nl_max=100) ! should never be bigger than 100
      real*8 pl,pl_leg
      real*8 x
      real*8 pln(0:n)
      integer n, k

      pln(0) = 1.0
      pln(1) = x

      if (n.le.1) then
        pl = pln(n)
      else if (n.le.nl_max) then
        do k=1,n-1
          pln(k+1) = ((2.0*k+1.0)*x*pln(k)-dble(k)*pln(k-1))/(dble(k+1))
        end do
        pl = pln(n)
      else
        pl = 0.0
      end if

      pl_leg = pl*sqrt(dble(2*n+1)/2.0)

      return
      end
c-----------------------------------------------------------------------
! calculates Fourier polynomials Fn(x)
      function fl_four(x,n)
      real*8 fl_four,pi
      real*8 x, A
      integer n, k

      pi=4.0*atan(1.0)
      A=1.0/sqrt(pi)

      if (n.eq.0) A=1.0/sqrt(2*pi)
      fl_four=A*cos(n*x)

      return
      end
c-----------------------------------------------------------------------
c-----------------------------------------------------------------------
      subroutine cdscal_mod (igeom)
C
C     Solve the convection-diffusion equation for passive scalar IPSCAL
C
      include 'SIZE'
      include 'INPUT'
      include 'GEOM'
      include 'MVGEOM'
      include 'SOLN'
      include 'MASS'
      include 'TSTEP'
      COMMON  /CPRINT/ IFPRINT
      LOGICAL          IFPRINT
      LOGICAL          IFCONV
C
      COMMON /SCRNS/ TA(LX1,LY1,LZ1,LELT)
     $              ,TB(LX1,LY1,LZ1,LELT)
      COMMON /SCRVH/ H1(LX1,LY1,LZ1,LELT)
     $              ,H2(LX1,LY1,LZ1,LELT)

      include 'ORTHOT'

      if (ifdg) then
         call cdscal_dg(igeom)
         return
      endif


      napprox(1) = laxt  ! Fix this... pff 10/10/15

      nel    = nelfld(ifield)
      n   = nx1*ny1*nz1*nel

      if (igeom.eq.1) then   ! geometry at t^{n-1}

         call makeq
         call lagscal

      else                   ! geometry at t^n

         IF (IFPRINT) THEN
         IF (IFMODEL .AND. IFKEPS) THEN
            NFLDT = NFIELD - 1
            IF (IFIELD.EQ.NFLDT.AND.NID.EQ.0) THEN
               WRITE (6,*) ' Turbulence Model - k/epsilon solution'
            ENDIF
         ELSE
            IF (IFIELD.EQ.2.AND.NID.EQ.0) THEN
               WRITE (6,*) ' Temperature/Passive scalar solution'
            ENDIF
         ENDIF
         ENDIF
         if1=ifield-1
         write(name4,1) if1-1
    1    format('PS',i2)
         if(ifield.eq.2) write(name4,'(A4)') 'TEMP'

C
C        New geometry
C
         isd = 1
         if (ifaxis.and.ifaziv.and.ifield.eq.2) isd = 2
c        if (ifaxis.and.ifmhd) isd = 2 !This is a problem if T is to be T!

         do 1000 iter=1,nmxnl ! iterate for nonlin. prob. (e.g. radiation b.c.)

         INTYPE = 0
         IF (IFTRAN) INTYPE = -1
         CALL SETHLM  (H1,H2,INTYPE)
         CALL BCNEUSC (TA,-1)
         CALL ADD2    (H2,TA,N)
         CALL BCDIRSC (T(1,1,1,1,IFIELD-1))
         CALL AXHELM  (TA,T(1,1,1,1,IFIELD-1),H1,H2,IMESH,isd)
         CALL SUB3    (TB,BQ(1,1,1,1,IFIELD-1),TA,N)
         CALL BCNEUSC (TA,1)
         CALL ADD2    (TB,TA,N)

c        CALL HMHOLTZ (name4,TA,TB,H1,H2
c    $                 ,TMASK(1,1,1,1,IFIELD-1)
c    $                 ,TMULT(1,1,1,1,IFIELD-1)
c    $                 ,IMESH,TOLHT(IFIELD),NMXH,isd)

         if(iftmsh(ifield)) then
           call hsolve  (name4,TA,TB,H1,H2 
     $                   ,tmask(1,1,1,1,ifield-1)
     $                   ,tmult(1,1,1,1,ifield-1)
     $                   ,imesh,tolht(ifield),nmxh,1
     $                   ,approx,napprox,bintm1)
         else
           call hsolve  (name4,TA,TB,H1,H2 
     $                   ,tmask(1,1,1,1,ifield-1)
     $                   ,tmult(1,1,1,1,ifield-1)
     $                   ,imesh,tolht(ifield),nmxh,1
     $                   ,approx,napprox,binvm1)
         endif 

         call add2    (t(1,1,1,1,ifield-1),ta,n)

         call cvgnlps (ifconv) ! Check convergence for nonlinear problem 
         if (ifconv) goto 2000

C        Radiation case, smooth convergence, avoid flip-flop (ER).
         CALL CMULT (TA,0.5,N)
         CALL SUB2  (T(1,1,1,1,IFIELD-1),TA,N)

 1000    CONTINUE
 2000    CONTINUE
         CALL BCNEUSC (TA,1)

      endif

      return
      end
c-----------------------------------------------------------------------
      subroutine heat_mod (igeom)
C
C     Driver for temperature or passive scalar.
C
C     Current version:
C     (1) Varaiable properties.
C     (2) Implicit time stepping.
C     (3) User specified tolerance for the Helmholtz solver
C         (not based on eigenvalues).
C     (4) A passive scalar can be defined on either the 
C         temperatur or the velocity mesh.
C     (5) A passive scalar has its own multiplicity (B.C.).  
C
      include 'SIZE'
      include 'INPUT'
      include 'TSTEP'
      include 'TURBO' 
      include 'DEALIAS'

      real*8 ts, dnekclock

      ts = dnekclock()

      if (nio.eq.0 .and. igeom.eq.2) 
     &    write(*,'(13x,a)') 'Solving for Hmholtz scalars'

      do ifield = 2,nfield
         if (idpss(ifield-1).eq.0) then      ! helmholtz
            intype        = -1
            if (.not.iftmsh(ifield)) imesh = 1
            if (     iftmsh(ifield)) imesh = 2
            call unorm
            call settolt
            call cdscal_mod(igeom)
         endif
      enddo

      if (nio.eq.0 .and. igeom.eq.2)
     &   write(*,'(4x,i7,a,1p2e12.4)') 
     &   istep,'  Scalars done',time,dnekclock()-ts

      return
      end
c-----------------------------------------------------------------------
      subroutine nek_advance_moose

      include 'SIZE'
      include 'TOTAL'
      include 'CTIMER'

      common /cgeom/ igeom

      call nekgsync
      if (iftran) call settime
      if (ifmhd ) call cfl_check
      call setsolv
      call comment

      if (ifcmt) then
         if (nio.eq.0.and.istep.le.1) write(6,*) 'CMT branch active'
         call cmt_nek_advance
         return
      endif

      if (ifsplit) then   ! PN/PN formulation

         do igeom=1,ngeom

         ! within cvode we use the lagged wx for 
         ! extrapolation, that's why we have to call it before gengeom 
         if (ifheat .and. ifcvode) call heat_cvode (igeom)   

         if (ifgeom) then
               call gengeom (igeom)
               call geneig  (igeom)
         endif

         if (ifheat)               call heat_mod (igeom)

         if (igeom.eq.2) then  
                                   call setprop
            if (iflomach)          call qthermal(.true.,.false.,dummy)
         endif

         if (ifflow)               call fluid         (igeom)
         if (ifmvbd)               call meshv         (igeom)
         if (param(103).gt.0)      call q_filter      (param(103))
                                   call setup_convect (igeom)     ! Save convective velocity _after_ filter 
         enddo

      else                ! PN-2/PN-2 formulation

         call setprop
         do igeom=1,ngeom

            if (igeom.gt.2) call userchk_set_xfer

            if (ifgeom) then
               call gengeom (igeom)
               call geneig  (igeom)
            endif

            if (ifneknekm.and.igeom.eq.2) call multimesh_create

            if (ifmhd) then
               if (ifheat)      call heat     (igeom)
                                call induct   (igeom)
            elseif (ifpert) then
               if (ifbase.and.ifheat)  call heat          (igeom)
               if (ifbase.and.ifflow)  call fluid         (igeom)
               if (ifflow)             call fluidp        (igeom)
               if (ifheat)             call heatp         (igeom)
            else  ! std. nek case
               if (ifheat)             call heat_mod      (igeom)
               if (ifflow)             call fluid         (igeom)
               if (ifmvbd)             call meshv         (igeom)
            endif

            if (igeom.eq.ngeom.and.param(103).gt.0) 
     $          call q_filter(param(103))

            call setup_convect (igeom) ! Save convective velocity _after_ filter

         enddo
      endif

      return
      end

c-----------------------------------------------------------------------
