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
	App(HINSTANCE hInstance, int ShowWnd , int width, int height, bool fullScreen);
	~App();
	void Run(std::function<void(App*)> callBack);
	LRESULT WinCallBack(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected: /*Private Helper Method*/
	void InitD3D12();
	void InitMainWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool isfullScreen);
	void InitWindow(HINSTANCE& hInstance); 
	void BuildRootSignature();


	void testGrawTriangle();  /*deletable */
	void UpdateTriangle(); /*deletable*/
	 
	void Update(); 
	//	virtual void Update(const GameTimer& gt)=0;
	void Render(); 
	void CleanUp(); 
	virtual void OnResize();
	void WaitForPreviousFrame(); 

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

	/*GPUCommandQueue & CPUCommandList*/
	ID3D12CommandAllocator* m_CommandAllocator[FrameBufferCount];		/*命令分配器*/

		/*Fence CPU*/
	//Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence[FrameBufferCount];
	HANDLE m_FenceEvent;
	UINT64 m_FenceValue[FrameBufferCount];

	ID3D12DescriptorHeap* mainDescriptorHeap[FrameBufferCount];						// 用于存储描述符的缓冲堆
	ID3D12Resource* constantBufferUploadHeap[FrameBufferCount];						//每一帧上面都有特定的共享内存Buffer来进行传输CPU 端上的ConstantBuffer 

	ConstantBuffer cbColorMultiplierData;											//用于vs中的VB/IB 和PS 中的颜色变化传入Buffer 

	UINT8* cbColorMultiplierGPUAddress[FrameBufferCount];							// this is a pointer to the memory location we get when we map our constant buffer


#pragma endregion
	
#pragma region tmpObject
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineStateObject;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

	Graphic::VertexBuffer VBviews;
	Graphic::IndexBuffer IBViews;

	std::unique_ptr<example::GeoRetangle> retExample;
#pragma endregion 

	/*屏幕*/
	D3D12_VIEWPORT m_ViewPort;
	D3D12_RECT m_ScissorRect; 

};  
#endif // !APP_H
