#include "ImageLoader.h"

bool ImageLoader::LoadImageFromDisk(const std::filesystem::path& imagePath, ImageData& data)
{
	ComPointer<IWICImagingFactory> wicFactory;
	__ImageLoader_CAR(
		CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory))
	);

	// Load the image
	ComPointer<IWICStream> wicFileStream;
	__ImageLoader_CAR(
		wicFactory->CreateStream(&wicFileStream)
	);
	__ImageLoader_CAR(
		wicFileStream->InitializeFromFilename(imagePath.wstring().c_str(), GENERIC_READ)
	);

	ComPointer<IWICBitmapDecoder> wicDecoder;
	__ImageLoader_CAR(
		wicFactory->CreateDecoderFromStream(wicFileStream, nullptr, WICDecodeMetadataCacheOnDemand, &wicDecoder)
	);

	ComPointer<IWICBitmapFrameDecode> wicFrameDecoder;
	__ImageLoader_CAR(
		wicDecoder->GetFrame(0,&wicFrameDecoder)
	);

	// Trivial metadata
	__ImageLoader_CAR(
		wicFrameDecoder->GetSize(&data.width, &data.height)
	);
	__ImageLoader_CAR(
		wicFrameDecoder->GetPixelFormat(&data.pixelFormat)
	);

	// Metadata of pixel format
	ComPointer<IWICComponentInfo> wicComponentInfo;
	__ImageLoader_CAR(
		wicFactory->CreateComponentInfo(data.pixelFormat, &wicComponentInfo)
	);

	ComPointer<IWICPixelFormatInfo> wicPixelFormatInfo;
	__ImageLoader_CAR(
		wicComponentInfo->QueryInterface(&wicPixelFormatInfo)
	);

	__ImageLoader_CAR(
		wicPixelFormatInfo->GetBitsPerPixel(&data.bpp)
	);

	__ImageLoader_CAR(
		wicPixelFormatInfo->GetChannelCount(&data.cc)
	);

	// DXGI Pixel format

	// Image loading
	//wicFrameDecoder->CopyPixels();

}
