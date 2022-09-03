#include "rgb_window.h"

LRESULT windowEventHandler(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) {
	std::queue<rgb::SWindowEvent>		& windowEventQueue		= *(std::queue<rgb::SWindowEvent>*)GetWindowLongPtr(windowHandle, GWLP_USERDATA);	// Get our rgb::SCoord instance defined in main()
	switch(message) {
	case WM_LBUTTONUP	: { rgb::SWindowEvent newEvent = {rgb::EVENT_MOUSE_BUTTON_UP	}; newEvent.Data.push_back(0); windowEventQueue.push(newEvent); break; }
	case WM_LBUTTONDOWN	: { rgb::SWindowEvent newEvent = {rgb::EVENT_MOUSE_BUTTON_DOWN}; newEvent.Data.push_back(0); windowEventQueue.push(newEvent); break; }
	case WM_RBUTTONUP	: { rgb::SWindowEvent newEvent = {rgb::EVENT_MOUSE_BUTTON_UP	}; newEvent.Data.push_back(1); windowEventQueue.push(newEvent); break; }
	case WM_RBUTTONDOWN	: { rgb::SWindowEvent newEvent = {rgb::EVENT_MOUSE_BUTTON_DOWN}; newEvent.Data.push_back(1); windowEventQueue.push(newEvent); break; }
	case WM_MBUTTONUP	: { rgb::SWindowEvent newEvent = {rgb::EVENT_MOUSE_BUTTON_UP	}; newEvent.Data.push_back(2); windowEventQueue.push(newEvent); break; }
	case WM_MBUTTONDOWN	: { rgb::SWindowEvent newEvent = {rgb::EVENT_MOUSE_BUTTON_DOWN}; newEvent.Data.push_back(2); windowEventQueue.push(newEvent); break; }
	case WM_MOUSEMOVE: {
		rgb::SCoord							mousePosition			= 
			{ GET_X_LPARAM(lParam)
			, GET_Y_LPARAM(lParam)
			};

		rgb::SWindowEvent					newEvent				= {rgb::EVENT_MOUSE_MOVE};
		newEvent.Data.resize(sizeof(mousePosition));
		*(rgb::SCoord*)&newEvent.Data[0]	= mousePosition;
		windowEventQueue.push(newEvent);
		break;
	}
	case WM_MOUSEWHEEL: {
		float							wheelDelta				= GET_WHEEL_DELTA_WPARAM(wParam) / float(WHEEL_DELTA);
		rgb::SWindowEvent					newEvent				= {rgb::EVENT_MOUSE_WHEEL};
		newEvent.Data.resize(sizeof(wheelDelta));
		*(float*)&newEvent.Data[0]	= wheelDelta;
		windowEventQueue.push(newEvent);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0); // post WM_QUIT to event queue
	}

	return DefWindowProc(windowHandle, message, wParam, lParam);
}

int rgb::windowCreate(SWindow & window) {
	window.Class.hInstance			= GetModuleHandle(NULL);
	window.Class.lpszClassName		= TEXT("MY_WINDOW_CLASS");
	window.Class.lpfnWndProc		= windowEventHandler;
	RegisterClass(&window.Class);

	window.Size						= {640, 480};	// Store the window. size here so we can use it in places other than the CreateWindow() call.
	window.EventQueue				= {};

	window.Handle					= CreateWindow(window.Class.lpszClassName, TEXT("My first window."), WS_OVERLAPPEDWINDOW, 0, 0, window.Size.x, window.Size.y, 0, 0, window.Class.hInstance, 0);
	window.MousePosition			= {};	// Store in this variable the mouse position in window. space.
	ShowWindow(window.Handle, SW_SHOW);
	SetWindowLongPtr(window.Handle, GWLP_USERDATA, (LONG_PTR)&window.EventQueue);	// Set the address of our mousePosition variable so the window.EventHandler() can retrieve it with GetWindowLongPtr() and assign new coordinates to it.
	return 0;
}

int rgb::windowUpdate(SWindow & window) {
	MSG								msg					= {};
	while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if(msg.message == WM_QUIT) {
			window.EventQueue.push({rgb::EVENT_QUIT});
		}
	}

	return 0;
}

int rgb::windowPresent(::rgb::SWindow & window, const uint32_t * pixels) {
	const HDC								renderContext		= GetDC(window.Handle);	// Retrieve a handle to the drawing subsystem of the window..
	const HDC								compatibleContext	= CreateCompatibleDC(renderContext);	// Retrieve a handle to the drawing subsystem of the window..
	if(compatibleContext) {
		const uint32_t							windowArea			= window.Size.x * window.Size.y;

		BITMAPINFO								bitmapInfo			= {sizeof(BITMAPINFO)};
		bitmapInfo.bmiHeader.biWidth		= window.Size.x;
		bitmapInfo.bmiHeader.biHeight		= window.Size.y;
		bitmapInfo.bmiHeader.biBitCount		= 32;
		bitmapInfo.bmiHeader.biPlanes		= 1;
		bitmapInfo.bmiHeader.biCompression	= BI_RGB;
		bitmapInfo.bmiHeader.biSizeImage	= windowArea * sizeof(uint32_t);

		uint32_t								* dibBytes			= 0;
		HBITMAP									dibSection			= CreateDIBSection(compatibleContext, &bitmapInfo, DIB_RGB_COLORS, (void**)&dibBytes, 0, 0);
		if(dibSection) {
			const uint32_t							rowSizeInBytes		= window.Size.x * sizeof(uint32_t);
			for(uint32_t y = 0; y < (uint32_t)window.Size.y; ++y) {
				const uint32_t							cellOffset			= y * window.Size.x;
				memcpy(&dibBytes[window.Size.y * window.Size.x - window.Size.x - cellOffset], &pixels[cellOffset], rowSizeInBytes);
			}
			HGDIOBJ									oldObject			= SelectObject(compatibleContext, dibSection);
			BitBlt(renderContext, 0, 0, window.Size.x, window.Size.y, compatibleContext, 0, 0, SRCCOPY);
			SelectObject(compatibleContext, oldObject);
			DeleteObject(dibSection);
		}
		DeleteDC(compatibleContext);
	}

	ReleaseDC(window.Handle, renderContext);	// Use this to tell the window. system we're not drawing anymore so it can draw its own window. elements such as the title bar, close/minimize/maximize buttons, etc. 
	// As drawing on the window. needs to be done in order, this GetDC() and ReleaseDC() allow window.s to know it's your turn to draw and it prevents the window. system to draw on the window. area until you tell you're done drawing.
	return 0;
}