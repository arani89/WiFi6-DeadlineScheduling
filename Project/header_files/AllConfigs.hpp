#pragma once

#include <vector>
#include "commonDefines.hpp"
using namespace std;

vector<vector<int>> configs;


void createRUConfigs(int curr, vector<int>& config){
    //Print
    

    vector<int> tempfigs(config);
    configs.push_back(tempfigs);
    //try divisions
    for(int times = 1; times <= config[curr]; times++){
        config[curr] -= times;
        if(curr == RU_1992){
            config[RU_996] += times * 2;
            createRUConfigs(curr+1, config);
            config[RU_996] -= times * 2;
        }else if(curr == RU_996){
            config[RU_484] += times * 2;
            config[RU_26] += times;
            createRUConfigs(curr+1, config);
            config[RU_484] -= times * 2;
            config[RU_26] -= times;
        } else if(curr == RU_484){
            config[RU_242] += times * 2;
            createRUConfigs(curr+1, config);
            config[RU_242] -= times * 2;
        } else if(curr == RU_242){
            config[RU_106] += 2*times;
            config[RU_26] += times;
            createRUConfigs(curr+1, config);
            config[RU_26] -= times;
            config[RU_106] -= 2*times;
        } else if(curr == RU_106){
            config[RU_52] += 2*times;
            createRUConfigs(curr+1, config);
            config[RU_52] -= 2*times;
        } else if(curr == RU_52){
            config[RU_26] += 2*times;
            createRUConfigs(curr+1, config);
            config[RU_26] -= 2*times;
        } else if(curr == RU_26){
            //cannot divide more
        }
        config[curr] += times;
    }
}