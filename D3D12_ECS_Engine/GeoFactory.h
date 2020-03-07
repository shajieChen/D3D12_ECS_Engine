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
	private:
		std::array<D3D12_INPUT_ELEMENT_DESC, 2> m_ied; 
		Graphic::Context& m_ct; 

	}; 
}