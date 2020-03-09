#pragma once
#include "pch.h"
#include "SContext.h" 
#include "IGameObject.h"
#ifndef TRIANGLE_H
#define TRIANGLE_H
namespace example
{ 
	class GeoRetangle : public IGameObejct
	{
	public:
		GeoRetangle(Graphic::Context& context);
		virtual ~GeoRetangle();
		virtual void Render() override; 
		virtual void Update() override; 
		virtual void UpdateGUI() override; 
	private:
		Graphic::Context& m_ct; 
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineStateObject;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

		Graphic::VertexBuffer m_VertexBuffer;
		Graphic::IndexBuffer m_IndexBuffer; 
	};
}



#endif // !TRIANGLE_H
