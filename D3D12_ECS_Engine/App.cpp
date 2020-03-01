#include "App.h"
#include "Vertex.h"

App* App::instance = nullptr; 

LRESULT CALLBACK WndProMessage(HWND hWnd, UINT msg, WPARAM wParam , LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			DestroyWindow(hWnd);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}  
App::App(HINSTANCE hInstance, int ShowWnd, int width, int height, bool fullScreen)
{
	instance = this;
	this->InitializeWind(hInstance, ShowWnd, width, height, fullScreen); 
	this->InitD3D12(); 
	this->InitWindow(hInstance);
	this->testGrawTriangle();
}

App::~App()
{
	CleanUp(); 
}

void App::InitializeWind(HINSTANCE hInstance, int ShowWnd, int width, int height, bool isfullScreen)
{
	m_Width = width;
	is_FullScreen = isfullScreen;
	m_Height = height;
	if (isfullScreen)
	{
		HMONITOR hmon = MonitorFromWindow(hWnd,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);
		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}

	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProMessage;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpszClassName = WinsName;
	RegisterClassEx(&wndClass);
	RECT clientRect;
	SetRect(&clientRect, 0, 0, width, height);
	AdjustWindowRect(
		&clientRect,
		WS_OVERLAPPEDWINDOW,
		false);

	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
	int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);
	hWnd = CreateWindow(
		wndClass.lpszClassName,
		WinsTitle,
		WS_OVERLAPPEDWINDOW,
		centeredX,
		centeredY,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top,
		0,
		0,
		hInstance,
		0);

	if (isfullScreen)
	{
		SetWindowLong(hWnd, GWL_STYLE, 0);
	}
	ShowWindow(hWnd, ShowWnd);
	UpdateWindow(hWnd);
}
void App::InitD3D12()
{	
	/*用于创建之后的交换链接口以及枚举适配器*/
	IDXGIFactory4* dxgiFactory;
	DX::ThrowIfFailed(CALL_INFO, 
		CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))
	);

	/*枚举适配器*/
	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false; 
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			adapterIndex++;
			continue;
		}
		 
		HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}
		adapterIndex++;
	}
	if (!adapterFound)
	{
		return;
	}


	/*创建device*/
	DX::ThrowIfFailed(CALL_INFO , 
		D3D12CreateDevice(adapter,D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&m_device))
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
	/*Sampler State*/

}

void App::InitWindow(HINSTANCE& hInstance)
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;  
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature; 
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

	//填充m_ViewPort
	m_ViewPort.TopLeftX = 0;
	m_ViewPort.TopLeftY = 0;
	m_ViewPort.Width = (float)m_Width;
	m_ViewPort.Height = (float)m_Height;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;

	//填充scisoorRect 
	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = m_Width;
	m_ScissorRect.bottom = m_Height;
}


void App::testGrawTriangle()
{
	//vs 
	ID3DBlob* vertexShader;
	D3DReadFileToBlob(L"DefaultVS.cso", &vertexShader);
	D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
	vertexShaderBytecode.BytecodeLength = vertexShader->GetBufferSize();
	vertexShaderBytecode.pShaderBytecode = vertexShader->GetBufferPointer();

	//ps
	ID3DBlob* pixelShader;
	D3DReadFileToBlob(L"DefaultPS.cso", &pixelShader);
	D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
	pixelShaderBytecode.BytecodeLength = pixelShader->GetBufferSize();
	pixelShaderBytecode.pShaderBytecode = pixelShader->GetBufferPointer();

	//ied 
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//inputlayout 
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout;

	/*Samplr*/
	DXGI_SAMPLE_DESC  sampleDesc = {};
	sampleDesc.Count = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; 
	psoDesc.InputLayout = inputLayoutDesc; 
	psoDesc.pRootSignature = m_RootSignature; 
	psoDesc.VS = vertexShaderBytecode; 
	psoDesc.PS = pixelShaderBytecode; 
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; 
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; 
	psoDesc.SampleDesc = sampleDesc; 
	psoDesc.SampleMask = 0xffffffff; 
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); 
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1; 

	m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineStateObject));

	//vb 
	Vertex vList[] = {
		{ 0.0f, 0.5f, 0.5f, 0.2f, 0.2f, 0.2f, 1.0f },
		{ 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f },
		{ -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f },
	};
	int vBufferSize = sizeof(vList);

	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_VB));
		m_VB->SetName(L"Vertex Buffer Resource Heap");

	/*创建上传缓冲区*/
	ID3D12Resource* vBufferUploadHeap;
	m_device->CreateCommittedResource
	(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
		vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

		//传输到上传缓冲区 
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList); 
	vertexData.RowPitch = vBufferSize; 
	vertexData.SlicePitch = vBufferSize;  
	UpdateSubresources(m_CommandList, m_VB, vBufferUploadHeap, 0, 0, 1, &vertexData);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_VB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	//创建VBView 
	m_VBView.BufferLocation = m_VB->GetGPUVirtualAddress();
	m_VBView.StrideInBytes = sizeof(Vertex);
	m_VBView.SizeInBytes = vBufferSize;

	DWORD iList[] = {
		0, 1, 2,
		0, 3, 1
	};

	int iBufferSize = sizeof(iList);

	/*IndexBuffer 创建*/
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
		D3D12_HEAP_FLAG_NONE, 
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST, 
		nullptr, 
		IID_PPV_ARGS(&m_IB));

	m_VB->SetName(L"Index Buffer Resource Heap");

	ID3D12Resource* iBufferUploadHeap;
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), 
		D3D12_HEAP_FLAG_NONE, 
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(iList);
	indexData.RowPitch = iBufferSize; 
	indexData.SlicePitch = iBufferSize; 

	UpdateSubresources(m_CommandList, m_IB, iBufferUploadHeap, 0, 0, 1, &indexData);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IB, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_IBView.BufferLocation = m_IB->GetGPUVirtualAddress();
	m_IBView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	m_IBView.SizeInBytes = iBufferSize;



	m_CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_CommandList };
	m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	//添加m_Fence 保证再会之前
	m_FenceValue[m_FrameIndex]++;
	DX::ThrowIfFailed(CALL_INFO, 
		m_CommandQueue->Signal(m_Fence[m_FrameIndex], m_FenceValue[m_FrameIndex])
	);

}

void App::UpdateTriangle()
{
	WaitForPreviousFrame();

	DX::ThrowIfFailed(CALL_INFO, 
		m_CommandAllocator[m_FrameIndex]->Reset()
	);
	DX::ThrowIfFailed(CALL_INFO,
		m_CommandList->Reset(m_CommandAllocator[m_FrameIndex], m_PipelineStateObject)
	);

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTarget[m_FrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RTVDescriptorSize);
	m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	const float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	/*绘制三角形*/
	m_CommandList->SetGraphicsRootSignature(m_RootSignature);
	m_CommandList->RSSetViewports(1, &m_ViewPort);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CommandList->IASetVertexBuffers(0, 1, &m_VBView);
	m_CommandList->IASetIndexBuffer(&m_IBView);
	m_CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTarget[m_FrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	DX::ThrowIfFailed(CALL_INFO,
		m_CommandList->Close()
	);

}

void App::Update() 
{
}

void App::Render()
{
	UpdateTriangle(); //test

	ID3D12CommandList* ppCommandLists[] = { m_CommandList };

	m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	DX::ThrowIfFailed(CALL_INFO,  
		m_CommandQueue->Signal(m_Fence[m_FrameIndex], m_FenceValue[m_FrameIndex])
	);

	DX::ThrowIfFailed(CALL_INFO, 
		m_SwapChain->Present(0, 0)
	);


}

void App::CleanUp()
{ 
	for (int i = 0; i < frameBufferCount; ++i)
	{
		m_FrameIndex = i;
		WaitForPreviousFrame();
	}

	BOOL fs = false;
	if (m_SwapChain->GetFullscreenState(&fs, NULL))
		m_SwapChain->SetFullscreenState(false, NULL);

	if (m_device)m_device->Release();
	m_SwapChain->Release();
	m_CommandQueue->Release();
	m_rtvDescriptorHeap->Release();
	m_CommandList->Release();
	m_IB->Release();

	for (int i = 0; i < frameBufferCount; ++i)
	{
		m_RenderTarget[i]->Release();
		m_CommandAllocator[i]->Release();
		m_Fence[i]->Release();
	};
	m_PipelineStateObject->Release();
	m_RootSignature->Release();
	m_VB->Release();
}

void App::WaitForPreviousFrame()
{
	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

	if (m_Fence[m_FrameIndex]->GetCompletedValue() < m_FenceValue[m_FrameIndex])
	{
		DX::ThrowIfFailed(CALL_INFO , 
			m_Fence[m_FrameIndex]->SetEventOnCompletion(m_FenceValue[m_FrameIndex], m_FenceEvent)
		);
		
		WaitForSingleObject(m_FenceEvent, INFINITE);
	} 
	m_FenceValue[m_FrameIndex]++;
}

void App::Run(std::function<void(App*)> callBack)
{
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (callBack)
			{
				callBack(this);
			}
			Render();
		}
	}
	WaitForPreviousFrame();
	CloseHandle(m_FenceEvent);
}

LRESULT App::WinCallBack(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) { 
				DestroyWindow(hWnd);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
