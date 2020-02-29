#pragma once
#include "pch.h"
#include <functional>
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

private:
	HWND hWnd; 
	const LPCWSTR WinsName = L"D3D12_With_ECS";
	const LPCWSTR WinsTitle = L"D3D12_With_ECS";

	int m_Width; 
	int m_Height; 
	bool is_FullScreen; 
	bool is_Running; 

	/*D3D12*/
	const int frameBufferCount = FrameBufferCount;						/*ʹ��������������*/

	ID3D12Device* m_device;													
	IDXGISwapChain3* m_SwapChain; 
	ID3D12DescriptorHeap* m_rtvDescriptorHeap; 
	ID3D12Resource* m_RenderTarget[FrameBufferCount];

		/*GPUCommandQueue & CPUCommandList*/
	ID3D12CommandQueue* m_CommandQueue;									/*GPU�����б�*/
	ID3D12GraphicsCommandList* m_CommandList;							/*CPU�����б�*/
	ID3D12CommandAllocator* m_CommandAllocator[FrameBufferCount];		/*���������*/

		/*Fence CPU*/
	ID3D12Fence* m_Fence[FrameBufferCount];
	HANDLE m_FenceEvent; 
	UINT64 m_FenceValue[FrameBufferCount];

	/*��Ļ*/
	ID3D12PipelineState* m_PipelineStateObject; 
	ID3D12RootSignature* m_RootSignature;
	D3D12_VIEWPORT* m_ViewPort; 
	D3D12_RECT m_ScissorRect; 

	D3D12_VERTEX_BUFFER_VIEW m_VB; //test
	ID3D12Resource* m_shader; //test

	int m_FrameIndex; //��ǰRenderTarget
	int m_RTVDescriptorSize; //������ǰǰ��buffer�Ĵ�С 

private: /*Private Helper Method*/
	void InitD3D12();
	//bool InitImGUI();
	//bool InitGraphicSingletonEntity(); 

	void Update(); 
	void Render(); 
	void CleanUp(); 
	void WaitForPreviousFrame(); 

	void InitializeWind(HINSTANCE hInstance, int ShowWnd, int width, int height, bool isfullScreen);
};  
#endif // !APP_H
