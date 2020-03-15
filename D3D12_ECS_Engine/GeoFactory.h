#pragma once
#include <array>
#include "pch.h"
#include "SContext.h"
namespace example
{
	class GeoFactory
	{
	public:
		GeoFactory(Graphic::Context& context);
		~GeoFactory();

		Graphic::Mesh CreateTriangle();
		Graphic::Mesh CreateRetangle();
		Graphic::Mesh CreateBox();


		D3D12_INPUT_LAYOUT_DESC* GetIed() { return (D3D12_INPUT_LAYOUT_DESC*)m_ied.data(); }
		unsigned int GetIedElementCount() { return (unsigned int)m_ied.size(); }
		unsigned int GetIedByteWidth() { return sizeof(m_ied.data()); }
	private:
		std::array<D3D12_INPUT_ELEMENT_DESC, 2> m_ied;
		Graphic::Context& m_ct;
	};
}