#pragma once
#include "pch.h"
#ifndef FrameBufferCount
#define FrameBufferCount 3 
#endif // !FrameBufferCount

#ifndef DXObjects
#define DXObjects
struct  DXObject
{
	Microsoft::WRL::ComPtr<ID3D12Device> device; 
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain; /*后端三buffer*/
	/*Microsoft::WRL::ComPtr<ID3D12DescriptorHeap*/
	Microsoft::WRL::ComPtr<ID3D12Resource> RenderTarget[FrameBufferCount];

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;  /*相当于D3D中的Context*/

	//Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;/*GPU上层调用commandList*/
	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator*> CommandAllocator[FrameBufferCount];

	/*CPU Fence*/
	/*Microsoft::WRL::ComPtr<ID3D12Fence*> Fence[FrameBufferCount];
	HANDLE FenceEvnet;
	UINT64 FenceValue[FrameBufferCount];*/
};


#endif // !DXObjects
