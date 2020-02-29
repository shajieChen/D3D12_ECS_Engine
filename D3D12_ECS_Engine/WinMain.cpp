#include "WinMain.h"
#include "pch.h"
#include "App.h" 
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	//获取当前exe运行时候的路径
	{
		LPWSTR currentDir = {}; 
		GetModuleFileName(0, currentDir, 1024);
		char* last = strrchr(reinterpret_cast<char*>(currentDir), '\\');
		if (last)
		{
			*last = 0; 
			SetCurrentDirectory(currentDir);
		}
	}
	App* app = new App(hInstance,  nCmdShow , 1280 , 720 , true);
	app->Run([&](App* app)
	{
		//运行游戏逻辑
	});
	return 0; 
}