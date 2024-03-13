#include <iostream>
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <DxDebug/DXDebugLayer.h>
#include <D3D/DXContext.h>
#include <Support/Window.h>

int main()
{
    DXDebugLayer::Get().Init();
    if (DXContext::Get().Init() && DXWindow::Get().Init())
    {
        // Start in full screen?
        //DXWindow::Get().SetFullscreen(true);

        const char* hello = "Hello World!";

        D3D12_HEAP_PROPERTIES hpUpload{};
		hpUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
		hpUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpUpload.CreationNodeMask = 0;
		hpUpload.VisibleNodeMask = 0;

		D3D12_HEAP_PROPERTIES hpDefault{};
		hpDefault.Type = D3D12_HEAP_TYPE_DEFAULT;
		hpDefault.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		hpDefault.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		hpDefault.CreationNodeMask = 0;
		hpDefault.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC rd{};
		rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rd.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		rd.Width = 1024;
		rd.Height = 1;
		rd.DepthOrArraySize = 1;
		rd.MipLevels = 1;
		rd.Format = DXGI_FORMAT_UNKNOWN;
		rd.SampleDesc.Count = 1;
		rd.SampleDesc.Quality = 0;
		rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rd.Flags = D3D12_RESOURCE_FLAG_NONE;

        ComPointer<ID3D12Resource2> uploadBuffer, vertexBuffer;
        HRESULT hr = DXContext::Get().GetDevice()->CreateCommittedResource(
            &hpUpload, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)
        );
        hr = DXContext::Get().GetDevice()->CreateCommittedResource(
            &hpDefault, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&vertexBuffer)
        );

        while (!DXWindow::Get().ShouldClose())
        {
            // Execute pending windows messages
            DXWindow::Get().Update();

            // Resize window if necessary
            if (DXWindow::Get().ShouldResize())
            {
                // flush command queue before resizing
				DXContext::Get().Flush(DXWindow::Get().GetFrameCount());
				DXWindow::Get().Resize();
            }

            // prep for Draw
            auto* cmdlist = DXContext::Get().InitCommandList();

            // draw to window
            DXWindow::Get().BeginFrame(cmdlist);

            // drawing

            DXWindow::Get().EndFrame(cmdlist);

            // finish Draw and present
            DXContext::Get().ExecuteCommandList();
            DXWindow::Get().Present();
        }

        // Flush command queue, buffer count number of times
        DXContext::Get().Flush(DXWindow::Get().GetFrameCount());
    
        DXWindow::Get().Shutdown();
        DXContext::Get().Shutdown();
    }

    DXDebugLayer::Get().Shutdown();



    POINT pt;
    GetCursorPos(&pt);
    std::cout << "x: " << pt.x << " y: " << pt.y << std::endl;
}