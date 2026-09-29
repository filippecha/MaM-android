#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <span>

#include "Engine/Objects/Character.h"

/**
 * Characters of the party. MM6 and MM7 always have four, MM8 starts with one and can have up to five. Iteration and
 * `size()` only see the characters that are in the party.
 */
class PartyCharacters {
 public:
    static constexpr size_t CAPACITY = 5;

    [[nodiscard]] size_t size() const {
        return _size;
    }

    /**
     * @param size                      New number of characters in the party, 1 to `CAPACITY`. Characters past the
     *                                  new size keep their data, but aren't seen until the party grows again.
     */
    void resize(size_t size) {
        assert(size >= 1 && size <= CAPACITY);
        _size = size;
    }

    [[nodiscard]] Character &operator[](size_t index) {
        assert(index < _size);
        return _characters[index];
    }

    [[nodiscard]] const Character &operator[](size_t index) const {
        assert(index < _size);
        return _characters[index];
    }

    [[nodiscard]] Character *data() {
        return _characters.data();
    }

    [[nodiscard]] const Character *data() const {
        return _characters.data();
    }

    [[nodiscard]] Character &front() {
        return _characters[0];
    }

    [[nodiscard]] Character *begin() {
        return _characters.data();
    }

    [[nodiscard]] Character *end() {
        return _characters.data() + _size;
    }

    [[nodiscard]] const Character *begin() const {
        return _characters.data();
    }

    [[nodiscard]] const Character *end() const {
        return _characters.data() + _size;
    }

    /**
     * @return                          All the slots, also the ones past `size()`, e.g. for resetting the party.
     */
    [[nodiscard]] std::span<Character, CAPACITY> slots() {
        return _characters;
    }

    /**
     * @return                          The first four slots, which MM6 and MM7 saves store.
     */
    [[nodiscard]] std::span<Character, 4> firstFour() {
        return std::span<Character, 4>(_characters.data(), 4);
    }

    [[nodiscard]] std::span<const Character, 4> firstFour() const {
        return std::span<const Character, 4>(_characters.data(), 4);
    }

 private:
    std::array<Character, CAPACITY> _characters;
    size_t _size = 4;
};
