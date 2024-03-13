#pragma once

#define  NOMINMAX

#include <Windows.h>
#include <wrl/client.h>
#include <cstdlib>

#include <d3d12.h>
#include <dxgi1_6.h>

#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif // _DEBUG
