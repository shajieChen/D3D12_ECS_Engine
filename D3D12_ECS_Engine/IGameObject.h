#pragma once
#ifndef IGAMEOBJECT_H
#define IGAMEOBJECT_H 
namespace example
{
	class IGameObejct
	{
	public: 
		virtual	~IGameObejct() {} ;
		virtual void UpdateGUI() = 0 ;
		virtual void Render() = 0; 
		virtual void Update() = 0 ; 
		virtual void Release() = 0; 
	}; 
}  
#endif // !IGAMEOBJECT_H
