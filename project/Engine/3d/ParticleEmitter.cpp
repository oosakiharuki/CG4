#include "ParticleEmitter.h"
#include <numbers>

using namespace MyMath;

//水面に出る波紋のようなパーティクル
Particles ParticleEmitter::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {
	//random
	std::uniform_real_distribution<float> distribution(-3.0f, 3.0f);//position用
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);//color用
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);


	std::uniform_real_distribution<float> distRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	std::uniform_real_distribution<float> distScale(0.4f, 1.0f);

	Particles particle;
	particle.transform.scale = { 0.5f,0.5f,0.5f };
	particle.transform.rotate = { distRotate(randomEngine),distRotate(randomEngine),distRotate(randomEngine) };

	particle.transform.translate = translate;

	particle.velocity = { distScale(randomEngine),distScale(randomEngine),0 };
	particle.color = { 1.0f,1.0f,1.0f,1.0f };

	particle.lifeTime = 1.0f;
	particle.currentTime = 0;

	return particle;
}

std::list<Particles> ParticleEmitter::MakeEmit(const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<Particles> particles;
	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back(MakeNewParticle(randomEngine, emitter.transform.translate));
	}
	return particles;
}
