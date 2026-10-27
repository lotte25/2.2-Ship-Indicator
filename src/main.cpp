#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelCell.hpp>

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

CircleButtonSprite* createIndicator() {
    auto sprite = CCSprite::createWithSpriteFrameName("portal_04_extra_2_001.png");
    auto indicator = CircleButtonSprite::create(
        sprite, 
        CircleBaseColor::Red, 
        CircleBaseSize::Large
    );
    indicator->setID("indicator"_spr);

    return indicator;
}

class $modify(LevelInfoLayer) {
    void addIndicator() {
        auto creatorInfoMenu = this->getChildByID("creator-info-menu");
        if (!creatorInfoMenu) return;

        // again, thanks ery 🙏
        const float copyIndicatorOffset = getChildByID("copy-indicator") ? 18.f : 0.f;
		const float highObjIndicatorOffset = getChildByID("high-object-indicator") ? 18.f : 0.f;
		const float creatorInfoMenuHalfWidth = creatorInfoMenu->getScaledContentSize().width / 2.f;

        const float xPos = creatorInfoMenuHalfWidth + 6.f + copyIndicatorOffset + highObjIndicatorOffset;

        auto indicator = createIndicator();
        indicator->setAnchorPoint({ 0.f, 0.5f });
        indicator->setPosition(creatorInfoMenu->getPosition() + ccp(xPos, 8.6f));
        indicator->setScale(0.15f);

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
std::unordered_set<std::string> levelsThatHaveTheFix;

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
                levelsThatHaveTheFix.insert(std::to_string(level->m_levelID));
            }
        }
    }
};

class $modify(LevelCell) {
    void loadCustomLevelCell() {
        LevelCell::loadCustomLevelCell();
        if (!levelsThatHaveTheFix.contains(std::to_string(m_level->m_levelID))) return;

        auto creatorName = m_mainMenu->getChildByID("creator-name");
        if (creatorName == nullptr) return;

        auto copyIndicator = m_mainLayer->getChildByID("copy-indicator");
        auto highObjIndicator = m_mainLayer->getChildByID("high-object-indicator");

        
        auto indicator = createIndicator();
        indicator->setAnchorPoint({ 0.5f, 0.5f });

        // https://github.com/Zilko/Level-Showcases/blob/main/src/main.cpp
        float scale = 1.f;
        CCPoint pos = { 0, 0 };

        auto maxRight = highObjIndicator ? highObjIndicator : copyIndicator;
        
        if (copyIndicator || highObjIndicator) {
            if (copyIndicator)
                if (copyIndicator->getPositionX() > maxRight->getPositionX())
                    maxRight = copyIndicator;

            scale = maxRight->getContentWidth() * maxRight->getScale() / indicator->getContentWidth();
            pos = maxRight->getPosition() + ccp(15, 0);
        } else {
            scale = getContentHeight() < 80 ? 0.8f : 1.f;
            pos = m_mainMenu->getPosition() + creatorName->getPosition();
            pos += ccp(creatorName->getContentWidth() / 2.f + 5, -1);
        }

        indicator->setPosition(pos);
        indicator->setScale(scale);

        m_mainLayer->addChild(indicator);
    }
};