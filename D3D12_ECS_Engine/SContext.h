#pragma once
#include "pch.h"
#include "CRenderCommand.h"
namespace Graphic
{
	struct Context
	{
		Context() {}
		std::unique_ptr<RenderCommand> rcommand;
	};
}