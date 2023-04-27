c-----------------------------------------------------------------------
c  subroutines that follow are for fintpts based method
c-----------------------------------------------------------------------
      subroutine field_copy_si(fieldout,fieldin,idlist,nptsi)
      include 'SIZE'
      include 'TOTAL'

      real    fieldin(1),fieldout(1)
      integer idlist(1)

      do i=1,nptsi
        idx = idlist(i)
        fieldout(idx) = fieldin(i)
      enddo

      return
      end
C--------------------------------------------------------------------------
      subroutine field_eval_si(fieldout,fieldstride,fieldin)
      include 'SIZE'
      include 'TOTAL'

      real fieldout(1),fieldin(1)

      integer fieldstride,nptsi

      parameter (lt=lelv*lx1*lz1)

      integer elid_si(lt),proc_si(lt),ptid(lt),rcode_si(lt)
      common /ptlist_int/ elid_si,proc_si,ptid,rcode_si,nptsi

      real    rst_si(lt*ldim)
      common /ptlist_real/ rst_si

      integer inth_si
      common / fpt_h_si/ inth_si

c     Used for fgslib_findpts_eval of various fields
      call fgslib_findpts_eval(inth_si,fieldout,fieldstride,
     &                     rcode_si,1,
     &                     proc_si,1,
     &                     elid_si,1,
     &                     rst_si,ndim,nptsi,
     &                     fieldin)

      return
      end
c-----------------------------------------------------------------------
      subroutine rescale_inflow_fpt(ubar_in)  ! rescale inflow
      include 'SIZE'
      include 'TOTAL'

      integer icalld,e,eg,f
      save    icalld
      data    icalld /0/
      common /cvelbc/ uin(lx1,ly1,lz1,lelv)
     $              , vin(lx1,ly1,lz1,lelv)
     $              , win(lx1,ly1,lz1,lelv)
     $              , tin(lx1,ly1,lz1,lelv,ldimt)

      call get_flux_and_area(ubar,abar)
      ubar  = ubar/abar        ! Ubar
      scale = ubar_in/ubar     ! Scale factor

      if (nid.eq.0.and.(istep.le.100.or.mod(istep,100).eq.0))
     $  write(6,1) istep,time,scale,ubar,abar
    1   format(i11,1p4e14.6,' rescale')

c     Rescale the flow to match ubar_in
      do e=1,nelv
      do f=1,2*ldim
        if (cbc(f,e,1).eq.'v  ') then
           call facind (kx1,kx2,ky1,ky2,kz1,kz2,nx1,ny1,nz1,f)
           do iz=kz1,kz2
           do iy=ky1,ky2
           do ix=kx1,kx2
              uin(ix,iy,iz,e) =  scale*uin(ix,iy,iz,e)
              vin(ix,iy,iz,e) =  scale*vin(ix,iy,iz,e)
              win(ix,iy,iz,e) =  scale*win(ix,iy,iz,e)
           enddo
           enddo
           enddo
        endif
      enddo
      enddo

      ifield = 1       ! Project into H1, just to be sure....
      call dsavg(uin)
      call dsavg(vin)
      if (ldim.eq.3) call dsavg(win)

      return
      end
c-----------------------------------------------------------------------
      subroutine get_flux_and_area(vvflux,vvarea)
      include 'SIZE'
      include 'TOTAL'
      common /cvelbc/ uin(lx1,ly1,lz1,lelv)
     $              , vin(lx1,ly1,lz1,lelv)
     $              , win(lx1,ly1,lz1,lelv)
     $              , tin(lx1,ly1,lz1,lelv,ldimt)
      real vvflux,vvarea
      real work(lx1*ly1*lz1)
      integer e,f

      nxz   = nx1*nz1
      nface = 2*ndim

      vvflux = 0.
      vvarea = 0.

      do e=1,nelv
      do f=1,nface
         if (cbc(f,e,1).eq.'v  ') then
            call surface_flux(dq,uin,vin,win,e,f,work)
            vvflux = vvflux + dq
            vvarea = vvarea + vlsum(area(1,1,f,e),nxz)
         endif
      enddo
      enddo
      vvflux = glsum(vvflux,1)
      vvarea = glsum(vvarea,1)
      vvflux = -vvflux !flux in is negative

      return
      end
c-----------------------------------------------------------------------
      subroutine rescale_tinflow_fpt(tbar_in,ifld)  ! rescale inflow
      include 'SIZE'
      include 'TOTAL'

      integer icalld,e,eg,f,ifld
      save    icalld
      data    icalld /0/
      common /cvelbc/ uin(lx1,ly1,lz1,lelv)
     $              , vin(lx1,ly1,lz1,lelv)
     $              , win(lx1,ly1,lz1,lelv)
     $              , tin(lx1,ly1,lz1,lelv,ldimt)
      real get_tbar

      tbar  = get_tbar(ifld)
      scale = tbar_in - tbar

      if (nid.eq.0.and.(istep.le.100.or.mod(istep,100).eq.0))
     $  write(6,1) istep,time,scale,tbar,tbar_in,ifld+1
  1     format(i11,1p4e14.6,' rescale for field',i3)

c     Rescale the flow to match tbar_in
      do e=1,nelv
      do f=1,2*ldim
        if (cbc(f,e,1).eq.'v  ') then
           call facind (kx1,kx2,ky1,ky2,kz1,kz2,nx1,ny1,nz1,f)
           do iz=kz1,kz2
           do iy=ky1,ky2
           do ix=kx1,kx2
              tin(ix,iy,iz,e,ifld) = tin(ix,iy,iz,e,ifld)+scale
           enddo
           enddo
           enddo
        endif
      enddo
      enddo

c     ifield = 1       ! Project into H1, just to be sure....
c     call dsavg(uin)
c     call dsavg(vin)
c     if (ldim.eq.3) call dsavg(win)

      return
      end
c-----------------------------------------------------------------------
      real function get_tbar(ifld)
      implicit none
      include 'SIZE'
      include 'TOTAL'
      real uin,vin,win,tin
      common /cvelbc/ uin(lx1,ly1,lz1,lelv)
     $              , vin(lx1,ly1,lz1,lelv)
     $              , win(lx1,ly1,lz1,lelv)
     &              , tin(lx1,ly1,lz1,lelv,ldimt)
      real maflux,vtflux,glsum,dm,dq
      real txflux(lx1*ly1*lz1*lelv)
      real tyflux(lx1*ly1*lz1*lelv)
      real tzflux(lx1*ly1*lz1*lelv)
      real mxflux(lx1*ly1*lz1*lelv)
      real myflux(lx1*ly1*lz1*lelv)
      real mzflux(lx1*ly1*lz1*lelv)
      real work(lx1*ly1*lz1)
      integer e,f,ifld,nv,nface

      nv    = nx1*ny1*nz1*nelv
      nface = 2*ndim

      call col3(mxflux,uin,vtrans(1,1,1,1,ifld+1),nv)
      call col3(myflux,vin,vtrans(1,1,1,1,ifld+1),nv)
      call col3(mzflux,win,vtrans(1,1,1,1,ifld+1),nv)

      call col4(txflux,uin,tin(1,1,1,1,ifld),vtrans(1,1,1,1,ifld+1),nv)
      call col4(tyflux,vin,tin(1,1,1,1,ifld),vtrans(1,1,1,1,ifld+1),nv)
      call col4(tzflux,win,tin(1,1,1,1,ifld),vtrans(1,1,1,1,ifld+1),nv)

      maflux = 0.
      vtflux = 0.

      do e=1,nelv
      do f=1,nface
         if (cbc(f,e,1).eq.'v  ') then
            call surface_flux(dm,mxflux,myflux,mzflux,e,f,work)
            call surface_flux(dq,txflux,tyflux,tzflux,e,f,work)
            maflux = maflux + dm
            vtflux = vtflux + dq
         endif
      enddo
      enddo
      maflux = -glsum(maflux,1)
      vtflux = -glsum(vtflux,1)
      get_tbar = vtflux/maflux

      return
      end
c-----------------------------------------------------------------------
      subroutine set_inflow_fpt_setup(dxx,dyy,dzz)   ! set up inflow BCs
      include 'SIZE'
      include 'TOTAL'
c
c setup recirculation boundary condition based on user supplied dx,dy,dz
c dx,dy,dz is the vector from the inflow where the user wants the velocity
c data to be interpolated from
c
      integer icalld,e,eg,i,f,nptsi
      save    icalld
      data    icalld /0/
      real dxx,dyy,dzz

      parameter (lt=lx1*lz1*lelv)
      real rst_si(lt*ldim),xyz_si(lt*ldim)
      real dist_si(lt),vals_si(lt)

      integer elid_si(lt), proc_si(lt),ptid(lt)
      integer rcode_si(lt)
      common /ptlist_real/ rst_si
      common /ptlist_int/ elid_si,proc_si,ptid,rcode_si,nptsi
      integer inth_si
      common / fpt_h_si/ inth_si
      common /cvelbc/ uin(lx1,ly1,lz1,lelv)
     $              , vin(lx1,ly1,lz1,lelv)
     $              , win(lx1,ly1,lz1,lelv)
     $              , tin(lx1,ly1,lz1,lelv,ldimt)
      common /nekmpi/ nidd,npp,nekcomm,nekgroup,nekreal

      n = nx1*ny1*nz1*nelv
ccc
c     Gather info for findpts
ccc
      nptsi = 0
      nxyz = nx1*ny1*nz1

      do e=1,nelv
      do f=1,2*ndim  !Identify the xyz of the points that are to be found
       if (cbc(f,e,1).eq.'v  ') then
           call facind (kx1,kx2,ky1,ky2,kz1,kz2,nx1,ny1,nz1,f)
           do iz=kz1,kz2
           do iy=ky1,ky2
           do ix=kx1,kx2
            nptsi = nptsi+1
            xyz_si(ldim*(nptsi-1)+1) = xm1(ix,iy,iz,e) + dxx
            xyz_si(ldim*(nptsi-1)+2) = ym1(ix,iy,iz,e) + dyy
      if (ldim.eq.3) xyz_si(ldim*(nptsi-1)+ldim) = zm1(ix,iy,iz,e) + dzz
            ptid(nptsi) = (e-1)*nxyz+(iz-1)*lx1*ly1+(iy-1)*lx1+ix
           enddo
           enddo
           enddo
       endif
      enddo
      enddo
      mptsi=iglmax(nptsi,1)
      if (mptsi.gt.lt)
     $  call exitti('ERROR: increase lt in inflow_fpt routines.$',mptsi)

c     Setup findpts

      tol     = 1e-10
      npt_max = 256
      nxf     = 2*nx1 ! fine mesh for bb-test
      nyf     = 2*ny1
      nzf     = 2*nz1
      bb_t    = 0.1 ! relative size to expand bounding boxes by
      bb_t    = 0.1 ! relative size to expand bounding boxes by
      call fgslib_findpts_setup(inth_si,nekcomm,npp,ndim,
     &                   xm1,ym1,zm1,nx1,ny1,nz1,
     &                   nelt,nxf,nyf,nzf,bb_t,n,n,
     &                   npt_max,tol)


c     Call findpts to determine el,proc,rst of the xyzs determined above

      call fgslib_findpts(inth_si,rcode_si,1,
     &             proc_si,1,
     &             elid_si,1,
     &             rst_si,ndim,
     &             dist_si,1,
     &             xyz_si(1),ldim,
     &             xyz_si(2),ldim,
     &             xyz_si(3),ldim,nptsi)

      return
      end
C-----------------------------------------------------------------------
      subroutine set_inflow_fpt(dxx,dyy,dzz,ubar,tbar)   ! set up inflow BCs
      include 'SIZE'
      include 'TOTAL'

c setup recirculation boundary condition based on user supplied dx,dy,dz
c dx,dy,dz is the vector from the inflow where the user wants the
c velocity data to be interpolated from

      integer icalld
      save    icalld
      data    icalld /0/
      real dxx,dyy,dzz
      real ubar,tbar

      parameter (lt=lx1*lz1*lelv)
      real rst_si(lt*ldim),xyz_si(lt*ldim)
      real dist_si(lt),vals_si(lt)
      common /ptlist_real/ rst_si

      integer elid_si(lt), proc_si(lt),ptid(lt),rcode_si(lt)
      common /ptlist_int/ elid_si,proc_si,ptid,rcode_si,nptsi
      integer inth_si
      common / fpt_h_si/ inth_si
      logical iffield
      common /lvelbc/ iffield(ldimt)
      common /cvelbc/ uin(lx1,ly1,lz1,lelv)
     $              , vin(lx1,ly1,lz1,lelv)
     $              , win(lx1,ly1,lz1,lelv)
     $              , tin(lx1,ly1,lz1,lelv,ldimt) 


c     Gather info for findpts and set up inflow BC
      if (icalld.eq.0) call set_inflow_fpt_setup(dxx,dyy,dzz)
      icalld=1


      if(ifflow) then
c       Eval fields and copy to uvwin array
        call field_eval_si(vals_si,1,vx)
        call field_copy_si(uin,vals_si,ptid,nptsi)

        call field_eval_si(vals_si,1,vy)
        call field_copy_si(vin,vals_si,ptid,nptsi)

        if (ldim.eq.3) then
          call field_eval_si(vals_si,1,vz)
          call field_copy_si(win,vals_si,ptid,nptsi)
        endif

c       Rescale the flow to preserve mean flow rate
        call rescale_inflow_fpt(ubar)
      endif

      do i=1,ldimt
        if(iffield(i)) then
          call field_eval_si(vals_si,1,t(1,1,1,1,i))
          call field_copy_si(tin(1,1,1,1,i),vals_si,ptid,nptsi)
          if(i.eq.1) call rescale_tinflow_fpt(tbar,i) !only temperature needs to be rescaled
        endif
      enddo

      return
      end
C-----------------------------------------------------------------------
