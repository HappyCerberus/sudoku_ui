/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#ifndef SUDOKU_UI_SUDOKUGRID_H
#define SUDOKU_UI_SUDOKUGRID_H

#include "SudokuSquare.h"
#include "solver/Sudoku.h"
#include "solver/SolveStats.h"
#include "solver/SmartSolver.h"
#include "util.h"
#include <vector>
#include <string>
#include <exception>

namespace UI {

class SudokuGrid {
public:
    SudokuGrid(std::vector<std::function<bool(emscripten::val e)>> callbacks) : puzzle_(), callbacks_(std::move(callbacks)) {}
    asmdom::VNode *Render();

    void FlipSquareHighlight(std::string squareId) {
        int id = std::stoi(squareId);
        squares_[id].FlipBackground();
    }

    void UpdateSquareHighlight(std::string squareId, bool highlighted) {
        int id = std::stoi(squareId);
        squares_[id].UpdateBackground(highlighted);
    }

    void UpdateSquareValue(std::string squareId, std::string key, bool candidate_mode);
    void ResetSquare(std::string squareId);
    void RestoreState(const std::string &state);
    void SolveAndUpdateSquares();

private:
    sudoku::Sudoku puzzle_;
    asmdom::VNode *container_;
    std::vector<SudokuSquare> squares_;
    std::vector<std::function<bool(emscripten::val e)>> callbacks_;

    asmdom::VNode *render_squares();
    asmdom::VNode *render_background();
    asmdom::VNode *render_blocks();

    void UpdateSquareContent(std::string squareId);
    void PushHistory();
    void UpdateAllSquares();
};

}

#endif //SUDOKU_UI_SUDOKUGRID_H
