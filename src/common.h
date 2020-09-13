/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#ifndef SUDOKU_UI_COMMON_H
#define SUDOKU_UI_COMMON_H

#include <cstdint>

namespace UI {

enum InputMode : std::uint8_t {
    NormalInput = 0,
    CandidateInput = 1
};

enum CallBacks : std::uint8_t {
    onSquareClick = 0,
    onMouseDown = 1,
    onMouseUp = 2,
    onMouseOver = 3
};

}

#endif //SUDOKU_UI_COMMON_H
