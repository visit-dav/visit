//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2008 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : VideoCardInfo.c
//    Author : Milan Ikits
//    Date   : Sun Jul 11 10:15:33 2004

#include <slivr/VideoCardInfo.h>

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xproto.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <X11/extensions/sync.h>
#include <X11/extensions/Xdbe.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/XInput.h>

/*
 * We need to keep a list of open displays since the Xlib display list isn't
 * public.  We also have to per-display info in a separate block since it isn't
 * stored directly in the Display structure.
 */
typedef struct _XExtDisplayInfo {
    struct _XExtDisplayInfo *next;	/* keep a linked list */
    Display *display;			/* which display this is */
    XExtCodes *codes;			/* the extension protocol codes */
    XPointer data;			/* extra data for extension to use */
} XExtDisplayInfo;

typedef struct _XExtensionInfo {
    XExtDisplayInfo *head;		/* start of list */
    XExtDisplayInfo *cur;		/* most recently used */
    int ndisplays;			/* number of displays */
} XExtensionInfo;

typedef struct _XExtensionHooks {
    int (*create_gc)(
#if NeedNestedPrototypes
	      Display*			/* display */,
	      GC			/* gc */,
	      XExtCodes*		/* codes */
#endif
);
    int (*copy_gc)(
#if NeedNestedPrototypes
	      Display*			/* display */,
              GC			/* gc */,
              XExtCodes*		/* codes */
#endif
);
    int (*flush_gc)(
#if NeedNestedPrototypes
	      Display*			/* display */,
              GC			/* gc */,
              XExtCodes*		/* codes */
#endif
);
    int (*free_gc)(
#if NeedNestedPrototypes
	      Display*			/* display */,
              GC			/* gc */,
              XExtCodes*		/* codes */
#endif
);
    int (*create_font)(
#if NeedNestedPrototypes
	      Display*			/* display */,
              XFontStruct*		/* fs */,
              XExtCodes*		/* codes */
#endif
);
    int (*free_font)(
#if NeedNestedPrototypes
	      Display*			/* display */,
              XFontStruct*		/* fs */,
              XExtCodes*		/* codes */
#endif
);
    int (*close_display)(
#if NeedNestedPrototypes
	      Display*			/* display */,
              XExtCodes*		/* codes */
#endif
);
    Bool (*wire_to_event)(
#if NeedNestedPrototypes
	       Display*			/* display */,
               XEvent*			/* re */,
               xEvent*			/* event */
#endif
);
    Status (*event_to_wire)(
#if NeedNestedPrototypes
	      Display*			/* display */,
              XEvent*			/* re */,
              xEvent*			/* event */
#endif
);
    int (*error)(
#if NeedNestedPrototypes
	      Display*			/* display */,
              xError*			/* err */,
              XExtCodes*		/* codes */,
              int*			/* ret_code */
#endif
);
    char *(*error_string)(
#if NeedNestedPrototypes
	        Display*		/* display */,
                int			/* code */,
                XExtCodes*		/* codes */,
                char*			/* buffer */,
                int			/* nbytes */
#endif
);
} XExtensionHooks;

extern XExtensionInfo *XextCreateExtension(
#if NeedFunctionPrototypes
    void
#endif
);
extern void XextDestroyExtension(
#if NeedFunctionPrototypes
    XExtensionInfo*	/* info */
#endif
);
extern XExtDisplayInfo *XextAddDisplay(
#if NeedFunctionPrototypes
    XExtensionInfo*	/* extinfo */,
    Display*		/* dpy */,
    char*		/* ext_name */,
    XExtensionHooks*	/* hooks */,
    int			/* nevents */,
    XPointer		/* data */
#endif
);
extern int XextRemoveDisplay(
#if NeedFunctionPrototypes
    XExtensionInfo*	/* extinfo */,
    Display*		/* dpy */
#endif
);
extern XExtDisplayInfo *XextFindDisplay(
#if NeedFunctionPrototypes
    XExtensionInfo*	/* extinfo */,
    Display*		/* dpy */
#endif
);

#define XextHasExtension(i) ((i) && ((i)->codes))
#define XextCheckExtension(dpy,i,name,val) \
  if (!XextHasExtension(i)) { XMissingExtension (dpy, name); return val; }
#define XextSimpleCheckExtension(dpy,i,name) \
  if (!XextHasExtension(i)) { XMissingExtension (dpy, name); return; }

/*
 * helper macros to generate code that is common to all extensions; caller
 * should prefix it with static if extension source is in one file; this
 * could be a utility function, but have to stack 6 unused arguments for 
 * something that is called many, many times would be bad.
 */
#define XEXT_GENERATE_FIND_DISPLAY(proc,extinfo,extname,hooks,nev,data) \
XExtDisplayInfo *proc (Display *dpy) \
{ \
    XExtDisplayInfo *dpyinfo; \
    if (!extinfo) { if (!(extinfo = XextCreateExtension())) return NULL; } \
    if (!(dpyinfo = XextFindDisplay (extinfo, dpy))) \
      dpyinfo = XextAddDisplay (extinfo,dpy,extname,hooks,nev,data); \
    return dpyinfo; \
}

#define XEXT_GENERATE_CLOSE_DISPLAY(proc,extinfo) \
int proc (Display *dpy, XExtCodes *codes) \
{ \
    return XextRemoveDisplay (extinfo, dpy); \
}

#define ATIFGL_EXTENSION_NAME "ATIFGLEXTENSION"
#define ATIFGL_NUMBER_EVENTS 7
#define X_FGLGetDriverData 0
#define XREQ_SZ(name) sizeof(x##name##Req)

static XExtensionInfo _fglx_info_data;
static XExtensionInfo* fglx_info = &_fglx_info_data;
static char* fglx_extension_name = ATIFGL_EXTENSION_NAME;
static XEXT_GENERATE_CLOSE_DISPLAY (close_display_fglx, fglx_info)
static /* const */ XExtensionHooks fglx_extension_hooks = {
    NULL,               /* create_gc */
    NULL,               /* copy_gc */
    NULL,               /* flush_gc */
    NULL,               /* free_gc */
    NULL,               /* create_font */
    NULL,               /* free_font */
    close_display_fglx, /* close_display */
    NULL,               /* wire_to_event */
    NULL,               /* event_to_wire */
    NULL,               /* error */
    NULL,               /* error_string */
};
static XEXT_GENERATE_FIND_DISPLAY (find_display_fglx, fglx_info,
                                   fglx_extension_name,
                                   &fglx_extension_hooks, 
                                   ATIFGL_NUMBER_EVENTS, NULL)

// requests must be multiple of 4
typedef struct _FGLGetDriverData {
    CARD8   reqType;            //    1
    CARD8   fireglReqType;      //    1
    CARD16  length B16;         //    2
    /* specific */
    CARD32  screen B32;         //    4
    CARD16  size B16;           //    2
    CARD16  pad1;               //    2
} xFGLGetDriverDataReq;         // = 12

#define sz_xFGLGetDriverDataReq XREQ_SZ(FGLGetDriverData)
#define sz_xFGLGetDriverDataReply XREP_SZ(FGLGetDriverData)

typedef struct {
    BYTE    type;               //    1    (X_Reply)
    BYTE    pad1;               //    1
    CARD16  sequenceNumber B16; //    2
    CARD32  length B32;         //    4
    /* specific */
    CARD8   majorVersion;       //    1
    CARD8   minorVersion;       //    1
    CARD8   patchlevel B16;     //    1
    CARD8   BIOSVersionMajor;   //    1
    CARD8   BIOSVersionMinor;   //    1
    CARD8   HasSecondary;       //    1
    CARD16  pad3 B16;           //    2
    CARD16  usBoardType B16;    //    2
    CARD16  usChipType  B16;    //    2
    CARD16  usVideoRam  B16;    //    2
    CARD8   sATiRevID[17];      //   17 (previousely 1, now 1+16 to fullfill alignment)
    CARD8   AGPTransferMode;    //    1
    CARD32  AGPCapPtr;          //    4
    CARD32  AGPStatus;          //    4
    CARD32  AGPCommand;         //    4
    CARD32  ulGamma1  B32;      //    4
    CARD32  ulGamma2  B32;      //    4
    CARD32  ulDriverType  B32;  //    4
    CARD32  ulDesktopSetup B32; //    4
    CARD32  ulPrimary B32;      //    4
} xFGLGetDriverDataReply;       // = 72
// 72 bytes 

int video_card_memory_size_ATI()
{
  XExtDisplayInfo* info;
  Display* dpy;
  int size;

  dpy = XOpenDisplay(NULL); //glXGetCurrentDisplay();
  info = find_display_fglx(dpy);
  if (XextHasExtension(info))
  {
    xFGLGetDriverDataReq* req;
    xFGLGetDriverDataReply rep;
    
    GetReq(FGLGetDriverData, req);

    // fill in the request structure
    req->reqType = info->codes->major_opcode;
    // calculate size of request structure in 4 byte units 
    req->length = (sizeof(*req)+3) / 4;
    req->fireglReqType = X_FGLGetDriverData;
    req->screen = DefaultScreen(dpy);
    // carry out the extension function and wait for reply
    // 3rd parameter, called "extra": number of extra 32 bit words
    // reply structures must be multiple of 4 with a minimum of 32 bytes
#ifdef __ia64__
    //SGI prism crashes in the _XReply() call, need to track this down
    //the firegl cards actually have 256MB of RAM, so return that.
    SyncHandle();
    return 256;
#endif
    if (!_XReply(dpy, (xReply *)&rep, (sizeof(rep) - 32) / 4, xFalse))
    {
      SyncHandle();
      return 0;
    }
    switch (rep.usVideoRam) {
      // DriverData.ulVideoRam is the contents of mmCONFIG_MEMSIZE shr 16
    case 0x1000:
      size = 256;
      break;
    case 0x800:
      size = 128;
      break;
    case 0x400:
      size = 64;
      break;
    case 0x200:
      size = 32;
      break;
    default:
      size = 0;
    }
    SyncHandle();
  }
  else
  {
    return 0;
  }
  return size;
}

#define NV_CONTROL_NAME "NV-CONTROL"
#define NV_CONTROL_EVENTS 1
#define X_nvCtrlQueryAttribute 2
#define NV_CTRL_VIDEO_RAM 6

static XExtensionInfo _nvctrl_ext_info_data;
static XExtensionInfo *nvctrl_ext_info = &_nvctrl_ext_info_data;
static /* const */ char *nvctrl_extension_name = NV_CONTROL_NAME;
static XEXT_GENERATE_CLOSE_DISPLAY (close_display, nvctrl_ext_info)
static /* const */ XExtensionHooks nvctrl_extension_hooks = {
    NULL,                               /* create_gc */
    NULL,                               /* copy_gc */
    NULL,                               /* flush_gc */
    NULL,                               /* free_gc */
    NULL,                               /* create_font */
    NULL,                               /* free_font */
    close_display,                      /* close_display */
    NULL,                               /* wire_to_event */
    NULL,                               /* event_to_wire */
    NULL,                               /* error */
    NULL,                               /* error_string */
};
static XEXT_GENERATE_FIND_DISPLAY (find_display, nvctrl_ext_info,
                                   nvctrl_extension_name, 
                                   &nvctrl_extension_hooks,
                                   NV_CONTROL_EVENTS, NULL)

typedef struct {
    CARD8 reqType;
    CARD8 nvReqType;
    CARD16 length B16;
    CARD32 screen B32;
    CARD32 display_mask B32;
    CARD32 attribute B32;
} xnvCtrlQueryAttributeReq;
#define sz_xnvCtrlQueryAttributeReq 16

typedef struct {
    BYTE type;
    BYTE pad0;
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD32 flags B32;
    INT32  value B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
    CARD32 pad6 B32;
    CARD32 pad7 B32;
} xnvCtrlQueryAttributeReply;
#define sz_xnvCtrlQueryAttributeReply 32

int video_card_memory_size_NV()
{
  XExtDisplayInfo* info;
  Display* dpy;
  Bool exists = False;
  int size;

  dpy = XOpenDisplay(NULL); //glXGetCurrentDisplay();
  info = find_display(dpy);

  if (XextHasExtension(info))
  {
    xnvCtrlQueryAttributeReply rep;
    xnvCtrlQueryAttributeReq *req;

    LockDisplay(dpy);
    GetReq(nvCtrlQueryAttribute, req);
    req->reqType = info->codes->major_opcode;
    req->nvReqType = X_nvCtrlQueryAttribute;
    req->screen = DefaultScreen(dpy);
    req->display_mask = 0;
    req->attribute = NV_CTRL_VIDEO_RAM;
    if (!_XReply (dpy, (xReply *)&rep, 0, xTrue)) {
      UnlockDisplay(dpy);
      SyncHandle();
      return 0;
    }
    size = rep.value/1024;
    exists = rep.flags;
    UnlockDisplay (dpy);
    SyncHandle ();
  }
  else
  {
    return 0;
  }
  return exists == True ? size : 0;
}

int video_card_memory_size()
{
  int size;
  size = video_card_memory_size_ATI();
  if(size) return size;
  return video_card_memory_size_NV();
}

#elif __APPLE__
#define __CARBONSOUND__    
//                            <- I had to add the #define __CARBONSOUND__ 
//                               line above because on my 10.3.9 Mac this 
//                               directory is empty:
//                               /System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/CarbonSound.framework/Versions/Current/Headers
//                               I believe an Apple update from early 
//                               May 2005 deleted those files for some
//                               reason (and my guess is that this has
//                               broken lots of other software as well).
//                               Anyhow, with the #define hack above
//                               everything compiles and runs again.  -DMW

#include <AGL/agl.h>

int video_card_memory_size()
{
    AGLRendererInfo info;
    info = aglQueryRendererInfo(NULL, 0);
    if(!info) {
	return 0;
    }
    else {
      GLint mem;
      aglDescribeRenderer(info, AGL_VIDEO_MEMORY, &mem);
      return mem/1024/1024;
    }
}

#else

// TODO: Implement on SGI
int video_card_memory_size()
{
  return 16;
}

#endif
