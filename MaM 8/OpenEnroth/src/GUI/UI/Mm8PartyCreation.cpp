#include "Mm8PartyCreation.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "Engine/AssetsManager.h"
#include "Engine/Engine.h"
#include "Engine/Graphics/Image.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Localization.h"
#include "Engine/Objects/Character.h"
#include "Engine/Objects/Mm8Ids.h"
#include "Engine/Party.h"
#include "Engine/Random/Random.h"
#include "Engine/Spells/SpellEnums.h"

#include "GUI/GUIButton.h"
#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/GUIWindow.h"
#include "GUI/UI/Mm8Paperdoll.h"
#include "GUI/UI/UIPartyCreation.h"

#include "Io/KeyboardInputHandler.h"
#include "Io/Mouse.h"

#include "Media/Audio/AudioPlayer.h"

extern GUIButton *pPlayerCreationUI_BtnReset;
extern GUIButton *pPlayerCreationUI_BtnOK;
extern GUIButton *pPlayerCreationUI_BtnPlus;
extern GUIButton *pPlayerCreationUI_BtnMinus;
extern int uPlayerCreationUI_SelectedCharacter;

namespace {

// Screen places, from MM8.exe 0x4C7726 (drawing) and 0x4C7A10 (buttons) and measured on the original screen.
constexpr Pointi PORTRAIT_POS = {4, 162};
constexpr Pointi FACE_LEFT_POS = {69, 165};
constexpr Pointi FACE_RIGHT_POS = {161, 165};
constexpr Pointi VOICE_LEFT_POS = {69, 189};
constexpr Pointi VOICE_RIGHT_POS = {161, 189};
constexpr Pointi DEFAULT_VOICE_POS = {93, 214};
constexpr Pointi CLEAR_POS = {373, 440};
constexpr Pointi CANCEL_POS = {459, 440};
constexpr Pointi OK_POS = {546, 440};
constexpr int STATS_Y = 298;
constexpr int STATS_STEP = 18;
constexpr int MINUS_X = 105;
constexpr int PLUS_X = 167;
constexpr Pointi DOLL_POS = {454, 54}; // Where MM8 puts the paper doll background, body parts are relative to it.

constexpr int SKILL_COLUMN_WIDTH = 103;
constexpr std::array<int, 2> SKILL_COLUMN_X = {226, 329};
constexpr int SKILL_BOX_Y = 111;
constexpr int SKILL_BOX_STEP = 36;
constexpr int OTHER_SKILLS_Y = 246; // Middle of the first row.
constexpr int OTHER_SKILLS_STEP = 34;

constexpr Color SKILL_START_COLOR = Color(255, 255, 0);
constexpr Color SKILL_CHOSEN_COLOR = Color(0, 203, 255);

struct Images {
    GraphicsImage *background = nullptr;
    GraphicsImage *ring = nullptr;
    std::array<GraphicsImage *, 2> faceLeft = {}; // Normal and hovered.
    std::array<GraphicsImage *, 2> faceRight = {};
    std::array<GraphicsImage *, 2> defaultVoice = {};
    std::array<GraphicsImage *, 2> clear = {};
    std::array<GraphicsImage *, 2> cancel = {};
    std::array<GraphicsImage *, 2> ok = {};
    std::array<GraphicsImage *, 2> minus = {};
    std::array<GraphicsImage *, 2> plus = {};
};

Images images;
Duration errorExpireTime;

std::string faceName(int face) {
    return fmt::format("pc{:02}", face + 1);
}

/**
 * @return                              Skills that the MM8 character can pick at creation, in the order of MM8
 *                                      skilldes.txt.
 */
std::vector<Skill> pickableSkills(const Character &character) {
    std::vector<Skill> result;
    Mm8ClassKind kind = mm8ClassKind(character.classType);
    for (int mm8Skill = 0; mm8Skill < MM8_SKILL_COUNT; mm8Skill++) {
        Skill skill = skillFromMm8(mm8Skill);
        if (skill != SKILL_INVALID && skill != SKILL_MM8_RACIAL && mm8StartingSkill(kind, skill) == 1)
            result.push_back(skill);
    }
    return result;
}

std::vector<Skill> startingSkills(const Character &character) {
    std::vector<Skill> result;
    Mm8ClassKind kind = mm8ClassKind(character.classType);
    for (int mm8Skill = 0; mm8Skill < MM8_SKILL_COUNT; mm8Skill++) {
        Skill skill = skillFromMm8(mm8Skill);
        if (skill != SKILL_INVALID && mm8StartingSkill(kind, skill) == 2 && std::find(result.begin(), result.end(), skill) == result.end())
            result.push_back(skill);
    }
    return result;
}

std::vector<Skill> chosenSkills(const Character &character) {
    std::vector<Skill> result;
    for (Skill skill : pickableSkills(character))
        if (character.pActiveSkills[skill])
            result.push_back(skill);
    return result;
}

std::string skillName(const Character &character, Skill skill) {
    return characterSkillName(character, skill);
}

/**
 * @return                              Left edge of the pickable skill `index` out of `count`.
 */
int pickableSkillX(int index, int count) {
    bool alone = index == count - 1 && index % 2 == 0; // The last odd one is centered in the box.
    return alone ? (SKILL_COLUMN_X[0] + SKILL_COLUMN_X[1]) / 2 : SKILL_COLUMN_X[index % 2];
}

void drawCentered(GUIWindow *window, GUIFont *font, int x, int width, int y, Color color, const std::string &text) {
    window->DrawTitleText(font, x, y, color, text, 3, Recti(0, 0, x + width, 480));
}

void changeFace(Character &character, int face) {
    character.uCurrentFace = face;
    character.uVoiceID = face;
    character.ChangeClass(mm8BaseClass(mm8FaceClassKind(face)));
    character.SetInitialStats();
    character.SetSexByVoice();
    character.RandomizeName();
}

void changeVoice(Character &character, int step) {
    Sex sex = character.GetSexByVoice();
    int voice = character.uVoiceID;
    do {
        voice = (voice + step + MM8_STARTING_FACE_COUNT) % MM8_STARTING_FACE_COUNT;
        character.uVoiceID = voice;
    } while (character.GetSexByVoice() != sex);
}

} // namespace

void createMm8DefaultCharacter(Character &character) {
    changeFace(character, 1);
    static constexpr std::array<int, 7> stats = {18, 9, 9, 15, 15, 15, 11}; // As the original offers it.
    for (Attribute stat : allStatAttributes())
        character._stats[stat] = stats[std::to_underlying(stat)];
    character.pActiveSkills[SKILL_BOW] = CombinedSkillValue::novice();
    character.pActiveSkills[SKILL_REPAIR] = CombinedSkillValue::novice();
}

void GUIWindow_PartyCreation::createMm8Controls() {
    main_menu_background = assets->getImage_PCXFromIconsLOD("makeme.pcx");
    images.ring = assets->getImage_ColorKey("selring");
    images.faceLeft = {assets->getImage_ColorKey("cc_up_l"), assets->getImage_ColorKey("cc_ht_l")};
    images.faceRight = {assets->getImage_ColorKey("cc_up_r"), assets->getImage_ColorKey("cc_ht_r")};
    images.defaultVoice = {assets->getImage_ColorKey("c_dft_up"), assets->getImage_ColorKey("c_dft_ht")};
    images.clear = {assets->getImage_ColorKey("c_clr_up"), assets->getImage_ColorKey("c_clr_ht")};
    images.cancel = {assets->getImage_ColorKey("c_cncl_up"), assets->getImage_ColorKey("c_cncl_ht")};
    images.ok = {assets->getImage_ColorKey("c_ok_up"), assets->getImage_ColorKey("c_ok_ht")};
    images.minus = {assets->getImage_ColorKey("cminup"), assets->getImage_ColorKey("cminht")};
    images.plus = {assets->getImage_ColorKey("cplusup"), assets->getImage_ColorKey("cplusht")};
    errorExpireTime = Duration();

    CreateButton({66, 72}, {128, 24}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationChangeName, 0);
    CreateButton(FACE_LEFT_POS, {31, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FacePrev, 0);
    CreateButton(FACE_RIGHT_POS, {31, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_FaceNext, 0);
    CreateButton(VOICE_LEFT_POS, {31, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoicePrev, 0);
    CreateButton(VOICE_RIGHT_POS, {31, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreation_VoiceNext, 0);
    CreateButton(DEFAULT_VOICE_POS, {75, 27}, BUTTON_TYPE_NORMAL, 0, UIMSG_Mm8CreationDefaultVoice, 0);
    for (int row = 0; row < 7; row++) {
        CreateButton({MINUS_X, STATS_Y + row * STATS_STEP}, {16, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_Mm8CreationStatMinus, row);
        CreateButton({PLUS_X, STATS_Y + row * STATS_STEP}, {16, 17}, BUTTON_TYPE_NORMAL, 0, UIMSG_Mm8CreationStatPlus, row);
    }
    // Chosen skills in the upper box and the pickable ones below, 12 at most. Their labels come with the character.
    for (int i = 0; i < 2; i++)
        CreateButton({SKILL_COLUMN_X[i], SKILL_BOX_Y + SKILL_BOX_STEP - 4}, {SKILL_COLUMN_WIDTH, 24}, BUTTON_TYPE_NORMAL, 0,
                     i == 0 ? UIMSG_PlayerCreationRemoveUpSkill : UIMSG_PlayerCreationRemoveDownSkill, 0);
    for (int i = 0; i < 12; i++)
        CreateButton({SKILL_COLUMN_X[i % 2], OTHER_SKILLS_Y - 15 + (i / 2) * OTHER_SKILLS_STEP}, {SKILL_COLUMN_WIDTH, 30},
                     BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationSelectActiveSkill, i);
    pPlayerCreationUI_BtnReset = CreateButton(CLEAR_POS, {83, 30}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickReset, 0);
    CreateButton(CANCEL_POS, {83, 30}, BUTTON_TYPE_NORMAL, 0, UIMSG_Escape, 0);
    pPlayerCreationUI_BtnOK = CreateButton("PartyCreation_OK", OK_POS, {83, 30}, BUTTON_TYPE_NORMAL, 0, UIMSG_PlayerCreationClickOK, 0,
                                           INPUT_ACTION_PARTY_CREATION_DONE);
    pPlayerCreationUI_BtnMinus = nullptr;
    pPlayerCreationUI_BtnPlus = nullptr;
}

void GUIWindow_PartyCreation::updateMm8() {
    Character &character = pParty->pCharacters[0];
    GUIFont *font = assets->pFontCreate.get();
    Pointi mousePos = mouse->position();
    auto hovered = [&](Pointi pos, Sizei size) { return Recti(pos, size).contains(mousePos); };
    auto drawButton = [&](const std::array<GraphicsImage *, 2> &pictures, Pointi pos) {
        render->DrawQuad2D(pictures[hovered(pos, pictures[0]->size()) ? 1 : 0], pos);
    };

    render->DrawQuad2D(main_menu_background, {0, 0});

    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
        int width = DrawTextInRect(font, {70, 77}, colorTable.White, keyboardInputHandler->GetTextInput(), 120, 1);
        DrawFlashingInputCursor(72 + width, 77, font, frameRect);
    } else {
        if (keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
            keyboard_input_status = WINDOW_INPUT_NONE;
            const std::string &input = keyboardInputHandler->GetTextInput();
            if (input.find_first_not_of(' ') != std::string::npos)
                character.name = input;
        }
        DrawTextInRect(font, {70, 77}, colorTable.White, character.name, 120, 0);
    }
    DrawText(font, {70, 122}, colorTable.White, localization->className(character.classType), frameRect);

    // Portrait, voice and paper doll.
    render->DrawQuad2D(assets->getImage_ColorKey(fmt::format("{}-01", faceName(character.uCurrentFace))), PORTRAIT_POS);
    render->DrawQuad2D(images.ring, PORTRAIT_POS);
    drawButton(images.faceLeft, FACE_LEFT_POS);
    drawButton(images.faceRight, FACE_RIGHT_POS);
    drawButton(images.faceLeft, VOICE_LEFT_POS);
    drawButton(images.faceRight, VOICE_RIGHT_POS);
    drawButton(images.defaultVoice, DEFAULT_VOICE_POS);

    for (const Mm8DollPiece &piece : mm8PaperdollPieces(character, DOLL_POS))
        render->DrawQuad2D(piece.image, piece.position);

    static constexpr std::array<LstrId, 7> statNames = {LSTR_MIGHT, LSTR_INTELLECT, LSTR_PERSONALITY, LSTR_ENDURANCE, LSTR_ACCURACY, LSTR_SPEED, LSTR_LUCK};
    for (Attribute stat : allStatAttributes()) {
        int row = std::to_underlying(stat);
        int y = STATS_Y + row * STATS_STEP;
        Color color = character.GetStatColor(stat);
        int raceBase = mm8ClassData(character.classType).stats[row].base;
        Color nameColor = raceBase > 11 ? ui_character_stat_buffed_color : raceBase < 11 ? ui_character_stat_debuffed_color : colorTable.White; // Against a human.
        DrawText(font, {4, y}, nameColor, localization->str(statNames[row]), frameRect);
        std::string value = fmt::format("{}", character._stats[stat]);
        DrawText(font, {144 - font->GetLineWidth(value) / 2, y + 2}, color, value, frameRect);
        drawButton(images.minus, {MINUS_X, y});
        drawButton(images.plus, {PLUS_X, y});
    }
    std::string bonus = fmt::format("{}", CharacterCreation_GetUnspentAttributePointCount());
    DrawText(font, {172 - font->GetLineWidth(bonus) / 2, 264}, colorTable.White, bonus, frameRect);

    // Skills: the two the class starts with, the two the player picks, and the ones to pick from.
    std::vector<Skill> start = startingSkills(character);
    std::vector<Skill> chosen = chosenSkills(character);
    for (int i = 0; i < 2; i++) {
        if (i < start.size())
            drawCentered(this, font, SKILL_COLUMN_X[i], SKILL_COLUMN_WIDTH, SKILL_BOX_Y, SKILL_START_COLOR, skillName(character, start[i]));
        std::string name = i < chosen.size() ? skillName(character, chosen[i]) : localization->str(LSTR_NONE);
        drawCentered(this, font, SKILL_COLUMN_X[i], SKILL_COLUMN_WIDTH, SKILL_BOX_Y + SKILL_BOX_STEP, SKILL_CHOSEN_COLOR, name);
    }
    std::vector<Skill> pickable = pickableSkills(character);
    for (int i = 0; i < pickable.size(); i++) {
        std::string name = font->WrapText(skillName(character, pickable[i]), SKILL_COLUMN_WIDTH, 0);
        int lines = 1 + std::count(name.begin(), name.end(), '\n');
        int y = OTHER_SKILLS_Y + (i / 2) * OTHER_SKILLS_STEP - lines * (font->GetHeight() - 3) / 2;
        int x = pickableSkillX(i, pickable.size());
        Color color = character.pActiveSkills[pickable[i]] ? SKILL_CHOSEN_COLOR : colorTable.White;
        drawCentered(this, font, x, SKILL_COLUMN_WIDTH, y, color, name);
    }

    drawButton(images.clear, CLEAR_POS);
    drawButton(images.cancel, CANCEL_POS);
    drawButton(images.ok, OK_POS);

    if (errorExpireTime > animTimer->time()) {
        int unspent = CharacterCreation_GetUnspentAttributePointCount();
        const std::string &hint = unspent < 0 ? localization->str(LSTR_YOU_CANT_SPEND_MORE_THAN_50_POINTS) : localization->str(LSTR_CREATE_PARTY_CANNOT_BE_COMPLETED_UNLESS);
        Recti box(120, 200, 300, 100);
        DrawMessageBox(box, hint);
    }

    // Draw the queued text now, popups drawn later must cover it.
    render->DrawTwodVerts();
    render->EndTextNew();
}

Skill mm8CreationSkillAt(const Character &character, Pointi pos, bool *known) {
    auto contains = [&](int x, int top, int height) {
        return pos.x >= x && pos.x < x + SKILL_COLUMN_WIDTH && pos.y >= top && pos.y < top + height;
    };
    std::vector<Skill> start = startingSkills(character);
    std::vector<Skill> chosen = chosenSkills(character);
    *known = true;
    for (int i = 0; i < 2; i++) {
        if (i < start.size() && contains(SKILL_COLUMN_X[i], SKILL_BOX_Y - 4, 24))
            return start[i];
        if (i < chosen.size() && contains(SKILL_COLUMN_X[i], SKILL_BOX_Y + SKILL_BOX_STEP - 4, 24))
            return chosen[i];
    }
    std::vector<Skill> pickable = pickableSkills(character);
    for (int i = 0; i < pickable.size(); i++) {
        if (contains(pickableSkillX(i, pickable.size()), OTHER_SKILLS_Y - 15 + (i / 2) * OTHER_SKILLS_STEP, 30)) {
            *known = static_cast<bool>(character.pActiveSkills[pickable[i]]);
            return pickable[i];
        }
    }
    *known = false;
    return SKILL_INVALID;
}

bool handleMm8CreationMessage(UIMessageType message, int param) {
    Character &character = pParty->pCharacters[0];
    switch (message) {
    case UIMSG_PlayerCreation_FacePrev:
    case UIMSG_PlayerCreation_FaceNext: {
        int step = message == UIMSG_PlayerCreation_FaceNext ? 1 : -1;
        changeFace(character, (character.uCurrentFace + step + MM8_STARTING_FACE_COUNT) % MM8_STARTING_FACE_COUNT);
        pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
        pAudioPlayer->stopVoiceSounds();
        character.playReaction(SPEECH_PICK_ME);
        return true;
    }
    case UIMSG_PlayerCreation_VoicePrev:
    case UIMSG_PlayerCreation_VoiceNext:
        changeVoice(character, message == UIMSG_PlayerCreation_VoiceNext ? 1 : -1);
        pAudioPlayer->playUISound(SOUND_SelectingANewCharacter);
        pAudioPlayer->stopVoiceSounds();
        character.playReaction(SPEECH_PICK_ME);
        return true;
    case UIMSG_Mm8CreationDefaultVoice:
        character.uVoiceID = character.uCurrentFace;
        pAudioPlayer->stopVoiceSounds();
        character.playReaction(SPEECH_PICK_ME);
        return true;
    case UIMSG_Mm8CreationStatPlus:
        character.IncreaseAttribute(static_cast<Attribute>(param));
        pAudioPlayer->playUISound(SOUND_ClickPlus);
        return true;
    case UIMSG_Mm8CreationStatMinus:
        character.DecreaseAttribute(static_cast<Attribute>(param));
        pAudioPlayer->playUISound(SOUND_ClickMinus);
        return true;
    case UIMSG_PlayerCreationSelectActiveSkill: {
        std::vector<Skill> pickable = pickableSkills(character);
        if (param >= pickable.size())
            return true;
        Skill skill = pickable[param];
        if (character.pActiveSkills[skill]) {
            character.pActiveSkills[skill] = CombinedSkillValue::none();
        } else if (chosenSkills(character).size() < 2) {
            character.pActiveSkills[skill] = CombinedSkillValue::novice();
        }
        pAudioPlayer->playUISound(SOUND_ClickSkill);
        return true;
    }
    case UIMSG_PlayerCreationRemoveUpSkill:
    case UIMSG_PlayerCreationRemoveDownSkill: {
        std::vector<Skill> chosen = chosenSkills(character);
        int index = message == UIMSG_PlayerCreationRemoveUpSkill ? 0 : 1;
        if (index < chosen.size())
            character.pActiveSkills[chosen[index]] = CombinedSkillValue::none();
        return true;
    }
    case UIMSG_PlayerCreationClickReset:
        character.SetInitialStats();
        for (Skill skill : pickableSkills(character))
            character.pActiveSkills[skill] = CombinedSkillValue::none();
        return true;
    case UIMSG_PlayerCreationClickOK:
        if (CharacterCreation_GetUnspentAttributePointCount() != 0 || chosenSkills(character).size() != 2) {
            errorExpireTime = animTimer->time() + Duration::fromRealtimeSeconds(4);
        } else {
            uGameState = GAME_STATE_STARTING_NEW_GAME;
        }
        return true;
    default:
        return false;
    }
}

void giveMm8StartingItems(Character &character) {
    // MM8.exe 0x495F17, in MM8 item ids. Staff to plate armor, blasters give nothing.
    static constexpr std::array<int, 12> weaponsAndArmor = {79, 1, 21, 31, 41, 56, 66, 0, 99, 84, 89, 94};
    for (int mm8Skill = 0; mm8Skill < MM8_SKILL_COUNT; mm8Skill++) {
        Skill skill = skillFromMm8(mm8Skill);
        if (skill == SKILL_INVALID || skill == SKILL_MM8_RACIAL || !character.pActiveSkills[skill])
            continue;
        if (mm8Skill < 12) {
            if (weaponsAndArmor[mm8Skill])
                character.inventory.add(Item(itemIdFromMm8(weaponsAndArmor[mm8Skill])));
        } else if (mm8Skill <= 20 && skill != SKILL_LIGHT) {
            int firstSpell = 1 + (mm8Skill - 12) * 11;
            character.bHaveSpell[static_cast<SpellId>(firstSpell)] = true;
            character.inventory.add(Item(itemIdFromMm8(400 + firstSpell))); // The book of the second spell.
        } else if (skill == SKILL_ITEM_ID || skill == SKILL_REPAIR || skill == SKILL_MEDITATION || skill == SKILL_PERCEPTION ||
                   skill == SKILL_TRAP_DISARM || skill == SKILL_LEARNING) {
            character.inventory.add(Item(itemIdFromMm8(220))); // Empty bottle.
            character.inventory.add(Item(itemIdFromMm8(200 + 5 * grng->random(3)))); // A red, blue or yellow reagent.
        }
    }
}
