#include "TestClass.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Input.h"

#include "Object3dCommon.h"
#include "ParticleCommon.h"

using namespace MyMath;

TestClass::TestClass(){}

TestClass::~TestClass() {
	delete object_;
	delete object2_;
	delete camera;

}

void TestClass::Init() {
	camera = new Camera();
	//Vector3 cameraRotate = { 1.4f,0.0f,0.0f };
	//Vector3 cameraTranslate = { 0.0f,30.0f,-8.0f };
	//cameraRotate = { 0.3f,0.0f,0.0f };
	cameraTranslate = { 0.0f,1.0f,-11.0f };

	camera->SetRotate(cameraRotate);
	camera->SetTranslate(cameraTranslate);

	Object3dCommon::GetInstance()->SetDefaultCamera(camera);
	ParticleCommon::GetInstance()->SetDefaultCamera(camera);
	DebugWireframes::GetInstance()->SetDefaultCamera(camera);

	worldTransform_.Initialize();
	worldTransform2_.Initialize();

	object_ = new Object3d();
	object_->Initialize();
	object_->SetModelFile("stop.gltf");
	
	object2_ = new Object3d();
	object2_->Initialize();
	object2_->SetModelFile("sneakWalk.gltf");

	worldTransform_.translation_.y = 0.0f;
	worldTransform2_.translation_.x = 3.0f;
	//worldTransform_.rotation_.x = -1.277f;

	input = Input::GetInstance();

	//worldTransformを親クラスに
	camera->SetParent(&worldTransform_);
}

void TestClass::Update() {
	
	input->GetJoyStickState(0, state);
	input->GetJoystickStatePrevious(0, preState);

	onLight = true;

	float x;
	float z;

	const float deadZone = 0.2f;
	if (input->GetJoyStickState(0, state)) {
		x = static_cast<float>(state.Gamepad.sThumbLX) / 32768.0f;
		z = static_cast<float>(state.Gamepad.sThumbLY) / 32768.0f;

		if (fabs(x) < deadZone)
			x = 0.0f;
		if (fabs(z) < deadZone)
			z = 0.0f;
		worldTransform_.translation_.x += x / 10;
		worldTransform_.translation_.z += z / 10;
	}

	//まだ
	// GamePad右スティックによるカメラ回転処理
	//float xCamera = 0.0f, zCamera = 0.0f;
	//if (input->GetJoyStickState(0, state)) {

	//	// 右スティックの入力
	//	xCamera = static_cast<float>(state.Gamepad.sThumbRX) / 32768.0f; // -1.0f～1.0f
	//	zCamera = static_cast<float>(state.Gamepad.sThumbRY) / 32768.0f; // -1.0f～1.0f

	//	// デッドゾーン処理
	//	if (abs(xCamera) < deadZone) {
	//		xCamera = 0.0f;
	//		if (fabs(zCamera) < deadZone)
	//		zCamera = 0.0f;
	//	}

	//	// カメラ向き
	//	// Y軸
	//	cameraYaw += xCamera * 2.5f;
	//	// X軸（上下反転）
	//	cameraPitch -= zCamera * 2.5f;  // += から -= に変更して反転
	//}

	if ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !(preState.Gamepad.wButtons & XINPUT_GAMEPAD_A) && !isJump) {
		isJump = true;
		velocity.y += 0.5f;
	}
	
	if (input->PushKey(DIK_K) && !isJump) {
		isJump = true;
		velocity.y += 0.5f;
	}

	if (input->PushKey(DIK_A)) {
		isWalk = true;
	}
	else {
		isWalk = false;
	}

	if (isJump) {
		float grabity = -0.02f;
		velocity.y += grabity;
	}
	worldTransform_.translation_.y += velocity.y;

	if (worldTransform_.translation_.y <= 0.0f) {
		worldTransform_.translation_.y = 0.0f;
		velocity.y = 0.0f;
		isJump = false;
	}


	if (isJump) {
		mosion = jump;
	}
	else if(isWalk){
		mosion = walk;
	}
	else {
		mosion = stop;
	}

	//現在のモーションと前回のモーションが違う場合
	if (mosion != preMosion) {
		isChangeMosion = true;
	}

	if (isChangeMosion) {
		switch (mosion)
		{
		case TestClass::stop:
			object_->SetModelFile("stop.gltf");
			isChangeMosion = false;
			break;
		case TestClass::walk:
			object_->SetModelFile("walk.gltf");
			isChangeMosion = false;
			break;
		case TestClass::jump:
			object_->SetModelFile("sneakWalk.gltf");
			isChangeMosion = false;
			break;
		}
	}

	//前回のモーション
	preMosion = mosion;


#ifdef _DEBUG

	ImGui::Begin("TestModel");

	ImGui::InputFloat3("VertexModel", &worldTransform_.translation_.x);
	ImGui::SliderFloat3("SliderVertexModel", &worldTransform_.translation_.x, -5.0f, 5.0f);

	ImGui::InputFloat3("RotateModel", &worldTransform_.rotation_.x);
	ImGui::SliderFloat3("SliderRotateModel", &worldTransform_.rotation_.x, -10.0f, 10.0f);

	ImGui::InputFloat3("ScaleModel", &worldTransform_.scale_.x);
	ImGui::SliderFloat3("SliderScaleModel", &worldTransform_.scale_.x, 0.5f, 5.0f);

	ImGui::End();


	ImGui::Begin("TestModel2");

	ImGui::InputFloat3("VertexModel", &worldTransform2_.translation_.x);
	ImGui::SliderFloat3("SliderVertexModel", &worldTransform2_.translation_.x, -5.0f, 5.0f);

	ImGui::InputFloat3("RotateModel", &worldTransform2_.rotation_.x);
	ImGui::SliderFloat3("SliderRotateModel", &worldTransform2_.rotation_.x, -10.0f, 10.0f);

	ImGui::InputFloat3("ScaleModel", &worldTransform2_.scale_.x);
	ImGui::SliderFloat3("SliderScaleModel", &worldTransform2_.scale_.x, 0.5f, 5.0f);

	ImGui::End();


	ImGui::Begin("camera");

	//カメラ
	ImGui::SliderFloat3("cameraTranslate", &cameraTranslate.x, -30.0f, 30.0f);

	ImGui::SliderFloat("cameraRotateX", &cameraRotate.x, -10.0f, 10.0f);
	ImGui::SliderFloat("cameraRotateY", &cameraRotate.y, -10.0f, 10.0f);
	ImGui::SliderFloat("cameraRotateZ", &cameraRotate.z, -10.0f, 10.0f);
	camera->SetRotate(cameraRotate);
	camera->SetTranslate(cameraTranslate);

	ImGui::End();
#endif // _DEBUG

	object_->LightSwitch(onLight);

	worldTransform_.UpdateMatrix();
	worldTransform2_.UpdateMatrix();

	camera->Update();

	object_->Update(worldTransform_);
	object2_->Update(worldTransform2_);
}


void TestClass::Draw() {
	object_->Draw();
	object2_->Draw();
}
