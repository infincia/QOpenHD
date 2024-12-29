#ifndef GLHELP_H
#define GLHELP_H

extern "C" {
#include <EGL/egl.h>
#include <EGL/eglext.h>
// Crude hack for QT on the rpi. Fucking hell, how annyoing this QT crap with redefining opengl is !
//#include "/usr/include/GLES2/gl2.h"
//#include "/usr/include/GLES2/gl2ext.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

//#include <GLES3/gl32.h>
//#include <GLES3/gl3ext.h>
}
#include <cassert>

/// Some gl extensions that don't seem to be part of raspberry gl. 
/// I think you could get these from other places like GLAD or libepoxy.
///

#ifndef __windows__
EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list) __attribute__((weak)); // May not be in libEGL symbol table, resolve manually :(
EGLImageKHR eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
	static PFNEGLCREATEIMAGEKHRPROC createImageProc = 0;
	if(!createImageProc) {
		createImageProc = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
		assert(createImageProc);
	}
	return createImageProc(dpy, ctx, target, buffer, attrib_list);
}

EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image) __attribute__((weak)); // May not be in libEGL symbol table, resolve manually :(
EGLBoolean eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
	static PFNEGLDESTROYIMAGEKHRPROC destroyImageProc = 0;
	if(!destroyImageProc) {
		destroyImageProc = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
	  	assert(destroyImageProc);
	}
	return destroyImageProc(dpy, image);
}

/*void glDebugMessageCallbackKHR(GLDEBUGPROCKHR callback, const void *userParam) __attribute__((weak)); // May not be in libEGL symbol table, resolve manually :(
void glDebugMessageCallbackKHR(GLDEBUGPROCKHR callback, const void *userParam)
{
	static PFNGLDEBUGMESSAGECALLBACKKHRPROC debugMessageCallbackProc = 0;
	if(!debugMessageCallbackProc) {
		debugMessageCallbackProc = (PFNGLDEBUGMESSAGECALLBACKKHRPROC)eglGetProcAddress("glDebugMessageCallbackKHR");
	  	assert(debugMessageCallbackProc);
	}
	debugMessageCallbackProc(callback, userParam);
}*/


void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image) __attribute__((weak)); // May not be in libEGL symbol table, resolve manually :(
void glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
	static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC imageTargetTexture2DOES = 0;
	if(!imageTargetTexture2DOES) {
		imageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
		assert(imageTargetTexture2DOES);
	}
	imageTargetTexture2DOES(target, image);
}

#endif
/// END Gl Extensions --------------------------------------------------




#endif
