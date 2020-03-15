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
		~GeoRetangle();
		virtual void Render() override;
		virtual void Update() override;
		virtual void UpdateGUI() override;
		virtual void Release() override;
	private:
		Graphic::Context& m_ct;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineStateObject;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

		Graphic::VertexBuffer VBviews;
		Graphic::IndexBuffer IBViews;

		std::unique_ptr<example::GeoRetangle> retExample;

		ID3D12DescriptorHeap* mainDescriptorHeap[FrameBufferCount];						// ���ڴ洢�������Ļ����
		ID3D12Resource* constantBufferUploadHeap[FrameBufferCount];						//ÿһ֡���涼���ض��Ĺ����ڴ�Buffer�����д���CPU ���ϵ�ConstantBuffer

		UINT8* cbColorMultiplierGPUAddress[FrameBufferCount];							// this is a pointer to the memory location we get when we map our constant buffer

		//ConstantBuffer cbColorMultiplierData;											//����vs�е�VB/IB ��PS �е���ɫ�仯����Buffer
	};
}

#endif // !TRIANGLE_H
