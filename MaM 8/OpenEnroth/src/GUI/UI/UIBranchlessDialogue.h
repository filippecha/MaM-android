#pragma once

#include <string>
#include <vector>

#include "Engine/Evt/EvtEnums.h"

#include "GUI/GUIWindow.h"

class GUIWindow_BranchlessDialogue : public GUIWindow {
 public:
    explicit GUIWindow_BranchlessDialogue(EvtOpcode event);

    /**
     * Creates a window that asks the player a question in the status bar and waits for a typed answer.
     *
     * @param question                  Text shown in front of the typed answer.
     * @param answers                   Answers that count as correct, compared case-insensitively.
     * @param rightAnswerStep           Event step to continue at if the answer is correct.
     */
    GUIWindow_BranchlessDialogue(std::string question, std::vector<std::string> answers, int rightAnswerStep);
    virtual ~GUIWindow_BranchlessDialogue();

    virtual void Update() override;

    EvtOpcode event() const {
        return _event;
    }

 private:
    void updateQuestion();

    EvtOpcode _event = EVENT_Invalid;
    std::string _question;
    std::vector<std::string> _answers;
    int _rightAnswerStep = 0;
};

void startBranchlessDialogue(int eventid, int entryline, EvtOpcode type);

/**
 * Pauses the event and asks the player a question, see `GUIWindow_BranchlessDialogue`.
 *
 * @param eventid                       Event to continue after the answer.
 * @param wrongAnswerStep               Step to continue at after a wrong answer.
 * @param rightAnswerStep               Step to continue at after a right answer.
 * @param question                      Question text.
 * @param answers                       Right answers.
 */
void startQuestionDialogue(int eventid, int wrongAnswerStep, int rightAnswerStep, std::string question, std::vector<std::string> answers);
void releaseBranchlessDialogue();
