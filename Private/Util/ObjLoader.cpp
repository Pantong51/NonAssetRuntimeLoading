// Copyright Pan


#include "ObjLoader.h"
#include "ThirdParty/TinyObjLoader/tiny_obj_loader.h"

#define UIL_LOG(Verbosity, Format, ...)	UE_LOG(LogTemp, Verbosity, Format, __VA_ARGS__)

FReturnedData UObjLoader::LoadMesh(const FString& FilePath)
{
	std::string _FilePath(TCHAR_TO_UTF8(*FilePath));
	tinyobj::attrib_t Attributes;
	std::vector<tinyobj::shape_t> Shapes;
	std::vector<tinyobj::material_t> Materials;

	std::string _Warnning;
	std::string _Error;

	bool Success = tinyobj::LoadObj(&Attributes, &Shapes, &Materials, &_Warnning, &_Error, _FilePath.c_str());

	if (!_Warnning.empty())
	{
		FString Log(_Warnning.c_str());
		UIL_LOG(Error, TEXT("%s"), *Log);
		return FReturnedData();
	}

	if (!_Error.empty())
	{
		FString Log(_Error.c_str());
		UIL_LOG(Error, TEXT("%s"), *Log);
		return FReturnedData();
	}

	if (!Success)
	{
		UIL_LOG(Error, TEXT("Failed to load"));
		return FReturnedData();
	}
	FReturnedData ReturnData = FReturnedData();
	ReturnData.bSuccess = Success;

	for (size_t LShapes = 0; LShapes < Shapes.size(); LShapes++)
	{
		ReturnData.MeshInfo.Add(FMeshInfo());
		size_t Index_Offset = 0;
		for (size_t LFaces = 0; LFaces < Shapes[LShapes].mesh.num_face_vertices.size(); LFaces++)
		{
			int FaceVertices = Shapes[LShapes].mesh.num_face_vertices[LFaces];
			for (size_t LVerticies = 0; LVerticies < FaceVertices; LVerticies++)
			{
				tinyobj::index_t idx = Shapes[LShapes].mesh.indices[Index_Offset + LVerticies];

				//Noticed i need to swap the Z and Y Axis here if i did not the object would face the wrong way
				tinyobj::real_t vx = Attributes.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vz = Attributes.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vy = Attributes.vertices[3 * idx.vertex_index + 2];

				ReturnData.MeshInfo[LShapes].Vertices.Add(FVector(vx, vy, vz));

				//Noticed i need to swap the Z and Y Axis here if i did not the normals will be inverted
				tinyobj::real_t nx = Attributes.normals[3 * idx.normal_index + 0];
				tinyobj::real_t nz = Attributes.normals[3 * idx.normal_index + 1];
				tinyobj::real_t ny = Attributes.normals[3 * idx.normal_index + 2];

				ReturnData.MeshInfo[LShapes].Normals.Add(FVector(nx, ny, nz));

				tinyobj::real_t tx = Attributes.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = Attributes.texcoords[2 * idx.texcoord_index + 1];

				ReturnData.MeshInfo[LShapes].UV0.Add(FVector2D(tx, ty));

				tinyobj::real_t red = Attributes.colors[3 * idx.vertex_index + 0];
				tinyobj::real_t green = Attributes.colors[3 * idx.vertex_index + 1];
				tinyobj::real_t blue = Attributes.colors[3 * idx.vertex_index + 2];

				ReturnData.MeshInfo[LShapes].VertexColors.Add(FLinearColor(red, green, blue, 1.0f));

				ReturnData.MeshInfo[LShapes].Triangles.Add(Index_Offset + LVerticies);
			}
			Index_Offset += FaceVertices;
			Shapes[LShapes].mesh.material_ids[FaceVertices];
		}
	}
	return ReturnData;
}

UObjLoader* UObjLoader::LoadObjFromDiskAsync(UObject* Outer, const FString& ObjPath)
{
	UObjLoader* Loader = NewObject<UObjLoader>();
	Loader->LoadObjAsync(Outer, ObjPath);
	return Loader;
}

TFuture<FReturnedData> UObjLoader::LoadObjFromDiskAsync(UObject* Outer, const FString& ObjectPath, TFunction<void()> CompletionCallback)
{
	return Async(EAsyncExecution::ThreadPool, [=]() { return LoadMesh(ObjectPath); }, CompletionCallback);
}

void UObjLoader::LoadObjAsync(UObject* Outer, const FString& ObjPath)
{
	Future = LoadObjFromDiskAsync(Outer, ObjPath, [this]()
		{
			if (Future.IsValid())
			{
				AsyncTask(ENamedThreads::GameThread, [this]() {LoadCompleted.Broadcast(Future.Get());});
			}
		});
}
