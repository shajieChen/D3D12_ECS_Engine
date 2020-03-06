#pragma once
#include "pch.h"
#include <functional>
#include "Vertex.h"
#include "DXObjects.h"
#include "SContext.h"
#include "SConstantBuffer.h"
#ifndef APP_H
#define APP_H

#ifndef FrameBufferCount
#define FrameBufferCount 3 
#endif // !FrameBufferCount
 
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
	/*D3D12*/
	const int frameBufferCount = FrameBufferCount;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvDescriptorHeap;

	/*GPUCommandQueue & CPUCommandList*/
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;			/*GPU命令列表*/
	//ID3D12GraphicsCommandList* m_CommandList;							/*CPU命令列表*/
	ID3D12CommandAllocator* m_CommandAllocator[FrameBufferCount];		/*命令分配器*/

		/*Fence CPU*/
	ID3D12Fence* m_Fence[FrameBufferCount];
	HANDLE m_FenceEvent;
	UINT64 m_FenceValue[FrameBufferCount];

	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilBuffer;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsDescriptorHeap;					//深度模板的描述符缓冲堆

	ID3D12DescriptorHeap* mainDescriptorHeap[FrameBufferCount];						// 用于存储描述符的缓冲堆
	ID3D12Resource* constantBufferUploadHeap[FrameBufferCount];						//每一帧上面都有特定的共享内存Buffer来进行传输CPU 端上的ConstantBuffer 

	ConstantBuffer cbColorMultiplierData;											//用于vs中的VB/IB 和PS 中的颜色变化传入Buffer 

	UINT8* cbColorMultiplierGPUAddress[FrameBufferCount];							// this is a pointer to the memory location we get when we map our constant buffer


#pragma endregion
	/*屏幕*/
	ID3D12PipelineState* m_PipelineStateObject;
	ID3D12RootSignature* m_RootSignature;
	D3D12_VIEWPORT m_ViewPort;
	D3D12_RECT m_ScissorRect;

	D3D12_VERTEX_BUFFER_VIEW m_VBView; //test
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VB; //test

	ID3D12Resource* m_IB; //test
	D3D12_INDEX_BUFFER_VIEW m_IBView;//test

	int m_FrameIndex; //当前RenderTarget
	int m_RTVDescriptorSize; //描述当前前后buffer的大小 

};  
#endif // !APP_H
