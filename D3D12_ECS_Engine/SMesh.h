#pragma once
#include "pch.h"
#ifndef MESH_H
#define MESH_H
namespace Graphic
{
	struct AttributeBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer; 
		unsigned int stride = 0;
		unsigned int count = 0;
		unsigned int byteWidth = 0;
	};

	struct VertexBuffer
	{

	}; 

	struct IndexBuffer
	{

	};

	struct Mesh
	{
		VertexBuffer VB;  
		IndexBuffer IB;

		/*Material*/
		unsigned int matType = 0; 
		//.....TODO adding the material Type
	};
}

#endif // !MESH_H
