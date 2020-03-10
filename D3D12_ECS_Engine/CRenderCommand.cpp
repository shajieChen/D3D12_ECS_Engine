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
	/*创建资源独立堆*/
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

	/*上传中间缓冲区*/
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

	//传输到上传缓冲区 
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

	/*TODO: 这里要确保上传缓冲区对象能够存活 只有在调用commandlist之后才能够进行销毁 */
    return ab;
}

ID3D12RootSignature* RenderCommand::CreateRootSignature() const
{
	ID3D12RootSignature* RootSignature; 

	D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1];/*TODO: 修改1 避免Magic Number */
	descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descriptorTableRanges[0].NumDescriptors = 1;
	descriptorTableRanges[0].BaseShaderRegister = 0;
	descriptorTableRanges[0].RegisterSpace = 0;
	descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	/*创建描述符表*/
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges);
	descriptorTable.pDescriptorRanges = &descriptorTableRanges[0];

	D3D12_ROOT_PARAMETER  rootParameters[1];
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].DescriptorTable = descriptorTable;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;


	/*创建根签名*/
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameters),
		rootParameters,
		0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);


	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	DX::ThrowIfFailed(CALL_INFO,
		D3D12SerializeRootSignature(&rootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			signature.GetAddressOf(),
			errorBlob.GetAddressOf())
	);
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.Device->CreateRootSignature(0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature))
	);
	return RootSignature; 
}

Graphic::Shader RenderCommand::CreateShader(LPCWSTR filePath) const
{
	Graphic::Shader shader = { }; 
	ID3DBlob* tmpShader;
	DX::ThrowIfFailed(CALL_INFO, 
		D3DReadFileToBlob(filePath, &tmpShader)
	);
	shader.byteCode = tmpShader; 
	return shader;
}

void RenderCommand::CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* psoDesc, ID3D12PipelineState* m_PipelineStateObject) const
{
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.Device->CreateGraphicsPipelineState(psoDesc,
			IID_PPV_ARGS(&m_PipelineStateObject))
	);
}

 
void RenderCommand::Clear() const
{
}

void RenderCommand::Swap() const
{
    m_dxo.SwapChain->Present(0, 0);
}

void RenderCommand::CPURecordStamp() const
{ 
	m_dxo.CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_dxo.CommandList.Get() };
	/*CPU上传到GPU CList */
	m_dxo.m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//添加m_Fence 保证在绘制之前
	m_dxo.m_FenceValue[m_dxo.FrameIndex]++;
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.m_CommandQueue->Signal(m_dxo.m_Fence[m_dxo.FrameIndex].Get(), m_dxo.m_FenceValue[m_dxo.FrameIndex])
	); 
}

void RenderCommand::WaitForPreviousFrame() const
{
	m_dxo.FrameIndex = m_dxo.SwapChain->GetCurrentBackBufferIndex();

	if (m_dxo.m_Fence[m_dxo.FrameIndex]->GetCompletedValue() < m_dxo.m_FenceValue[m_dxo.FrameIndex])
	{
		DX::ThrowIfFailed(CALL_INFO,
			m_dxo.m_Fence[m_dxo.FrameIndex]->SetEventOnCompletion(m_dxo.m_FenceValue[m_dxo.FrameIndex], m_dxo.m_FenceEvent)
		);

		WaitForSingleObject(m_dxo.m_FenceEvent, INFINITE);
	}
	m_dxo.m_FenceValue[m_dxo.FrameIndex]++;
}
 