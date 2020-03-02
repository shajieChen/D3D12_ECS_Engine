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
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain; /*�����buffer*/
	/*Microsoft::WRL::ComPtr<ID3D12DescriptorHeap*/
	Microsoft::WRL::ComPtr<ID3D12Resource> RenderTarget[FrameBufferCount];

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;  /*�൱��D3D�е�Context*/

	//Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue;/*GPU�ϲ����commandList*/
	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator*> CommandAllocator[FrameBufferCount];

	/*CPU Fence*/
	/*Microsoft::WRL::ComPtr<ID3D12Fence*> Fence[FrameBufferCount];
	HANDLE FenceEvnet;
	UINT64 FenceValue[FrameBufferCount];*/
};


#endif // !DXObjects
