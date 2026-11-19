#pragma once
#include <array>

// One width number and one height number.
// 1.0 = normal, 2.0 = twice as big, 0.5 = half as big.
struct HitboxMult {
	float w = 1.0f;
	float h = 1.0f;
};

// The three things you can change.
enum Target {
	TargetPlayer = 0,
	TargetSpikes = 1,
	TargetBlocks = 2,
};

constexpr float MULT_MIN = 0.1f;
constexpr float MULT_MAX = 10.0f;

inline std::array<HitboxMult, 3> g_mult; // one per Target
inline int g_target = 0;                 // which tab the menu is showing
