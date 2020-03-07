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
		virtual void Update() override; 
		virtual void UpdateGUI() override; 
	private:
		Graphic::Context& m_ct; 
	};
}



#endif // !TRIANGLE_H
