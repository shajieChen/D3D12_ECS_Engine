#pragma once
#include "DXObjects.h"
#include "SShader.h"
#include "SMesh.h"
#include "pch.h"
#ifndef RENDERCOMMAND_H
#define RENDERCOMMAND_H 
class RenderCommand
{
public:
	RenderCommand(DXObject& dxObjects);
	~RenderCommand();

	//TODO : Create Buffer VB/IB
	/*
	* vertices:��ǰ��������ݣ� 
	  count:��ǰ����������С, 
	  stride:��ǰԪ�صĴ�С��
	  isNeededUpdate: �ж��Ƿ�Ҫ���뻺����
	*/
	Graphic::AttributeBuffer CreateDefaultBuffer(void* vertices, unsigned int count, unsigned int stide) const; 
	void CreateIndexBuffer() const; 
	void CreateConstantBuffer() const;
	
	/*
	*/
	ID3D12RootSignature* CreateRootSignature() const;

	//TODO: Create Shader 
	Graphic::Shader CreateShader(LPCWSTR filePath) const;
	void CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC*, ID3D12PipelineState*) const;
	void CreateSampler() const; 

	//TODO : Bind Buffer
	void BindVertexBuffer() const; 
	void BindIndexBuffer() const; 

	void BindSampler() const; 
	void BindTexture() const; 

	void BindVertexShader() const; 
	void BindPixelShader() const; 

	/*�ϴ����������� ---- ��ΪDefault ��*/
	void UploadVertexBuffer(Graphic::AttributeBuffer& ab) const;
	void UpdateConstantBuffer() const; 

	void Draw(unsigned int count) const;
	void DrawIndexed(unsigned int count) const;

	void Clear() const;
	void Swap() const;
		 
	void CPURecordStamp() const; 

	void WaitForPreviousFrame() const; 
/*HelperMethod*/
public : 
	inline int GetFrameIndex() const {return m_dxo.FrameIndex; }
private:
	DXObject& m_dxo;   
}; 

#endif // !RENDERCOMMAND_H
