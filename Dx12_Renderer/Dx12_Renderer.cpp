#include <iostream>
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>

int main()
{
//#ifdef _DEBUG
//	{
//	}
//#endif
	ComPointer<ID3D12Debug> debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	debugController->EnableDebugLayer();

	ComPointer<ID3D12Device> device;
	HRESULT hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
	if (FAILED(hr))
	{
		// Handle error
		std::cout << "Berry sad\n";
	}

	ComPointer<ID3D12Debug1> debug6;
	hr = device->QueryInterface(IID_PPV_ARGS(&debug6));
	if (FAILED(hr))
	{
		// Handle error
		std::cout << "Berry sad\n";
	}

	hr = device->QueryInterface(IID_PPV_ARGS(&debugController));


    std::cout << "Hello World!\n";

    POINT pt;
    GetCursorPos(&pt);
    std::cout << "x: " << pt.x << " y: " << pt.y << std::endl;
}