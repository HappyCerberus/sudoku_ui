//
// Created by Happy on 12/09/2020.
//

#include "util.h"

std::string debugString(emscripten::val e) {
    if (e.isString()) {
        return e.as<std::string>();
    } else if (e.isArray()) {
        return "is an array";
    } else if (e.isNull()) {
        return "is null";
    } else if (e.isNumber()) {
        return std::to_string(e.as<double>());
    } else if (e.isUndefined()) {
        return "is undefined";
    } else if (e.isTrue()) {
        return "is true";
    } else if (e.isFalse()) {
        return "is false";
    } else {
        return "something else";
    }
}


bool debugLog(emscripten::val e) {
    emscripten::val::global("console").call<void>("log", e);
    return true;
}

void debugLogX(std::string s) {
    debugLog(emscripten::val(s));
}