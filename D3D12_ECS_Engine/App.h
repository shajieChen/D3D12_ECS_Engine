#pragma once
#include "pch.h"
#include <functional>
#include "Vertex.h"
#include "DXObjects.h"
#include "SContext.h"
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
#pragma region Renderer 
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
#pragma endregion
	/*��Ļ*/
	ID3D12PipelineState* m_PipelineStateObject; 
	ID3D12RootSignature* m_RootSignature;
	D3D12_VIEWPORT m_ViewPort; 
	D3D12_RECT m_ScissorRect; 

	D3D12_VERTEX_BUFFER_VIEW m_VBView; //test
	ID3D12Resource* m_VB; //test

	ID3D12Resource* m_IB; //test
	D3D12_INDEX_BUFFER_VIEW m_IBView;//test

	int m_FrameIndex; //��ǰRenderTarget
	int m_RTVDescriptorSize; //������ǰǰ��buffer�Ĵ�С 

private: /*Private Helper Method*/
	void InitD3D12();
	void InitWindow(HINSTANCE& hInstance);
	//bool InitImGUI();
	//bool InitGraphicSingletonEntity(); 

	void testGrawTriangle();  /*deletable */
	void UpdateTriangle(); /*deletable*/


	void Update(); 
	void Render(); 
	void CleanUp(); 
	void WaitForPreviousFrame(); 

	void InitializeWind(HINSTANCE hInstance, int ShowWnd, int width, int height, bool isfullScreen);

private:
	DXObject m_dxo; 
	//Graphic::Context m_ct; 


};  
#endif // !APP_H
