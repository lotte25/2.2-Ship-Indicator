#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

using namespace geode::prelude;

// I wanted to find out how to get the keys myself, but ery did it in his EvenMoreLevelTags mod,
// so I just used that. Thanks ery.
bool isFixEnabled(const GJGameLevel* level) {
    if (level->m_levelString.empty()) return false;
    const std::string levelString = ZipUtils::decompressString(level->m_levelString, true, 0);
    if (levelString.empty()) return false;
    
    const int bgKeyIndex = static_cast<int>(levelString.find("kA6,"));
    if (bgKeyIndex == std::string::npos) return false;
    const int semicolonIndex = static_cast<int>(levelString.find_first_of(";", bgKeyIndex));
    if (semicolonIndex == std::string::npos) return false;
    const std::string& keys = levelString.substr(bgKeyIndex, std::abs(bgKeyIndex - semicolonIndex));
    if (keys.empty()) return false;

    const bool fixgravitybug = geode::utils::string::contains(keys, "kA32,1"); 
    if (fixgravitybug) Notification::create(fmt::format("{} has fix", level->m_levelName), NotificationIcon::Success)->show();
    return fixgravitybug;
}


class $modify(LevelInfoLayer) {
    void addIndicator() {
        auto creatorInfoMenu = this->getChildByID("creator-info-menu");
        if (!creatorInfoMenu) return;

        // again, thanks ery 🙏
        const float copyIndicatorOffset = getChildByID("copy-indicator") ? 18.f : 0.f;
		const float highObjIndictorOffset = getChildByID("high-object-indicator") ? 18.f : 0.f;
		const float creatorInfoMenuHalfWidth = creatorInfoMenu->getScaledContentSize().width / 2.f;

        const float xPos = creatorInfoMenuHalfWidth + 9.f + copyIndicatorOffset + highObjIndictorOffset;

        auto sprite = CCSprite::createWithSpriteFrameName("portal_04_extra_2_001.png");
        auto indicator = CircleButtonSprite::create(
            sprite, 
            CircleBaseColor::Red, 
            CircleBaseSize::Large
        );
        indicator->setID("indicator"_spr);
        indicator->setScale(0.15f);
        indicator->setPosition(creatorInfoMenu->getPosition() + ccp(xPos, 8.6f));

        this->addChild(indicator);
    }

    bool init(GJGameLevel* level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;
        if (isFixEnabled(level)) {
            this->addIndicator();
        };

        return true;
    }

    void levelDownloadFinished(GJGameLevel* level) {    
        LevelInfoLayer::levelDownloadFinished(level);
        if (isFixEnabled(level)) {
            this->addIndicator();
        };
    }
};

// I tried to come up with a good name but you can see the result... pls someone suggest something better 
std::map<std::string, bool> levelsThatHaveTheFix;

class $modify(LevelBrowserLayer) {
    void loadLevelsFinished(CCArray* levels, char const* key, int type) {
        LevelBrowserLayer::loadLevelsFinished(levels, key, type);
        if (m_searchObject->m_searchType == SearchType::MyLevels || !levels || levels->count() == 0) return;

        for (auto level : CCArrayExt<GJGameLevel*>(levels)) {
            if (!geode::cast::typeinfo_cast<GJGameLevel*>(level)) return;
            
            // bool isLevelStringEmpty = level->m_levelString.empty();
            // geode::log::info("{}, is level string empty? {}", level->m_levelName, isLevelStringEmpty ? "yes" : "no");

            bool hasFixEnabled = false;
            if (m_searchObject->m_searchType != SearchType::SavedLevels) {
                GJGameLevel* savedLevel = GameLevelManager::get()->getSavedLevel(level->m_levelID);
                if (savedLevel == nullptr || savedLevel->m_levelString.empty()) continue;
                hasFixEnabled = isFixEnabled(savedLevel);
            } else {
                hasFixEnabled = isFixEnabled(level);
            }

            if (hasFixEnabled) {
                geode::log::info("{} ({}) has fix enabled", level->m_levelName, level->m_levelID);
                levelsThatHaveTheFix.insert({ std::to_string(level->m_levelID), hasFixEnabled });
            }
        }
    }
};