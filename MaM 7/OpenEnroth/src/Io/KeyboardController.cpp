#include "KeyboardController.h"

#include "Library/Platform/Interface/PlatformEvents.h"

KeyboardController::KeyboardController()
    : PlatformEventFilter({EVENT_KEY_PRESS, EVENT_KEY_RELEASE, EVENT_GAMEPAD_KEY_PRESS, EVENT_GAMEPAD_KEY_RELEASE, EVENT_GAMEPAD_AXIS}) {}

bool KeyboardController::isKeyPressedThisFrame(PlatformKey key) const {
    if (key == PlatformKey::KEY_NONE)
        return false;

    return _isKeyPressedThisFrame[key];
}

bool KeyboardController::isKeyDownThisFrame(PlatformKey key) const {
    if (key == PlatformKey::KEY_NONE)
        return false;

    return _isKeyDown[key] || _isKeyPressedThisFrame[key];
}

bool KeyboardController::keyPressEvent(const PlatformKeyEvent *event) {
    if (_isKeyDown[event->key])
        return false; // Auto repeat

    _isKeyDown[event->key] = true;
    _isKeyPressedThisFrame[event->key] = true;
    return false;
}

bool KeyboardController::keyReleaseEvent(const PlatformKeyEvent *event) {
    _isKeyDown[event->key] = false;
    return false;
}

// The gamepad's buttons and sticks count as keys held down, the same way the actions bound to them are read.
bool KeyboardController::gamepadKeyPressEvent(const PlatformGamepadKeyEvent *event) {
    if (event->key == PlatformKey::KEY_NONE || _isKeyDown[event->key])
        return false;
    _isKeyDown[event->key] = true;
    _isKeyPressedThisFrame[event->key] = true;
    return false;
}

bool KeyboardController::gamepadKeyReleaseEvent(const PlatformGamepadKeyEvent *event) {
    if (event->key != PlatformKey::KEY_NONE)
        _isKeyDown[event->key] = false;
    return false;
}

bool KeyboardController::gamepadAxisEvent(const PlatformGamepadAxisEvent *event) {
    // An axis arrives as its positive direction, a negative value means the other one. Tilted past half it is
    // a key held down, back under half both directions are released.
    PlatformKey positive = event->axis, negative = PlatformKey::KEY_NONE;
    switch (positive) {
    case PlatformKey::KEY_GAMEPAD_LEFTSTICK_RIGHT:  negative = PlatformKey::KEY_GAMEPAD_LEFTSTICK_LEFT; break;
    case PlatformKey::KEY_GAMEPAD_LEFTSTICK_DOWN:   negative = PlatformKey::KEY_GAMEPAD_LEFTSTICK_UP; break;
    case PlatformKey::KEY_GAMEPAD_RIGHTSTICK_RIGHT: negative = PlatformKey::KEY_GAMEPAD_RIGHTSTICK_LEFT; break;
    case PlatformKey::KEY_GAMEPAD_RIGHTSTICK_DOWN:  negative = PlatformKey::KEY_GAMEPAD_RIGHTSTICK_UP; break;
    default: break;
    }
    if (positive == PlatformKey::KEY_NONE)
        return false;
    PlatformKey down = event->value >= 0.5f ? positive : event->value <= -0.5f ? negative : PlatformKey::KEY_NONE;
    for (PlatformKey key : {positive, negative}) {
        if (key == PlatformKey::KEY_NONE)
            continue;
        if (key == down) {
            if (!_isKeyDown[key]) {
                _isKeyDown[key] = true;
                _isKeyPressedThisFrame[key] = true;
            }
        } else {
            _isKeyDown[key] = false;
        }
    }
    return false;
}

void KeyboardController::processMessages(PlatformEventHandler *eventHandler) {
    _isKeyPressedThisFrame.fill(false);
    ProxyEventLoop::processMessages(eventHandler);
}

void KeyboardController::reset() {
    _isKeyDown.fill(false);
    _isKeyPressedThisFrame.fill(false);
}
