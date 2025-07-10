#include "Object3d.h"
#include "Object3dCommon.h"
#include "externals/imgui/imgui.h"
#include "TextureManager.h"

#include <fstream>
#include <sstream>
#include "ModelManager.h"
#include <numbers>
#include "Logger.h"

using namespace MyMath;

Object3d::Object3d(){}

Object3d::~Object3d(){
	for (auto it : debugSphere) {
		delete it;
	}
	debugSphere.clear();
}

void Object3d::Initialize() {
	this->object3dCommon = Object3dCommon::GetInstance();
	this->camera = object3dCommon->GetDefaultCamera();
	wvpResource = object3dCommon->GetDirectXCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));


	wvpData->World = MakeIdentity4x4();
	wvpData->WVP = MakeIdentity4x4();

	//ライト用のリソース
	directionalLightSphereResource = object3dCommon->GetDirectXCommon()->CreateBufferResource(sizeof(DirectionalLight));
	//書き込むためのアドレス
	directionalLightSphereResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightSphereData));
	//色の設定
	directionalLightSphereData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightSphereData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightSphereData->intensity = 0.0f;


	//Phong Reflection Model
	cameraResource = object3dCommon->GetDirectXCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));

	cameraData->worldPosition = { 0,0,0 };

	//ライト用のリソース
	pointLightResource = object3dCommon->GetDirectXCommon()->CreateBufferResource(sizeof(PointLight));
	//書き込むためのアドレス
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	//設定
	pointLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData->position = { 0.0f,2.0f,0.0f };
	pointLightData->intensity = 1.0f;
	pointLightData->radius = 5.0f;
	pointLightData->decay = 1.0f;


	//ライト用のリソース
	spotLightResource = object3dCommon->GetDirectXCommon()->CreateBufferResource(sizeof(SpotLight));
	//書き込むためのアドレス
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	//設定
	spotLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	spotLightData->position = { 2.0f,1.25f,0.0f };
	spotLightData->distance = 70.0f;
	spotLightData->direction = Normalize({ -1.0f,-1.0f,0.0f });
	spotLightData->intensity = 0.0f;
	spotLightData->decay = 2.0f;
	spotLightData->cosAngle = std::cos(std::numbers::pi_v<float> / 3.0f);
	spotLightData->cosFalloffStart = std::cos(std::numbers::pi_v<float> / 4.0f);






	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	transformL = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };




}

void Object3d::Update(const WorldTransform& worldTransform) {

	//作るときはフレームレートを60FPSにする
	animationTime += 1.0f / 60.0f;
	animationTime = std::fmod(animationTime, animation.duration);
	
	if (isChange) {
		animationTime2 += 1.0f / 60.0f;
		Interpolation(skeleton, animation, animation2, animationTime2);	
		
		if (animationTime2 >= animation.duration) {
			isChange = false;
			animationTime2 = 0;
		}
		SkeletonUpdate(skeleton2);
		SkinClusterUpdate(skinCluster2, skeleton2);
	}
	else {
		ApplyAnimation(skeleton, animation, animationTime);
	}

	SkeletonUpdate(skeleton);
	SkinClusterUpdate(skinCluster, skeleton);



	Matrix4x4 skaletonSpaceMatrix;
	Matrix4x4 WorldViewProjectionMatrix;
	if (camera) {
		Matrix4x4 projectionMatrix = camera->GetViewProjectionMatrix();
		WorldViewProjectionMatrix = worldTransform.matWorld_ * projectionMatrix;
	}
	else {
		WorldViewProjectionMatrix = worldTransform.matWorld_;
	}
	Matrix4x4 JointWorldMatrix = skaletonSpaceMatrix * worldTransform.matWorld_;

	wvpData->World = JointWorldMatrix * worldTransform.matWorld_;
	wvpData->World = modelData.rootNode.localMatrix * worldTransform.matWorld_;
	//wvpData->World = worldMatrix;
	wvpData->WVP = WorldViewProjectionMatrix;

	directionalLightSphereData->direction = Normalize(directionalLightSphereData->direction);
}


void Object3d::Draw() {
	//モデル
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightSphereResource->GetGPUVirtualAddress());
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());
	if (model) {
		model->Draw();
	}

	DebugWireframes::GetInstance()->Command();

	for (auto it : debugSphere) {
		it->Draw();
	}

	Object3dCommon::GetInstance()->Command();

}

void Object3d::Draw(const std::string& textureData) {
	//モデル
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightSphereResource->GetGPUVirtualAddress());
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	object3dCommon->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());
	if (model) {
		model->Draw(textureData);
	}
}

void Object3d::SetModelFile(const std::string& filePath) {

	model = ModelManager::GetInstance()->FindModel(filePath);
	modelData = model->GetModelData();
	animation = model->GetAnimationData();
	skeleton = model->GetSkeleton();
	skinCluster = model->GetSkinCluster();

	//デバッグワイヤーフレーム
	//親ノード
	//SetWireframe();
	//子ノード
	for (uint32_t childIndex = 0; childIndex < skeleton.joints.size(); ++childIndex) {
		SetWireframe();
	}

	SkeletonUpdate(skeleton);
	SkinClusterUpdate(skinCluster,skeleton);

}

void Object3d::LightSwitch(bool isLight) {
	if (model) {
		model->LightOn(isLight);
	}
}

void Object3d::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime) {
	for (Joint& joint : skeleton.joints) {
		//jointにアニメーションがある場合
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.translate = CalculateValue(rootNodeAnimation.translate, animationTime);
			joint.transform.rotate = CalculateValueQuaternion(rootNodeAnimation.rotate, animationTime);
			joint.transform.scale = CalculateValue(rootNodeAnimation.scale, animationTime);

		}
	}
}


void Object3d::SkeletonUpdate(Skeleton& skeleton) {
	int i = 0;//一から順番に
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;//Jointに親がいるとき(子)
			debugSphere[i]->Update(joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix);
		}
		else {
			joint.skeletonSpaceMatrix = joint.localMatrix;//jointに親がいない場合(親)
			debugSphere[i]->Update(joint.localMatrix);
		}
		i++;
	}
}

void Object3d::SkinClusterUpdate(SkinCluster& skinCluster, const Skeleton& skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex) {
		assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix =
			skinCluster.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			Transpose(Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
	}
}

void Object3d::SetWireframe() {
	SphereModel* sphere = new SphereModel();
	sphere->Initialize();

	debugSphere.push_back(sphere);
}

void Object3d::ChangeAnimation(const std::string& filePath) {
	
	if (model == ModelManager::GetInstance()->FindModel(filePath)) {
		return;
	}

	//変わる前のアニメーションデータ
	model2 = model;
	modelData2 = modelData;
	animation2 = animation;
	skeleton2 = skeleton;
	skinCluster2 = skinCluster;

	//変更するアニメーションデータ
	model = ModelManager::GetInstance()->FindModel(filePath);
	modelData = model->GetModelData();
	animation = model->GetAnimationData();
	skeleton = model->GetSkeleton();
	skinCluster = model->GetSkinCluster();
	
	SkeletonUpdate(skeleton);
	SkinClusterUpdate(skinCluster, skeleton);
	
	isChange = true;

}

void Object3d::Interpolation(Skeleton& skeleton, const Animation& animation, const Animation& nextAnimation, float animationTime) {
	for (Joint& joint : skeleton.joints) {
		//jointにアニメーションがある場合
		if (auto itA = animation.nodeAnimations.find(joint.name); itA != animation.nodeAnimations.end()) {
			if (auto itB = nextAnimation.nodeAnimations.find(joint.name); itB != nextAnimation.nodeAnimations.end()) {
				const NodeAnimation& rootNodeAnimation = (*itA).second;
				const NodeAnimation& nextRootNodeAnimation = (*itB).second;
				joint.transform.translate = InterpolationValue(rootNodeAnimation.translate,nextRootNodeAnimation.translate, animationTime);
				joint.transform.rotate = InterpolationValueQuaternion(rootNodeAnimation.rotate, nextRootNodeAnimation.rotate, animationTime);
				joint.transform.scale = InterpolationValue(rootNodeAnimation.scale, nextRootNodeAnimation.scale, animationTime);

			}
		}
	}
}