//
// Created by Happy on 12/09/2020.
//

#ifndef SUDOKU_UI_UTIL_H
#define SUDOKU_UI_UTIL_H

#include <string>
#include <emscripten/val.h>

void debugLogX(std::string s);
bool debugLog(emscripten::val e);
std::string debugString(emscripten::val e);

#endif //SUDOKU_UI_UTIL_H
