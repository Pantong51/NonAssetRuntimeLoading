// Copyright Pan

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ImageLoader.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnImageLoadCompleted, UTexture2D*, Texture);

UCLASS()
class NARL_API UImageLoader : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "ImageLoader", meta = (HidePin = "Outer", DefaultToSelf = "Outer"))
	static UImageLoader* LoadImageFromDiskAsync(UObject* Outer, const FString& ImagePath);

	static TFuture<UTexture2D*> LoadImageFromDiskAsync(UObject* Outer, const FString& ImagePath, TFunction<void()> CompletionCallback);

	UFUNCTION(BlueprintCallable, Category = "ImageLoader", meta = (HidePin = "Outer", DefaultToSelf = "Outer"))
	static UTexture2D* LoadImageFromDisk(UObject* Outer, const FString& ImagePath);

	FOnImageLoadCompleted& OnLoadCompleted()
	{
		return LoadCompleted;
	}

private:
	void LoadImageAsync(UObject* Outer, const FString& imagePath);

	static UTexture2D* CreateTexture(UObject* Outer, const TArray<uint8>& PixleData, int32 InSizeX, int32 InSizeY, EPixelFormat PixelFormat = EPixelFormat::PF_B8G8R8A8, FName BaseName = NAME_None);
private:
	UPROPERTY(BlueprintAssignable, Category = "ImageLoader", meta = (AllowPrivateAccess = true))
	FOnImageLoadCompleted LoadCompleted;
	TFuture<UTexture2D*> Future;
};
