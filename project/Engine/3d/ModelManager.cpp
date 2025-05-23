#include "ModelManager.h"

ModelManager* ModelManager::instance = nullptr;


ModelManager* ModelManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ModelManager;
	}
	return instance;
}

void ModelManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {
	modelCommon = new ModelCommon;
	modelCommon->Initialize(dxCommon);
}

void ModelManager::LoadModel(const std::string& filePath) {
	if (models.contains(filePath)) {
		return;
	}

	std::string file = "/";
	std::string fileType = "";
	bool typeMode = false;

	for (char value : filePath) {
		if (value != '.' && !typeMode) {
			file += value;    //保管しているファイル名
		}
		else {
			fileType += value;//.obj / .gltf
			typeMode = true;
		}
	}
	file += "/";

	std::unique_ptr<Model> model = std::make_unique<Model>();

	if (fileType == ".obj") {
		model->SetObjType(ObjectType::obj);
	}
	else if (fileType == ".gltf") {
		model->SetObjType(ObjectType::gltf);
	}

	model->Initialize(modelCommon,"resource", "Object" + file + filePath);//model,file名,OBJ本体


	models.insert(std::make_pair(filePath, std::move(model)));

}

Model* ModelManager::FindModel(const std::string& filePath) {
	if(models.contains(filePath)){
		return models.at(filePath).get();
	}

	//ファイル一致なし
	return nullptr;
}