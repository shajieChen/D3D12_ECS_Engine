#include "CRenderCommand.h" 
#include "pch.h"
RenderCommand::RenderCommand(DXObject& dxObjects):m_dxo(dxObjects)
{

}

RenderCommand::~RenderCommand()
{
}

Graphic::AttributeBuffer RenderCommand::CreateDefaultBuffer(void* vertices, unsigned int count, unsigned int stride) const
{
	ID3D12Resource* dataBuffer = {};
	/*������Դ������*/
	DX::ThrowIfFailed(CALL_INFO, 
		m_dxo.Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(count),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&dataBuffer))
	);
	dataBuffer->SetName(L"Vertex Buffer Resource Heap");

	/*�ϴ��м仺����*/
	ID3D12Resource* interUploadHeap;
	DX::ThrowIfFailed(CALL_INFO, 
		m_dxo.Device->CreateCommittedResource
		(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(count),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&interUploadHeap)
		)
	);
	interUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	//���䵽�ϴ������� 
	D3D12_SUBRESOURCE_DATA data = {};
	data.pData = reinterpret_cast<BYTE*>(vertices);
	data.RowPitch = count;
	data.SlicePitch = count; 

    Graphic::AttributeBuffer ab = {};
	ab.DataBuffer = dataBuffer;  
	ab.UploadBuffer = interUploadHeap;  
	ab.SizeInByte = count;  
	ab.StrideInByte = stride; 
	ab.ByteWidth = count * stride;  

	m_dxo.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ab.DataBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)); 
	UpdateSubresources(m_dxo.CommandList.Get(),
					   ab.DataBuffer.Get(), 
					  ab.UploadBuffer.Get(), 0, 0, 1, &data);
	m_dxo.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(ab.DataBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	/*TODO: ����Ҫȷ���ϴ������������ܹ���� ֻ���ڵ���commandlist֮����ܹ��������� */
    return ab;
}

void RenderCommand::Clear() const
{
}

void RenderCommand::Swap() const
{
    m_dxo.SwapChain->Present(0, 0);
}
 