#include "WinMain.h"
#include "pch.h"
 
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {

	const LPCWSTR pClassName = L"D3D12_With_ECS";
	 
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = pClassName;
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);
	 
	HWND hWnd = CreateWindowEx(
		0, pClassName, L"D3D12_With_ECS",
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		200, 200, 640, 480, nullptr, nullptr,
		hInstance, nullptr
	);

	ShowWindow(hWnd, SW_SHOW);
	while (true);

	return 0;
}