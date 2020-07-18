// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "asm-dom.hpp"
#include <emscripten/val.h>
#include <functional>
#include <string>

using namespace asmdom;

asmdom::VNode *render_squares() {
    asmdom::Children result;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int x = 50 + j * 100;
            int y = 50 + i * 100;
            result.push_back(h("rect", Data(Attrs{{"width",  "100px"},
                                                  {"height", "100px"},
                                                  {"x",      std::to_string(x)},
                                                  {"y",      std::to_string(y)},
                                                  {"fill",   "transparent"}})));
        }
    }
    return h("g", Data(Attrs{{"id", "squares"}}), result);
}

asmdom::VNode *render_blocks() {
    asmdom::Children result;
    for (int i = 0; i < 9; i++) {
        int x = 50 + (i % 3) * 300;
        int y = 50 + (i / 3) * 300;
        int width = 300;
        int height = 300;
        result.push_back(h("rect",
                           Data(Attrs{{"class",  "line-thick"},
                                      {"fill",   "transparent"},
                                      {"ns",     "http://www.w3.org/2000/svg"},
                                      {"x",      std::to_string(x)},
                                      {"y",      std::to_string(y)},
                                      {"width",  std::to_string(width)},
                                      {"height", std::to_string(height)}})));
    }
    return h("g", Data(Attrs{{"id", "blocks"}}), result);
}

asmdom::VNode *render_background() {
    // TODO: generate path, so it works for all sizes of Sudoku
    asmdom::Children grid{
            h("path", Data(Attrs{{"class", "line"},
                                 {"d",     "M 50 50 h 900 M 50 150 h 900 M 50 250 h 900 M 50 350 h 900 M 50 450 h 900 M 50 550 h 900 M 50 650 h 900 M 50 750 h 900 M 50 850 h 900 M 50 950 h 900"}})),
            h("path", Data(Attrs{{"class", "line"},
                                 {"d",     "M 50 50 v 900 M 150 50 v 900 M 250 50 v 900 M 350 50 v 900 M 450 50 v 900 M 550 50 v 900 M 650 50 v 900 M 750 50 v 900 M 850 50 v 900 M 950 50 v 900"}})),
            h("rect", Data(Attrs{{"class",  "line-thick"},
                                 {"x",      "50"},
                                 {"y",      "50"},
                                 {"width",  "900"},
                                 {"height", "900"},
                                 {"fill",   "transparent"}}))
    };
    return h("g", Data(Attrs{{"id", "background"}}), grid);
}

asmdom::VNode *render_puzzle() {
    asmdom::VNode *puzzle = h("svg",
                              Data(Attrs{{"id",      "root"},
                                         {"xmlns",   "http://www.w3.org/2000/svg"},
                                         {"viewBox", "0 0 1000 1000"},
                                         {"ns",      "http://www.w3.org/2000/svg"}}),
                              Children{
                                      render_squares(),
                                      render_background(),
                                      render_blocks(),
                              });
    return puzzle;
}

int main() {
    // Initialize asm-dom.
    asmdom::Config config = asmdom::Config();
    asmdom::init(config);

    // Replace <div id="root"/> by our virtual dom.
    emscripten::val document = emscripten::val::global("document");
    VNode *root_view = patch(document.call<emscripten::val>("getElementById", emscripten::val("root")), render_puzzle());

    return 0;
};

// vim: filetype=cpp
