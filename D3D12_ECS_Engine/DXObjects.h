#pragma once
#include "pch.h"
#ifndef FrameBufferCount
#define FrameBufferCount 3 
#endif // !FrameBufferCount

#ifndef DXObjects
#define DXObjects
struct  DXObject
{
	Microsoft::WRL::ComPtr<ID3D12Device> Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain; /*后端三buffer*/
	Microsoft::WRL::ComPtr<ID3D12Resource> RenderTarget[FrameBufferCount];
	
	/*显存内存管理*/
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;  /*相当于D3D中的Context -- CPU命令列表*/
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;			/*GPU命令队列*/
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence[FrameBufferCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator[FrameBufferCount];
	UINT64 m_FenceValue[FrameBufferCount];
	HANDLE m_FenceEvent; 

	int FrameIndex; //当前RenderTarget
	int RTVDescSize;

	const int frameBufferCount = FrameBufferCount;
};


#endif // !DXObjects
