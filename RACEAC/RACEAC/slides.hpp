#pragma once
#include <iostream>

template <class T>
constexpr std::vector<T> generate_slide(size_t size, T filler)
{
    std::vector<T> slide;
    slide.reserve(size);
    for (auto i = 0; i < size; ++i) slide.push_back(filler);
    return slide;
}