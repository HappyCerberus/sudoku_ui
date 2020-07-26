// (c) 2020 RNDr. Simon Toth (happy.cerberus@gmail.com)

#include "SudokuSquare.h"

namespace UI {
    void SudokuSquare::Render(std::string squareId, int x, int y, std::function<bool(emscripten::val)> callback) {
        x_ = x;
        y_ = y;
        square_id_ = squareId;
        candidates_ = render_candidates(squareId, x, y, "small-digit", std::vector<std::string>(9,""));
        text_ = render_square_text(squareId, x + 50, y + 75, "digit");
        background_ = render_square_background(squareId, x, y, "square");
        container_ = h("g", asmdom::Data(
                asmdom::Attrs{{"data-cell-id", squareId}},
                asmdom::Callbacks{{"onclick", callback}}),
                       asmdom::Children{background_, text_, candidates_});
    }

    void SudokuSquare::UpdateBackground(std::string css_class) {
        background_ = patch(background_, render_square_background(square_id_, x_, y_, css_class));
    }

    void SudokuSquare::UpdateText(std::string css_class, std::string value) {
        text_ = patch(text_, render_square_text(square_id_, x_ + 50, y_ + 75, css_class, value));
    }

    void SudokuSquare::UpdateCandidates(std::string css_class, const std::vector<std::string>& cand) {
        candidates_ = patch(candidates_, render_candidates(square_id_, x_, y_, css_class, cand));
    }


    asmdom::VNode *SudokuSquare::render_candidates(std::string id, int x, int y, std::string css_class, const std::vector<std::string>& values) {
        asmdom::Children candidates;
        for (int i = 0; i < 9; i++) {
            std::string subId = std::to_string(i+1);
            candidates.push_back(
                    h("text", asmdom::Data(asmdom::Attrs{
                              {"id",           std::string("square_candidate_text_") + id + "_" + subId},
                              {"ns",           "http://www.w3.org/2000/svg"},
                              {"data-cell-id", id},
                              {"data-cell-candidate-id", subId},
                              {"data-type",    std::string{"text"}},
                              {"class",        css_class},
                              {"x",            std::to_string(x+20+(i%3)*30)},
                              {"y",            std::to_string(y+30+(i/3)*30)},
                              {"text-anchor",  std::string("middle")}}),
                      values[i]));
        }
        return h("g", asmdom::Data(asmdom::Attrs{{"id", std::string("square_candidates_")+id}}),
                 candidates);
    }

    asmdom::VNode *SudokuSquare::render_square_text(std::string id, int x, int y, std::string css_class, std::string value) {
        return h("text", asmdom::Data(asmdom::Attrs{
                         {"id",           std::string("square_text_") + id},
                         {"ns",           "http://www.w3.org/2000/svg"},
                         {"data-cell-id", id},
                         {"data-type",    std::string{"text"}},
                         {"class",        css_class},
                         {"x",            std::to_string(x)},
                         {"y",            std::to_string(y)},
                         {"text-anchor",  std::string("middle")}}),
                 value);
    }

    asmdom::VNode *SudokuSquare::render_square_background(std::string id, int x, int y, std::string css_class) {
        return h("rect", asmdom::Data(asmdom::Attrs{
                {"id",           std::string("square_rect_") + id},
                {"ns",           "http://www.w3.org/2000/svg"},
                {"class",        css_class},
                {"data-cell-id", id},
                {"data-type",    std::string{"square"}},
                {"width",        "100px"},
                {"height",       "100px"},
                {"x",            std::to_string(x)},
                {"y",            std::to_string(y)},
                {"fill",         "transparent"}}));
    }

}