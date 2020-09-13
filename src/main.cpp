/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#include "SudokuSquare.h"
#include "SudokuGrid.h"
#include "Button.h"
#include "ButtonGrid.h"

#include "asm-dom.hpp"
#include <emscripten/val.h>
#include <functional>
#include <string>
#include <sstream>
#include <libc/search.h>
#include <emscripten.h>
#include <emscripten/bind.h>


using asmdom::VNode;
using asmdom::h;
using asmdom::Attrs;
using asmdom::Data;
using asmdom::Children;
using asmdom::Callbacks;


bool onSquareClick(emscripten::val e);
bool onSquareMouseDown(emscripten::val e);
bool onSquareMouseUp(emscripten::val e);
bool onSquareMouseOver(emscripten::val e);
bool onMouseDown(emscripten::val e);
UI::SudokuGrid ui{std::vector<std::function<bool(emscripten::val)>>{onSquareClick, onSquareMouseDown, onSquareMouseUp, onSquareMouseOver}};
UI::ButtonGrid buttons{std::function<bool(emscripten::val)>(onMouseDown)};

bool onSquareClick(emscripten::val e) {
    static std::chrono::steady_clock::time_point last;
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    if (e["target"]["dataset"]["type"].isUndefined())
        return true;

    if (e["target"]["dataset"]["type"].as<std::string>() == "square" ||
        e["target"]["dataset"]["type"].as<std::string>() == "text") {
        std::string squareId = e["target"]["dataset"]["cellId"].as<std::string>();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
        if (elapsed > 200) {
            ui.FlipSquareHighlight(squareId);
            last = now;
        }
        return true;
    }
    return true;
}

bool mouse_select_dragging_ = false;

bool onSquareMouseDown(emscripten::val e) {
    mouse_select_dragging_ = true;
    return onSquareClick(e);
}

bool onSquareMouseUp(emscripten::val e) {
    mouse_select_dragging_ = false;
    return true;
}

bool onSquareMouseOver(emscripten::val e) {
    if (!mouse_select_dragging_) return true;

    for (unsigned i = 0; i < e["target"]["childElementCount"].as<unsigned>(); i++) {
        if (e["target"]["children"][i]["dataset"]["type"].isUndefined())
            continue;
        if (e["target"]["children"][i]["dataset"]["type"].as<std::string>() != "square")
            continue;

        std::string squareId = e["target"]["children"][i]["dataset"]["cellId"].as<std::string>();
        ui.FlipSquareHighlight(squareId);
    }

    return true;
}

bool onMouseDown(emscripten::val e) {
    if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "delete") {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            ui.ResetSquare(squareId);
        }
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "candidate") {
        buttons.SwitchToInputMode(UI::CandidateInput);
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "normal") {
        buttons.SwitchToInputMode(UI::NormalInput);
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>() == "solve") {
        ui.SolveAndUpdateSquares();
    } else if (e["target"]["dataset"]["buttonValue"].as<std::string>()[0] > '0' && e["target"]["dataset"]["buttonValue"].as<std::string>()[0] <= '9') {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            ui.UpdateSquareValue(squareId, e["target"]["dataset"]["buttonValue"].as<std::string>(), buttons.GetInputMode());
        }
    }
    return true;
}

bool onKeyDown(emscripten::val e) {
    if (e["key"].as<std::string>()[0] > '0' && e["key"].as<std::string>()[0] <= '9') {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            ui.UpdateSquareValue(squareId, e["key"].as<std::string>(), buttons.GetInputMode());
        }
    } else if (e["key"].as<std::string>() == "Enter") {
        ui.SolveAndUpdateSquares();
    } else if (e["key"].as<std::string>() == "Delete") {
        emscripten::val res = emscripten::val::global("document").call<emscripten::val>("getElementsByClassName",
                                                                                        emscripten::val(
                                                                                                "square-highlighted"));
        unsigned len = res["length"].as<unsigned>();
        for (unsigned i = 0; i < len; i++) {
            std::string squareId = res[i]["dataset"]["cellId"].as<std::string>();
            ui.ResetSquare(squareId);
        }
    }
    return true;
}

VNode *render_puzzle() {
    VNode *puzzle =
            h("body",
              Callbacks{{"onkeydown", onKeyDown}},
              Children{
                      ui.Render(),
                      buttons.Render(),
                      h("div", Children{
                              h("canvas",
                                Data{Attrs{{"id", "myChart"},
                                           {"width", "400"},
                                           {"height", "400"}}}),
                      }),
                      });
    return puzzle;
}

bool onpopstate(emscripten::val e) {
    debugLog(e);
    return true;
};

int main() {
    // Initialize asm-dom.
    asmdom::Config config = asmdom::Config();
    config.unsafePatch = true;
    asmdom::init(config);

    emscripten::val document = emscripten::val::global("document");
    VNode *root_view = patch(document.call<emscripten::val>("getElementById", emscripten::val("root")),
                             render_puzzle());
    emscripten::val window = emscripten::val::global("window");
    std::string data = window["location"]["search"].as<std::string>();
    size_t pos = data.find("?puzzle=");
    size_t len = strlen("?puzzle=");
    if (pos != std::string::npos && data.length() > len) {
        ui.RestoreState(data.substr(pos+len,data.length()-len-pos));
    }
    (void) root_view;

    EM_ASM(window['onpopstate'] = Module['onpopstate'];);
//    EM_ASM(initGraph(););

    return 0;
};

EMSCRIPTEN_BINDINGS(app) {
        emscripten::function("onpopstate", &onpopstate);
};

// vim: filetype=cpp
