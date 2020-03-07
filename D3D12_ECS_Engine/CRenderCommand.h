#pragma once
#include "DXObjects.h"
#include "SMesh.h"
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
	void CreateVertexShader() const; 
	void CreatePixelShader() const; 
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
		 
private:
	DXObject& m_dxo;   
}; 

#endif // !RENDERCOMMAND_H
