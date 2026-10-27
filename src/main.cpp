#include <Geode/Geode.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

using namespace geode::prelude;

// long ass name, sorry
// I wanted to find out how to get the keys myself, but ery did it in his EvenMoreLevelTags mod,
// so I just used that. Thanks ery.
bool isGravityBugFixEnabled(const GJGameLevel* level) {
    const std::string levelString = ZipUtils::decompressString(level->m_levelString, true, 0);
    if (levelString.empty()) return false;
    
    const int bgKeyIndex = static_cast<int>(levelString.find("kA6,"));
    if (bgKeyIndex == std::string::npos) return false;
    const int semicolonIndex = static_cast<int>(levelString.find_first_of(";", bgKeyIndex));
    if (semicolonIndex == std::string::npos) return false;
    const std::string& keys = levelString.substr(bgKeyIndex, std::abs(bgKeyIndex - semicolonIndex));
    if (keys.empty()) return false;

    geode::utils::clipboard::write(keys);
    const bool fixgravitybug = geode::utils::string::contains(keys, "kA32,1"); 
    Notification::create("copied", fixgravitybug ? NotificationIcon::Success : NotificationIcon::Error)->show();
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
        if (isGravityBugFixEnabled(level)) {
            this->addIndicator();
        };

        return true;
    }

    void levelDownloadFinished(GJGameLevel* level) {    
        LevelInfoLayer::levelDownloadFinished(level);
        if (isGravityBugFixEnabled(level)) {
            this->addIndicator();
        };
    }
};