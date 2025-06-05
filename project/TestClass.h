#pragma once
#include "WorldTransform.h"
#include "Object3d.h"
#include "MyMath.h"


class TestClass {
public:

	TestClass();
	~TestClass();

	void Init();
	void Update();
	void Draw();

private:
	WorldTransform worldTransform_;
	WorldTransform worldTransform2_;
	Object3d* object_ = nullptr;
	Object3d* object2_ = nullptr;
	Camera* camera_ = nullptr;
	bool onLight = false;
};
