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