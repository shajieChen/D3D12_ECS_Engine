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
		////vb
		//Vertex vList[] = {
		//	{ -0.5f,  0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f },
		//	{ 0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f },
		//	{ -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
		//	{ 0.5f,  0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f }
		//};
		//Graphic::AttributeBuffer ab = {};
		//ab = m_ct.rcommand->CreateDefaultBuffer(vList, sizeof(vList), sizeof(Vertex));

		//D3D12_VERTEX_BUFFER_VIEW m_VBView;
		//m_VBView.BufferLocation = ab.DataBuffer->GetGPUVirtualAddress();
		//m_VBView.StrideInBytes = ab.StrideInByte;
		//m_VBView.SizeInBytes = ab.SizeInByte;

		//Graphic::VertexBuffer vertexBuffer;
		//vertexBuffer.VBViews.push_back(m_VBView);

		////ib
		//DWORD iList[] = {
		//	0, 1, 2,
		//	0, 3, 1
		//};
		//ab = m_ct.rcommand->CreateDefaultBuffer(iList, sizeof(iList), sizeof(DWORD));

		//D3D12_INDEX_BUFFER_VIEW m_IBView;
		//m_IBView.BufferLocation = ab.DataBuffer->GetGPUVirtualAddress();
		//m_IBView.Format = DXGI_FORMAT_R32_UINT;
		//m_IBView.SizeInBytes = ab.SizeInByte;

		//Graphic::IndexBuffer indexBuffer;
		//indexBuffer.IBViews.push_back(m_IBView);

		Graphic::Mesh mesh = {};
		//mesh.IB = indexBuffer;
		//mesh.VB = vertexBuffer;
		return mesh;
	}
	Graphic::Mesh GeoFactory::CreateBox()
	{
		//vb

		Vertex vList[] = {
			// front face
			{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f },
			{ 0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
			{ -0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f,  0.5f, -0.5f, 1.0f, 0.0f },

			// right side face
			{ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
			{ 0.5f, -0.5f,  0.5f, 1.0f, 1.0f },
			{ 0.5f,  0.5f, -0.5f, 0.0f, 0.0f },

			// left side face
			{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
			{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
			{ -0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, -0.5f, 1.0f, 0.0f },

			// back face
			{ 0.5f,  0.5f,  0.5f, 0.0f, 0.0f },
			{ -0.5f, -0.5f,  0.5f, 1.0f, 1.0f },
			{ 0.5f, -0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f,  0.5f, 1.0f, 0.0f },

			// top face
			{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f },
			{ 0.5f,  0.5f,  0.5f, 1.0f, 0.0f },
			{ 0.5f,  0.5f, -0.5f, 1.0f, 1.0f },
			{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f },

			// bottom face
			{ 0.5f, -0.5f,  0.5f, 0.0f, 0.0f },
			{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
			{ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
			{ -0.5f, -0.5f,  0.5f, 1.0f, 0.0f },
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
				// front face
				0, 1, 2, // first triangle
				0, 3, 1, // second triangle

				 // left face
				4, 5, 6, // first triangle
				4, 7, 5, // second triangle

				// right face
				8, 9, 10, // first triangle
				8, 11, 9, // second triangle

				// back face
				12, 13, 14, // first triangle
				12, 15, 13, // second triangle

				// top face
				16, 17, 18, // first triangle
				16, 19, 17, // second triangle

				// bottom face
				20, 21, 22, // first triangle
				20, 23, 21, // second triangle
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