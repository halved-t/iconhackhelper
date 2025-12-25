#include <Geode/Geode.hpp>
#include <Geode/modify/GJGarageLayer.hpp>

using namespace geode::prelude;

bool isIconActuallyUnlocked(int iconID, IconType type) {
    // Exceptions (these aren't stored in the save file) (i think)
    if (iconID == 1 || (type == IconType::Cube && (iconID == 2 || iconID == 3 || iconID == 4))) 
        return true;
    
    auto gm = GameManager::sharedState();
    
    gd::string saveKey = gm->iconKey(iconID, type);
    
    auto valueKeeper = gm->m_valueKeeper;
    if (!valueKeeper) return false;
    
    bool valueObj = valueKeeper->valueForKey(saveKey)->boolValue();
    if (valueObj) return true;

    auto gsm = GameStatsManager::sharedState();
    UnlockType unlockType = gm->iconTypeToUnlockType(type);

    if (auto storeItem = gsm->getStoreItem(iconID, (int)unlockType)) {
        if (gsm->isStoreItemUnlocked(storeItem->m_index)) return true;
    }

    int chestID = gsm->getSecretChestForItem(iconID, unlockType);
    if (chestID > 0) {
        if (gsm->isSecretChestUnlocked(chestID)) return true;
    }

    if (auto chestKey = gsm->getSpecialChestKeyForItem(iconID, unlockType)) {
        if (gsm->isSpecialChestUnlocked(chestKey->getCString())) return true;
    }
    
    return false;
}

class $modify(IconGarageLayer, GJGarageLayer) {
    void setupPage(int page, IconType type) {
        GJGarageLayer::setupPage(page, type);
        
        this->applyIconEffects();
    }

    void applyIconEffects() {
        auto page = m_iconSelection->m_scrollLayer->m_extendedLayer->getChildByType<ListButtonPage>(0);
        if (!page) return;

        auto menu = page->getChildByType<CCMenu>(0);
        if (!menu) return;

        bool useOpacity = Mod::get()->getSettingValue<bool>("use-opacity-mode");
        auto children = menu->getChildren();
        if (!children) return;

        for (auto child : CCArrayExt<CCMenuItemSpriteExtra*>(children)) {
            this->applyIconEffect(child, useOpacity);
        }

        CCMenu* menu2 = nullptr;
        if (m_iconType == IconType::Special) {
            auto bar = m_iconSelection->getChildByType<ListButtonBar>(0);
            if (!bar) return;

            auto page2 = bar->m_scrollLayer->m_extendedLayer->getChildByType<ListButtonPage>(0);
            if (!page2) return;

            menu2 = page2->getChildByType<CCMenu>(0);
            if (!menu2) return;
        }

        if (menu2) {
            auto children2 = menu2->getChildren();
            if (!children2) return;

            for (auto child : CCArrayExt<CCMenuItemSpriteExtra*>(children2)) {
                this->applyIconEffect(child, useOpacity);
            }
        }
    }

    void applyIconEffect(CCMenuItemSpriteExtra* button, bool useOpacity) {
        int iconNumber = button->getTag();
        bool isUnlocked = isIconActuallyUnlocked(iconNumber, button->m_iconType);
        
        if (auto oldBg = button->getChildByID("lock-indicator-bg"_spr)) {
            oldBg->removeFromParent();
        }
        
        auto iconSprite = static_cast<CCSprite*>(button->getNormalImage());
        
        if (useOpacity) {
            if (iconSprite) {
                iconSprite->setOpacity(isUnlocked? 255:128);
            }
        } else {
            auto bg = CCSprite::create("bg.png"_spr);
            if (bg) {
                bg->setID("lock-indicator-bg"_spr);
                bg->setColor((!isUnlocked) ? ccColor3B{255, 0, 0} : ccColor3B{0, 255, 0});
                
                auto buttonSize = button->getContentSize();
                bg->setPosition(buttonSize / 2);
                bg->setScale(buttonSize.width / bg->getContentSize().width);
                bg->setZOrder(-1);
                bg->setOpacity(55);
                
                button->addChild(bg);
            }
        }
    }
};