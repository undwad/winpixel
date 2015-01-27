/*
** main.cpp by undwad
** Pick color from windows desktop
** https://github.com/undwad/winpixel mailto:undwad@mail.ru
** see copyright notice in ./LICENCE
*/

#include <stdio.h>
#include <windows.h>

using namespace std;

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define HID_USAGE_PAGE_GENERIC 1
#define HID_USAGE_GENERIC_MOUSE 2

#define CHECK(CONDITION) \
    if(!CONDITION) \
	    { \
        PostQuitMessage(-1); \
        return 0; \
	    }

LRESULT window_proc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (WM_INPUT == msg)
	{
		HRAWINPUT hraw = (HRAWINPUT)lParam;
		RAWINPUT raw = { 0 };
		UINT size = sizeof(raw);
		UINT res = GetRawInputData(hraw, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));
		CHECK((UINT)-1 == res);
		if (sizeof(raw) == res && RIM_TYPEMOUSE == raw.header.dwType && raw.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
		{
			POINT pos = { 0 };
			CHECK(GetCursorPos(&pos));
			HDC dc0 = GetDC(0);
			CHECK(dc0);
			HDC dc = CreateCompatibleDC(dc0);
			CHECK(dc);
			BITMAPINFO info = { { sizeof(BITMAPINFOHEADER), 1, 1, 1, 32, BI_RGB, 0, 0, 0, 0, 0 }, { 0 } };
			unsigned char* ptr = nullptr;
			HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, (void**)&ptr, 0, 0);
			CHECK(bitmap && ptr);
			CHECK(SelectObject(dc, bitmap));
			CHECK(BitBlt(dc, 0, 0, 1, 1, dc0, pos.x, pos.y, SRCCOPY | CAPTUREBLT));
			wchar_t result[MAX_PATH] = { 0 };
			swprintf_s(result, sizeof(result), L"#%02x%02x%02x%02x", (int)ptr[3], (int)ptr[2], (int)ptr[1], (int)ptr[0]);
			CHECK(OpenClipboard(0));
			CHECK(EmptyClipboard());
			HGLOBAL handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, sizeof(result));
			CHECK(handle);
			void* buffer = GlobalLock(handle);
			CHECK(buffer);
			memcpy(buffer, result, sizeof(result));
			GlobalUnlock(handle);
			SetClipboardData(CF_UNICODETEXT, handle);
			PostQuitMessage(0);
		}
		return 0;
	}
	return DefWindowProc(window, msg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE module, HINSTANCE, LPSTR, int)
{
	WNDCLASSEX info = { sizeof(WNDCLASSEX), 0, (WNDPROC)window_proc, 0, 0, module, nullptr, nullptr, nullptr, nullptr, L"Desktop Color Picker by Undwad", nullptr };
	RegisterClassEx(&info);
	HWND window = CreateWindowEx(WS_EX_APPWINDOW, info.lpszClassName, info.lpszClassName, WS_POPUP | WS_VISIBLE | WS_SYSMENU, 0, 0, 0, 0, nullptr, nullptr, module, 0);
	if (window)
	{
		RAWINPUTDEVICE rid = { HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_MOUSE, RIDEV_INPUTSINK, window };
		if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
		{
			MSG msg = { 0 };
			while (WM_QUIT != msg.message)
				if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
				{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				}
			return (int)msg.wParam;
		}
	}
	return -1;
}

