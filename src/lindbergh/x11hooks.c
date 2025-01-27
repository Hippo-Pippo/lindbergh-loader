#ifndef __i386__
#define __i386__

#endif
#undef __x86_64__
#include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86vmode.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "config.h"
#include "securityboard.h"
#include "fps_limiter.h"
#include "sdlcalls.h"

extern bool SDLWindowCreated;
extern bool gettingGPUVendor;
extern bool GLUTGame;

bool creatingWindow = false;

Window win;

extern Display *x11Display;
extern Window x11Window;

/**
 * Stop the house of the dead games turning keyboard repeating off.
 */
int XAutoRepeatOff(Display *display)
{
    return 0;
}

Display *XOpenDisplay(const char *display_name)
{
    Display *(*_XOpenDisplay)(const char *display_name) = dlsym(RTLD_NEXT, "XOpenDisplay");
    if (SDLWindowCreated || gettingGPUVendor || GLUTGame || getConfig()->noSDL)
        return _XOpenDisplay(display_name);

    creatingWindow = true;
    initSDL(0, 0);
    creatingWindow = false;
    return x11Display;
}

Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width,
                     int depth, unsigned int class, Visual *visual, unsigned long valueMask, XSetWindowAttributes *attributes)
{
    Window (*_XCreateWindow)(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height,
                             unsigned int border_width, int depth, unsigned int class, Visual *visual,
                             unsigned long valueMask, XSetWindowAttributes *attributes) =
        dlsym(RTLD_NEXT, "XCreateWindow");

    if ((gettingGPUVendor || creatingWindow) || (GLUTGame && getConfig()->noSDL == 0))
    {
        Window window = _XCreateWindow(display, parent, x, y, width, height, border_width, depth, class, visual,
                                       valueMask, attributes);
        return window;
    }

    if (getConfig()->noSDL)
    {
        width = getConfig()->width;
        height = getConfig()->height;

        attributes->event_mask = attributes->event_mask | KeyPressMask | KeyReleaseMask | PointerMotionMask;

        Window window = _XCreateWindow(display, parent, x, y, width, height, border_width, depth, class, visual, valueMask, attributes);
        printf("  X11 RESOLUTION: %dx%d\n\n", width, height);

        if (getConfig()->fullscreen)
        {
            Atom wm_state = XInternAtom(display, "_NET_WM_STATE", true);
            Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", true);
            XChangeProperty(display, window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
        }
        win = window;
    }
    else
    {
        win = x11Window;
    }
    return win;
}

int XGrabPointer(Display *display, Window grab_window, Bool owner_events, unsigned int event_mask, int pointer_mode,
                 int keyboard_mode, Window confine_to, Cursor cursor, Time time)
{
    int (*_XGrabPointer)(Display *display, Window grab_window, Bool owner_events, unsigned int event_mask,
                         int pointer_mode, int keyboard_mode, Window confine_to, Cursor cursor, Time time) =
        dlsym(RTLD_NEXT, "XGrabPointer");

    if (getConfig()->noSDL)
    {
        int returnValue =
            _XGrabPointer(display, grab_window, owner_events, event_mask, pointer_mode, keyboard_mode, confine_to, cursor, time);
        XUngrabPointer(display, time);
        return returnValue;
    }
    return _XGrabPointer(display, grab_window, owner_events, event_mask, pointer_mode, keyboard_mode, confine_to, cursor, time);
}

int XGrabKeyboard(Display *display, Window grab_window, Bool owner_events, int pointer_mode, int keyboard_mode,
                  Time time)
{
    int (*_XGrabKeyboard)(Display *display, Window grab_window, Bool owner_events, int pointer_mode, int keyboard_mode,
                          Time time) = dlsym(RTLD_NEXT, "XGrabKeyboard");

    if (getConfig()->noSDL)
    {
        int returnValue = _XGrabKeyboard(display, grab_window, owner_events, pointer_mode, keyboard_mode, time);
        XUngrabKeyboard(display, time);
        return returnValue;
    }
    return _XGrabKeyboard(display, grab_window, owner_events, pointer_mode, keyboard_mode, time);
}

int XStoreName(Display *display, Window w, const char *window_name)
{
    int (*_XStoreName)(Display *display, Window w, const char *window_name) = dlsym(RTLD_NEXT, "XStoreName");

    if (getConfig()->noSDL)
    {
        char windowTitle[512];
        sprintf(windowTitle, "%s - FPS: %.2f", getGameName(), calculateFps());
        return _XStoreName(display, w, windowTitle);
    }
    return _XStoreName(display, w, window_name);
}

int XSetStandardProperties(Display *display, Window window, const char *window_name, const char *icon_name,
                           Pixmap icon_pixmap, char **argv, int argc, XSizeHints *hints)
{
    int (*_XSetStandardProperties)(Display *display, Window window, const char *window_name, const char *icon_name,
                                   Pixmap icon_pixmap, char **argv, int argc, XSizeHints *hints) =
        dlsym(RTLD_NEXT, "XSetStandardProperties");

    if (getConfig()->noSDL)
    {
        char gameTitle[256] = {0};
        strcat(gameTitle, getGameName());
        return _XSetStandardProperties(display, window, gameTitle, icon_name, icon_pixmap, argv, argc, hints);
    }
    return _XSetStandardProperties(display, window, window_name, icon_name, icon_pixmap, argv, argc, hints);
}

int XPending(Display *display)
{
    int (*_XPending)(Display *display) = dlsym(RTLD_NEXT, "XPending");
    if (SDLWindowCreated)
        return 0;
    return _XPending(display);
}

int XFlush(Display *display)
{
    int (*_XFlush)(Display *display) = dlsym(RTLD_NEXT, "XFlush");
    if (SDLWindowCreated)
        return 0;
    return _XFlush(display);
}

Bool XF86VidModeSwitchToMode(Display *display, int screen, XF86VidModeModeInfo *modesinfo)
{
    return 0;
}

Bool XF86VidModeGetViewPort(Display *display, int screen, int *x_return, int *y_return)
{
    return 0;
}

Bool XF86VidModeGetModeLine(Display *display, int screen, int *dotclock_return, XF86VidModeModeLine *modeline)
{
    return 0;
}

int XF86VidModeGetAllModeLines(Display *display, int screen, int *modecount_return, XF86VidModeModeInfo ***modesinfo)
{
    int (*_XF86VidModeGetAllModeLines)(Display *display, int screen, int *modecount_return,
                                       XF86VidModeModeInfo ***modesinfo) =
        dlsym(RTLD_NEXT, "XF86VidModeGetAllModeLines");

    if (_XF86VidModeGetAllModeLines(display, screen, modecount_return, modesinfo) != 1)
    {
        printf("Error: Could not get list of screen modes.\n");
        exit(1);
    }
    else
    {
        XF86VidModeModeInfo **modes = *modesinfo;
        modes[0]->hdisplay = getConfig()->width;
        modes[0]->vdisplay = getConfig()->height;
    }
    return true;
}

// Cursor XCreatePixmapCursor(Display *display, Pixmap source, Pixmap mask, XColor *fg, XColor *bg, unsigned int x,
//                            unsigned int y)
// {
//     int (*_XCreatePixmapCursor)(Display *display, Pixmap source, Pixmap mask, XColor *fg, XColor *bg, unsigned int x,
//                                 unsigned int y) = dlsym(RTLD_NEXT, "XCreatePixmapCursor");
//     if (SDLGame)
//         return 0;
//     return _XCreatePixmapCursor(display, source, mask, fg, bg, x, y);
// }

// int XFreePixmap(Display *display, Pixmap pixmap)
// {
//     int (*_XFreePixmap)(Display *display, Pixmap pixmap) = dlsym(RTLD_NEXT, "XFreePixmap");
//     if (SDLGame)
//         return 0;
//     return _XFreePixmap(display, pixmap);
// }

// Pixmap XCreateBitmapFromData(Display *display, Drawable drawable, const char *data, unsigned int x, unsigned int y)
// {
//     Pixmap (*_XCreateBitmapFromData)(Display *display, Drawable drawable, const char *data, unsigned int x,
//                                      unsigned int y) = dlsym(RTLD_NEXT, "XCreateBitmapFromData");
//     if (SDLGame)
//         return 0;
//     return _XCreateBitmapFromData(display, drawable, data, x, y);
// }

// int XDefineCursor(Display *display, Window w, Cursor cursor)
// {
//     return 0;
// }

typedef unsigned int uint;

void glGenFencesNV(int n, uint *fences)
{
    static unsigned int curf = 1;
    while (n--)
    {
        *fences++ = curf++;
    }
    return;
}

void glDeleteFencesNV(int a, const uint *b)
{
    return;
}
