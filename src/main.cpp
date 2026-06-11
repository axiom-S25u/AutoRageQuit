#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PlayLayer) {
	struct Fields {
		float lastPercent = -1.f;
		int sameSpotCount = 0;
		int lastAttempts = -1;
	};

	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

		// reset tracking when entering a level
		m_fields->lastPercent = -1.f;
		m_fields->sameSpotCount = 0;
		m_fields->lastAttempts = m_attempts;

		return true;
	}

	void destroyPlayer(PlayerObject* player, GameObject* object) {
		PlayLayer::destroyPlayer(player, object);

		if (m_isPracticeMode) return;

		if (m_isPlatformer) return;

		bool enabled = Mod::get()->getSettingValue<bool>("enabled");
		if (!enabled) return;

		// only proceed if the games attempt counter actually went up
		if (m_attempts <= m_fields->lastAttempts) return;
		m_fields->lastAttempts = m_attempts;

		int maxAttempts = Mod::get()->getSettingValue<int64_t>("attempts-before-quit"); // again, min 2 so it wont just kill it for no reason

		// truncate to 1 decimal so 3.43 becomes 3.4
		float percent = std::floor(getCurrentPercent() * 10.f) / 10.f; // i mean on long ass levels, this is somethin else but i mean, idk what to say no one with the mod on would play anyways

		// check if we died on the same % as last time
		if (std::fabs(percent - m_fields->lastPercent) < 0.001f) {
			m_fields->sameSpotCount++;
		} else {
			m_fields->sameSpotCount = 1;
			m_fields->lastPercent = percent;
		}

		// our own counter, not the games attempts
		if (m_fields->sameSpotCount >= maxAttempts) {
			log::warn("died on same % {} times in a row, giving up", m_fields->sameSpotCount);

			auto playLayer = PlayLayer::get();
			if (playLayer) {
				playLayer->onQuit();
			}
		}
	}
};