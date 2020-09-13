//
// Created by Happy on 26/07/2020.
//

#include "SudokuGrid.h"

#include <sstream>

namespace UI {

asmdom::VNode *SudokuGrid::render_squares() {
    asmdom::Children result;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int x = 50 + j * 100;
            int y = 50 + i * 100;
            std::string squareId = std::to_string(i * 9 + j);

            squares_.push_back(UI::SudokuSquare{});
            auto container = squares_.back().Render(squareId, x, y, callbacks_);
            result.push_back(container);
        }
    }
    return asmdom::h("g", asmdom::Data(asmdom::Attrs{{"id", "squares"}}), result);
}

asmdom::VNode *SudokuGrid::render_background() {
    // TODO: generate path, so it works for all sizes of Sudoku
    asmdom::Children grid{
            asmdom::h("path", asmdom::Data(asmdom::Attrs{{"class", "line"},
                                 {"d",     "M 50 50 h 900 M 50 150 h 900 M 50 250 h 900 M 50 350 h 900 M 50 450 h 900 M 50 550 h 900 M 50 650 h 900 M 50 750 h 900 M 50 850 h 900 M 50 950 h 900"}})),
            asmdom::h("path", asmdom::Data(asmdom::Attrs{{"class", "line"},
                                 {"d",     "M 50 50 v 900 M 150 50 v 900 M 250 50 v 900 M 350 50 v 900 M 450 50 v 900 M 550 50 v 900 M 650 50 v 900 M 750 50 v 900 M 850 50 v 900 M 950 50 v 900"}})),
            asmdom::h("path", asmdom::Data(asmdom::Attrs{{"class", "line-thick"},
                                 {"d",     "M 50 50 h 900 v 900 h -900 v -900"},
                                 {"fill",  "none"}}))
    };
    return asmdom::h("g", asmdom::Data(asmdom::Attrs{{"id", "background"}}), grid);
}

asmdom::VNode *SudokuGrid::render_blocks() {
    asmdom::Children result;
    for (int i = 0; i < 9; i++) {
        int x = 50 + (i % 3) * 300;
        int y = 50 + (i / 3) * 300;
        std::stringstream path;
        path << "M " << std::to_string(x) << " " << std::to_string(y) << " h 300 v 300 h -300 v -300";
        result.push_back(asmdom::h("path",
                           asmdom::Data(asmdom::Attrs{{"class", "line-thick"},
                                      {"ns",    "http://www.w3.org/2000/svg"},
                                      {"d",     path.str()},
                                      {"fill",  "none"}})));
    }
    return asmdom::h("g", asmdom::Data(asmdom::Attrs{{"id", "blocks"}}), result);
}

asmdom::VNode *SudokuGrid::Render() {
    container_ = asmdom::h("svg",
      asmdom::Data(asmdom::Attrs{{"id",      "canvas"},
                 {"xmlns",   "http://www.w3.org/2000/svg"},
                 {"viewBox", "0 0 1000 1000"},
                 {"ns",      "http://www.w3.org/2000/svg"}}),
      asmdom::Children{
              render_squares(),
              render_background(),
              render_blocks(),
      });
    return container_;
}

void SudokuGrid::UpdateSquareContent(std::string squareId) {
    int id = std::stoi(squareId);
    std::string value = std::to_string(puzzle_[id / 9][id % 9].Value());
    std::vector<std::string> cand;
    for (int i = 1; i <= 9; i++) {
        if (puzzle_[id / 9][id % 9].IsPossible(i)) {
            cand.push_back(std::to_string(i));
        } else {
            cand.push_back("");
        }
    }

    bool big_text_hidden = false;
    bool small_text_hidden = false;
    if (puzzle_[id / 9][id % 9].IsSet()) {
        small_text_hidden = true;
    } else {
        big_text_hidden = true;
    }

    squares_[id].UpdateText(big_text_hidden, value);
    squares_[id].UpdateCandidates(small_text_hidden, cand);
}

void SudokuGrid::UpdateSquareValue(std::string squareId, std::string key, InputMode mode) {
    int id = std::stoi(squareId);
    int value = key[0] - '0';
    if (mode == InputMode::CandidateInput) {
        if (puzzle_[id / 9][id % 9].IsPossible(value)) {
            puzzle_[id / 9][id % 9] -= value;
        } else {
            puzzle_[id / 9][id % 9] += value;
        }
    } else {
        if (puzzle_[id / 9][id % 9].Value() == value) {
            puzzle_[id / 9][id % 9].Reset();
        } else {
            puzzle_[id / 9][id % 9] = value;
        }
    }

    UpdateSquareContent(squareId);
    PushHistory();
}

void SudokuGrid::ResetSquare(std::string squareId) {
    int id = std::stoi(squareId);
    puzzle_[id / 9][id % 9].Reset();
    UpdateSquareContent(squareId);
    PushHistory();
}

void SudokuGrid::PushHistory() {
    std::string data = puzzle_.Serialize();
    auto history = emscripten::val::global("history");
    std::string url = std::string("?puzzle=") + data + std::string("");
    history.call<void>("pushState", emscripten::val(data), emscripten::val(""), emscripten::val(url));
}


void SudokuGrid::RestoreState(const std::string &state) {
    try {
        puzzle_.Deserialize(state);
    } catch (std::exception &e) {
        debugLogX(e.what());
    }
    UpdateAllSquares();
}

void SudokuGrid::UpdateAllSquares() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int id = i * 9 + j;
            std::string squareId = std::to_string(id);
            UpdateSquareContent(squareId);
        }
    }
}

void SudokuGrid::SolveAndUpdateSquares() {
    SolveStats stats;
    if (!SmartSolver::Solve(puzzle_, stats)) {
        debugLogX("unable to solve this puzzle");
    }
    UpdateAllSquares();
}

}