#include "Window.h"

bool DXWindow::Init()
{
	// Register the Window class
	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = &DXWindow::OnWindowMessage;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandleW(nullptr);
	wcex.hIcon = LoadIconW(nullptr,IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"D3D12ExWndCls";
	wcex.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);

	m_wndClass = RegisterClassExW(&wcex);
	if (m_wndClass == 0)
		return false;

	// Place on current screen
	POINT pos{ 0,0 };
	GetCursorPos(&pos);
	HMONITOR monitor = MonitorFromPoint(pos, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitorInfo{};
	monitorInfo.cbSize = sizeof(monitorInfo);
	GetMonitorInfoW(monitor, &monitorInfo);

	// Create a Window
	m_window = CreateWindowExW(
		WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, 
		(LPCWSTR)m_wndClass, 
		L"Dx12Renderer", 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
		monitorInfo.rcWork.left + 10,
		monitorInfo.rcWork.top	+10, 
		m_width,
		m_height,
		nullptr, nullptr, wcex.hInstance, nullptr
	);

	if (m_window == nullptr)
		return false;

	// Swap Chain description
	DXGI_SWAP_CHAIN_DESC1 swd{};
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC sfd{};

	swd.Width = m_width;
	swd.Height = m_height;
	swd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swd.Stereo = false;
	swd.SampleDesc.Count = 1;
	swd.SampleDesc.Quality = 0;
	swd.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swd.BufferCount = GetFrameCount();
	swd.Scaling = DXGI_SCALING_STRETCH;
	swd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	sfd.Windowed = true;

	// Create swap chain
	auto& factory = DXContext::Get().GetFactory();
	ComPointer<IDXGISwapChain1> sc1;
	factory->CreateSwapChainForHwnd(DXContext::Get().GetCommandQueue(), m_window, &swd, &sfd, nullptr, &sc1);
	if (!sc1.QueryInterface(m_swapChain))
		return false;

	//Create RTV Heap
	D3D12_DESCRIPTOR_HEAP_DESC dHeapDesc{};
	dHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	dHeapDesc.NumDescriptors = FrameCount;
	dHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dHeapDesc.NodeMask = 0;

	if (FAILED(DXContext::Get().GetDevice()->CreateDescriptorHeap(&dHeapDesc, IID_PPV_ARGS(&m_rtvDescHeap))))
		return false;

	// Create handles for each heap created (NumDescriptors), here - 2
	auto baseHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto handleIncrement = DXContext::Get().GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (size_t i = 0; i < dHeapDesc.NumDescriptors; ++i)
	{
		m_rtvHandles[i].ptr = (baseHandle.ptr + (handleIncrement * i));
	}

	// Get Buffers
	if (!GetBuffers())
		return false;
	
	return true;
}

void DXWindow::Update()
{
	MSG msg;
	while (PeekMessageW(&msg, m_window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

void DXWindow::Present()
{
	m_swapChain->Present(1, 0);
}

void DXWindow::Shutdown()
{
	ReleaseBuffers();

	m_rtvDescHeap.Release();
	
	m_swapChain.Release();

	if (m_window)
		DestroyWindow(m_window);

	if (m_wndClass)
		UnregisterClassW((LPCWSTR)m_wndClass, GetModuleHandleW(nullptr));
}

void DXWindow::Resize()
{
	ReleaseBuffers();

	RECT cr;
	if (GetClientRect(m_window, &cr))
	{
		m_width = cr.right - cr.left;
		m_height = cr.bottom - cr.top;

		m_swapChain->ResizeBuffers(
			GetFrameCount(),
			m_width, m_height,
			DXGI_FORMAT_UNKNOWN,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
		);
		m_shouldResize = false;
	}

	// can throw if failed getting buffers
	GetBuffers();
}

void DXWindow::SetFullscreen(bool enabled)
{
	// Update window styling
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	DWORD exStyle = WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW;
	if (enabled)
	{
		style = WS_POPUP | WS_VISIBLE;
		exStyle = WS_EX_APPWINDOW;
	}

	SetWindowLongW(m_window, GWL_STYLE, style);
	SetWindowLongW(m_window, GWL_EXSTYLE, exStyle);

	// Adjust window size
	if (enabled)
	{
		HMONITOR monitor = MonitorFromWindow(m_window, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO monitorInfo{};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (GetMonitorInfoW(monitor, &monitorInfo))
		{
			SetWindowPos(
				m_window, nullptr,
				monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.top,
				monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
				monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
				SWP_NOZORDER
			);
		}
	}

	m_isFullscreen = enabled;
}

void DXWindow::BeginFrame(ID3D12GraphicsCommandList6* cmdList)
{
	m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_buffers[m_currentBufferIndex];
	barrier.Transition.Subresource = 0;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	cmdList->ResourceBarrier(1, &barrier);

	float clearColor[] = { 0.392f, 0.584f, 0.929f, 1.f }; // CORNFLOWER BLUE!!!
	cmdList->ClearRenderTargetView(m_rtvHandles[m_currentBufferIndex], clearColor, 0, nullptr);

	cmdList->OMSetRenderTargets(1, &m_rtvHandles[m_currentBufferIndex], false, nullptr);
}

void DXWindow::EndFrame(ID3D12GraphicsCommandList6* cmdList)
{
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_buffers[m_currentBufferIndex];
	barrier.Transition.Subresource = 0;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	cmdList->ResourceBarrier(1, &barrier);
}

bool DXWindow::GetBuffers()
{
	for (size_t i = 0; i < FrameCount; ++i)
	{
		if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]))))
			return false;

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		DXContext::Get().GetDevice()->CreateRenderTargetView(
			m_buffers[i],
			&rtvDesc,
			m_rtvHandles[i]
		);
	}
	return true;
}

void DXWindow::ReleaseBuffers()
{
	for (size_t i = 0; i < FrameCount; ++i)
	{
		m_buffers[i].Release();
	}
}

LRESULT CALLBACK DXWindow::OnWindowMessage(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_F11)
			Get().SetFullscreen(!Get().IsFullscreen());
		break;

	case WM_CLOSE:
		Get().m_shouldClose = true;
		return 0;

	case WM_SIZE:
		if (lParam && (HIWORD(lParam) != Get().m_height || LOWORD(lParam) != Get().m_width))
			Get().m_shouldResize = true;
		break;

	default:
		break;
	}
	return DefWindowProcW(wnd, msg, wParam, lParam);
}
