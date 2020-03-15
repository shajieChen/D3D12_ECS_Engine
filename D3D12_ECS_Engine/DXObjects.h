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
	Microsoft::WRL::ComPtr<IDXGISwapChain3> SwapChain; /*�����buffer*/
	Microsoft::WRL::ComPtr<ID3D12Resource> RenderTarget[FrameBufferCount];

	/*�Դ��ڴ����*/
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;  /*�൱��D3D�е�Context -- CPU�����б�*/
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;			/*GPU�������*/
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence[FrameBufferCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator[FrameBufferCount];
	UINT64 m_FenceValue[FrameBufferCount];
	HANDLE m_FenceEvent;

	int FrameIndex; //��ǰRenderTarget
	int RTVDescSize;

	const int frameBufferCount = FrameBufferCount;
};

#endif // !DXObjects
