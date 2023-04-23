#pragma once

#include <vector>
#include "commonDefines.hpp"
using namespace std;

const vector<vector<double>> bitvalues = {
    {0.8, 1.7, 3.5, 8.1, 16.3, 34},
    {1.7, 3.3, 7.1, 16.3, 32.5, 68.1},
    {2.5, 5, 10.6, 24.4, 48.8, 102.1},
    {3.3, 6.7, 14.2, 32.5, 65, 136.1},
    {5, 10, 21.3, 48.8, 97.5, 204.2},
    {6.7, 13.3, 28.3, 65, 130, 272.2},
    {7.5, 15, 31.9, 73.1, 146.3, 306.3},
    {8.3, 16.7, 35.4, 81.3, 162.5, 340.3},
    {10, 20, 42.5, 97.5, 195, 408.3},
    {11.1, 22.2, 47.2, 108.3, 216.7, 453.7},
    {11.1, 22.2, 47.2, 121.9, 243.8, 510.4},
    {11.1, 22.2, 47.2, 135.4, 270.8, 576.1}};

double calcTransmissionTimeMs(double size, int mode, int bitmode, int mcs_range, int mcs_lowerval)
{
    double bitrate;
    bitmode = (bitmode % mcs_range)+mcs_lowerval;
    if (mode == RU_26)
        bitrate = bitvalues[bitmode][0];
    else if (mode == RU_52)
        bitrate = bitvalues[bitmode][1];
    else if (mode == RU_106)
        bitrate = bitvalues[bitmode][2];
    else if (mode == RU_242)
        bitrate = bitvalues[bitmode][3];
    else if (mode == RU_484)
        bitrate = bitvalues[bitmode][4];
    else if (mode == RU_996)
        bitrate = bitvalues[bitmode][5];
    else if (mode == RU_1992)
        bitrate = 2 * bitvalues[bitmode][5];
    else
        bitrate = -1;

    double res = (size * 8.0) / (bitrate * 1000.0);
    return res;
}

int dataTransferrableBytes(int mode, int bitmode, double timeMs)
{
    double bitrate;
    bitmode = (bitmode % 1) + 11;
    if (mode == RU_26)
        bitrate = bitvalues[bitmode][0];
    else if (mode == RU_52)
        bitrate = bitvalues[bitmode][1];
    else if (mode == RU_106)
        bitrate = bitvalues[bitmode][2];
    else if (mode == RU_242)
        bitrate = bitvalues[bitmode][3];
    else if (mode == RU_484)
        bitrate = bitvalues[bitmode][4];
    else if (mode == RU_996)
        bitrate = bitvalues[bitmode][5];
    else if (mode == RU_1992)
        bitrate = 2 * bitvalues[bitmode][5];
    else
        bitrate = -1;

    double size = ((bitrate * 1000.0) * timeMs) / 8.0;
    int sz = (int)size;
    return sz;
}