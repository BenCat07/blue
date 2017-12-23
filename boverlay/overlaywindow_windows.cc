#include "stdafx.hh"

#ifdef _MSC_VER

#include "overlay_draw.hh"
#include "overlaywindow.hh"

#include <cassert>
#include <thread>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "dwmapi.lib")

static const char *overlay_class_name = "boverlay";

static HINSTANCE get_this_module() {
    return GetModuleHandleA(nullptr);
}

class OverlayWindow : public IOverlayWindow {
    HDC   hdc;
    HGLRC hglrc;

    HWND target;
    HWND local;

    OverlayWindowOnFrameCallback frame_callback;
    void *                       frame_userdata;

    Vector2 overlay_size;

    IDrawManager *dm;

    // message handlers
    static LRESULT APIENTRY window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT                 overlay_proc(UINT message, WPARAM wparam, LPARAM lparam);

    bool create_window();
    bool init_gl();

public:
    OverlayWindow(IDrawManager *dm) : local(nullptr), target(nullptr), hdc(nullptr), hglrc(nullptr), overlay_size{0, 0}, frame_callback(nullptr), dm(dm) {}

    bool init() override;
    void shutdown() override;

    void make_current() override { wglMakeCurrent(hdc, hglrc); }

    void set_target(void *t) override;

    void frame() override;

    void set_on_frame_callback(OverlayWindowOnFrameCallback f, void *userdata) override;

    Vector2 size() override { return overlay_size; }

    void resize();
};

LRESULT APIENTRY OverlayWindow::window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    auto instance = reinterpret_cast<OverlayWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (instance == nullptr) return DefWindowProc(hwnd, message, wparam, lparam);

    return instance->overlay_proc(message, wparam, lparam);
}

LRESULT OverlayWindow::overlay_proc(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CREATE: {
        break;
    }
    case WM_CLOSE: {
        DestroyWindow(local);
        break;
    }
    //release resources
    case WM_DESTROY: {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hglrc);
        DeleteDC(hdc);
        ReleaseDC(local, hdc);
        PostQuitMessage(0);
        UnregisterClass(overlay_class_name, get_this_module());
        break;
    }
    case WM_ERASEBKGND: {
        return 0;
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(local, &ps);

        while (glGetError() != GL_NO_ERROR) continue;

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT);

        // call out to the frame callback
        if (frame_callback != nullptr) frame_callback(frame_userdata);

        auto error = glGetError();

        printf("error is 0x%X\n", error);

        assert(error == GL_NO_ERROR);

        //copy the backbuffer into the window
        SwapBuffers(hdc);
        //end painting
        EndPaint(local, &ps);
        return 0;
    }
    }

    return DefWindowProc(local, message, wparam, lparam);
}

bool OverlayWindow::create_window() {
    // TODO: only do this once
    WNDCLASSEX wclass;

    wclass.cbSize        = sizeof(WNDCLASSEX);
    wclass.style         = CS_OWNDC;
    wclass.lpfnWndProc   = &OverlayWindow::window_proc;
    wclass.cbClsExtra    = 0;
    wclass.cbWndExtra    = 0;
    wclass.hInstance     = get_this_module(); // get the local dll
    wclass.hIcon         = NULL;
    wclass.hCursor       = NULL;
    wclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wclass.lpszMenuName  = NULL;
    wclass.lpszClassName = overlay_class_name;
    wclass.hIconSm       = NULL;

    if (!RegisterClassEx(&wclass)) return false;

    local = CreateWindowEx(
        WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED,
        overlay_class_name,
        overlay_class_name,
        WS_POPUP | WS_VISIBLE | WS_MAXIMIZE,
        0,
        0,
        100,
        100,
        nullptr, nullptr, get_this_module(), nullptr);
    if (local == nullptr) {
        UnregisterClass(overlay_class_name, get_this_module());
        return false;
    }

    // initialise gl here as we cannot get our thisptr in the mesage loop when
    // WM_CREATE happens
    hdc = GetDC(local);
    if (!init_gl()) return false;

    SetWindowLongPtr(local, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // make sure our window is layered
    // NOTE: SOME WINDOWS VERSIONS DO NOT SUPPORT THIS
    if (!SetLayeredWindowAttributes(local, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA)) {
        UnregisterClass(overlay_class_name, get_this_module());
        return false;
    }

    // the region that is not blurred is rendered based off the alpha channel, therefore, we create an invalid region so that nothing is blurred but the alpha blending remains. We need this due to... who the fuck knows why layered window attributes wont work? seriously tell me please :'(
    DWM_BLURBEHIND bb = {DWM_BB_ENABLE | DWM_BB_BLURREGION, true, CreateRectRgn(0, 0, -1, -1), true};
    // enable trasnparency via dwm
    // NOTE: SOME WINDOWS VERSIONS DO NOT SUPPORT THIS
    if (DwmEnableBlurBehindWindow(local, &bb) != S_OK) {
        UnregisterClass(overlay_class_name, get_this_module());
        return false;
    }

    ShowWindow(local, SW_SHOWNORMAL);

    // we have to process messages on the same thread that we create the window on
    MSG msg;
    while (GetMessage(&msg, local, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

bool OverlayWindow::init_gl() {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                            // Version Number
        PFD_DRAW_TO_WINDOW |          // Format Must Support Window
            PFD_SUPPORT_OPENGL |      // Format Must Support OpenGL
            PFD_SUPPORT_COMPOSITION | // Format Must Support Composition
            PFD_DOUBLEBUFFER,         // Must Support Double Buffering
        PFD_TYPE_RGBA,                // Request An RGBA Format
        32,                           // Select Our Color Depth
        0, 0, 0, 0, 0, 0,             // Color Bits Ignored
        8,                            // An Alpha Buffer
        0,                            // Shift Bit Ignored
        0,                            // No Accumulation Buffer
        0, 0, 0, 0,                   // Accumulation Bits Ignored
        0,                            // No Z-Buffer (Depth Buffer)
        8,                            // Some Stencil Buffer
        0,                            // No Auxiliary Buffer
        PFD_MAIN_PLANE,               // Main Drawing Layer
        0,                            // Reserved
        0, 0, 0                       // Layer Masks Ignored
    };
    int pf = ChoosePixelFormat(hdc, &pfd);
    if (pf == 0) return false;
    if (SetPixelFormat(hdc, pf, &pfd) == false) return false;
    if (DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd) == false) return false;

    hglrc = wglCreateContext(hdc);
    if (hglrc == nullptr) return false;
    if (wglMakeCurrent(hdc, hglrc) == false) return false;

    glMatrixMode(GL_PROJECTION);

    assert(glGetError() == GL_NO_ERROR);

    resize();
    //color to set when screen is cleared, black with no alpha
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return true;
}

//setup openGL
bool OverlayWindow::init() {

    // TODO: use futures here to see if this fails / exits abnormally
    std::thread{&OverlayWindow::create_window, this}.detach();

    return true;
}

void OverlayWindow::shutdown() {
}

void OverlayWindow::set_target(void *t) {
    target = static_cast<HWND>(t);
    resize();
}

void OverlayWindow::frame() {
    resize();

    auto active_window = GetForegroundWindow();

    // dont paint if the target is minimised
    if (IsIconic(target) || active_window != target) {
        ShowWindow(local, SW_HIDE);
        return;
    }

    ShowWindow(local, SW_SHOWNORMAL);

    // cause a WM_PAINT message to be send
    InvalidateRect(local, nullptr, false);
}

void OverlayWindow::set_on_frame_callback(OverlayWindowOnFrameCallback f, void *userdata) {
    frame_callback = f;
    frame_userdata = userdata;
}

void OverlayWindow::resize() {
    if (target == nullptr || local == nullptr) return; // we cant size the window yet

    RECT window_bounds;
    RECT client_bounds;

    // get the inner and outer bounds of the target
    GetWindowRect(target, &window_bounds);
    GetClientRect(target, &client_bounds);

    auto rect_size_zero = [](const RECT &a) {
        return (a.right - a.left) == 0 || (a.bottom - a.top) == 0;
    };

    if (rect_size_zero(window_bounds) || rect_size_zero(client_bounds)) return;

    //width and height of client rect
    auto width  = client_bounds.right - client_bounds.left;
    auto height = client_bounds.bottom - client_bounds.top;

    auto posx = window_bounds.right - width;
    auto posy = window_bounds.bottom - height;

    SetWindowPos(local, 0, posx, posy, width, height, 0);

    glViewport(0, 0, width, height);

    glOrtho(client_bounds.left, client_bounds.right, client_bounds.bottom, client_bounds.top, 0, 1);

    overlay_size = {static_cast<float>(width),
                    static_cast<float>(height)};
}

IOverlayWindow *create_overlay_window(IDrawManager *dm) {
    return new OverlayWindow(dm);
}

#endif
