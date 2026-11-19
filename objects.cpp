// This file makes SPIKES and BLOCKS bigger or smaller.
// It is separate on purpose: if it ever gives a build error,
// you can delete this one file and the PLAYER part still works.

#include <Geode/Geode.hpp>
#include <Geode/modify/GameObject.hpp>
#include "shared.hpp"

using namespace geode::prelude;

// The player is also a "GameObject", so we skip it here.
// (The player has its own multiplier in main.cpp.)
static bool isPlayer(GameObject* obj) {
	auto layer = GJBaseGameLayer::get();
	return layer && (obj == layer->m_player1 || obj == layer->m_player2);
}

class $modify(HitboxGameObject, GameObject) {
	cocos2d::CCRect const& getObjectRect() {
		auto const& original = GameObject::getObjectRect();

		// what kind of object is this?
		int slot;
		switch (m_objectType) {
			case GameObjectType::Hazard:
			case GameObjectType::AnimatedHazard:
				slot = TargetSpikes;
				break;
			case GameObjectType::Solid:
				slot = TargetBlocks;
				break;
			default:
				return original; // everything else stays normal
		}

		auto const& m = g_mult[slot];
		if (m.w == 1.0f && m.h == 1.0f) return original;
		if (isPlayer(this)) return original;

		// Make a resized copy around the same middle point.
		// (A small ring of rectangles, so a rect we handed out a moment
		// ago is not overwritten right away.)
		static cocos2d::CCRect ring[256];
		static size_t next = 0;
		auto& out = ring[next++ % 256];

		float const w = original.size.width * m.w;
		float const h = original.size.height * m.h;
		out = cocos2d::CCRect(original.getMidX() - w / 2.f, original.getMidY() - h / 2.f, w, h);
		return out;
	}
};
