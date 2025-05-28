#include "poliz.h"
#include <algorithm>
#include <string>
#include <iostream>

std::vector<int> labels;
std::vector<int> address_label;
std::vector<int> gotos;

void new_label(int num) {
    auto k = std::find(labels.begin(), labels.end(), num);
    if (k != labels.end()) {
        std::string s = "Попытка переопределения метки";
        throw s;
    } else {
        labels.push_back(num);
    }
}

int new_goto_label(const int buf, const int num) {
    auto k = std::find(gotos.begin(), gotos.end(), buf);
    if (k != gotos.end()) {
        return address_label[k - gotos.begin()];
    }
    if (num != 0) {
        gotos.push_back(buf);
        address_label.push_back(num);
    }
    return 0;
}
