#include "CRenderCommand.h"

RenderCommand::RenderCommand(DXObject& dxObjects):m_dxo(dxObjects)
{

}

RenderCommand::~RenderCommand()
{
}

void RenderCommand::CleanUp() const
{
}

void RenderCommand::Swap() const
{
    m_dxo.SwapChain->Present(0, 0);
}
 

void RenderCommand::DrawTriangle()
{
}
