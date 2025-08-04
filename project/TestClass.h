#pragma once
#include "WorldTransform.h"
#include "Object_glTF.h"
#include "MyMath.h"

#include "Input.h"

class TestClass {
public:

	TestClass();
	~TestClass();

	void Init();
	void Update();
	void Draw();

	enum Mosion {
		stop,
		walk,
		jump
	};

private:
	WorldTransform worldTransform_;
	WorldTransform worldTransformCamera_;
	Object_glTF* object_ = nullptr;

	WorldTransform worldTransform2_;
	Object_glTF* object2_ = nullptr;

	Camera* camera = nullptr;
	Vector3 cameraRotate = { 0.0f,0.0f,0.0f };
	Vector3 cameraTranslate = { 0.0f,0.0f,-15.0f };
	float cameraYaw = 0.0f;
	float cameraPitch = 0.0f;

	bool onLight = false;

	Input* input = nullptr;
	XINPUT_STATE state;
	XINPUT_STATE preState;

	bool isJump = false;
	Vector3 velocity;

	bool isWalk = false;
	float preX;
	float preZ;

	bool isChangeMosion = false;

	Mosion mosion = Mosion::stop;
	Mosion preMosion = Mosion::stop;

};
