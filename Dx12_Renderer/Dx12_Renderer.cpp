#include <iostream>
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <DxDebug/DXDebugLayer.h>
#include <D3D/DXContext.h>
#include <Support/Window.h>
#include <Support/Shader.h>
#include <Support/ImageLoader.h>

void pukeColor(float* color)
{
    static int pukeState = 0;

    color[pukeState] += 0.005f;
    if (color[pukeState] > 1.f)
    {
        pukeState++;
        if (pukeState == 3)
        {
            color[0] = 0.f;
			color[1] = 0.f;
			color[2] = 0.f;
            pukeState = 0;
        }
    }
}


int main()
{
    DXDebugLayer::Get().Init();
    if (DXContext::Get().Init() && DXWindow::Get().Init())
    {
        // Start in full screen?
        //DXWindow::Get().SetFullscreen(true);


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

        // === Vertex Data ===
        struct Vertex
        {
            float x, y;
        };

		Vertex vertices[] =
        {
            {-1.f, -1.f},
            {0.f, 1.f},
            {1.f, -1.f}
        };
        D3D12_INPUT_ELEMENT_DESC vertexLayout[] =
        {
            {"Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

        // === Upload & Vertex Buffers ===
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

        // Copy void* -> cpu resource
        void* uploadBufferAddress;
        D3D12_RANGE uploadRange;
        uploadRange.Begin = 0;
        uploadRange.End = 1023; // size - 1
        uploadBuffer->Map(0, &uploadRange, &uploadBufferAddress);
        memcpy(uploadBufferAddress, vertices, sizeof(vertices));
        uploadBuffer->Unmap(0, &uploadRange);

        // === Texture ===
        ImageLoader::ImageData textureData;
        ImageLoader::LoadImageFromDisk("./auge_512_512_BGRA_32BPP.png", textureData);


        // Copy CPU resource -> gpu resource
        auto* tempCmdList = DXContext::Get().InitCommandList();
        tempCmdList->CopyBufferRegion(vertexBuffer, 0, uploadBuffer, 0, 1024);
		DXContext::Get().ExecuteCommandList();

        // === Shader File ===
		Shader rootSignatureShader("RootSignature.cso");
		Shader vertexShader("VertexShader.cso");
        Shader pixelShader("PixelShader.cso");

        // === Create Root Signature ===
        ComPointer<ID3D12RootSignature> rootSignature;
        DXContext::Get().GetDevice()->CreateRootSignature(
            0, rootSignatureShader.GetBuffer(), rootSignatureShader.GetSize(), IID_PPV_ARGS(&rootSignature)
        );

        // === Pipeline State ===
         D3D12_GRAPHICS_PIPELINE_STATE_DESC gfxPsod{};
        gfxPsod.pRootSignature = rootSignature;
		gfxPsod.InputLayout.NumElements = _countof(vertexLayout);
		gfxPsod.InputLayout.pInputElementDescs = vertexLayout;
        gfxPsod.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

        gfxPsod.VS.BytecodeLength = vertexShader.GetSize();
        gfxPsod.VS.pShaderBytecode = vertexShader.GetBuffer();
        gfxPsod.PS.BytecodeLength = pixelShader.GetSize();
        gfxPsod.PS.pShaderBytecode = pixelShader.GetBuffer();
		gfxPsod.DS.BytecodeLength = 0;
        gfxPsod.DS.pShaderBytecode = nullptr;
		gfxPsod.HS.BytecodeLength = 0;
        gfxPsod.HS.pShaderBytecode = nullptr;
		gfxPsod.GS.BytecodeLength = 0;
        gfxPsod.GS.pShaderBytecode = nullptr;

        // Rasterizer
        gfxPsod.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		gfxPsod.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		gfxPsod.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		gfxPsod.RasterizerState.FrontCounterClockwise = FALSE;
		gfxPsod.RasterizerState.DepthBias = 0;
		gfxPsod.RasterizerState.DepthBiasClamp = 0.f;
		gfxPsod.RasterizerState.SlopeScaledDepthBias = 0.f;
		gfxPsod.RasterizerState.DepthClipEnable = FALSE;
		gfxPsod.RasterizerState.MultisampleEnable = FALSE;
		gfxPsod.RasterizerState.AntialiasedLineEnable = FALSE;
		gfxPsod.RasterizerState.ForcedSampleCount = 0;
		gfxPsod.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        
        gfxPsod.NumRenderTargets = 1;
        gfxPsod.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        gfxPsod.DSVFormat = DXGI_FORMAT_UNKNOWN;
        gfxPsod.BlendState.AlphaToCoverageEnable = FALSE;
        gfxPsod.BlendState.IndependentBlendEnable = FALSE;
		gfxPsod.BlendState.RenderTarget[0].BlendEnable = TRUE;
		gfxPsod.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
		gfxPsod.BlendState.RenderTarget[0].SrcBlend  = D3D12_BLEND_ONE;
		gfxPsod.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		gfxPsod.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		gfxPsod.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		gfxPsod.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		gfxPsod.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		gfxPsod.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		gfxPsod.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        gfxPsod.DepthStencilState.DepthEnable = FALSE;
        gfxPsod.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        gfxPsod.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        gfxPsod.DepthStencilState.StencilEnable = FALSE;
        gfxPsod.DepthStencilState.StencilReadMask = 0;
        gfxPsod.DepthStencilState.StencilWriteMask = 0;
        gfxPsod.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		gfxPsod.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		gfxPsod.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		gfxPsod.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;

        gfxPsod.SampleMask = 0xFFFFFFFF;
        gfxPsod.SampleDesc.Count = 1;
        gfxPsod.SampleDesc.Quality = 0;
       
        gfxPsod.StreamOutput.NumEntries = 0;
        gfxPsod.StreamOutput.NumStrides = 0;
        gfxPsod.StreamOutput.pBufferStrides = nullptr;
        gfxPsod.StreamOutput.pSODeclaration = nullptr;
        gfxPsod.StreamOutput.RasterizedStream = 0;
        
        gfxPsod.NodeMask = 0;
		gfxPsod.CachedPSO.CachedBlobSizeInBytes = 0;
		gfxPsod.CachedPSO.pCachedBlob = nullptr;
        gfxPsod.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

        ComPointer<ID3D12PipelineState> pso;
        DXContext::Get().GetDevice()->CreateGraphicsPipelineState(&gfxPsod, IID_PPV_ARGS(&pso));

        // TODO: Output Merger

        // === Vertex Buffer View ===
		D3D12_VERTEX_BUFFER_VIEW vbv{};
		vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = sizeof(Vertex) * _countof(vertices);
		vbv.StrideInBytes = sizeof(Vertex);

		bool waxing = true;

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

            // === PSO ===
            cmdlist->SetPipelineState(pso);
            cmdlist->SetGraphicsRootSignature(rootSignature);

            // === Input Assembler ===
            cmdlist->IASetVertexBuffers(0, 1, &vbv);
            cmdlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // === Rasterizer ===
            D3D12_VIEWPORT vp;
            vp.TopLeftX = vp.TopLeftY = 0;
            vp.Width = DXWindow::Get().GetWidth();
            vp.Height = DXWindow::Get().GetHeight();
            vp.MinDepth = 1.f;
            vp.MaxDepth = 0.f;
            cmdlist->RSSetViewports(1, &vp);
            RECT scRect;
            scRect.left = scRect.top = 0;
            scRect.right = DXWindow::Get().GetWidth();
            scRect.bottom = DXWindow::Get().GetHeight();
            cmdlist->RSSetScissorRects(1, &scRect);

            // === Root signature ===
            static float color[] = { 0.f,0.f,0.f };
            pukeColor(color);
            cmdlist->SetGraphicsRoot32BitConstants(0, 3, color, 0);

            // === Output Merger ===
			/*static float bf_ff = 0.0f;
			if (waxing)
				bf_ff += 0.01f;
			else
				bf_ff -= 0.01f;

			if (bf_ff > 0.3f)
				waxing = false;
			if (bf_ff < 0.f)
				waxing = true;

			float bf[] = { bf_ff,bf_ff,bf_ff,bf_ff };
			cmdlist->OMSetBlendFactor(bf);*/

            // drawing
            cmdlist->DrawInstanced(_countof(vertices), 1, 0, 0);

            DXWindow::Get().EndFrame(cmdlist);

            // finish Draw and present
            DXContext::Get().ExecuteCommandList();
            DXWindow::Get().Present();
        }

        // Flush command queue, buffer count number of times
        DXContext::Get().Flush(DXWindow::Get().GetFrameCount());

        vertexBuffer.Release();
        uploadBuffer.Release();
    
        DXWindow::Get().Shutdown();
        DXContext::Get().Shutdown();
    }

    DXDebugLayer::Get().Shutdown();



    POINT pt;
    GetCursorPos(&pt);
    std::cout << "x: " << pt.x << " y: " << pt.y << std::endl;
}