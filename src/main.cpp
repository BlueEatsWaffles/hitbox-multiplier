#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "shared.hpp"

using namespace geode::prelude;

static constexpr char const* kNames[3] = {"player", "spikes", "blocks"};
static constexpr char const* kTitles[3] = {"Player", "Spikes", "Blocks"};

// ---------------------------------------------------------------
//  Saving and loading (so the game remembers your numbers)
// ---------------------------------------------------------------
static void saveAll() {
	for (int i = 0; i < 3; i++) {
		Mod::get()->setSavedValue<double>(fmt::format("{}-w", kNames[i]), g_mult[i].w);
		Mod::get()->setSavedValue<double>(fmt::format("{}-h", kNames[i]), g_mult[i].h);
	}
}

static void loadAll() {
	for (int i = 0; i < 3; i++) {
		g_mult[i].w = static_cast<float>(Mod::get()->getSavedValue<double>(fmt::format("{}-w", kNames[i]), 1.0));
		g_mult[i].h = static_cast<float>(Mod::get()->getSavedValue<double>(fmt::format("{}-h", kNames[i]), 1.0));
	}
}

// The slider goes from 0 to 1. We turn that into 0.1x ... 10x.
// The middle of the slider is exactly 1.0x, so small changes are easy.
static float fromSlider(float s) {
	float v = std::pow(10.f, -1.f + 2.f * s);
	return std::clamp(std::round(v * 100.f) / 100.f, MULT_MIN, MULT_MAX);
}

static float toSlider(float v) {
	return std::clamp((std::log10(std::max(v, MULT_MIN)) + 1.f) / 2.f, 0.f, 1.f);
}

// ---------------------------------------------------------------
//  The menu
// ---------------------------------------------------------------
class HitboxPopup : public geode::Popup {
protected:
	CCLabelBMFont* m_targetLabel = nullptr;
	Slider* m_wSlider = nullptr;
	Slider* m_hSlider = nullptr;
	TextInput* m_wInput = nullptr;
	TextInput* m_hInput = nullptr;

	bool initPopup() {
		if (!Popup::init(340.f, 240.f)) return false;

		this->setTitle("Hitbox Multiplier");

		// three tabs at the top: Player / Spikes / Blocks
		this->addTab(TargetPlayer, -105.f);
		this->addTab(TargetSpikes, 0.f);
		this->addTab(TargetBlocks, 105.f);

		// which tab are we on?
		m_targetLabel = CCLabelBMFont::create("", "bigFont.fnt");
		m_targetLabel->setScale(0.5f);
		m_mainLayer->addChildAtPosition(m_targetLabel, Anchor::Center, {0.f, 42.f});

		// width and height rows (slider + number box)
		this->addRow("Width", 12.f, true);
		this->addRow("Height", -28.f, false);

		// reset button
		auto resetBtn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("Reset"), this, menu_selector(HitboxPopup::onReset)
		);
		m_buttonMenu->addChildAtPosition(resetBtn, Anchor::Center, {0.f, -75.f});

		this->refreshAll();
		s_instance = this;
		return true;
	}

	void addTab(int index, float x) {
		auto btn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create(kTitles[index]), this, menu_selector(HitboxPopup::onTab)
		);
		btn->setTag(index);
		m_buttonMenu->addChildAtPosition(btn, Anchor::Center, {x, 78.f});
	}

	void addRow(char const* name, float y, bool isWidth) {
		auto label = CCLabelBMFont::create(name, "goldFont.fnt");
		label->setScale(0.55f);
		m_mainLayer->addChildAtPosition(label, Anchor::Center, {-128.f, y});

		auto slider = Slider::create(
			this,
			isWidth ? menu_selector(HitboxPopup::onWidthSlider) : menu_selector(HitboxPopup::onHeightSlider),
			0.5f
		);
		m_mainLayer->addChildAtPosition(slider, Anchor::Center, {-30.f, y});

		auto input = TextInput::create(80.f, "1.00", "bigFont.fnt");
		input->setFilter("0123456789.");
		input->setMaxCharCount(6);
		input->setCallback([this, isWidth](std::string const& text) {
			this->onTyped(text, isWidth);
		});
		m_mainLayer->addChildAtPosition(input, Anchor::Center, {110.f, y});

		(isWidth ? m_wSlider : m_hSlider) = slider;
		(isWidth ? m_wInput : m_hInput) = input;
	}

	// put the right numbers on the screen for the tab we are on
	void refreshAll() {
		auto& m = g_mult[g_target];
		m_targetLabel->setString(fmt::format("{} hitbox", kTitles[g_target]).c_str());
		m_wSlider->setValue(toSlider(m.w));
		m_hSlider->setValue(toSlider(m.h));
		m_wInput->setString(fmt::format("{:.2f}", m.w), false);
		m_hInput->setString(fmt::format("{:.2f}", m.h), false);
	}

	// ---- things you can press ----
	void onTab(CCObject* sender) {
		g_target = static_cast<CCNode*>(sender)->getTag();
		this->refreshAll();
	}

	void onReset(CCObject*) {
		g_mult[g_target] = HitboxMult{};
		saveAll();
		this->refreshAll();
	}

	void onWidthSlider(CCObject* sender) {
		this->onSlid(static_cast<SliderThumb*>(sender)->getValue(), true);
	}

	void onHeightSlider(CCObject* sender) {
		this->onSlid(static_cast<SliderThumb*>(sender)->getValue(), false);
	}

	// the slider moved -> change the number and the number box
	void onSlid(float sliderValue, bool isWidth) {
		float v = fromSlider(sliderValue);
		(isWidth ? g_mult[g_target].w : g_mult[g_target].h) = v;
		saveAll();
		(isWidth ? m_wInput : m_hInput)->setString(fmt::format("{:.2f}", v), false);
	}

	// you typed a number -> change the number and move the slider
	void onTyped(std::string const& text, bool isWidth) {
		char* end = nullptr;
		float v = std::strtof(text.c_str(), &end);
		// still typing (empty, ".", "0")? just wait.
		if (text.empty() || end == text.c_str() || !(v > 0.f)) return;

		v = std::clamp(v, MULT_MIN, MULT_MAX);
		(isWidth ? g_mult[g_target].w : g_mult[g_target].h) = v;
		saveAll();
		(isWidth ? m_wSlider : m_hSlider)->setValue(toSlider(v));
	}

public:
	inline static HitboxPopup* s_instance = nullptr;

	// closes the popup (keyBackClicked is protected, so we call it from inside)
	void closeMe() {
		this->keyBackClicked();
	}

	~HitboxPopup() override {
		if (s_instance == this) s_instance = nullptr;
	}

	static HitboxPopup* create() {
		auto ret = new HitboxPopup();
		if (ret->initPopup()) {
			ret->autorelease();
			return ret;
		}
		delete ret;
		return nullptr;
	}
};

// Press Y once = open. Press Y again = close.
static void toggleMenu() {
	log::info("Hitbox Multiplier: toggling the menu");
	if (HitboxPopup::s_instance && HitboxPopup::s_instance->getParent()) {
		HitboxPopup::s_instance->closeMe();
		return;
	}
	if (auto popup = HitboxPopup::create()) {
		popup->show();
	}
}

// ---------------------------------------------------------------
//  Listen for the Y key (it is a setting, so you can change it
//  later in the mod's settings page)
// ---------------------------------------------------------------
$on_mod(Loaded) {
	loadAll();
	log::info("Hitbox Multiplier: loaded, waiting for the menu key");

	listenForKeybindSettingPresses(
		"open-menu-key",
		[](Keybind const&, bool down, bool repeat, double) {
			log::info("Hitbox Multiplier: menu key event (down={}, repeat={})", down, repeat);
			if (down && !repeat) toggleMenu();
		},
		Mod::get()
	);
}

// ---------------------------------------------------------------
//  The PLAYER hitbox change.
//  While the game checks "did the player touch something?", we make
//  the player wider/taller (or smaller). Right after, we put the
//  size back, so what you SEE does not change - only the hitbox.
// ---------------------------------------------------------------
class $modify(HitboxGameLayer, GJBaseGameLayer) {
	void checkCollisions(PlayerObject* player, float dt, bool p2) {
		auto const& m = g_mult[TargetPlayer];

		if (!player || (m.w == 1.0f && m.h == 1.0f)) {
			GJBaseGameLayer::checkCollisions(player, dt, p2);
			return;
		}

		float const oldX = player->getScaleX();
		float const oldY = player->getScaleY();
		float const bigX = oldX * m.w;
		float const bigY = oldY * m.h;

		player->setScaleX(bigX);
		player->setScaleY(bigY);

		GJBaseGameLayer::checkCollisions(player, dt, p2);

		// Only put the size back if the game didn't change it itself
		// (for example, a mini portal).
		if (player->getScaleX() == bigX && player->getScaleY() == bigY) {
			player->setScaleX(oldX);
			player->setScaleY(oldY);
		}
	}
};

// ---------------------------------------------------------------
//  A "Hitbox" button on the main menu. Click it to open the menu.
//  (Works even if the keyboard key does not.)
// ---------------------------------------------------------------
class $modify(HitboxMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		if (auto menu = this->getChildByID("bottom-menu")) {
			auto spr = ButtonSprite::create("Hitbox");
			spr->setScale(0.6f);
			auto btn = CCMenuItemSpriteExtra::create(
				spr, this, menu_selector(HitboxMenuLayer::onHitboxMenu)
			);
			btn->setID("hitbox-multiplier-button"_spr);
			menu->addChild(btn);
			menu->updateLayout();
		}
		return true;
	}

	void onHitboxMenu(CCObject*) {
		toggleMenu();
	}
};
