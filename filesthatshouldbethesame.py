from itertools import *
import sys,os

FilesStr=r"""
libX11\include\X11\cursorfont.h                   X11\cursorfont.h
libX11\include\X11\Xlocale.h                      X11\Xlocale.h
libX11\include\X11\XKBlib.h                       X11\XKBlib.h
libX11\include\X11\Xcms.h                         X11\Xcms.h   
libX11\include\X11\Xregion.h                      X11\Xregion.h
libX11\include\X11\ImUtil.h                       X11\ImUtil.h 
libX11\include\X11\Xlib.h                         X11\Xlib.h     
libX11\include\X11\Xresource.h                    X11\Xresource.h
libX11\include\X11\Xlib-xcb.h                     X11\Xlib-xcb.h 
libX11\include\X11\Xutil.h                        X11\Xutil.h
libX11\include\X11\Xlibint.h                      X11\Xlibint.h

libXt\include\X11\CallbackI.h                     X11\CallbackI.h
libXt\include\X11\Composite.h                     X11\Composite.h  
libXt\include\X11\CompositeP.h                    X11\CompositeP.h 
libXt\include\X11\ConstrainP.h                    X11\ConstrainP.h 
libXt\include\X11\Constraint.h                    X11\Constraint.h 
libXt\include\X11\ConvertI.h                      X11\ConvertI.h   
libXt\include\X11\Core.h                          X11\Core.h       
libXt\include\X11\CoreP.h                         X11\CoreP.h      
libXt\include\X11\CreateI.h                       X11\CreateI.h    
libXt\include\X11\EventI.h                        X11\EventI.h     
libXt\include\X11\HookObjI.h                      X11\HookObjI.h   
libXt\include\X11\InitialI.h                      X11\InitialI.h   
libXt\include\X11\Intrinsic.h                     X11\Intrinsic.h  
libXt\include\X11\IntrinsicI.h                    X11\IntrinsicI.h 
libXt\include\X11\IntrinsicP.h                    X11\IntrinsicP.h 
libXt\include\X11\Object.h                        X11\Object.h     
libXt\include\X11\ObjectP.h                       X11\ObjectP.h    
libXt\include\X11\PassivGraI.h                    X11\PassivGraI.h 
libXt\include\X11\RectObj.h                       X11\RectObj.h    
libXt\include\X11\RectObjP.h                      X11\RectObjP.h   
libXt\include\X11\ResConfigP.h                    X11\ResConfigP.h 
libXt\include\X11\ResourceI.h                     X11\ResourceI.h  
libXt\include\X11\SelectionI.h                    X11\SelectionI.h 
libXt\include\X11\Shell.h                         X11\Shell.h      
libXt\include\X11\ShellI.h                        X11\ShellI.h     
libXt\include\X11\ShellP.h                        X11\ShellP.h     
libXt\include\X11\StringDefs.h                    X11\StringDefs.h 
libXt\include\X11\ThreadsI.h                      X11\ThreadsI.h   
libXt\include\X11\TranslateI.h                    X11\TranslateI.h 
libXt\include\X11\VarargsI.h                      X11\VarargsI.h   
libXt\include\X11\Vendor.h                        X11\Vendor.h     
libXt\include\X11\VendorP.h                       X11\VendorP.h    
libXt\include\X11\Xtos.h                          X11\Xtos.h       

libXau\include\X11\Xauth.h                        X11\Xauth.h 

libXdmcp\include\X11\Xdmcp.h                      X11\Xdmcp.h

include\xcb\xcbext.h                              libxcb\src\xcbext.h
include\xcb\render.h                              libxcb\src\render.h
include\xcb\xcb.h                                 libxcb\src\xcb.h
include\xcb\xc_misc.h                             libxcb\src\xc_misc.h
include\xcb\bigreq.h                              libxcb\src\bigreq.h
include\xcb\xproto.h                              libxcb\src\xproto.h

libXinerama\include\X11\extensions\panoramiXext.h X11\extensions\panoramiXext.h
libXinerama\include\X11\extensions\Xinerama.h     X11\extensions\Xinerama.h

gl\glcore.h                                       gl\internal\glcore.h

gl\glext.h                                        mesalib\include\gl\glext.h
gl\glx.h                                          mesalib\include\gl\glx.h
gl\glxext.h                                       mesalib\include\gl\glxext.h
gl\glx_mangle.h                                   mesalib\include\gl\glx_mangle.h
gl\gl_mangle.h                                    mesalib\include\gl\gl_mangle.h
gl\internal\dri_interface.h                       mesalib\include\gl\internal\dri_interface.h
gl\internal\glcore.h                              mesalib\include\gl\internal\glcore.h
"""

Files=FilesStr.split()

Diff=False

def CompareFiles(SrcFile, DestFile):
  try:
    if len(sys.argv)>1:
      print "Comparing",SrcFile,DestFile
    Src=open(SrcFile,'rb')
    Dest=open(DestFile,'rb')
    while 1:
      SrcLine=Src.read(4096)
      DestLine=Dest.read(4096)
      if SrcLine!=DestLine:
        print "\n!!!!",SrcFile,"and",DestFile,"are different\n"
        return True
      if not SrcLine:
        return False
  except:
    if not os.path.exists(SrcFile): print "\n",SrcFile,"does not exist\n"
    if not os.path.exists(DestFile): print "\n",DestFile,"does not exist\n"
    return False
  

for SrcFile,DestFile in izip(Files[0::2], Files[1::2]):
  Diff |= CompareFiles(SrcFile, DestFile)

if not Diff:
  print "All Files are the same"
