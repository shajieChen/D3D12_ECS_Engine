#include "WinMain.h"
#include "pch.h"
#include "App.h"
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
#if defined(DEBUG) | defined(_DEBUG)
	// 检测是否内存泄漏
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//获取当前exe运行时候的路径
	{
		char currentDir[1024] = {};
		GetModuleFileName(0, reinterpret_cast<LPWSTR>(currentDir), 1024);
		char* last = strrchr(currentDir, '\\');
		if (last)
		{
			*last = 0;
			SetCurrentDirectory(reinterpret_cast<LPWSTR>(currentDir));
		}
	}

	App* app = new App(hInstance, nCmdShow, 1280, 720, false);
	app->Run([&](App* app)
	{
		//运行游戏逻辑
	});
	return 0;
}