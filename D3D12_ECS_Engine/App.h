#pragma once
#include "pch.h"
#include "Vertex.h"
#include "DXObjects.h"
#include "SContext.h"
#include "SConstantBuffer.h"
#include "Geo_Retangle.h"
#include <functional>
#ifndef APP_H
#define APP_H

class App
{
public:
	static App* instance;
	App(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullScreen);
	~App();
	void Run(std::function<void(App*)> callBack);
	LRESULT WinCallBack(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected: /*Private Helper Method*/
	void InitD3D12();
	void InitMainWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool isfullScreen);
	void InitWindow(HINSTANCE& hInstance);

	void testGrawTriangle();  /*deletable */
	void UpdateTriangle(); /*deletable*/

	void Update();
	//	virtual void Update(const GameTimer& gt)=0;
	void Render();
	void CleanUp();
	virtual void OnResize();

	/*Pirvate Helper Method*/
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChain();
	void CreateCommandObjects();

	void FlushCommandQueue();

private:
	DXObject m_dxo;
	Graphic::Context m_ct;

	IDXGIFactory4* m_DXGIFactory;

private:
	HWND hWnd;
	const LPCWSTR WinsName = L"D3D12_With_ECS";
	const LPCWSTR WinsTitle = L"D3D12_With_ECS";

	int m_Width;
	int m_Height;
	bool is_FullScreen;

#pragma region Renderer
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsDescriptorHeap;					//深度模板的描述符缓冲堆
	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilBuffer;
#pragma endregion

#pragma region tmpObject
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineStateObject;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

	Graphic::VertexBuffer VBviews;
	Graphic::IndexBuffer IBViews;

	std::unique_ptr<example::GeoRetangle> retExample;

	ID3D12Resource* textureBuffer;													// 默认堆上的资源缓冲
	ID3D12Resource* textureBuffer1;													// 默认堆上的资源缓冲
	ID3D12DescriptorHeap* mainDescriptorHeap;
	ID3D12Resource* textureBufferUploadHeap;										// 用于存储描述符的缓冲堆
	ID3D12Resource* constantBufferUploadHeap[FrameBufferCount];						//每一帧上面都有特定的共享内存Buffer来进行传输CPU 端上的ConstantBuffer

	UINT8* cbGPUAddress[FrameBufferCount];
	ConstantBuffer cbPerObject;														//用于vs中的VB/IB 和PS 中的颜色变化传入Buffer
	int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBuffer) + 255) & ~255;
	UINT8* cbvGPUAddress[FrameBufferCount];											// 用于存储存放常量缓冲区的指针


	XMFLOAT4X4 cameraProjMat;		// 用于存储投影矩阵
	XMFLOAT4X4 cameraViewMat;		// 用于存储View矩阵

	XMFLOAT4 cameraPosition;		// 相机位置向量
	XMFLOAT4 cameraTarget;			// 相加前方
	XMFLOAT4 cameraUp;				// 相机向上的方向

	XMFLOAT4X4 cube1WorldMat;		// 世界矩阵
	XMFLOAT4X4 cube1RotMat;			// 旋转矩阵
	XMFLOAT4 cube1Position;			// 正方体位置

	XMFLOAT4X4 cube2WorldMat;
	XMFLOAT4X4 cube2RotMat;
	XMFLOAT4 cube2PositionOffset;

	int numCubeIndices;				//当前绘制正方体的数量
#pragma endregion

	/*屏幕*/
	D3D12_VIEWPORT m_ViewPort;
	D3D12_RECT m_ScissorRect;
};
#endif // !APP_H
