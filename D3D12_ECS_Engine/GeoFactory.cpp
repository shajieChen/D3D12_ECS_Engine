#include "GeoFactory.h"
#include "Vertex.h"
namespace example
{
	GeoFactory::GeoFactory(Graphic::Context& context) : m_ct(context)
	{
		m_ied.at(0) = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		m_ied.at(1) = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	}

	GeoFactory::~GeoFactory()
	{

	}

	Graphic::Mesh GeoFactory::CreateRetangle()
	{
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

		Graphic::VertexBuffer vertexBuffer;
		vertexBuffer.VBViews.push_back(m_VBView);

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

		Graphic::IndexBuffer indexBuffer;
		indexBuffer.IBViews.push_back(m_IBView);


		Graphic::Mesh mesh = {};
		mesh.IB = indexBuffer;
		mesh.VB = vertexBuffer;
		return mesh;
	}

}