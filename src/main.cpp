#include <Geode/Geode.hpp>
#include <Geode/modify/GJGarageLayer.hpp>

using namespace geode::prelude;

template <typename T> T* findChildOfType(CCNode* root) {
    if (!root) return nullptr;
    
    auto children = root->getChildren();
    if (!children) return nullptr;
    
    for (auto child : CCArrayExt<CCNode*>(children)) {
        if (auto casted = typeinfo_cast<T*>(child))
            return casted;
        
        if (auto found = findChildOfType<T>(child))
            return found;
    }
    return nullptr;
}

bool isIconActuallyUnlocked(int iconID, IconType type) {
    // Exceptions (these aren't stored in the save file) (i think)
    if (iconID == 1 || (type == IconType::Cube && (iconID == 2 || iconID == 3 || iconID == 4))) 
        return true;
    
    auto gm = GameManager::sharedState();
    
    std::string saveKey;
    switch (type) {
        case IconType::Cube: saveKey = fmt::format("i_{}", iconID); break;
        case IconType::Ship: saveKey = fmt::format("ship_{}", iconID); break;
        case IconType::Ball: saveKey = fmt::format("ball_{}", iconID); break;
        case IconType::Ufo: saveKey = fmt::format("bird_{}", iconID); break;
        case IconType::Wave: saveKey = fmt::format("dart_{}", iconID); break;
        case IconType::Robot: saveKey = fmt::format("robot_{}", iconID); break;
        case IconType::Spider: saveKey = fmt::format("spider_{}", iconID); break;
        case IconType::Swing: saveKey = fmt::format("swing_{}", iconID); break;
        case IconType::Jetpack: saveKey = fmt::format("jetpack_{}", iconID); break;
        default: 
            // We don't know - let's get out
            return true;
    }
    
    auto valueKeeper = gm->m_valueKeeper;
    if (!valueKeeper) return false;
    
    auto valueObj = valueKeeper->objectForKey(saveKey);
    if (!valueObj) return false;
    
    if (auto boolObj = typeinfo_cast<CCBool*>(valueObj)) {
        return boolObj->getValue();
    }
    
    if (auto strObj = typeinfo_cast<CCString*>(valueObj)) {
        return strObj->getCString() == std::string("1");
    }
    
    return false;
}

class $modify(IconGarageLayer, GJGarageLayer) {
    struct Fields {
        int m_currentPage = 0;
        IconType m_currentIconType = IconType::Cube;
    };

    bool init() {
        if (!GJGarageLayer::init())
            return false;
        
        return true;
    }

    void setupPage(int page, IconType type) {
        GJGarageLayer::setupPage(page, type);
        
        m_fields->m_currentPage = page;
        m_fields->m_currentIconType = type;
        
        this->applyIconEffects();
        
        // Debug check
        if (Mod::get()->getSettingValue<bool>("show-debug-info")) {
            this->addStupidButton();
            this->addIconNumbers();
        }
    }

    void applyIconEffects() {
        auto page = findChildOfType<ListButtonPage>(this);
        if (!page) return;

        auto menu = findChildOfType<CCMenu>(page);
        if (!menu) return;

        bool useOpacity = Mod::get()->getSettingValue<bool>("use-opacity-mode");
        int iconCount = 0;
        auto children = menu->getChildren();
        if (!children) return;

        for (auto child : CCArrayExt<CCNode*>(children)) {
            if (auto button = typeinfo_cast<CCMenuItemSpriteExtra*>(child)) {
                int iconNumber = (m_fields->m_currentPage * 36) + iconCount + 1;  
                bool isUnlocked = isIconActuallyUnlocked(iconNumber, m_fields->m_currentIconType);
                
                if (auto oldBg = button->getChildByID("lock-indicator-bg"_spr)) {
                    oldBg->removeFromParent();
                }
                
                GJItemIcon* iconSprite = nullptr;
                if (button->getChildren() && button->getChildren()->count() > 0) {
                    iconSprite = typeinfo_cast<GJItemIcon*>(button->getChildren()->objectAtIndex(0));
                }
                
                if (!isUnlocked) {
                    if (useOpacity) {
                        if (iconSprite) {
                            iconSprite->setOpacity(128);
                        }
                    } else {
                        auto bg = CCSprite::create("bg.png"_spr);
                        if (bg) {
                            bg->setID("lock-indicator-bg"_spr);
                            bg->setColor({255, 0, 0});
                            
                            auto buttonSize = button->getContentSize();
                            bg->setPosition(buttonSize / 2);
                            bg->setScale(buttonSize.width / bg->getContentSize().width);
                            bg->setZOrder(-1);
                            bg->setOpacity(55);
                            
                            button->addChild(bg);
                        }
                    }
                } else {
                    if (iconSprite) {
                        iconSprite->setOpacity(255);
                    }
                    
                    if (!useOpacity) {
                        auto bg = CCSprite::create("bg.png"_spr);
                        if (bg) {
                            bg->setID("lock-indicator-bg"_spr);
                            bg->setColor({0,255,0});
                            
                            auto buttonSize = button->getContentSize();
                            bg->setPosition(buttonSize / 2);
                            bg->setScale(buttonSize.width / bg->getContentSize().width);
                            bg->setZOrder(-1);
                            bg->setOpacity(55);
                            
                            button->addChild(bg);
                        }
                    }
                }
                
                iconCount++;
            }
        }
    }

    void addStupidButton() {
        auto page = findChildOfType<ListButtonPage>(this);
        if (!page) return;

        auto menu = findChildOfType<CCMenu>(page);
        if (!menu) return;

        if (menu->getChildByID("my-button"_spr)) return;

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png");
        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(IconGarageLayer::onMyButton)
        );

        button->setID("my-button"_spr);
        menu->addChild(button);

        if (auto layout = menu->getLayout()) {
            layout->apply(menu);
        }
    }

    void onMyButton(CCObject*) {
        std::string iconTypeName;
        switch (m_fields->m_currentIconType) {
            case IconType::Cube: iconTypeName = "Cube"; break;
            case IconType::Ship: iconTypeName = "Ship"; break;
            case IconType::Ball: iconTypeName = "Ball"; break;
            case IconType::Ufo: iconTypeName = "UFO"; break;
            case IconType::Wave: iconTypeName = "Wave"; break;
            case IconType::Robot: iconTypeName = "Robot"; break;
            case IconType::Spider: iconTypeName = "Spider"; break;
            case IconType::Swing: iconTypeName = "Swing"; break;
            case IconType::Jetpack: iconTypeName = "Jetpack"; break;
            default: iconTypeName = "Unknown"; break;
        }
        FLAlertLayer::create(
            "debug info",
            fmt::format("Page: {}\nType: {}", 
                       m_fields->m_currentPage, 
                       iconTypeName),
            ":3"
        )->show();
    }

    void addIconNumbers() {
        auto page = findChildOfType<ListButtonPage>(this);
        if (!page) return;

        auto menu = findChildOfType<CCMenu>(page);
        if (!menu) return;

        int iconCount = 0;
        auto children = menu->getChildren();
        if (!children) return;

        for (auto child : CCArrayExt<CCNode*>(children)) {
            if (auto button = typeinfo_cast<CCMenuItemSpriteExtra*>(child)) {
                int iconNumber = (m_fields->m_currentPage * 36) + iconCount + 1;
                
                if (auto oldLabel = button->getChildByID("icon-number-label"_spr)) {
                    oldLabel->removeFromParent();
                }
                
                auto label = CCLabelBMFont::create(
                    fmt::format("{}", iconNumber).c_str(),
                    "bigFont.fnt"
                );
                label->setID("icon-number-label"_spr);
                label->setScale(0.3f);
                
                auto buttonSize = button->getContentSize();
                label->setPosition({buttonSize.width / 2, buttonSize.height + 10});
                
                button->addChild(label);
                iconCount++;
            }
        }
    }
};