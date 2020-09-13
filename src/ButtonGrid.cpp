/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#include "ButtonGrid.h"
#include "Button.h"

namespace UI {

asmdom::VNode *ButtonGrid::render_buttons() {
    asmdom::Children grid;
    for (int i = 1; i <= 9; i++) {
        std::string buttonId = std::to_string(i);
        unsigned x = 50+((i-1)%3)*100;
        unsigned y = 50+((i-1)/3)*100;
        buttons_.emplace(buttonId, Button(x, y, 80, buttonId, buttonId, callback_));
        grid.push_back(buttons_.at(buttonId).Render());
    }

    unsigned x = 150;
    unsigned y = 370;
    std::string buttonId = "normal";
    buttons_.emplace(buttonId, Button(x, y, 80, buttonId, std::string("✒"), callback_));

    x = 250;
    y = 370;
    buttonId = "candidate";
    buttons_.emplace(buttonId, Button(x, y, 80, buttonId, std::string("✏"), callback_));

    x = 50;
    y = 370;
    buttonId = "delete";
    buttons_.emplace(buttonId, Button(x, y, 80, buttonId, std::string("🗑"), callback_));

    grid.push_back(buttons_.at("normal").Render(true));
    grid.push_back(buttons_.at("candidate").Render());
    grid.push_back(buttons_.at("delete").Render());

    x = 50;
    y = 470;
    buttonId = "solve";
    buttons_.emplace(buttonId, Button(x, y, 280, buttonId, std::string("Solve"), callback_));

    grid.push_back(buttons_.at("solve").Render());

    return asmdom::h("g", asmdom::Data(asmdom::Attrs{{"id", "button_grid"}}), grid);
}


asmdom::VNode *ButtonGrid::Render() {
    container_ = asmdom::h("svg",
                           asmdom::Data(asmdom::Attrs{{"id", "buttons"},
                 {"xmlns",   "http://www.w3.org/2000/svg"},
                 {"ns",      "http://www.w3.org/2000/svg"},
                 {"viewBox", "0 0 400 800"}}),
                           asmdom::Children({
                       render_buttons(),
               })
    );
    return container_;
}

void ButtonGrid::SwitchToInputMode(InputMode target) {
    mode_ = target;
    switch(target) {
        case CandidateInput:
            buttons_.at("normal").UpdateSelectState(false);
            buttons_.at("candidate").UpdateSelectState(true);
            for (unsigned i = 1; i <= 9; i++) {
                buttons_.at(std::to_string(i)).UpdateInputMode(CandidateInput);
            }
            break;
        case NormalInput:
            buttons_.at("normal").UpdateSelectState(true);
            buttons_.at("candidate").UpdateSelectState(false);
            for (unsigned i = 1; i <= 9; i++) {
                buttons_.at(std::to_string(i)).UpdateInputMode(NormalInput);
            }
            break;
    }
}

}