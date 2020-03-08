#pragma once
#include "pch.h"
#ifndef SHADER_H
#define SHADER_H 
namespace Graphic
{
	struct ConstantBuffer
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
		unsigned int byteWidth = 0;
	};

	struct Shader
	{ 
		Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
		std::vector< Microsoft::WRL::ComPtr<ID3D12Resource>> ConstantBuffers;//TODO : »»³ÉConstantBufferView
	};

	struct PSO
	{

	};
}
#endif // !SHADER_H
