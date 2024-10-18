#pragma once


template <typename T>
bool checkInRange(T x, T min, T max)
{
    return min <= x && x <= max;
}
