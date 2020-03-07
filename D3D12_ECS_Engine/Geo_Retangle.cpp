#include "Geo_Retangle.h"
#include "GeoFactory.h"
namespace example
{
	GeoRetangle::GeoRetangle(Graphic::Context& context) : m_ct(context)
	{
		GeoFactory geoFactory(context);

		/* 
		//vs 
		ID3DBlob* vertexShader;
		D3DReadFileToBlob(L"DefaultVS.cso", &vertexShader);
		D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
		vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
		vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

		//ps
		ID3DBlob* pixelShader;
		D3DReadFileToBlob(L"DefaultPS.cso", &pixelShader);
		D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
		pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
		pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

		//ied 
		D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		//inputlayout 
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
		inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
		inputLayoutDesc.pInputElementDescs = inputLayout;

		//Samplr
		DXGI_SAMPLE_DESC  sampleDesc = {};
		sampleDesc.Count = 1;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = inputLayoutDesc;
		psoDesc.pRootSignature = m_RootSignature;
		psoDesc.VS = vertexShaderBytecode;
		psoDesc.PS = pixelShaderBytecode;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc = sampleDesc;
		psoDesc.SampleMask = 0xffffffff;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.NumRenderTargets = 1;
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

		m_dxo.Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineStateObject));

		//vb 
		Vertex vList[] = {
			{ -0.5f,  0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f },
			{ -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
			{ 0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f }
		};
		Graphic::AttributeBuffer ab = {};
		ab = m_ct.rcommand->CreateDefaultBuffer(vList, sizeof(vList), sizeof(Vertex));

		D3D12_VERTEX_BUFFER_VIEW m_VBView;
		m_VBView.BufferLocation = ab.DataBuffer->GetGPUVirtualAddress();
		m_VBView.StrideInBytes = ab.StrideInByte;
		m_VBView.SizeInBytes = ab.SizeInByte;

		VBviews.VBViews.push_back(m_VBView);
		//ib
		DWORD iList[] = {
			0, 1, 2,
			0, 3, 1
		};
		ab = m_ct.rcommand->CreateDefaultBuffer(iList, sizeof(iList), sizeof(DWORD));

		D3D12_INDEX_BUFFER_VIEW m_IBView;
		m_IBView.BufferLocation = ab.DataBuffer->GetGPUVirtualAddress();
		m_IBView.Format = DXGI_FORMAT_R32_UINT;
		m_IBView.SizeInBytes = ab.SizeInByte;

		IBViews.IBViews.push_back(m_IBView);

		// 为每一帧都创建一个常量缓冲区描述符
		for (int i = 0; i < frameBufferCount; ++i)
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.NumDescriptors = 1;
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			m_dxo.Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap[i]));
		}

		// 创建上传缓冲区(共享缓冲区) --- 包括 resources heap/Descriptor Heap/指向ConstantBufferView的指针
		for (int i = 0; i < frameBufferCount; ++i)
		{
			m_dxo.Device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //指明为上传缓冲堆
				D3D12_HEAP_FLAG_NONE, // no flags
				&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), //资源堆的大小必须为64KB的大小的倍数
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&constantBufferUploadHeap[i]));
			constantBufferUploadHeap[i]->SetName(L"常量缓冲区的上传缓冲堆");

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = constantBufferUploadHeap[i]->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;    //按照256字节排布
			m_dxo.Device->CreateConstantBufferView(&cbvDesc, mainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());

			ZeroMemory(&cbColorMultiplierData, sizeof(cbColorMultiplierData));

			CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
			constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbColorMultiplierGPUAddress[i]));
			memcpy(cbColorMultiplierGPUAddress[i], &cbColorMultiplierData, sizeof(cbColorMultiplierData));
		}


		m_dxo.CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { m_dxo.CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		//添加m_Fence 保证再会之前
		m_FenceValue[m_FrameIndex]++;
		DX::ThrowIfFailed(CALL_INFO,
			m_CommandQueue->Signal(m_Fence[m_FrameIndex], m_FenceValue[m_FrameIndex])
		);
		*/
	}

	GeoRetangle::~GeoRetangle()
	{
	}

	void GeoRetangle::Render()
	{
	}
	 
	void GeoRetangle::Update()
	{

	}

	void GeoRetangle::UpdateGUI()
	{
	}

}