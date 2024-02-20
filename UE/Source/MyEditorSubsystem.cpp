/**
* Import Post���� for OAR   by Fumi.Iseki
**/

#include "MyEditorSubsystem.h"
#include "Subsystems/ImportSubsystem.h"

//#include <Editor/UnrealEdEngine.h>
//#include <WorldPartition/ContentBundle/ContentBundleEngineSubsystem.h>

//#include "Subsystems/EditorAssetSubsystem.h"
//#include <Subsystems/AssetEditorSubsystem.h>
//#include <Subsystems/ActorEditorContextSubsystem.h>
//#include <Toolkits/AssetEditorModeUILayer.h>
//#include <TransformMeshesTool.h>
//#include <Subsystems/EditorActorSubsystem.h>


void UMyEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	//
	if (GEditor != NULL) {
		UImportSubsystem* ImportSubsystem = GEditor->GetEditorSubsystem<UImportSubsystem>();
		if (ImportSubsystem != NULL) {
			OnAssetPostImportHandle = ImportSubsystem->OnAssetPostImport.AddUObject(this, &UMyEditorSubsystem::OnAssetPostImport);
		}
	}
}


void UMyEditorSubsystem::Deinitialize()
{
	if (GEditor != NULL) {
		UImportSubsystem* ImportSubsystem = GEditor->GetEditorSubsystem<UImportSubsystem>();
		if (ImportSubsystem != NULL) {
			ImportSubsystem->OnAssetPostImport.Remove(OnAssetPostImportHandle);
		}
	}
	Super::Deinitialize();
}


void UMyEditorSubsystem::OnAssetPostImport(UFactory* Factory, UObject* CreatedObject)
{
	if (CreatedObject != NULL) {
		FString _class_name = CreatedObject->GetClass()->GetName();

		// Static Mesh
		if (_class_name.Equals(FString(TEXT("StaticMesh")))) {
			UStaticMesh* mesh = Cast<UStaticMesh>(CreatedObject);
			FString _mesh_name = mesh->GetName();

			// �t�@���g������
			if (_mesh_name.Find(FString(TEXT(OBJ_PHANTOM_PREFIX))) == 0) {
				UStaticMeshEditorSubsystem* MeshSubsystem = GEditor->GetEditorSubsystem<UStaticMeshEditorSubsystem>();
				MeshSubsystem->RemoveCollisions(mesh);
				
			}

			FString path = mesh->GetPathName();
			FString xxxx = path.Left(path.Find("."));
			UE_LOG(LogTemp, Log, TEXT("NNNNN %s"), *xxxx);

			FVector center = mesh->GetBoundingBox().GetCenter();
			
			UPackage* pack = (UPackage*)mesh->GetOuter();
			
			TArray<UObject*> abcs;
			pack->GetDefaultSubobjects(abcs);
			for (auto* aaa : abcs) {
				FString xxx = aaa->GetClass()->GetName();
				UE_LOG(LogTemp, Log, TEXT("-------------------> %s"), *xxx);
			}


			//UMetaData* dat = pack->GetMetaDatas();
			//TMap<FName, FString> values;
			//values.
			//dat->SetObjectValues(dat, values);
			//pack->set

			//UE_LOG(LogTemp, Log, TEXT("-------------------> %s"), *xxx);
			
			
			//mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, *path));
			//smhc->AddLocalOffset(-center);
			
			//UAssetImportData* dat = mesh->GetAssetImportData();

			//mesh->SetAssetImportData();

			//mesh->RemoveFromRoot();
			//mesh->get
			//UActorEditorContextSubsystem* ac = GEditor->GetEditorSubsystem<UActorEditorContextSubsystem>();
			//FFrame ff;
			//ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
			//ProjectileMeshComponent->SetStaticMesh(mesh);
			//ProjectileMeshComponent->AddLocalOffset(-center);
			//UPROPERTY(EditAnywhere, Category = Projectile)
			//UStaticMeshComponent sm;
			//sm.InitializeComponent();
			//a.SetStaticMesh(mesh);
			//a.AddLocalOffset(-center);

		



			

			/**/
			//UStaticMeshComponent* smhc = mesh->GetTypedOuter<UStaticMeshComponent>();
			UStaticMeshComponent* smhc = mesh->GetTypedOuter<UStaticMeshComponent>();
			//FVector center = mesh->GetBoundingBox().GetCenter();
			//UStaticMeshComponent* smhc = NewObject<UStaticMeshComponent>(mesh);
			if (smhc != NULL) {
			//smhc->RegisterComponent();
			//smhc->SetupAttachment(NULL);
			//if (smhc->SetStaticMesh(mesh)) {
					smhc->AddLocalOffset(-center);
			//		mesh = smhc->GetStaticMesh();
			//	}

				//FTransform trans(-center);
				//smhc->SetWorldTransform(trans);
				//memcpy(mesh, smhc->GetStaticMesh(), sizeof(UStaticMesh));
			}
			else{
				UE_LOG(LogTemp, Log, TEXT("NNNNNNNNNNNNNNNNNNNNNNNUUUUUULL"));

				AActor* act = mesh->GetTypedOuter<AActor>();
				if (act != NULL) {
					act->SetPivotOffset(center);
					//o->offcet > SetoffLocalOffset(-center);
				}
				else {
					UE_LOG(LogTemp, Log, TEXT("NNNNNNNNNNNNNNNNNNNNNNNUUUUUULL222222222222"));
				}
 
			}


			/*
			UEditorAssetSubsystem* EditorAssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
			FString xxxx= EditorAssetSubsystem->GetMetadataTag(CreatedObject, FName(TEXT("offset_translation")));
			UE_LOG(LogTemp, Log, TEXT("XXXXXXXXXXXXXX = %s"), *xxxx);

			EditorAssetSubsystem->SetMetadataTag(CreatedObject, FName(TEXT("ImportOffsetTranslation")), TEXT("{X:0.0, Y:0.0, Z:1000.0}"));
			*/

			//GEditor->SetPivot(FVector(0.0, 0.0, 0.0), false, false);
			//FVector vv = GEditor->GetPivotLocation();
			//GEditor->ResetPivot();
			//GEditor->SetPivot(center, false, false, true);
			//GEditor->
			//UStaticMeshComponent* aaa = CreateDefaultSubobject<UStaticMeshComponent>(mesh->GetName());

			//UE_LOG(LogTemp, Log, TEXT("center = %lf %lf %lf"), center.X, center.Y, center.Z);

			//UStaticMeshComponent* bbb = Cast<UStaticMeshComponent>(StaticLoadObject(UTexture::StaticClass(), NULL, *mesh->GetPathName()));
			//bbb->AddLocalOffset(-center);

			//UContentBundleEngineSubsystem* engine = GEngine->GetEngineSubsystem<UContentBundleEngineSubsystem>();
			//engine->execTransformConst()
			//engine- SetPivot(center, true, false);


			//UEditorEngine* engine = Cast<UEditorEngine>(GEngine);
			//UUnrealEdEngine* enginex = Cast<UUnrealEdEngine>(GEngine);

			//UE_LOG(LogTemp, Log, TEXT("center = %lf %lf %lf"), center.X, center.Y, center.Z);
			//UE_LOG(LogTemp, Log, TEXT("pivot  = %lf %lf %lf"), vv.X, vv.Y, vv.Z);

			//engine->SetPivot(center, true, false);
			//FVector vv = engine->GetPivotLocation();
			//UE_LOG(LogTemp, Log, TEXT("pivot  = %lf %lf %lf"), vv.X, vv.Y, vv.Z);

			//FTransform()
			//UStaticMeshComponent* meshcmp; // = UStaticMeshComponent::AddLo
			//mesh->
			//Factory->

			//	::Create(new_mtlif, NULL);
			//UStaticMeshComponent* meshcmp = UStaticMeshComponent::Create Create(new_mtlif, NULL);
			//UStaticMeshComponent* meshcmp = Cast<UStaticMeshComponent>(CreatedObject);
			//UMeshComponent* meshcmp = Cast<UMeshComponent>(CreatedObject);
			//meshcmp->AddLocalOffset(-center);
			//meshcmp->pivot

			//mesh->execTransformConst
			//mesh->AddlocalRotation()
			//UStaticMesh::execTransformConst(mesh, )

			//mesh->execTransformConst()
			//FFrame
			// UAssetUserData::pivo
			//UAssetUserData::execTransformConst(mesh, NULL, NULL);
			//mesh->AddAssetUserData()


			int lod = 0;
			auto* mtlif = mesh->GetMaterial(lod);
			while (mtlif != NULL) {
				FString _mtl_name = mtlif->GetName();
				// �p�����[�^�̕���
				TArray<float> params = GetTextureParams(_mtl_name);
				if (params[MATERIAL_PARAMS_KIND] == 0.0f) {
					mtlif = mesh->GetMaterial(++lod);
					continue;
				}
				// �}�e���A���̑I��
				UMaterialInterface* new_mtlif = SelectMaterialInterface(params);
				if (new_mtlif == NULL) {
					mtlif = mesh->GetMaterial(++lod);
					continue;
				}
				// �p�����[�^�̓K�p
				UMaterialInstanceDynamic* material = UMaterialInstanceDynamic::Create(new_mtlif, NULL);
				if (material != NULL) {
					// Texture
					FString _obj_path = mtlif->GetPathName();
					FString texture_path = GetTexturePath(_obj_path, _mtl_name);
					UTexture* texture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL, *texture_path));
					if (texture != NULL) {
						material->SetTextureParameterValue(FName(TEXT("Texture")), texture);
					}
					// Some Parameters
					material->SetVectorParameterValue(FName(TEXT("BaseColor")), FLinearColor(params[0], params[1], params[2]));
					material->SetScalarParameterValue(FName(TEXT("Transparent")), params[3]);
					material->SetScalarParameterValue(FName(TEXT("Cutoff")), params[4]);
					material->SetScalarParameterValue(FName(TEXT("Shininess")), params[5]);
					material->SetScalarParameterValue(FName(TEXT("Glow")), params[6]);
					material->SetScalarParameterValue(FName(TEXT("Bright")), params[7]);
					material->SetScalarParameterValue(FName(TEXT("Light")), params[8]);
					// Setup
					mesh->SetMaterial(lod, material);
				}
				//mtlif->ClearGarbage();// ->InitDefaultMaterials();
				mtlif = mesh->GetMaterial(++lod);
			}
			memcpy(CreatedObject, mesh, sizeof(UStaticMesh));
		}

		// Material or MaterialInstanceConstant
		else if (CreatedObject->GetName().Find(FString(TEXT("MATERIAL_"))) == 0) {
			if (_class_name.Equals(FString(TEXT("MaterialInstanceConstant")))) {
				//UE_LOG(LogTemp, Log, TEXT("Material Instance = %s"), *(CreatedObject->GetName()));
			}
			else if (_class_name.Equals(FString(TEXT("Material")))) {
				//UE_LOG(LogTemp, Log, TEXT("Material = %s"), *(CreatedObject->GetName()));
			}
		}

		// Texture2D
		else if (_class_name.Equals(FString(TEXT("Texture2D")))) {
			//UE_LOG(LogTemp, Log, TEXT("Texture2D = %s"), *(CreatedObject->GetName()));
		}

	}
}


UMaterialInterface* UMyEditorSubsystem::SelectMaterialInterface(TArray<float> params)
{
	UMaterialInterface* mtlif = NULL;

	int kind = (int)(params[MATERIAL_PARAMS_KIND] + 0.5f);
	if (kind == 84 || kind == 71) {		// 'T' or 'G'
		mtlif = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL,
			*FString(TEXT(MATERIAL_SHADER_ALPHA))));
	}
	else if (kind == 79) {				// 'O'
		if (params[3] < 0.99f) {			// Transparent
			if (params[4] < 0.01f) {			// Cutoff
				mtlif = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL,
					*FString(TEXT(MATERIAL_SHADER_ALPHA))));
			}
			else {
				mtlif = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL,
					*FString(TEXT(MATERIAL_SHADER_MASK))));
			}
		}
		else {
			mtlif = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL,
				*FString(TEXT(MATERIAL_SHADER_OBJ))));
		}
	}
	else if (kind == 69) {				// 'E'
		mtlif = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL,
			*FString(TEXT(MATERIAL_SHADER_OBJ))));
	}

	return mtlif;
}


FString UMyEditorSubsystem::GetTexturePath(FString mtl_path, FString mtl_name)
{
	FString _txpath = mtl_path.Left(mtl_path.Find(mtl_name));
	FString _txname = FString(TEXT("00000000-0000-0000-0000-000000000000"));
	//
	int32 _stt_name = mtl_name.Find(TEXT("_"));
	int32 _end_name = mtl_name.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, mtl_name.Len());
	
	int32 _len_txname = _end_name - _stt_name - 1;
	if (_len_txname > 0) {
		_txname = mtl_name.Left(_end_name).Right(_len_txname);
	}
	FString texture_name = _txpath + FString(TEXT("TEX_")) + _txname;

	return texture_name;
}


TArray<float> UMyEditorSubsystem::GetTextureParams(FString mtl_name)
{
	TArray<float> params;
	params.Init(0.0, MATERIAL_PARAMS_SIZE);
	params[3] = 1.0f;	// Transparent

	int32 _name_end = mtl_name.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, mtl_name.Len());
	FString _params_str = mtl_name.Right(mtl_name.Len() - _name_end - 1);

	if (_params_str.Len() != MATERIAL_BASE64_SIZE) return params;
	_params_str = _params_str.Replace(TEXT("-"), TEXT("/"));
	//UE_LOG(LogTemp, Log, TEXT("params = %s"), *_params_str);

	TArray<uint8> dec;
	FBase64::Decode(_params_str, dec);
	if (dec.Num() != MATERIAL_PARAMS_SIZE) return params;

	params[0]  = 1.0f - (float)dec[0] / 255.0f;			// Red
	params[1]  = 1.0f - (float)dec[1] / 255.0f;			// Green
	params[2]  = 1.0f - (float)dec[2] / 255.0f;			// Blue
	params[3]  = 1.0f - (float)dec[3] / 255.0f;			// Alpha
	params[4]  = (float)dec[4] / 255.0f;				// Cutoff
	params[5]  = (float)dec[5] / 255.0f;				// Shinuness
	params[6]  = (float)dec[6] / 255.0f;				// Glow
	params[7]  = (float)dec[7] / 255.0f;				// Bright
	params[8]  = (float)dec[8] / 255.0f;				// Light
	params[23] = (float)dec[MATERIAL_PARAMS_SIZE - 1];	// Kind

	return params;
}
