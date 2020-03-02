#pragma once
#include "DXObjects.h"
#ifndef RENDERCOMMAND_H
#define RENDERCOMMAND_H 
class RenderCommand
{
public:
	RenderCommand(DXObject& dxObjects);
	~RenderCommand();

	//TODO : Create Buffer

	void CleanUp() const;
	void Swap() const; 
		
	//TEMP TO create Triangle
	void DrawTriangle(); //test deletable
private:
	DXObject& m_dxo;   
}; 

#endif // !RENDERCOMMAND_H
