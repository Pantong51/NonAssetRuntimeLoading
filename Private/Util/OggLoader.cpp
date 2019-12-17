// Copyright Pan


#include "OggLoader.h"
#include "VorbisAudioInfo.h"
#include "IAudioFormat.h"

USoundWave* UOggLoader::LoadOggSound(const FString& FilePath)
{
	USoundWave* NewSoundWave = NewObject<USoundWave>(USoundWave::StaticClass());

	if (NewSoundWave == nullptr)
	{
		return nullptr;
	}

	bool Loaded = false;

	TArray<uint8> RawFile;

	Loaded = FFileHelper::LoadFileToArray(RawFile, FilePath.GetCharArray().GetData());

	if (Loaded)
	{
		FByteBulkData* BulkData = &NewSoundWave->CompressedFormatData.GetFormat(TEXT("OGG"));

		BulkData->Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(BulkData->Realloc(RawFile.Num()), RawFile.GetData(), RawFile.Num());
		BulkData->Unlock();

		Loaded = fillSoundWave(NewSoundWave, &RawFile) == 0 ? true : false;
	}

	if (!Loaded)
	{
		return nullptr;
	}

	return NewSoundWave;
}

UOggLoader* UOggLoader::LoadOggFromDiscAsync(UObject* Outer, const FString& FilePath)
{
	UOggLoader* Loader = NewObject<UOggLoader>();
	Loader->LoadOggAsync(Outer, FilePath);
	return Loader;
}

TFuture<USoundWave*> UOggLoader::LoadOggFromDiscAsync(UObject* Outer, const FString& FilePath, TFunction<void()> CompletionCallback)
{
	return Async(EAsyncExecution::ThreadPool, [=]() { return LoadOggSound(FilePath); }, CompletionCallback);
}

void UOggLoader::LoadOggAsync(UObject* Outer, const FString& ObjPath)
{
	Future = LoadOggFromDiscAsync(Outer, ObjPath, [this]()
		{
			if (Future.IsValid())
			{
				AsyncTask(ENamedThreads::GameThread, [this]() {LoadCompleted.Broadcast(Future.Get());});
			}
		});
}

int32 UOggLoader::fillSoundWave(USoundWave* InSoundWave, TArray<uint8>* RawFile)
{
	FSoundQualityInfo Info;
	FVorbisAudioInfo Vorbis_Obj;

	if (!Vorbis_Obj.ReadCompressedInfo(RawFile->GetData(), RawFile->Num(), &Info))
	{
		return 1;
	}

	if (InSoundWave == nullptr)
	{
		return 1;
	}

	InSoundWave->SoundGroup = ESoundGroup::SOUNDGROUP_Default;
	InSoundWave->NumChannels = Info.NumChannels;
	InSoundWave->Duration = Info.Duration;
	InSoundWave->RawPCMDataSize = Info.SampleDataSize;
	InSoundWave->SetSampleRate(Info.SampleRate);

	return 0;
}

