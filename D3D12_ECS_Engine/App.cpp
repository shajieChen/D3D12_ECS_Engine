#include "App.h"
#include "GeoFactory.h"
#include "ImageLoader.h"
App* App::instance = nullptr;
int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBuffer) + 255) & ~255;
LRESULT CALLBACK WndProMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
	// 开启D3D12的Debug层
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		DX::ThrowIfFailed(CALL_INFO,
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))
		);
		debugController->EnableDebugLayer();
	}
#endif

	/*用于创建之后的交换链接口以及枚举适配器*/

	DX::ThrowIfFailed(CALL_INFO,
		CreateDXGIFactory1(IID_PPV_ARGS(&m_DXGIFactory))
	);

	/*枚举适配器*/
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

	/*创建Device*/
	DX::ThrowIfFailed(CALL_INFO,
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_dxo.Device))
	);

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	/*CPUFence*/
	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,
			m_dxo.Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_dxo.m_Fence[i]))
		);
		m_dxo.m_FenceValue[i] = 0;
	}
	m_dxo.m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_dxo.m_FenceEvent == nullptr)
	{
		return;
	}
	/*Sampler State*/
}

void App::InitWindow(HINSTANCE& hInstance)
{
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
	example::GeoFactory geoFactory(m_ct);

	//vs
	Graphic::Shader vertexShader = m_ct.rcommand->CreateShader(L"DefaultVS.cso");
	D3D12_SHADER_BYTECODE vertexShaderBytecode = { vertexShader.byteCode->GetBufferPointer(),
												   vertexShader.byteCode->GetBufferSize() };

	//ps
	Graphic::Shader pixelShader = m_ct.rcommand->CreateShader(L"DefaultPS.cso");
	D3D12_SHADER_BYTECODE pixelShaderBytecode = { pixelShader.byteCode->GetBufferPointer(),
												  pixelShader.byteCode->GetBufferSize() };
	//ied
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

	Graphic::Mesh meshViews = geoFactory.CreateBox();
	VBviews.VBViews.insert(VBviews.VBViews.end(), meshViews.VB.VBViews.begin(), meshViews.VB.VBViews.end());
	IBViews.IBViews.insert(IBViews.IBViews.end(), meshViews.IB.IBViews.begin(), meshViews.IB.IBViews.end());

	//// 为每一帧都创建一个常量缓冲区描述符
	//for (int i = 0; i < m_dxo.frameBufferCount; ++i)
	//{
	//	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//	heapDesc.NumDescriptors = 1;
	//	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//	m_dxo.Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap[i]));
	//}

	// 创建上传缓冲区(共享缓冲区) --- 包括 resources heap/Descriptor Heap/指向ConstantBufferView的指针
	for (int i = 0; i < m_dxo.frameBufferCount; ++i)
	{
		m_dxo.Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), //指明为上传缓冲堆
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), //资源堆的大小必须为64KB的大小的倍数
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&constantBufferUploadHeap[i]));
		constantBufferUploadHeap[i]->SetName(L"常量缓冲区的上传缓冲堆"); 

		ZeroMemory(&cbPerObject, sizeof(cbPerObject));

		CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (End is less than or equal to begin)
		constantBufferUploadHeap[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbGPUAddress[i]));
		memcpy(cbGPUAddress[i], &cbPerObject, sizeof(cbPerObject));
		memcpy(cbGPUAddress[i] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	m_dxo.Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap));

	BYTE* imageData;
	// 加载图像
	D3D12_RESOURCE_DESC textureDesc;
	int imageBytesPerRow;
	int imageSize = LoadImageDataFromFile(&imageData, textureDesc, L"braynzar.jpg", imageBytesPerRow);

	// 保证图像大小不为0
	if (imageSize <= 0)
	{
		return; 
	}
	//创建默认缓冲堆，用于上传缓冲堆应用
	m_dxo.Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
		D3D12_HEAP_FLAG_NONE,
		&textureDesc, // 创建纹理资源
		D3D12_RESOURCE_STATE_COPY_DEST, //资源访问设置
		nullptr, 
		IID_PPV_ARGS(&textureBuffer));

	textureBuffer->SetName(L"Texture Buffer Resource Heap");

	UINT64 textureUploadBufferSize;

	// 获取纹理贴图上传缓冲区的大小
	m_dxo.Device->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

	// 创建上传缓冲区
	m_dxo.Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // 指定为上传缓冲区
		D3D12_HEAP_FLAG_NONE, 
		&CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize), 
		D3D12_RESOURCE_STATE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS(&textureBufferUploadHeap));

	textureBufferUploadHeap->SetName(L"Texture Buffer Upload Resource Heap");

	// 将纹理图片放入上传缓冲堆中
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &imageData[0]; 
	textureData.RowPitch = imageBytesPerRow; 
	textureData.SlicePitch = imageBytesPerRow * textureDesc.Height; 

	// 将上传缓冲堆赋值到默认堆中
	UpdateSubresources(m_dxo.CommandList.Get(), textureBuffer, textureBufferUploadHeap, 0, 0, 1, &textureData);

	// 将默认缓冲堆 放入到PixelShader中采样
	m_dxo.CommandList.Get()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// 创建ShaderResourceView
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	m_dxo.Device->CreateShaderResourceView(textureBuffer, &srvDesc, mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	m_dxo.CommandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_dxo.CommandList.Get() };
	m_dxo.m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists); 

	//添加m_Fence 保证再会之前
	m_dxo.m_FenceValue[m_dxo.FrameIndex]++;
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.m_CommandQueue->Signal(m_dxo.m_Fence[m_dxo.FrameIndex].Get(), m_dxo.m_FenceValue[m_dxo.FrameIndex])
	);

	delete imageData;

#pragma region MatrixSetUp 
	XMMATRIX tmpMat = XMMatrixPerspectiveFovLH(45.0f * (3.14f / 180.0f), (float)m_Width / (float)m_Height, 0.1f, 1000.0f);
	XMStoreFloat4x4(&cameraProjMat, tmpMat);

	//初始化相机位置
	cameraPosition = XMFLOAT4(0.0f, 2.0f, -4.0f, 0.0f);
	cameraTarget = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cameraUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	// 设置相加View矩阵
	XMVECTOR cPos = XMLoadFloat4(&cameraPosition);
	XMVECTOR cTarg = XMLoadFloat4(&cameraTarget);
	XMVECTOR cUp = XMLoadFloat4(&cameraUp);
	tmpMat = XMMatrixLookAtLH(cPos, cTarg, cUp);
	XMStoreFloat4x4(&cameraViewMat, tmpMat);

	// 设置位置
	// 第一个Cube
	cube1Position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f); // 第一个cubde 的位置
	XMVECTOR posVec = XMLoadFloat4(&cube1Position); 

	tmpMat = XMMatrixTranslationFromVector(posVec); // 设置当前的translation
	XMStoreFloat4x4(&cube1RotMat, XMMatrixIdentity()); // 设置当前的旋转矩阵为单位矩阵 
	XMStoreFloat4x4(&cube1WorldMat, tmpMat); // 存储正方体的世界位置坐标

	// 第二个 cube
	cube2PositionOffset = XMFLOAT4(1.5f, 0.0f, 0.0f, 0.0f);
	posVec = XMLoadFloat4(&cube2PositionOffset) + XMLoadFloat4(&cube1Position); // 创建第二个cube 的位置
	
	tmpMat = XMMatrixTranslationFromVector(posVec);
	XMStoreFloat4x4(&cube2RotMat, XMMatrixIdentity()); 
	XMStoreFloat4x4(&cube2WorldMat, tmpMat); 
#pragma endregion 
}

void App::UpdateTriangle()
{
	m_ct.rcommand->WaitForPreviousFrame();

	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.m_CommandAllocator[m_dxo.FrameIndex]->Reset()
	);
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.CommandList->Reset(m_dxo.m_CommandAllocator[m_dxo.FrameIndex].Get(), m_PipelineStateObject.Get())
	);

	m_dxo.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_dxo.RenderTarget[m_dxo.FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_dxo.FrameIndex, m_dxo.RTVDescSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	m_dxo.CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	const float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_dxo.CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_dxo.CommandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	/*开始绘制*/
	m_dxo.CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mainDescriptorHeap };
	m_dxo.CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
	m_dxo.CommandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	m_dxo.CommandList->RSSetViewports(1, &m_ViewPort);
	m_dxo.CommandList->RSSetScissorRects(1, &m_ScissorRect);
	m_dxo.CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_dxo.CommandList->IASetVertexBuffers(0, 1, VBviews.VBViews.data());
	m_dxo.CommandList->IASetIndexBuffer(IBViews.IBViews.data());
	m_dxo.CommandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeap[m_dxo.FrameIndex]->GetGPUVirtualAddress());

	// draw first cube
	m_dxo.CommandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

	// second cube
	// cube2's constant buffer data is stored after (256 bits from the start of the heap).
	m_dxo.CommandList->SetGraphicsRootConstantBufferView(0, constantBufferUploadHeap[m_dxo.FrameIndex]->GetGPUVirtualAddress() + ConstantBufferPerObjectAlignedSize);

	// draw second cube
	m_dxo.CommandList->DrawIndexedInstanced(numCubeIndices, 1, 0, 0, 0);

	m_dxo.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_dxo.RenderTarget[m_dxo.FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.CommandList->Close()
	);
}

void App::Update()
{
	XMMATRIX rotXMat = XMMatrixRotationX(0.0001f);
	XMMATRIX rotYMat = XMMatrixRotationY(0.0002f);
	XMMATRIX rotZMat = XMMatrixRotationZ(0.0003f);

		/*设置旋转矩阵 */
	XMMATRIX rotMat = XMLoadFloat4x4(&cube1RotMat) * rotXMat * rotYMat * rotZMat;
	XMStoreFloat4x4(&cube1RotMat, rotMat);

	XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube1Position));

	/*设置世界矩阵*/
	XMMATRIX worldMat = rotMat * translationMat;
	 
	XMStoreFloat4x4(&cube1WorldMat, worldMat);

	// 更新常量缓冲区	
	XMMATRIX viewMat = XMLoadFloat4x4(&cameraViewMat); // 加载camera 的view 矩阵
	XMMATRIX projMat = XMLoadFloat4x4(&cameraProjMat); // 加载projection矩阵
	XMMATRIX wvpMat = XMLoadFloat4x4(&cube1WorldMat) * viewMat * projMat; // 创建MVP 
	XMMATRIX transposed = XMMatrixTranspose(wvpMat); // 转至成GPU 可读的列矩阵
	XMStoreFloat4x4(&cbPerObject.WorldViewProjection, transposed); // 存储到常量缓冲区中 

													  // copy our ConstantBuffer instance to the mapped constant buffer resource
	memcpy(cbvGPUAddress[m_dxo.FrameIndex], &cbPerObject, sizeof(cbPerObject));

	// now do cube2's world matrix
	// create rotation matrices for cube2
	rotXMat = XMMatrixRotationX(0.0003f);
	rotYMat = XMMatrixRotationY(0.0002f);
	rotZMat = XMMatrixRotationZ(0.0001f);

	// add rotation to cube2's rotation matrix and store it
	rotMat = rotZMat * (XMLoadFloat4x4(&cube2RotMat) * (rotXMat * rotYMat));
	XMStoreFloat4x4(&cube2RotMat, rotMat);

	// create translation matrix for cube 2 to offset it from cube 1 (its position relative to cube1
	XMMATRIX translationOffsetMat = XMMatrixTranslationFromVector(XMLoadFloat4(&cube2PositionOffset));

	// we want cube 2 to be half the size of cube 1, so we scale it by .5 in all dimensions
	XMMATRIX scaleMat = XMMatrixScaling(0.5f, 0.5f, 0.5f);

	// reuse worldMat. 
	// first we scale cube2. scaling happens relative to point 0,0,0, so you will almost always want to scale first
	// then we translate it. 
	// then we rotate it. rotation always rotates around point 0,0,0
	// finally we move it to cube 1's position, which will cause it to rotate around cube 1
	worldMat = scaleMat * translationOffsetMat * rotMat * translationMat;

	wvpMat = XMLoadFloat4x4(&cube2WorldMat) * viewMat * projMat; // create wvp matrix
	transposed = XMMatrixTranspose(wvpMat); // must transpose wvp matrix for the gpu
	XMStoreFloat4x4(&cbPerObject.WorldViewProjection, transposed); // store transposed wvp matrix in constant buffer

													  // copy our ConstantBuffer instance to the mapped constant buffer resource
	memcpy(cbvGPUAddress[m_dxo.FrameIndex] + ConstantBufferPerObjectAlignedSize, &cbPerObject, sizeof(cbPerObject));

	// store cube2's world matrix
	XMStoreFloat4x4(&cube2WorldMat, worldMat);
}

void App::Render()
{
	UpdateTriangle(); //test
	ID3D12CommandList* ppCommandLists[] = { m_dxo.CommandList.Get() };
	//CPU 传递CommandList 到CommandQueue
	m_dxo.m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	/*GPU 设置围栏值*/
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.m_CommandQueue->Signal(m_dxo.m_Fence[m_dxo.FrameIndex].Get(), m_dxo.m_FenceValue[m_dxo.FrameIndex])
	);

	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.SwapChain->Present(0, 0)
	);
}

void App::CleanUp()
{
	for (int i = 0; i < m_dxo.frameBufferCount; ++i)
	{
		m_dxo.FrameIndex = i;
		m_ct.rcommand->WaitForPreviousFrame();
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
		m_dxo.m_CommandAllocator[i].Reset();
		m_dxo.m_Fence[i].Reset(); 
		constantBufferUploadHeap[i]->Release();
	}; 
	mainDescriptorHeap->Release();
	m_PipelineStateObject.Reset();
	m_RootSignature.Reset();

	depthStencilBuffer.Reset();
	dsDescriptorHeap.Reset();
}

void App::OnResize()
{
	assert(m_dxo.Device);
	assert(m_dxo.SwapChain);
	assert(m_dxo.m_CommandAllocator);
}

void App::CreateRtvAndDsvDescriptorHeaps()
{
	/*RTV描述符堆*/
	m_dxo.FrameIndex = m_dxo.SwapChain->GetCurrentBackBufferIndex();
	//RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap))
	);
	m_dxo.RTVDescSize = m_dxo.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());;
	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,
			m_dxo.SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_dxo.RenderTarget[i]))
		);
		m_dxo.Device->CreateRenderTargetView(m_dxo.RenderTarget[i].Get(), nullptr, rtvHandle);

		rtvHandle.Offset(1, m_dxo.RTVDescSize);
	}
	/*创建Stencil DepthBuffer View*/
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
	//m_dxo.Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap));

	dsDescriptorHeap->SetName(L"Depth/Stencil Resource Heap");

	m_dxo.Device->CreateDepthStencilView(depthStencilBuffer.Get(),
		&depthStencilDesc,
		dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void App::CreateSwapChain()
{
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
		m_DXGIFactory->CreateSwapChain(m_dxo.m_CommandQueue.Get(),
			&swapChainDesc,
			&tmpSwapChain)
	);
	m_dxo.SwapChain = static_cast<IDXGISwapChain3*>(tmpSwapChain);
}

void App::CreateCommandObjects()
{
	/*创建GPUCommandQueue*/
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.Device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_dxo.m_CommandQueue))
	);

	/*CommandAllocator*/
	for (int i = 0; i < FrameBufferCount; i++)
	{
		DX::ThrowIfFailed(CALL_INFO,
			m_dxo.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_dxo.m_CommandAllocator[i]))
		);
	}

	/*CPUCommandlist*/
	DX::ThrowIfFailed(CALL_INFO,
		m_dxo.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_dxo.m_CommandAllocator[0].Get(), NULL, IID_PPV_ARGS(&m_dxo.CommandList))
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
			//绘制
			Render();
		}
	}
	m_ct.rcommand->WaitForPreviousFrame();
	CloseHandle(m_dxo.m_FenceEvent);
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