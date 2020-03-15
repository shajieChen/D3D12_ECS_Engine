#pragma once
#include "pch.h"
#ifndef MESH_H
#define MESH_H
namespace Graphic
{
	struct AttributeBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> DataBuffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> UploadBuffer;
		D3D12_SUBRESOURCE_DATA* Data;
		unsigned int SizeInByte;
		unsigned int StrideInByte;
		unsigned int ByteWidth;
	};
	/*
	* ���ڷ�װVertexBufferҪ��
	*     ������Ĭ�ϻ�����е�VB
	*     �Լ����ϴ�������д���UploadHeap

	*/
	struct VertexBuffer
	{
		std::vector<D3D12_VERTEX_BUFFER_VIEW> VBViews;
	};

	struct IndexBuffer
	{
		std::vector<D3D12_INDEX_BUFFER_VIEW> IBViews;
	};

	struct Mesh
	{
		VertexBuffer VB;
		IndexBuffer IB;

		/*TODO: Material*/
		//unsigned int matType = 0;
		//.....TODO adding the material Type
	};
}
#endif // !MESH_H
