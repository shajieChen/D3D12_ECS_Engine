#include "CRenderCommand.h" 
#include "pch.h"
RenderCommand::RenderCommand(DXObject& dxObjects):m_dxo(dxObjects)
{

}

RenderCommand::~RenderCommand()
{
}

Graphic::AttributeBuffer RenderCommand::CreateAttributeBuffer(void* vertices, unsigned int count, unsigned int stride, BOOLEAN isNeededUpload) const
{
	Microsoft::WRL::ComPtr<ID3D12Resource> VB = {};
	DX::ThrowIfFailed(CALL_INFO, 
		m_dxo.Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(count),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(VB.GetAddressOf()))
	);
	VB->SetName(L"Vertex Buffer Resource Heap");


	/*创建上传缓冲区*/
	Microsoft::WRL::ComPtr<ID3D12Resource> vBufferUploadHeap;
	DX::ThrowIfFailed(CALL_INFO, 
		m_dxo.Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(count),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vBufferUploadHeap)
		)
	);
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	//传输到上传缓冲区 
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vertices);
	vertexData.RowPitch = count;
	vertexData.SlicePitch = count;
	//创建VBView 
	D3D12_VERTEX_BUFFER_VIEW m_VBView;
	m_VBView.BufferLocation = VB->GetGPUVirtualAddress();
	m_VBView.StrideInBytes = stride;
	m_VBView.SizeInBytes = count;


    if (isNeededUpload)
    { 
		/*TODO : 封装成UploadConstantBuffer*/ 
		UpdateSubresources(m_dxo.CommandList.Get(), VB.Get(), vBufferUploadHeap.Get(),0 ,0 ,1 , &vertexData);
    }
    Graphic::AttributeBuffer ab = {};
	ab.DataBuffer = VB;  
	ab.UploadBuffer = vBufferUploadHeap;  
	ab.SizeInByte = count;  
	ab.StrideInByte = stride; 
	ab.ByteWidth = count * stride;  
    return ab;
}

void RenderCommand::Clear() const
{
}

void RenderCommand::Swap() const
{
    m_dxo.SwapChain->Present(0, 0);
}

void RenderCommand::UploadConstantBuffer() const
{
}
