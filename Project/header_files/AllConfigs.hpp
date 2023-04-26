#pragma once

#include <vector>
#include "commonDefines.hpp"
using namespace std;

vector<vector<int>> configs;


void createRUConfigs(int curr, Config &userconfig){
    //Print the config
    vector<int> tempfigs(userconfig.config);
    configs.push_back(tempfigs);
    //try divisions
    for(int times = 1; times <= userconfig.config[curr]; times++){
        userconfig.config[curr] -= times;
        if(curr == RU_1992){
            userconfig.config[RU_996] += times * 2;
            createRUConfigs(curr-1, userconfig);
            userconfig.config[RU_996] -= times * 2;
        }else if(curr == RU_996){
            userconfig.config[RU_484] += times * 2;
            userconfig.config[RU_26] += times;
            createRUConfigs(curr-1, userconfig);
            userconfig.config[RU_484] -= times * 2;
            userconfig.config[RU_26] -= times;
        } else if(curr == RU_484){
            userconfig.config[RU_242] += times * 2;
            createRUConfigs(curr-1, userconfig);
            userconfig.config[RU_242] -= times * 2;
        } else if(curr == RU_242){
            userconfig.config[RU_106] += 2*times;
            userconfig.config[RU_26] += times;
            createRUConfigs(curr-1, userconfig);
            userconfig.config[RU_26] -= times;
            userconfig.config[RU_106] -= 2*times;
        } else if(curr == RU_106){
            userconfig.config[RU_52] += 2*times;
            createRUConfigs(curr-1, userconfig);
            userconfig.config[RU_52] -= 2*times;
        } else if(curr == RU_52){
            userconfig.config[RU_26] += 2*times;
            createRUConfigs(curr-1, userconfig);
            userconfig.config[RU_26] -= 2*times;
        } else if(curr == RU_26){
            //cannot divide more
        }
        userconfig.config[curr] += times;
    }
}