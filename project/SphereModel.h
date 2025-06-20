#pragma once
#include "MyMath.h"
#include "DirectXCommon.h"
#include "Camera.h"
#include "DebugWireframes.h"

class SphereModel {
public:
	void Initialize(Camera* camera);
	void Update(Vector3 scale, Quaternion rotate, Vector3 translate);
	void Update(Vector3 scale, Quaternion rotate, Vector3 translate, Matrix4x4 parent);

	void Draw();


	struct Sphere {
		Vector3 center;
		float radius;
	};


	VertexData AddVert(const VertexData& v1, const VertexData& v2) {
		VertexData result{};

		result.position.x = v1.position.x + v2.position.x;
		result.position.y = v1.position.y + v2.position.y;
		result.position.z = v1.position.z + v2.position.z;
		result.position.s = v1.position.s + v2.position.s;
		result.texcoord.x = v1.texcoord.x + v2.texcoord.x;
		result.texcoord.y = v1.texcoord.y + v2.texcoord.y;
		return result;
	}

	void DrawSphere();
private:

	DebugWireframes* debugWireframes = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	TransformationMatrix* wvpData = nullptr;


	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	CameraForGPU* cameraData = nullptr;
	Camera* camera = nullptr;


	ModelData modelData;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;


	VertexData* vertexData = nullptr;
	Material* materialData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
};