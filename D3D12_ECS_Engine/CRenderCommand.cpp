#include "CRenderCommand.h"
#include "Vertex.h"
RenderCommand::RenderCommand(DXObject& dxObjects):m_dxo(dxObjects)
{

}

RenderCommand::~RenderCommand()
{
}

void RenderCommand::Clear() const
{
}

void RenderCommand::Swap() const
{
    m_dxo.SwapChain->Present(0, 0);
}
  