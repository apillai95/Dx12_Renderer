#pragma once
#include <Support/WinInclude.h>
#include <Support/ComPointer.h>
#include <vector>
#include <filesystem>

#define __ImageLoader_CAR(expr) do { if(FAILED(expr)) { return false; } } while(false)

class ImageLoader
{
public:
	struct ImageData
	{
		std::vector<char> data;
		uint32_t	width;
		uint32_t	height;
		uint32_t	bpp;
		uint32_t	cc;
		GUID		pixelFormat;
		DXGI_FORMAT giPixelFormat;
	};

	static bool LoadImageFromDisk(const std::filesystem::path& imagePath, ImageData& data);

private:
	ImageLoader() = default;
	ImageLoader(const ImageLoader&) = default;
	ImageLoader& operator= (const ImageLoader&) = default;
};