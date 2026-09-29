#include "EngineGlobals.h"

#include "Io/KeyboardInputHandler.h"

#include "Library/Platform/Application/PlatformApplication.h"

Platform *platform = nullptr;
PlatformWindow *window = nullptr;
PlatformOpenGLContext *openGLContext = nullptr;
PlatformEventLoop *eventLoop = nullptr;
PlatformEventHandler *eventHandler = nullptr;
PlatformApplication *application = nullptr;


void detail::globalProcessMessages() {
    application->processMessages();

    // Shows the on-screen keyboard on mobile while the player types a name.
    if (window && keyboardInputHandler) {
        bool textInputActive = keyboardInputHandler->IsTextInputActive();
        if (window->isTextInputActive() != textInputActive)
            window->setTextInputActive(textInputActive);
    }
}

void detail::globalWaitForMessages() {
    application->waitForMessages();
}
