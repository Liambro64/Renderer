#include "../Project.hpp"
#include <cstdio>

WindowManager::WindowManager(int w, int h, Renderer &renderer)
	: display(nullptr),
	  visual(nullptr),
	  depth(0),
	  root(0),
	  window(0),
	  gc(nullptr),
	  width(w),
	  height(h),
	  screen(0),
	  m_renderer(&renderer),
	  windowSizeChanged(false),
	  lookingUp(false),
	  lookingDown(false),
	  lookingLeft(false),
	  lookingRight(false),
	  movingForward(false),
	  movingBackward(false),
	  movingLeft(false),
	  movingRight(false),
	  running(false)
{

	// 1. Open a direct connection to the local Linux X display server
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "Unable to open the X display '%s'.\n",
				getenv("DISPLAY") ? getenv("DISPLAY") : "(not set)");
		return;
	}

	int supported;
	XkbSetDetectableAutoRepeat(display, True, &supported);
	screen = DefaultScreen(display);
	root = RootWindow(display, screen);

	// 2. Instruct the server to construct your canvas window
	window = XCreateSimpleWindow(
		display, root,
		10, 10, w, h,
		1, BlackPixel(display, screen), WhitePixel(display, screen));

	XStoreName(display, window, "Liam's Rendering Engine");

	// Choose what system event interactions we want to listen to
	XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask);
	XMapWindow(display, window);

	// 4. Wrap your bare array inside an XImage container structure
	visual = DefaultVisual(display, screen);
	depth = DefaultDepth(display, screen);

	// 5. thread that renders rendering
	gc = DefaultGC(display, screen);
	m_renderer->ScreenResize(width, height);
	running = true;

	thread = std::thread([this]()
						 {
								while (running) {
									//process events
									auto now = std::chrono::high_resolution_clock::now();
									ProcessEvents();
									Update();
									ShowBuffer(Render());
									auto end = std::chrono::high_resolution_clock::now();
									auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count();
									std::this_thread::sleep_for(std::chrono::milliseconds(std::max(0L, 16 - frameTime))); // Simple frame limiter (~60 FPS)
								} });
}

WindowManager::~WindowManager()
{
	Stop();
	if (display)
	{
		XDestroyWindow(display, window);
		XCloseDisplay(display);
	}
}

const std::vector<uint32_t> &WindowManager::Render()
{
	// m_renderer->RenderTo(*this);

	return m_renderer->RenderWithWireframe();
}

void WindowManager::ShowBuffer(const std::vector<uint32_t> &buffer)
{
	const size_t pixelCount = static_cast<size_t>(width) * height;
	if (buffer.size() < pixelCount)
		return;
	char *pixels = static_cast<char *>(malloc(sizeof(uint32_t) * pixelCount));
	if (!pixels)
		return;
	memcpy(pixels, buffer.data(), sizeof(uint32_t) * pixelCount);
	XImage *ximage = XCreateImage(
		display, visual, depth, ZPixmap, 0,
		pixels, width, height, 32, 0);
	if (!ximage)
	{
		free(pixels);
		return;
	}
	XPutImage(display, window, gc, ximage, 0, 0, 0, 0, width, height);
	XFlush(display);
	XDestroyImage(ximage);
}

void WindowManager::ProcessEvents()
{
	XEvent event;
	while (XPending(display))
	{
		XNextEvent(display, &event);
		if (event.type == KeyPress)
		{
			switch (XLookupKeysym(&event.xkey, 0))
			{
			case XK_q:
				running = false;
				break;
			case XK_Up:
				lookingUp = true;
				break;
			case XK_Down:
				// look down
				lookingDown = true;
				break;
			case XK_Left:
				// look left
				lookingLeft = true;
				break;
			case XK_Right:
				// look right
				lookingRight = true;
				break;
			case XK_w:
				// move forward
				movingForward = true;
				break;
			case XK_s:
				// move backward
				movingBackward = true;
				break;
			case XK_a:
				// move left
				movingLeft = true;
				break;
			case XK_d:
				// move right
				movingRight = true;
				break;
			default:
				break;
			}
		}
		else if (event.type == KeyRelease)
		{
			switch (XLookupKeysym(&event.xkey, 0))
			{
			case XK_q:
				running = false;
				break;
			case XK_Up:
				lookingUp = false;
				break;
				break;
			case XK_Down:
				// look down
				lookingDown = false;
				break;
			case XK_Left:
				// look left
				lookingLeft = false;
				break;
			case XK_Right:
				// look right
				lookingRight = false;
				break;
			case XK_w:
				// move forward
				movingForward = false;
				break;
			case XK_s:
				// move backward
				movingBackward = false;
				break;
			case XK_a:
				// move left
				movingLeft = false;
				break;
			case XK_d:
				// move right
				movingRight = false;
				break;
			default:
				break;
			}
		}
		if (event.type == ConfigureNotify)
		{
			XConfigureEvent xce = event.xconfigure;
			if (xce.width != width || xce.height != height)
			{
				width = xce.width;
				height = xce.height;
				windowSizeChanged = true;
			}
		}
	}
}

const float rotSpeed = 0.025f;
const float moveSpeed = 0.1f;

void WindowManager::Update()
{
	if (lookingUp)
	{
		m_renderer->RotateCamera(-rotSpeed, 0);
	}
	if (lookingDown)
	{
		m_renderer->RotateCamera(rotSpeed, 0);
	}
	if (lookingLeft)
	{
		m_renderer->RotateCamera(0, -rotSpeed);
	}
	if (lookingRight)
	{
		m_renderer->RotateCamera(0, rotSpeed);
	}
	if (movingForward)
	{
		m_renderer->MoveCamera(Vector3{0, 0, -moveSpeed});
	}
	if (movingBackward)
	{
		m_renderer->MoveCamera(Vector3{0, 0, moveSpeed});
	}
	if (movingLeft)
	{
		m_renderer->MoveCamera(Vector3{-moveSpeed, 0, 0});
	}
	if (movingRight)
	{
		m_renderer->MoveCamera(Vector3{moveSpeed, 0, 0});
	}
	if (windowSizeChanged)
	{
		m_renderer->ScreenResize(width, height);
		windowSizeChanged = false;
	}
}

void WindowManager::Stop()
{
	running = false;
	if (thread.joinable())
		thread.join();
}
