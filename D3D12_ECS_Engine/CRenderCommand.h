#pragma once
#include "DXObjects.h"
#ifndef RENDERCOMMAND_H
#define RENDERCOMMAND_H 
class RenderCommand
{
public:
	RenderCommand(DXObject& dxObjects);
	~RenderCommand();

	//TODO : Create Buffer VB/IB
	void CreateAttributeBuffer() const; 
	void CreateIndexBuffer() const; 
	void CreateConstantBuffer() const;

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

	void UpdateConstantBuffer() const; 

	void Draw(unsigned int count) const;
	void DrawIndexed(unsigned int count) const;

	void Clear() const;
	void Swap() const;
		 
private:
	DXObject& m_dxo;   
}; 

#endif // !RENDERCOMMAND_H
