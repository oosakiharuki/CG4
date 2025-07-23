#include "GameScene.h"
using namespace MyMath;

void GameScene::Initialize() {


	//ModelManager::GetInstance()->LoadModel("plane.gltf");
	//ModelManager::GetInstance()->LoadModel("plane.obj");
	//ModelManager::GetInstance()->LoadModel("grass01");
	//ModelManager::GetInstance()->LoadModel("TwoSidedPlane.gltf");
	//ModelManager::GetInstance()->LoadModel("AnimatedCube.gltf");
	//ModelManager::GetInstance()->LoadModel("MyBoxRotate.gltf");
	ModelManager::GetInstance()->LoadModel("simpleSkin.gltf");
	ModelManager::GetInstance()->LoadModel("sneakWalk.gltf");
	ModelManager::GetInstance()->LoadModel("walk.gltf");
	ModelManager::GetInstance()->LoadModel("stop.gltf");


	//camera = new Camera();
	////Vector3 cameraRotate = { 1.4f,0.0f,0.0f };
	////Vector3 cameraTranslate = { 0.0f,30.0f,-8.0f };
	////cameraRotate = { 0.3f,0.0f,0.0f };
	//cameraTranslate = { 0.0f,1.0f,-11.0f };

	//camera->SetRotate(cameraRotate);
	//camera->SetTranslate(cameraTranslate);
	//
	//Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	//ParticleCommon::GetInstance()->SetDefaultCamera(camera);

 	testClass = new TestClass();
 	testClass->Init();



	ParticleManager::GetInstance()->CreateParticleGroup("plane", "resource/Sprite/gradationLine.png");

	particle = new Particle();
	particle->Initialize(ParticleCommon::GetInstance(),"plane");

	//TextureManager::GetInstance()->LoadTexture("resource/rostock_laage_airport_4k.dds");
	skyBox = new BoxModel();
	skyBox->Initialize("resource/rostock_laage_airport_4k.dds");

	worldTransform.Initialize();
}

void GameScene::Update() {

	if (Input::GetInstance()->PushKey(DIK_0)) {
		OutputDebugStringA("Hit 0\n");
	}

	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		OutputDebugStringA("Hit 1\n");
	}

	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		//sceneNo = Title;//シーン切り替えはしない
	}

	testClass->Update();

	particle->Update();
	
	skyBox->Update(worldTransform.matWorld_ * MakeScaleMatrix({ 1000,1000,1000 }));//大きくするため

#ifdef  USE_IMGUI

	//ここにテキストを入れられる

	//開発用UIの処理
	//ImGui::ShowDemoWindow();

	//ImGui::Begin("camera");
	//ImGui::Text("ImGuiText");

	////カメラ
	//ImGui::SliderFloat3("cameraTranslate", &cameraTranslate.x, -30.0f, 30.0f);

	//ImGui::SliderFloat("cameraRotateX", &cameraRotate.x, -10.0f, 10.0f);
	//ImGui::SliderFloat("cameraRotateY", &cameraRotate.y, -10.0f, 10.0f);
	//ImGui::SliderFloat("cameraRotateZ", &cameraRotate.z, -10.0f, 10.0f);
	//camera->SetRotate(cameraRotate);
	//camera->SetTranslate(cameraTranslate);

	//ImGui::End();
#endif //  USE_IMGUI
}

void GameScene::Draw() {

	Cubemap::GetInstance()->Command();
	skyBox->Draw();

	//スプライト描画処理(背景用)
	SpriteCommon::GetInstance()->Command();


	//モデル描画処理
	Object3dCommon::GetInstance()->Command();

	testClass->Draw();

	//パーティクル描画処理
	ParticleCommon::GetInstance()->Command();

	//particle->Draw();

	//スプライト描画処理(UI用)
	SpriteCommon::GetInstance()->Command();

}

void GameScene::Finalize() {	
	delete testClass;
	delete particle;
	delete skyBox;
}
