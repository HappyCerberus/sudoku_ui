/* (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com) */

#include "Button.h"

namespace UI {

asmdom::VNode *Button::Render(bool selected) {
    //std::string buttonId = std::to_string(i);
    //int x = 50+((i-1)%3)*100;
    //int y = 50+((i-1)/3)*100;

    text_ = RenderText();
    background_ = RenderBackground(selected);

    container_ = asmdom::h("g", asmdom::Children{text_, background_});
    return container_;
}

asmdom::VNode *Button::RenderText(InputMode mode) const {
    switch (mode) {
        case InputMode::NormalInput:
            return asmdom::h("text", asmdom::Data(asmdom::Attrs{
                                     {"id",                std::string("button_text_") + buttonID_},
                                     {"ns",                "http://www.w3.org/2000/svg"},
                                     {"data-button-value", buttonID_},
                                     {"data-type",         std::string{"text"}},
                                     {"class",             "button-text"},
                                     {"x",                 std::to_string(x_ + 50 + 30 * (w_ / 100))},
                                     {"y",                 std::to_string(y_ + 75)},
                                     {"text-anchor",       std::string("middle")}}),
                             buttonText_);
        case InputMode::CandidateInput:
            int num = std::stoi(buttonID_);
            return asmdom::h("text", asmdom::Data(asmdom::Attrs{
                                     {"id",                std::string("button_text_") + buttonID_},
                                     {"ns",                "http://www.w3.org/2000/svg"},
                                     {"data-button-value", buttonID_},
                                     {"data-type",         std::string{"text"}},
                                     {"class",             "button-text-small"},
                                     {"x",                 std::to_string(x_+30+((num-1)%3)*20)},
                                     {"y",                 std::to_string(y_+40+((num-1)/3)*20)},
                                     {"text-anchor",       std::string("middle")}}),
                             buttonText_);
    }

}

asmdom::VNode *Button::RenderBackground(bool selected) const {
    std::string css_class = "button";
    if (selected)
        css_class = "button-selected";

    std::string width = std::to_string(w_)+"px";

    return asmdom::h("rect",
                         asmdom::Data(
                                        asmdom::Attrs{
                                                {"class",             css_class},
                                                {"data-button-value", buttonID_},
                                                {"x",                 std::to_string(x_ + 10)},
                                                {"y",                 std::to_string(y_ + 10)},
                                                {"width",             width},
                                                {"height",            "80px"},
                                                {"rx",                "10px"},
                                                {"ry",                "10px"}
                                        },
                                        asmdom::Callbacks{
                                                {"onmousedown", callback_}
                                        }));
}

void Button::UpdateSelectState(bool selected) {
    background_ = patch(background_, RenderBackground(selected));
}

void Button::UpdateInputMode(InputMode mode) {
    text_ = patch(text_, RenderText(mode));
}

}