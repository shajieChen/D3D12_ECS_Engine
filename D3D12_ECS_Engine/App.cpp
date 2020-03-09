#include "App.h"  
#include "GeoFactory.h"
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
	this->InitMainWindow(hInstance, ShowWnd, width, height, fullScreen); 
	this->InitD3D12();  
	this->InitWindow(hInstance); 
	this->OnResize(); 
	m_ct.rcommand = std::make_unique<RenderCommand>(m_dxo); 
	 
	m_RootSignature = m_ct.rcommand->CreateRootSignature();
	this->testGrawTriangle();
	//retExample = std::make_unique<example::GeoRetangle>(m_ct);
}

App::~App()
{
	CleanUp(); 
}

void App::InitMainWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool isfullScreen)
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
	if (!RegisterClassEx(&wndClass))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return;
	}

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
	if (!hWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return;
	}
	if (isfullScreen)
	{
		SetWindowLong(hWnd, GWL_STYLE, 0);
	}
	ShowWindow(hWnd, ShowWnd);
	UpdateWindow(hWnd);
}
void App::InitD3D12()
{	
#if defined(NDEBUG) || defined(_NDEBUG) 
	// ����D3D12��Debug��
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		DX::ThrowIfFailed(CALL_INFO,  
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))
		); 
		debugController->EnableDebugLayer();
	}
#endif 

	/*���ڴ���֮��Ľ������ӿ��Լ�ö��������*/
	
	DX::ThrowIfFailed(CALL_INFO, 
		CreateDXGIFactory1(IID_PPV_ARGS(&m_DXGIFactory))
	);

	/*ö��������*/
	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false; 
	while (m_DXGIFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software Device
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

	/*����Device*/
	DX::ThrowIfFailed(CALL_INFO , 
		D3D12CreateDevice(adapter,D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&m_dxo.Device))
	);

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();  

	/*CPUFence*/
	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,  
			m_dxo.Device->CreateFence(0 , D3D12_FENCE_FLAG_NONE , IID_PPV_ARGS(&m_dxo.m_Fence[i]))
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
	//���m_ViewPort
	m_ViewPort.TopLeftX = 0;
	m_ViewPort.TopLeftY = 0;
	m_ViewPort.Width = (float)m_Width;
	m_ViewPort.Height = (float)m_Height;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;

	//���scisoorRect 
	m_ScissorRect.left = 0;
	m_ScissorRect.top = 0;
	m_ScissorRect.right = m_Width;
	m_ScissorRect.bottom = m_Height;
} 

void App::testGrawTriangle()
{
	example::GeoFactory geoFactory(m_ct); 

	//vs  
	Graphic::Shader vertexShader = m_ct.rcommand->CreateShader(L"DefaultVS.cso");
	D3D12_SHADER_BYTECODE vertexShaderBytecode = { vertexShader.byteCode->GetBufferPointer(),
												   vertexShader.byteCode->GetBufferSize() } ;

	//ps 
	Graphic::Shader pixelShader = m_ct.rcommand->CreateShader(L"DefaultPS.cso");
	D3D12_SHADER_BYTECODE pixelShaderBytecode = { pixelShader.byteCode->GetBufferPointer(), 
												  pixelShader.byteCode->GetBufferSize() }; 
	//ied 
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	}; 
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {}; 
	inputLayoutDesc.NumElements = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	inputLayoutDesc.pInputElementDescs = inputLayout; 

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; 
	psoDesc.InputLayout = { inputLayout, sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC) };
	psoDesc.pRootSignature = m_RootSignature.Get(); 
	psoDesc.VS = vertexShaderBytecode; 
	psoDesc.PS = pixelShaderBytecode; 
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; 
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; 
	psoDesc.SampleDesc = { 1 };
	psoDesc.SampleMask = 0xffffffff; 
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); 
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1; 	
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); 
	DX::ThrowIfFailed(CALL_INFO,  
		m_dxo.Device->CreateGraphicsPipelineState(&psoDesc,
												  IID_PPV_ARGS(&m_PipelineStateObject))
	);  

	Graphic::Mesh meshViews = geoFactory.CreateRetangle();  
	VBviews.VBViews.insert(VBviews.VBViews.end() , meshViews.VB.VBViews.begin() , meshViews.VB.VBViews.end());
	IBViews.IBViews.insert(IBViews.IBViews.end() , meshViews.IB.IBViews.begin() , meshViews.IB.IBViews.end());

	// Ϊÿһ֡������һ������������������
	for (int i = 0; i < m_dxo.frameBufferCount; ++i)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; 
		m_dxo.Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap[i]));
	}

	// �����ϴ�������(��������) --- ���� resources heap/Descriptor Heap/ָ��ConstantBufferView��ָ��
	for (int i = 0; i < m_dxo.frameBufferCount; ++i)
	{
		m_dxo.Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //ָ��Ϊ�ϴ������
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), //��Դ�ѵĴ�С����Ϊ64KB�Ĵ�С�ı���
			D3D12_RESOURCE_STATE_GENERIC_READ, 
			nullptr, 
			IID_PPV_ARGS(&constantBufferUploadHeap[i]));
		constantBufferUploadHeap[i]->SetName(L"�������������ϴ������");

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = constantBufferUploadHeap[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;    //����256�ֽ��Ų�
		m_dxo.Device->CreateConstantBufferView(&cbvDesc, mainDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());

		ZeroMemory(&cbColorMultiplierData, sizeof(cbColorMultiplierData));

		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
		constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbColorMultiplierGPUAddress[i]));
		memcpy(cbColorMultiplierGPUAddress[i], &cbColorMultiplierData, sizeof(cbColorMultiplierData));
	} 


	m_dxo.CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_dxo.CommandList.Get() };
	m_dxo.m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//���m_Fence ��֤�ٻ�֮ǰ
	m_FenceValue[m_dxo.m_FrameIndex]++;
	DX::ThrowIfFailed(CALL_INFO, 
		m_dxo.m_CommandQueue->Signal(m_dxo.m_Fence[m_dxo.m_FrameIndex].Get(), m_FenceValue[m_dxo.m_FrameIndex])
	);

}

void App::UpdateTriangle()
{
	WaitForPreviousFrame();

	DX::ThrowIfFailed(CALL_INFO, 
		m_CommandAllocator[m_dxo.m_FrameIndex]->Reset()
	);
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.CommandList->Reset(m_CommandAllocator[m_dxo.m_FrameIndex], m_PipelineStateObject.Get())
	);

	m_dxo.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_dxo.RenderTarget[m_dxo.m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_dxo.m_FrameIndex, m_dxo.m_RTVDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	m_dxo.CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	
	const float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_dxo.CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_dxo.CommandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	/*��ʼ����*/ 	
	m_dxo.CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeap[m_dxo.m_FrameIndex] };
	m_dxo.CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	 
	m_dxo.CommandList->SetGraphicsRootDescriptorTable(0, mainDescriptorHeap[m_dxo.m_FrameIndex]->GetGPUDescriptorHandleForHeapStart());

	m_dxo.CommandList->RSSetViewports(1, &m_ViewPort);
	m_dxo.CommandList->RSSetScissorRects(1, &m_ScissorRect);
	m_dxo.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_dxo.CommandList->IASetVertexBuffers(0, 1, VBviews.VBViews.data());
	m_dxo.CommandList->IASetIndexBuffer(IBViews.IBViews.data());
	m_dxo.CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);


	m_dxo.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_dxo.RenderTarget[m_dxo.m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.CommandList->Close()
	);

}

void App::Update() 
{
	static float rIncrement = 0.00002f;
	static float gIncrement = 0.00006f;
	static float bIncrement = 0.00009f;
	
	cbColorMultiplierData.colorMultiplier.x += rIncrement;
	cbColorMultiplierData.colorMultiplier.y += gIncrement;
	cbColorMultiplierData.colorMultiplier.z += bIncrement;

	if (cbColorMultiplierData.colorMultiplier.x >= 1.0 || cbColorMultiplierData.colorMultiplier.x <= 0.0)
	{
		cbColorMultiplierData.colorMultiplier.x = cbColorMultiplierData.colorMultiplier.x >= 1.0 ? 1.0 : 0.0;
		rIncrement = -rIncrement;
	}
	if (cbColorMultiplierData.colorMultiplier.y >= 1.0 || cbColorMultiplierData.colorMultiplier.y <= 0.0)
	{
		cbColorMultiplierData.colorMultiplier.y = cbColorMultiplierData.colorMultiplier.y >= 1.0 ? 1.0 : 0.0;
		gIncrement = -gIncrement;
	}
	if (cbColorMultiplierData.colorMultiplier.z >= 1.0 || cbColorMultiplierData.colorMultiplier.z <= 0.0)
	{
		cbColorMultiplierData.colorMultiplier.z = cbColorMultiplierData.colorMultiplier.z >= 1.0 ? 1.0 : 0.0;
		bIncrement = -bIncrement;
	}

	//map/UnMap ��̬������
	memcpy(cbColorMultiplierGPUAddress[m_dxo.m_FrameIndex], &cbColorMultiplierData, sizeof(cbColorMultiplierData));
}

void App::Render()
{
	UpdateTriangle(); //test
	ID3D12CommandList* ppCommandLists[] = { m_dxo.CommandList.Get() };
	 //CPU ����CommandList ��CommandQueue 
	m_dxo.m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	/*GPU ����Χ��ֵ*/
	DX::ThrowIfFailed(CALL_INFO,  
		m_dxo.m_CommandQueue->Signal(m_dxo.m_Fence[m_dxo.m_FrameIndex].Get(), m_FenceValue[m_dxo.m_FrameIndex])
	);

	DX::ThrowIfFailed(CALL_INFO, 
		m_dxo.SwapChain->Present(0, 0)
	);


}

void App::CleanUp()
{ 
	for (int i = 0; i < m_dxo.frameBufferCount; ++i)
	{
		m_dxo.m_FrameIndex = i;
		WaitForPreviousFrame();
	}

	BOOL fs = false;
	if (m_dxo.SwapChain->GetFullscreenState(&fs, NULL))
		m_dxo.SwapChain->SetFullscreenState(false, NULL);

	if (m_dxo.Device)m_dxo.Device.Reset();
	m_dxo.SwapChain.Reset();
	m_dxo.m_CommandQueue.Reset();
	m_rtvDescriptorHeap.Reset();
	m_dxo.CommandList.Reset(); 

	for (int i = 0; i < m_dxo.frameBufferCount; ++i)
	{
		m_dxo.RenderTarget[i].Reset();
		m_CommandAllocator[i]->Release();
		m_dxo.m_Fence[i].Reset();
		mainDescriptorHeap[i]->Release();
		constantBufferUploadHeap[i]->Release();

	};
	m_PipelineStateObject.Reset();
	m_RootSignature.Reset(); 

	depthStencilBuffer.Reset();
	dsDescriptorHeap.Reset();
}

void App::OnResize()
{
	assert(m_dxo.Device);
	assert(m_dxo.SwapChain);
	assert(m_CommandAllocator);

	WaitForPreviousFrame(); 
}

void App::WaitForPreviousFrame()
{
	m_dxo.m_FrameIndex = m_dxo.SwapChain->GetCurrentBackBufferIndex();

	if (m_dxo.m_Fence[m_dxo.m_FrameIndex]->GetCompletedValue() < m_FenceValue[m_dxo.m_FrameIndex])
	{
		DX::ThrowIfFailed(CALL_INFO , 
			m_dxo.m_Fence[m_dxo.m_FrameIndex]->SetEventOnCompletion(m_FenceValue[m_dxo.m_FrameIndex], m_FenceEvent)
		);
		
		WaitForSingleObject(m_FenceEvent, INFINITE);
	} 
	m_FenceValue[m_dxo.m_FrameIndex]++;
}

void App::CreateRtvAndDsvDescriptorHeaps()
{ 
	/*RTV��������*/
	m_dxo.m_FrameIndex = m_dxo.SwapChain->GetCurrentBackBufferIndex();
	//RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap))
	);
	m_dxo.m_RTVDescriptorSize = m_dxo.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());;
	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,
			m_dxo.SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_dxo.RenderTarget[i]))
		);
		m_dxo.Device->CreateRenderTargetView(m_dxo.RenderTarget[i].Get(), nullptr, rtvHandle);

		rtvHandle.Offset(1, m_dxo.m_RTVDescriptorSize);
	}   
	/*����Stencil DepthBuffer View*/
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	m_dxo.Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_Width, m_Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	);
	m_dxo.Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));

	dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	m_dxo.Device->CreateDepthStencilView(depthStencilBuffer.Get(),
		&depthStencilDesc,
		dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void App::CreateSwapChain()
{ 
	/*����Swapchain*/
	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = m_Width;
	backBufferDesc.Height = m_Height;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = FrameBufferCount; //������ػ���
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //ָ����ˮ����Ⱦbuffer 
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc = sampleDesc;
	swapChainDesc.Windowed = !is_FullScreen;

	IDXGISwapChain* tmpSwapChain;

	DX::ThrowIfFailed(CALL_INFO,
		m_DXGIFactory->CreateSwapChain(m_dxo.m_CommandQueue.Get(),
			&swapChainDesc,
			&tmpSwapChain)
	);
	m_dxo.SwapChain = static_cast<IDXGISwapChain3*>(tmpSwapChain);
}

void App::CreateCommandObjects()
{ 
	/*����GPUCommandQueue*/
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.Device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_dxo.m_CommandQueue))
	); 

	/*CommandAllocator*/
	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,
			m_dxo.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator[i]))
		);
	} 

	/*CPUCommandlist*/
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator[0], NULL, IID_PPV_ARGS(&m_dxo.CommandList))
	); 
}
 
void App::Run(std::function<void(App*)> app)
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
			if (app)
			{
				app(this);
			} 
			Update(); 
			//����
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
