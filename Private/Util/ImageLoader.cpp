// Copyright Pan


#include "ImageLoader.h"
#include "IImageWrapper.h"
#include "RenderUtils.h"
#include "Engine/Texture2D.h"
#include "IImageWrapperModule.h"

#define UIL_LOG(Verbosity, Format, ...)	UE_LOG(LogTemp, Verbosity, Format, __VA_ARGS__)

static IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

UImageLoader* UImageLoader::LoadImageFromDiskAsync(UObject* Outer, const FString& ImagePath)
{
	UImageLoader* Loader = NewObject<UImageLoader>();
	Loader->LoadImageAsync(Outer, ImagePath);
	return Loader;
}

TFuture<UTexture2D*> UImageLoader::LoadImageFromDiskAsync(UObject* Outer, const FString& ImagePath, TFunction<void()> CompletionCallback)
{
	return Async(EAsyncExecution::ThreadPool, [=]() { return LoadImageFromDisk(Outer, ImagePath); }, CompletionCallback);
}

UTexture2D* UImageLoader::LoadImageFromDisk(UObject* Outer, const FString& ImagePath)
{
	if (!FPaths::FileExists(ImagePath))
	{
		UIL_LOG(Error, TEXT("File Not Found: %s"), *ImagePath);
		return nullptr;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
	{
		UIL_LOG(Error, TEXT("Failed to load file: %s"), *ImagePath);
		return nullptr;
	}

	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
	if (ImageFormat == EImageFormat::Invalid)
	{
		UIL_LOG(Error, TEXT("Unrecognized image file format: %s"), *ImagePath);
		return nullptr;
	}

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid())
	{
		UIL_LOG(Error, TEXT("Failed to create wrapper for file %s"), *ImagePath);
		return nullptr;
	}

	const TArray<uint8>* RawData = nullptr;
	ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num());
	ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData);
	if (RawData == nullptr)
	{
		UIL_LOG(Error, TEXT("Failed to decompress image file: %s"), *ImagePath);
		return nullptr;
	}

	FString TextureBaseName = TEXT("Texture_") + FPaths::GetBaseFilename(ImagePath);
	return CreateTexture(Outer, *RawData, ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), EPixelFormat::PF_B8G8R8A8, FName(*TextureBaseName));

}

void UImageLoader::LoadImageAsync(UObject* Outer, const FString& imagePath)
{
	Future = LoadImageFromDiskAsync(Outer, imagePath, [this]()
	{
		if (Future.IsValid())
		{
			AsyncTask(ENamedThreads::GameThread, [this]() { LoadCompleted.Broadcast(Future.Get()); });
		}
	});
}

UTexture2D* UImageLoader::CreateTexture(UObject* Outer, const TArray<uint8>& PixleData, int32 InSizeX, int32 InSizeY, EPixelFormat PixelFormat /*= EPixelFormat::PF_B8G8R8A8*/, FName BaseName /*= NAME_None*/)
{
	if (InSizeX <= 0 || InSizeY <= 0 || (InSizeX % GPixelFormats[PixelFormat].BlockSizeX) != 0 || (InSizeY % GPixelFormats[PixelFormat].BlockSizeY) != 0)
	{
		UIL_LOG(Error, TEXT("Invalid Parameters specified for UImageLoader"));
		return nullptr;
	}

	FName TextureName = MakeUniqueObjectName(Outer, UTexture2D::StaticClass(), BaseName);
	UTexture2D* NewTexture = nullptr;
	NewTexture = UTexture2D::CreateTransient(InSizeX, InSizeY, PF_B8G8R8A8);

	if (NewTexture != nullptr)
	{
		void* TextureData = NewTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, PixleData.GetData(), PixleData.Num());
		NewTexture->PlatformData->Mips[0].BulkData.Unlock();
		NewTexture->UpdateResource();
		return NewTexture;
	}
	return nullptr;
}
