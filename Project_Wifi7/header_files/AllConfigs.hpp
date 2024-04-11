#pragma once

#include <vector>
#include "commonDefines.hpp"
using namespace std;

vector<vector<int>> configs;
vector<vector<double>> configs_dropfactor;

void createRUDropFactor(Config &userconfig)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> distribution(0.1, 0.2);
    vector<double> temp;
    for (int i = 0; i < userconfig.masterconfig[0]; ++i)
    {
        temp.push_back(distribution(gen));
    }
    configs_dropfactor.push_back(temp);
    temp.clear();
    temp.resize(userconfig.masterconfig[1]);
    for (int j = 0; j < userconfig.masterconfig[0]; j += 2)
    {
        if (j == 4 || j == 13 || j == 18 || j == 23 || j == 32)
        {
            ++j;
            continue;
        }
        if(j > 32){
            temp[(j - 4) / 2] = max(configs_dropfactor[0][j], configs_dropfactor[0][(j + 1)]);
            continue;
        }
        else if(j > 23){
            temp[(j - 3) / 2] = max(configs_dropfactor[0][j], configs_dropfactor[0][(j + 1)]);
            continue;
        }
        else if (j > 18)
        {
            temp[(j - 2) / 2] = max(configs_dropfactor[0][j], configs_dropfactor[0][(j + 1)]);
            continue;
        }
        else if (j > 13)
        {
            temp[(j - 1) / 2] = max(configs_dropfactor[0][j], configs_dropfactor[0][(j + 1)]);
            continue;
        }
        else
        {
            temp[j / 2] = max(configs_dropfactor[0][j], configs_dropfactor[0][(j + 1)]);
        }
    }
    configs_dropfactor.push_back(temp);
    temp.clear();
    for (int i = 2; i < userconfig.masterconfig.size(); ++i)
    {
        temp.resize(userconfig.masterconfig[i]);
        for (int j = 0; j < userconfig.masterconfig[i - 1]; j += 2)
        {
            temp[j / 2] = max(configs_dropfactor[i - 1][j], configs_dropfactor[i - 1][(j + 1)]);
        }
        configs_dropfactor.push_back(temp);
        temp.clear();
    }
}

void createRUConfigs(int curr, Config &userconfig)
{
    // Print the config
    vector<int> tempfigs(userconfig.config);
    configs.push_back(tempfigs);
    // try divisions
    for (int times = 1; times <= userconfig.config[curr]; times++)
    {
        userconfig.config[curr] -= times;
        if (curr == RU_1992)
        {
            userconfig.config[RU_996] += times * 2;
            createRUConfigs(curr - 1, userconfig);
            userconfig.config[RU_996] -= times * 2;
        }
        else if (curr == RU_996)
        {
            userconfig.config[RU_484] += times * 2;
            userconfig.config[RU_26] += times;
            createRUConfigs(curr - 1, userconfig);
            userconfig.config[RU_484] -= times * 2;
            userconfig.config[RU_26] -= times;
        }
        else if (curr == RU_484)
        {
            userconfig.config[RU_242] += times * 2;
            createRUConfigs(curr - 1, userconfig);
            userconfig.config[RU_242] -= times * 2;
        }
        else if (curr == RU_242)
        {
            userconfig.config[RU_106] += 2 * times;
            userconfig.config[RU_26] += times;
            createRUConfigs(curr - 1, userconfig);
            userconfig.config[RU_26] -= times;
            userconfig.config[RU_106] -= 2 * times;
        }
        else if (curr == RU_106)
        {
            userconfig.config[RU_52] += 2 * times;
            createRUConfigs(curr - 1, userconfig);
            userconfig.config[RU_52] -= 2 * times;
        }
        else if (curr == RU_52)
        {
            userconfig.config[RU_26] += 2 * times;
            createRUConfigs(curr - 1, userconfig);
            userconfig.config[RU_26] -= 2 * times;
        }
        else if (curr == RU_26)
        {
            // cannot divide more
        }
        userconfig.config[curr] += times;
    }
}