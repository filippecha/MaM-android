#include "AdventurersInn.h"

#include <string>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineGlobals.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Objects/CharacterEnumFunctions.h"
#include "Engine/Objects/Mm8Roster.h"
#include "Engine/Party.h"
#include "Engine/Spells/Spells.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/UI/Mm8Paperdoll.h"
#include "GUI/UI/UIGame.h"
#include "GUI/UI/UIStatusBar.h"

#include "Io/Mouse.h"

#include "Library/Platform/Interface/Platform.h"

#include "Media/Audio/AudioPlayer.h"

#include "Utility/String/Transformations.h"

namespace {

constexpr int VISIBLE_PORTRAITS = 8; // Two columns of four.
const Recti PORTRAITS_RECT = {30, 44, 135, 300};
const Recti SCROLL_UP_RECT = {8, 45, 17, 17};
const Recti SCROLL_DOWN_RECT = {8, 324, 17, 17};
const Recti HIRE_RECT = {520, 380, 75, 26};
const Recti DISMISS_RECT = {520, 420, 75, 26};
const Recti RETURN_RECT = {520, 450, 75, 26};

Pointi portraitPosition(int slot) {
    return {slot % 2 ? 97 : 32, 46 + 75 * (slot / 2)};
}

bool canDismissActiveCharacter() {
    return pParty->hasActiveCharacter() && pParty->activeCharacterIndex() > 0; // The main character is always first.
}

std::string dismissLabel() {
    std::string label = localization->mm8Str(408); // "Dismiss %s".
    if (size_t pos = label.find("%s"); pos != std::string::npos)
        label.erase(pos, 2);
    return std::string(trim(label));
}

} // namespace

GUIWindow_AdventurersInn::GUIWindow_AdventurersInn(HouseId houseId) : GUIWindow_House(houseId) {
    _background = assets->getImage_ColorKey("rost_bg");
    for (int i = 0; i < _highlight.size(); i++)
        _highlight[i] = assets->getImage_ColorKey(fmt::format("rost_HL{}", i + 1));
    if (!pParty->mm8InnCharacters.empty())
        _selected = pParty->mm8InnCharacters.front().mm8RosterId;

    pBtn_ExitCancel->rect = RETURN_RECT;
    CreateButton(PORTRAITS_RECT.topLeft(), PORTRAITS_RECT.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_HouseScreenClick, 0);
    CreateButton(SCROLL_UP_RECT.topLeft(), SCROLL_UP_RECT.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_HouseScreenClick, 0);
    CreateButton(SCROLL_DOWN_RECT.topLeft(), SCROLL_DOWN_RECT.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_HouseScreenClick, 0);
    CreateButton(HIRE_RECT.topLeft(), HIRE_RECT.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_HouseScreenClick, 0, INPUT_ACTION_INVALID,
                 localization->str(LSTR_HIRE));
    CreateButton(DISMISS_RECT.topLeft(), DISMISS_RECT.size(), BUTTON_TYPE_NORMAL, 0, UIMSG_HouseScreenClick, 0, INPUT_ACTION_INVALID,
                 dismissLabel());
    if (pParty->hasActiveCharacter())
        _lastActiveCharacter = pParty->activeCharacterIndex();
}

Character *GUIWindow_AdventurersInn::selectedCharacter() const {
    for (Character &character : pParty->mm8InnCharacters)
        if (character.mm8RosterId == _selected)
            return &character;
    return nullptr;
}

void GUIWindow_AdventurersInn::Update() {
    int activeCharacter = pParty->hasActiveCharacter() ? pParty->activeCharacterIndex() : -1;
    if (activeCharacter != _lastActiveCharacter) {
        _lastActiveCharacter = activeCharacter;
        _showingParty = activeCharacter >= 0;
    }

    const std::vector<Character> &inn = pParty->mm8InnCharacters;
    render->DrawQuad2D(_background, {0, 23});
    render->DrawQuad2D(assets->getImage_ColorKey("ib-8pxbar"), {459, 21});
    if (inn.size() > VISIBLE_PORTRAITS) {
        render->DrawQuad2D(assets->getImage_ColorKey("ar_up_up"), SCROLL_UP_RECT.topLeft());
        render->DrawQuad2D(assets->getImage_ColorKey("ar_dn_up"), SCROLL_DOWN_RECT.topLeft());
    }

    for (int slot = 0; slot < VISIBLE_PORTRAITS && _scroll * 2 + slot < inn.size(); slot++) {
        const Character &character = inn[_scroll * 2 + slot];
        Pointi position = portraitPosition(slot);
        render->DrawQuad2D(assets->getImage_ColorKey(fmt::format("npc29{:02}", character.uCurrentFace + 1)), position);
        if (!_showingParty && character.mm8RosterId == _selected)
            render->DrawQuad2D(_highlight[(platform->tickCount() / 250) % _highlight.size()], position - Pointi(2, 2));
    }

    Character *shown = nullptr;
    if (_showingParty && pParty->hasActiveCharacter()) {
        shown = &pParty->activeCharacter();
        drawDetails(*shown, canDismissActiveCharacter() ? localization->mm8Str(738) : std::string()); // "Click Dismiss...".
        if (canDismissActiveCharacter())
            render->DrawQuad2D(assets->getImage_ColorKey("but26u"), DISMISS_RECT.topLeft());
    } else if ((shown = selectedCharacter())) {
        drawDetails(*shown, mm8RosterBlurb(shown->mm8RosterId));
        render->DrawQuad2D(assets->getImage_ColorKey("but25u"), HIRE_RECT.topLeft());
    }
    if (shown) {
        render->SetUIClipRect(Recti(467, 23, 173, 343)); // The doll background is taller than the space above the bottom bar.
        for (const Mm8DollPiece &piece : mm8PaperdollPieces(*shown, {467, 23}))
            render->DrawQuad2D(piece.image, piece.position);
        render->ResetUIClipRect();
    }
    render->DrawQuad2D(assets->getImage_ColorKey("but24u"), RETURN_RECT.topLeft());
}

void GUIWindow_AdventurersInn::drawDetails(Character &character, const std::string &blurb) const {
    // MM8.exe 0x4CB84A, two columns of "label: value" lines and the blurb under them.
    GUIFont *font = assets->pFontSmallnum.get();
    int lineHeight = font->GetHeight() + 2;
    int y = 47;
    auto line = [&](int x, int width, std::string_view text) {
        font->DrawText(Recti(0, 0, x + width, 480), {x, y}, colorTable.White, text, 480, colorTable.Black);
    };
    auto left = [&](int labelIndex, const std::string &value) {
        line(192, 150, fmt::format("{}: {}", localization->mm8Str(labelIndex), value));
    };
    auto right = [&](int labelIndex, const std::string &value) {
        line(332, 80, fmt::format("{}: {}", localization->mm8Str(labelIndex), value));
    };

    line(192, 230, fmt::format("{}: {}", localization->mm8Str(149), character.name));
    y += lineHeight;
    line(192, 250, fmt::format("{}: {}", localization->mm8Str(41), localization->className(character.classType)));
    y += lineHeight;
    left(107, std::to_string(character.health));
    right(131, std::to_string(character.GetActualLevel()));
    y += lineHeight;
    left(0, std::to_string(character.GetActualAC()));
    right(209, std::to_string(character.mana));
    y += lineHeight;
    left(18, fmt::format("{:+}", character.GetActualAttack(false)));
    right(66, character.GetMeleeDamageString());
    y += lineHeight;
    left(203, fmt::format("{:+}", character.GetRangedAttack()));
    right(66, character.GetRangedDamageString());
    y += lineHeight;
    int skillCount = 0;
    for (Skill skill : allVisibleSkills())
        if (character.pActiveSkills[skill])
            skillCount++;
    left(205, std::to_string(skillCount));
    right(168, std::to_string(character.uSkillPoints));
    y += lineHeight;
    line(192, 230, fmt::format("{}: {}", localization->mm8Str(45), localization->characterConditionName(character.GetMajorConditionIdx())));
    y += lineHeight;
    std::string quickSpell = character.uQuickSpell == SPELL_NONE ? localization->str(LSTR_NONE) : pSpellStats->pInfos[character.uQuickSpell].name;
    line(192, 230, fmt::format("{}: {}", localization->mm8Str(170), quickSpell));
    y += 2 * lineHeight;
    font->DrawText(Recti(0, 0, 192 + 240, 480), {192, y}, colorTable.White, blurb, 0, colorTable.Black);
}

void GUIWindow_AdventurersInn::houseScreenClick() {
    std::vector<Character> &inn = pParty->mm8InnCharacters;
    Pointi mousePos = mouse->position();

    if (SCROLL_UP_RECT.contains(mousePos)) {
        if (_scroll > 0)
            _scroll--;
        return;
    }
    if (SCROLL_DOWN_RECT.contains(mousePos)) {
        if (_scroll * 2 + VISIBLE_PORTRAITS < inn.size())
            _scroll++;
        return;
    }

    if (PORTRAITS_RECT.contains(mousePos)) {
        for (int slot = 0; slot < VISIBLE_PORTRAITS && _scroll * 2 + slot < inn.size(); slot++) {
            if (Recti(portraitPosition(slot), Sizei(63, 73)).contains(mousePos)) {
                _selected = inn[_scroll * 2 + slot].mm8RosterId;
                _showingParty = false;
                return;
            }
        }
        return;
    }

    if (HIRE_RECT.contains(mousePos) && !_showingParty && selectedCharacter()) {
        if (joinMm8RosterCharacter(_selected) == MM8_JOIN_PARTY_FULL) {
            engine->_statusBar->setEvent(LSTR_I_CANNOT_JOIN_YOU_YOURE_PARTY_IS_FULL);
            return;
        }
        int index = pParty->pCharacters.size() - 1;
        GameUI_ReloadPlayerPortraits(index, pParty->pCharacters[index].uCurrentFace);
        pAudioPlayer->playUISound(SOUND_51heroism03);
        _selected = inn.empty() ? -1 : inn.front().mm8RosterId;
        _scroll = 0;
        return;
    }

    if (DISMISS_RECT.contains(mousePos) && _showingParty && canDismissActiveCharacter()) {
        int index = pParty->activeCharacterIndex();
        _selected = pParty->pCharacters[index].mm8RosterId;
        dismissMm8RosterCharacter(index);
        for (int i = 0; i < pParty->pCharacters.size(); i++)
            GameUI_ReloadPlayerPortraits(i, pParty->pCharacters[i].uCurrentFace);
        pParty->setActiveCharacterIndex(0);
        _lastActiveCharacter = 0;
        _showingParty = false;
    }
}
