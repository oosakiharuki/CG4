#include "Model.h"
#include "TextureManager.h"
#include <fstream>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace MyMath;

void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& fileName) {
	this->modelCommon = modelCommon;

	switch (objectType)
	{
	case ObjectType::obj:	//.obj
		modelData = LoadObjFile(directorypath, fileName);
		break;
	case ObjectType::gltf:
		//.gltf
		modelData = LoadModelFile(directorypath, fileName);
		animation = LoadAnimationFile(directorypath, fileName);
		break;
	}

	InitialData = modelData;

	vertexResource = modelCommon->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());


	//Model用マテリアル
	//マテリアル用のリソース
	materialResource = modelCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	//書き込むためのアドレス
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色の設定
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();
	materialData->shininess = 70;

	//テクスチャ読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	modelData.material.textureIndex = TextureManager::GetInstance()->GetSrvIndex(modelData.material.textureFilePath);
}

void Model::Draw() {
	//objファイルに元々あったテクスチャ
	modelData = InitialData;

	modelCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	modelCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress()); //rootParameterの配列の0番目 [0]
	modelCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath));
	modelCommon->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

}

void Model::Draw(const std::string& textureFilePath) {

	TextureManager::GetInstance()->LoadTexture(textureFilePath);
	modelData.material.textureFilePath = textureFilePath;
	modelData.material.textureIndex = TextureManager::GetInstance()->GetSrvIndex(textureFilePath);
	

	modelCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	modelCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress()); //rootParameterの配列の0番目 [0]
	modelCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData.material.textureFilePath));
	modelCommon->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

}


MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	//ファイルを開く
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/Sprite/" + textureFilename;
		}
	}
	return materialData;
};


ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;

	const aiScene* scene = importer.ReadFile(filePath.c_str(),aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes()); //メッシュがないのは対応なし

	//VertexDataを読み取る
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());//法線があるか
		assert(mesh->HasTextureCoords(0));//Texcordがあるか

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);//三角形のみ
			
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y, normal.z, };
				vertex.texcoord = { texcoord.x,texcoord.y };

				//aiProcess_MakeleftHandleなので z *= -1,右手→左手(x *= -1)に変換する
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;

				modelData.vertices.push_back(vertex);
			}
		}
	}	
	//MaterialData
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/Sprite/" + textureFilePath.C_Str();		
		}

	}	
	//オブジェクトには親子ノードがないので単品
	Node result;
	result.localMatrix = MakeIdentity4x4();
	modelData.rootNode = result;

	return modelData;
}

ModelData Model::LoadModelFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;

	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes()); //メッシュがないのは対応なし

	//VertexDataを読み取る
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());//法線があるか
		assert(mesh->HasTextureCoords(0));//Texcordがあるか

		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3);//三角形のみ

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

				VertexData vertex;
				vertex.position = { position.x,position.y,position.z,1.0f };
				vertex.normal = { normal.x,normal.y, normal.z, };
				vertex.texcoord = { texcoord.x,texcoord.y };

				//aiProcess_MakeleftHandleなので z *= -1,右手→左手(x *= -1)に変換する
				vertex.position.x *= -1.0f;
				vertex.normal.x *= -1.0f;

				//テクスチャが逆にならないようにする(やり方は正解か不明)
				vertex.texcoord.x *= -1.0f;

				modelData.vertices.push_back(vertex);
			}
		}
	}

	modelData.material.textureFilePath = directoryPath + "/Sprite/uvChecker.png";
	//MaterialData
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			modelData.material.textureFilePath = directoryPath + "/Sprite/" + textureFilePath.C_Str();
		}

	}
	
	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

Node Model::ReadNode(aiNode* node) {
	Node result;

	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);
	result.transform.scale = { scale.x ,scale.y ,scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w };
	result.transform.translate = { -translate.x,translate.y ,translate.z };
	result.localMatrix = MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);

	//aiMatrix4x4 ailocalMatrix = node->mTransformation;//node localMatrixを取得
	//ailocalMatrix.Transpose();//列ベクトル→行ベクトル
	//result.localMatrix.m[0][0] = ailocalMatrix[0][0];
	//result.localMatrix.m[0][1] = ailocalMatrix[0][1];
	//result.localMatrix.m[0][2] = ailocalMatrix[0][2];
	//result.localMatrix.m[0][3] = ailocalMatrix[0][3];

	//result.localMatrix.m[1][0] = ailocalMatrix[1][0];
	//result.localMatrix.m[1][1] = ailocalMatrix[1][1];
	//result.localMatrix.m[1][2] = ailocalMatrix[1][2];
	//result.localMatrix.m[1][3] = ailocalMatrix[1][3];

	//result.localMatrix.m[2][0] = ailocalMatrix[2][0];
	//result.localMatrix.m[2][1] = ailocalMatrix[2][1];
	//result.localMatrix.m[2][2] = ailocalMatrix[2][2];
	//result.localMatrix.m[2][3] = ailocalMatrix[2][3];

	//result.localMatrix.m[3][0] = ailocalMatrix[3][0];
	//result.localMatrix.m[3][1] = ailocalMatrix[3][1];
	//result.localMatrix.m[3][2] = ailocalMatrix[3][2];
	//result.localMatrix.m[3][3] = ailocalMatrix[3][3];



	result.name = node->mName.C_Str();//nodeの名前
	result.children.resize(node->mNumChildren);//子供の数
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		//再帰関数で階級構造
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}


Animation  Model::LoadAnimationFile(const std::string& directoryPath, const std::string& filename) {
	Animation animation;
	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	assert(scene->mNumAnimations != 0);//アニメーションがないとき
	aiAnimation* animationAssimp = scene->mAnimations[0];//最初のアニメーションのみ。複数はまだ
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);//時間単位を秒に

	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			keyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x,keyAssimp.mValue.y ,keyAssimp.mValue.z };
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			keyframeQuatarnion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			//y,zを右手から左手に変更するため" - "に
			keyframe.value = { keyAssimp.mValue.x,-keyAssimp.mValue.y ,-keyAssimp.mValue.z,keyAssimp.mValue.w };
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}

		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			keyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x,keyAssimp.mValue.y ,keyAssimp.mValue.z };
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}
	}


	return animation;
}