/*
 * linux_x11_main.c
 *
 *  Created on: 2011-09-28
 *      Author: plaguemorin
 */

#include <stdio.h>
#include <stdlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>

#include <png.h>

#include "engine.h"

static Display *dpy;
static int screen;
static Window win;
static GLXContext ctx;
static XSetWindowAttributes attr;
static Bool fs;
static Bool doubleBuffered;
static XF86VidModeModeInfo deskMode;
static int x, y;
static unsigned int width, height;
static unsigned int depth;

/* attributes for a single buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */
static int attrListSgl[] = { GLX_RGBA, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, None };

/* attributes for a double buffered visual in RGBA format with at least
 * 4 bits per color and a 16 bit depth buffer */
static int attrListDbl[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, GLX_DEPTH_SIZE, 16, None };

GLvoid killGLWindow(GLvoid) {
	if (ctx) {
		if (!glXMakeCurrent(dpy, None, NULL)) {
			printf("Could not release drawing context.\n");
		}
		glXDestroyContext(dpy, ctx);
		ctx = NULL;
	}

	/* switch back to original desktop resolution if we were in fs */
	if (fs) {
		XF86VidModeSwitchToMode(dpy, screen, &deskMode);
		XF86VidModeSetViewPort(dpy, screen, 0, 0);
	}
	XCloseDisplay(dpy);
}

void setFullscreen(int requestWidth, int requestHeight) {
	XF86VidModeModeInfo **modes;
	int vidModeMajorVersion, vidModeMinorVersion;
	int i;
	int modeNum;
	int bestMode;

	XF86VidModeQueryVersion(dpy, &vidModeMajorVersion, &vidModeMinorVersion);
	printf("XF86VidModeExtension-Version %d.%d\n", vidModeMajorVersion, vidModeMinorVersion);
	XF86VidModeGetAllModeLines(dpy, screen, &modeNum, &modes);
	/* save desktop-resolution before switching modes */

	deskMode = *modes[0];

	/* look for mode with requested resolution */
	for (i = 0; i < modeNum; i++) {
		if ((modes[i]->hdisplay == requestWidth) && (modes[i]->vdisplay == requestHeight)) {
			bestMode = i;
		}
	}

	XF86VidModeSwitchToMode(dpy, screen, modes[bestMode]);
	XF86VidModeSetViewPort(dpy, screen, 0, 0);

	XFree(modes);
}

XVisualInfo * createGlxContext() {
	XVisualInfo *vi;
	int glxMajorVersion, glxMinorVersion;

	/* get an appropriate visual */
	vi = glXChooseVisual(dpy, screen, attrListDbl);
	if (vi == NULL)
	{
		vi = glXChooseVisual(dpy, screen, attrListSgl);
		doubleBuffered = False;
		printf("Only Singlebuffered Visual!\n");
	} else {
		doubleBuffered = True;
		printf("Got Doublebuffered Visual!\n");
	}

	glXQueryVersion(dpy, &glxMajorVersion, &glxMinorVersion);
	printf("glX-Version %d.%d\n", glxMajorVersion, glxMinorVersion);

	/* create a GLX context */
	ctx = glXCreateContext(dpy, vi, 0, GL_TRUE);

	return vi;
}

void createX11Window(int requestWidth, int requestHeight, const char * title) {
	XVisualInfo *vi;
	Colormap cmap;
	Atom wmDelete;
	Window winDummy;
	unsigned int borderDummy;

	/* get a connection */
	dpy = XOpenDisplay(NULL);
	screen = DefaultScreen(dpy);

	vi = createGlxContext();

	/* create a color map */
	cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
	attr.colormap = cmap;
	attr.border_pixel = 0;
	attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;

	width = requestWidth;
	height = requestHeight;

	if (fs) {
		setFullscreen(requestWidth, requestHeight);

		/* create a fullscreen window */
		attr.override_redirect = True;

		win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual,
				CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &attr);
		XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
		XWarpPointer(dpy, None, win, 0, 0, 0, 0, 0, 0);
	} else {
		/* create a window in window mode*/
		win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &attr);
	}

	XSetStandardProperties(dpy, win, title, title, None, NULL, 0, NULL);
	XMapRaised(dpy, win);

	wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dpy, win, &wmDelete, 1);

	/* connect the glx-context to the window */
	glXMakeCurrent(dpy, win, ctx);
	XGetGeometry(dpy, win, &winDummy, &x, &y, &width, &height, &borderDummy, &depth);

	printf("Depth %d\n", depth);

	if (glXIsDirect(dpy, ctx))
		printf("Congrats, you have Direct Rendering!\n");
	else
		printf("Sorry, no Direct Rendering possible!\n");
}

void GrabPointer() {
	XGrabPointer(dpy, win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
}

void ReleasePointer() {
	XUngrabPointer(dpy, CurrentTime);
}

int main(int argc, char **argv) {
	XEvent event;
	KeySym keySym;
	Bool done = 0;

	done = False;
	fs = False;
	createX11Window(800, 600, "eEngine");
	engine_init(800, 600);

	/* wait for events*/
	while (!done) {
		/* handle the events in the queue */
		while (XPending(dpy) > 0) {
			XNextEvent(dpy, &event);
			switch (event.type) {
				case Expose:
					if (event.xexpose.count != 0) break;
					break;
				case ConfigureNotify:
					/* call resizeGLScene only if our window-size changed */
					if ((event.xconfigure.width != width) || (event.xconfigure.height != height)) {
						width = event.xconfigure.width;
						height = event.xconfigure.height;
						printf("Resize event\n");
					}
					break;
					/* exit in case of a mouse button press */
				case ButtonPress:
					done = True;
					break;

				case KeyPress:
					keySym = XLookupKeysym(&event.xkey, 0);

					break;
				case ClientMessage:
					if (*XGetAtomName(dpy, event.xclient.message_type) == *"WM_PROTOCOLS") {
						printf("Exiting sanely...\n");
						done = True;
					}
					break;
				default:
					break;
			}
		}

		engine_update();

		engine_drawframe();
		glFlush();

		if (doubleBuffered) {
			glXSwapBuffers(dpy, win);
		}
	}

	killGLWindow();
	exit(0);
}
