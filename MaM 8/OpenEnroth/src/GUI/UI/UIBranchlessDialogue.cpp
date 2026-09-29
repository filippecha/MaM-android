#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "UIBranchlessDialogue.h"

#include "Engine/Engine.h"
#include "Engine/AssetsManager.h"
#include "Engine/Evt/Processor.h"
#include "Engine/Graphics/Renderer/Renderer.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Party.h"
#include "Engine/mm7_data.h"
#include "Engine/Graphics/Viewport.h"

#include "GUI/GUIFont.h"
#include "GUI/GUIMessageQueue.h"
#include "GUI/UI/UIHouses.h"
#include "GUI/UI/UIGame.h"

#include "Io/KeyboardInputHandler.h"

#include "Utility/String/Ascii.h"
#include "Utility/String/Transformations.h"

GUIWindow_BranchlessDialogue::GUIWindow_BranchlessDialogue(EvtOpcode event) : GUIWindow(WINDOW_GreetingNPC, {0, 0}, render->GetRenderDimensions()), _event(event) {
    prev_screen_type = current_screen_type;
    // Any key closes the dialogue, only an answer to a question is typed.
    keyboardInputHandler->StartTextInput(event == EVENT_InputString ? Io::TextInputType::Text : Io::TextInputType::AnyKey, 15, this);
    current_screen_type = SCREEN_BRANCHLESS_NPC_DIALOG;

    CreateCharacterButtons();
}

GUIWindow_BranchlessDialogue::GUIWindow_BranchlessDialogue(std::string question, std::vector<std::string> answers, int rightAnswerStep)
    : GUIWindow_BranchlessDialogue(EVENT_InputString) {
    _question = std::move(question);
    _answers = std::move(answers);
    _rightAnswerStep = rightAnswerStep;
}

GUIWindow_BranchlessDialogue::~GUIWindow_BranchlessDialogue() {
    current_screen_type = prev_screen_type;
    keyboardInputHandler->EndTextInput(this);
}

void GUIWindow_BranchlessDialogue::Update() {
    if (current_npc_text.length() > 0 && branchless_dialogue_str.empty())
        branchless_dialogue_str = current_npc_text;

    pGUIWindow_BranchlessDialogue->DrawDialoguePanel(branchless_dialogue_str);
    render->DrawQuad2D(game_ui_statusbar, {0, 352});

    if (_event == EVENT_InputString) {
        updateQuestion();
        return;
    }

    // Close branchless dialog on any keypress
    if (!keyboardInputHandler->GetTextInput().empty()) {
        keyboardInputHandler->EndTextInput();
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
        return;
    }

    // Also close branchless dialog on enter
    if (pGUIWindow_BranchlessDialogue->keyboard_input_status != WINDOW_INPUT_IN_PROGRESS) {
        engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
        return;
    }
}

void GUIWindow_BranchlessDialogue::updateQuestion() {
    if (keyboard_input_status == WINDOW_INPUT_IN_PROGRESS) {
        std::string text = fmt::format("{} {}", _question, keyboardInputHandler->GetTextInput());
        GUIFont *font = assets->pFontLucida.get();
        DrawText(font, {13, 357}, colorTable.White, text, frameRect);
        DrawFlashingInputCursor(font->GetLineWidth(text) + 13, 357, font, frameRect);
        return;
    }

    if (keyboard_input_status == WINDOW_INPUT_CONFIRMED) {
        std::string_view answer = trim(keyboardInputHandler->GetTextInput());
        for (const std::string &rightAnswer : _answers)
            if (!answer.empty() && ascii::noCaseEquals(answer, trim(rightAnswer)))
                savedEventStep = _rightAnswerStep;
    }
    keyboard_input_status = WINDOW_INPUT_NONE;
    engine->_messageQueue->addMessageCurrentFrame(UIMSG_Escape, 0, 0);
}

void startBranchlessDialogue(int eventid, int entryline, EvtOpcode type) {
    if (!pGUIWindow_BranchlessDialogue) {
        animTimer->setPaused(true);
        gameTimer->setPaused(true);
        savedEventID = eventid;
        savedEventStep = entryline;
        savedDecoration = activeLevelDecoration;
        pGUIWindow_BranchlessDialogue = std::make_unique<GUIWindow_BranchlessDialogue>(type);
    }
}

void startQuestionDialogue(int eventid, int wrongAnswerStep, int rightAnswerStep, std::string question, std::vector<std::string> answers) {
    if (pGUIWindow_BranchlessDialogue)
        return;
    animTimer->setPaused(true);
    gameTimer->setPaused(true);
    savedEventID = eventid;
    savedEventStep = wrongAnswerStep;
    savedDecoration = activeLevelDecoration;
    pGUIWindow_BranchlessDialogue = std::make_unique<GUIWindow_BranchlessDialogue>(std::move(question), std::move(answers), rightAnswerStep);
}

void releaseBranchlessDialogue() {
    pGUIWindow_BranchlessDialogue = nullptr;
    if (savedEventID) {
        // Do not run event engine whith no event, it may happen when you close talk window
        // with NPC that only say catch phrases
        activeLevelDecoration = savedDecoration;
        eventProcessor(savedEventID, Pid(), 1, savedEventStep);
    }
    activeLevelDecoration = nullptr;
    gameTimer->setPaused(false);
}

