// Copyright Pan

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundWave.h"
#include "OggLoader.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOggLoadCompleted, USoundWave*, SoundData);

UCLASS()
class FRIGRATEBRIDGESIM_API UOggLoader : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "OggLoader")
	static USoundWave* LoadOggSound(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "OggLoader", meta = (HidePin = "Outer", DefaultToSelf = "Outer"))
	static UOggLoader* LoadOggFromDiscAsync(UObject* Outer, const FString& FilePath);

	static TFuture<USoundWave*> LoadOggFromDiscAsync(UObject* Outer, const FString& FilePath, TFunction<void()> CompletionCallback);

	FOnOggLoadCompleted& OnLoadCompleted()
	{
		return LoadCompleted;
	}
private:
	void LoadOggAsync(UObject* Outer, const FString& ObjPath);

	static int32 fillSoundWave(USoundWave* InSoundWave, TArray<uint8>* RawFile);

private:
	UPROPERTY(BlueprintAssignable, Category = "ObjLoaded", meta = (AllowPrivateAccess = true))
	FOnOggLoadCompleted LoadCompleted;
	TFuture<USoundWave*> Future;
};
