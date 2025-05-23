#pragma once

#include"Vector2.h"
#include"Vector3.h"
#include"Vector4.h"
#include"Matrix4x4.h"
#include"Transform.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <string>
#include <map>
	
//model
struct MaterialData {
	std::string textureFilePath;
	uint32_t textureIndex;
};

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
	Node rootNode;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float shininess;
};

struct Quaternion {
	float x;
	float y;
	float z;
	float w;
};

template<typename tValue>
struct keyframe {
	tValue value; //キーフレームの値(Vector3/Quatanrion)
	float time;   //時刻(秒)
};

using keyframeVector3 = keyframe<Vector3>;      //トランスフォームとスケーリング用
using keyframeQuatarnion = keyframe<Quaternion>;//回転用

template<typename tValue>
struct AnimationCurve {
	std::vector<keyframe<tValue>> keyframes;
};

struct NodeAnimation {
	AnimationCurve<Vector3> translate;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animation {
	float duration; //アニメーション全体の尺
	std::map<std::string, NodeAnimation> nodeAnimations;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct CameraForGPU {
	Vector3 worldPosition;
};

struct PointLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	float radius;
	float decay;
	float padding[2];
};

struct SpotLight {
	Vector4 color;
	Vector3 position;
	float intensity;
	Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float padding[2];
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct AABB {
	Vector3 min;
	Vector3 max;
};

namespace MyMath {

	Vector3 operator+(const Vector3& v1, const Vector3& v2);
	Vector3 operator-(const Vector3& v1, const Vector3& v2);
	Vector3 operator*(const Vector3& v1, const Vector3& v2);
	Vector3 operator/(const Vector3& v1, const Vector3& v2);

	Vector3 operator*(const Vector3& v, const float f);
	Vector3 operator*(const float f, const Vector3& v);

	Vector3& operator+=(Vector3& v1, const Vector3& v2);
	Vector3& operator-=(Vector3& v1, const Vector3& v2);
	Vector3& operator*=(Vector3& v1, const Vector3& v2);
	Vector3& operator/=(Vector3& v1, const Vector3& v2);

	Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

	Matrix4x4 MakeIdentity4x4();
	Matrix4x4 MakeScaleMatrix(Vector3 scale);
	Matrix4x4 MakeRotateXMatrix(float radian);
	Matrix4x4 MakeRotateYMatrix(float radian);
	Matrix4x4 MakeRotateZMatrix(float radian);
	Matrix4x4 MakeTranslateMatrix(Vector3 translate);


	Vector3 Normalize(const Vector3& v);

#pragma region Affine

	Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
#pragma endregion

#pragma region 逆数
	Matrix4x4 Inverse(const Matrix4x4& m);
#pragma endregion

	Matrix4x4 MakePerspectiveFovMatrix(float forY, float aspectRatio, float nearClip, float farClip);

	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);


	Vector3 CalculateValue(const AnimationCurve<Vector3>& keyframes, float time);
	Vector3 CalculateValue(const AnimationCurve<Quaternion>& keyframes, float time);

	Vector3 Lerp(const Vector3& p0, const Vector3& p1, float t);
}
