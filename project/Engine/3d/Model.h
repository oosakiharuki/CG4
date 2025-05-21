#pragma once
#include "MyMath.h"
#include "ModelCommon.h"
#include <assimp/scene.h>

class Model{
public:

	void Initialize(ModelCommon* modelCommon,const std::string& directorypath,const std::string& fileName);

	void Draw();
	void Draw(const std::string& textureFilePath);

	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
	//gltf用
	static ModelData LoadModelFile(const std::string& directoryPath, const std::string& filename);

	void LightOn(bool Light) { materialData->enableLighting = Light; }

	static Node ReadNode(aiNode* node);

	ModelData GetModelData() { return modelData; }

private:
	ModelCommon* modelCommon = nullptr;

	ModelData modelData;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;


	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	ModelData InitialData;
};