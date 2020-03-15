#include "Geo_Retangle.h"
#include "GeoFactory.h"
#include "SConstantBuffer.h"
namespace example
{
	GeoRetangle::GeoRetangle(Graphic::Context& context) : m_ct(context)
	{
		GeoFactory geoFactory(context);

		//vs
		Graphic::Shader vertexShader = m_ct.rcommand->CreateShader(L"DefaultVS.cso");
		D3D12_SHADER_BYTECODE vertexShaderBytecode = { vertexShader.byteCode->GetBufferPointer(),
													   vertexShader.byteCode->GetBufferSize() };

		//ps
		Graphic::Shader pixelShader = m_ct.rcommand->CreateShader(L"DefaultPS.cso");
		D3D12_SHADER_BYTECODE pixelShaderBytecode = { pixelShader.byteCode->GetBufferPointer(),
													  pixelShader.byteCode->GetBufferSize() };
		//ied
		D3D12_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
		inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
		inputLayoutDesc.pInputElementDescs = inputLayout;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.VS = vertexShaderBytecode;
		psoDesc.PS = pixelShaderBytecode;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc = { 1 };
		psoDesc.SampleMask = 0xffffffff;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.NumRenderTargets = 1;
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		m_ct.rcommand->CreatePSO(&psoDesc, m_PipelineStateObject.Get());

		Graphic::Mesh meshViews = geoFactory.CreateRetangle();
		VBviews.VBViews.insert(VBviews.VBViews.end(), meshViews.VB.VBViews.begin(), meshViews.VB.VBViews.end());
		IBViews.IBViews.insert(IBViews.IBViews.end(), meshViews.IB.IBViews.begin(), meshViews.IB.IBViews.end());
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

	void GeoRetangle::Release()
	{
	}
}