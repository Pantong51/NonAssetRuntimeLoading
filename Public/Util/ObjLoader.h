// Copyright Pan

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProceduralMeshComponent.h"
#include "ObjLoader.generated.h"
USTRUCT(BlueprintType)
struct FMeshInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FVector> Vertices;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<int32> Triangles;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FVector> Normals;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FVector2D> UV0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FLinearColor> VertexColors;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FProcMeshTangent> Tangents;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FTransform RelativeTransform;
};

USTRUCT(BlueprintType)
struct FReturnedData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bSuccess;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 NumMeshes;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FMeshInfo> MeshInfo;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjLoadCompleted, FReturnedData, MeshData);

UCLASS()
class FRIGRATEBRIDGESIM_API UObjLoader : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "ObjLoader")
	static FReturnedData LoadMesh(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "ObjLoader", meta = (HidePin = "Outer", DefaultToSelf = "Outer"))
	static UObjLoader* LoadObjFromDiskAsync(UObject* Outer, const FString& ObjPath);

	static TFuture<FReturnedData> LoadObjFromDiskAsync(UObject* Outer, const FString& ObjectPath, TFunction<void()> CompletionCallback);

	FOnObjLoadCompleted& OnLoadCompleted()
	{
		return LoadCompleted;
	}
private:
	void LoadObjAsync(UObject* Outer, const FString& ObjPath);

private:
	UPROPERTY(BlueprintAssignable, Category = "ObjLoaded", meta = (AllowPrivateAccess = true))
	FOnObjLoadCompleted LoadCompleted;
	TFuture<FReturnedData> Future;
};
