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
    attr.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask;

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
    XGrabPointer(dpy, win, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
}

void ReleasePointer() {
    XUngrabPointer(dpy, CurrentTime);
}

int main(int argc, char **argv) {
    XEvent event;
    KeySym keySym;
    Bool done = 0;
    int beginX, beginY;
    int butX, butY;
    int drag, pressed;

    done = False;
    drag = False;
    pressed = False;

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

                case ButtonPress:
                    drag = False;
                    pressed = True;
                    butX = beginX = event.xbutton.x;
                    butY = beginY = event.xbutton.y;
                    break;

                case MotionNotify:
                    // The pointer is moving;
                    if (drag) {
                        engine_report_move(event.xmotion.x - butX, event.xmotion.y - butY);
                    }

                    butX = event.xmotion.x;
                    butY = event.xmotion.y;
                    if (!drag && pressed) {
                        drag = abs(beginX - butX) > 20 || abs(beginY - butY) > 20;
                    }

                    break;

                case ButtonRelease:
                    if (drag == False && event.xbutton.x < width && event.xbutton.y < height) {
                        engine_report_touch(event.xbutton.button, event.xbutton.x, event.xbutton.y);
                    }

                    pressed = False;
                    drag = False;
                    break;

                case KeyPress:
                    keySym = XLookupKeysym(&event.xkey, 0);
                    engine_report_key(keySym);
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

        if (!done) {
            done = engine_update() == NO;

            engine_drawframe();
            if (doubleBuffered) {
                glXSwapBuffers(dpy, win);
            }
        }
    }

    engine_shutdown();

    killGLWindow();
    exit(0);
}

// Loads a PNG into `text`
texture_t * loadNativePNG(const char * name) {
    unsigned char sig[8];
    texture_t * text;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    FILE * infile;
    int bit_depth, color_type;
    png_uint_32 i, rowbytes;
    png_bytepp row_pointers = NULL;

    infile = fopen(name, "rb");
    if (!infile) {
        perror("Unable to load texture: ");
        return NULL;
    }

    fread(sig, 1, 8, infile);
    if (!png_check_sig(sig, 8)) {
        printf("Bad PNG signature\n");
        fclose(infile);
        return NULL;
    }

    text = malloc(sizeof(texture_t));
    if (!text) {
        fclose(infile);
        return NULL;
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(infile);
        printf("Unable to create read structure\n");
        return NULL;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fclose(infile);
        printf("Unable to create info structure\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        printf("setjmp failed\n");
        fclose(infile);
        return NULL;
    }

    png_init_io(png_ptr, infile);
    png_set_sig_bytes(png_ptr, 8); /* we already read the 8 signature bytes */
    png_read_info(png_ptr, info_ptr); /* read all PNG info up to image data */
    png_get_IHDR(png_ptr, info_ptr, NULL, NULL, &bit_depth, &color_type, NULL, NULL, NULL);

    text->width = png_get_image_width(png_ptr, info_ptr);
    text->height = png_get_image_height(png_ptr, info_ptr);
    text->bpp = info_ptr->pixel_depth;

    //printf("Texture [%s] is %d by %d in %d\n", text->path, text->width, text->height, text->bpp);

    // Read it
    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("unable to read png\n");
        free(text->data);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(infile);
        free(text);
        return NULL;
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE ) png_set_expand(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);

    if (bit_depth == 16) png_set_strip_16(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA ) png_set_gray_to_rgb(png_ptr);

    /* all transformations have been registered; now update info_ptr data,
     * get rowbytes and channels, and allocate image memory */
    png_read_update_info(png_ptr, info_ptr);

    // Allocate Buffer
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    text->data = malloc(rowbytes * text->height);

    if ((row_pointers = (png_bytepp) malloc(text->height * sizeof(png_bytep))) == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(text->data);
        text->data = NULL;
        fclose(infile);
        printf("Unable to allocate the row-pointer\n");

        free(text);
        return NULL;
    }

    /* set the individual row_pointers to point at the correct offsets */
    for (i = 0; i < text->height; ++i)
        row_pointers[text->height - i - 1] = (text->data) + i * rowbytes;

    /* now we can go ahead and just read the whole image */
    png_read_image(png_ptr, row_pointers);

    free(row_pointers);
    row_pointers = NULL;

    text->type = (text->bpp == 24) ? TEXTURE_TYPE_RGB : TEXTURE_TYPE_RGBA;

    png_read_end(png_ptr, NULL);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(infile);

    return text;
}

unsigned long keySymForString(const char * keyName) {
    KeySym a = XStringToKeysym(keyName);

    // This is actually == 0 but it's better to use constants
    if (a == NoSymbol) {
        return 0;
    }

    return a;
}
