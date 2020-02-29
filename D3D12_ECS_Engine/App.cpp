#include "App.h"
#include "Vertex.h"

App* App::instance = nullptr; 

LRESULT CALLBACK WndProMessage(HWND hWnd, UINT msg, WPARAM wParam , LPARAM lParam)
{
	return App::instance->WinCallBack(hWnd, msg, wParam ,lParam);
}

void App::InitD3D12()
{	
	/*用于创建之后的交换链接口以及枚举适配器*/
	IDXGIFactory4* dxgiFactory = nullptr; 
	DX::ThrowIfFailed(CALL_INFO,
		CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory))
	);

	/*枚举适配器*/
	IDXGIAdapter1* adapter = nullptr; 
	//bool aFound = false;
	for (int index = 0 ; dxgiFactory->EnumAdapters1(index, &adapter) != DXGI_ERROR_NOT_FOUND ; )
	{
		DXGI_ADAPTER_DESC1 desc; 
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			index++; 
			continue; 
		}
		DX::ThrowIfFailed(CALL_INFO, 
			D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device) , nullptr)
		);
		index++; 
	}

	/*创建device*/
	DX::ThrowIfFailed(CALL_INFO , 
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0 , IID_PPV_ARGS(&m_device))
	);

	/*创建GPUCommandQueue*/
	D3D12_COMMAND_QUEUE_DESC cqDesc = {}; 
	DX::ThrowIfFailed(CALL_INFO ,
		m_device->CreateCommandQueue(&cqDesc , IID_PPV_ARGS(&m_CommandQueue))
	);

	/*创建Swapchain*/
	DXGI_MODE_DESC backBufferDesc = {}; 
	backBufferDesc.Width = m_Width;  
	backBufferDesc.Height = m_Height; 
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	DXGI_SAMPLE_DESC sampleDesc = {}; 
	sampleDesc.Count = 1; 

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {}; 
	swapChainDesc.BufferCount = FrameBufferCount; //添加三重缓冲
	swapChainDesc.BufferDesc = backBufferDesc; 
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //指定流水线渲染buffer 
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; 
	swapChainDesc.OutputWindow = hWnd; 
	swapChainDesc.SampleDesc = sampleDesc; 
	swapChainDesc.Windowed = !is_FullScreen; 

	IDXGISwapChain* tmpSwapChain; 

	DX::ThrowIfFailed(CALL_INFO, 
		dxgiFactory->CreateSwapChain(m_CommandQueue, 
									&swapChainDesc ,
									&tmpSwapChain)
	);
	m_SwapChain = static_cast<IDXGISwapChain3*>(tmpSwapChain);


	/*描述符堆*/
	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
		//RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {}; 
	rtvHeapDesc.NumDescriptors = FrameBufferCount; 
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; 
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; 
	DX::ThrowIfFailed(CALL_INFO, 
		m_device->CreateDescriptorHeap(&rtvHeapDesc , IID_PPV_ARGS(&m_rtvDescriptorHeap))
	);
	m_RTVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());;

	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,
			m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]))
		);
		m_device->CreateRenderTargetView(m_RenderTarget[i] , nullptr, rtvHandle);

		rtvHandle.Offset(1, m_RTVDescriptorSize);
	}

	/*CommandAllocator*/
	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,
			m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator[i]))
		);
	}

	/*CPUCommandlist*/
	DX::ThrowIfFailed(CALL_INFO,  
		m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT , m_CommandAllocator[0] , NULL, IID_PPV_ARGS(&m_CommandList))
	);
	//m_CommandList->Close();

	/*CPUFence*/
	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,  
			m_device->CreateFence(0 , D3D12_FENCE_FLAG_NONE , IID_PPV_ARGS(&m_Fence[i]))
		);
		m_FenceValue[i] = 0; 
	}
	m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_FenceEvent == nullptr)
	{
		return; 
	} 
}


void App::Update()

{
}

void App::Render()
{
}

void App::CleanUp()
{
}

void App::WaitForPreviousFrame()
{
}

void App::InitializeWind(HINSTANCE hInstance, int ShowWnd, int width, int height, bool isfullScreen)
{
}

App::App(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullScreen)
{
}

void App::Run(std::function<void(App*)> callBack)
{
}

LRESULT App::WinCallBack(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
}
