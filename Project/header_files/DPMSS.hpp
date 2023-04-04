#pragma once

#include <vector>
#include <queue>
#include "AllConfigs.hpp"
#include "Packet.hpp"
#include "IntervalData.hpp"
#include "commonDefines.hpp"
#include "utils.hpp"


//min priority queue
class myComparator
{
public:
    int operator() (const pair<int, int>& p1, const pair<int, int>& p2)
    {
        if(p1.first!=p2.first)
            return p1.first > p2.first;
        return p1.second > p2.second;
    }
};


// check for whether a packet can be scheduled in a given interval (intervalStart-intervalEnd)
bool packetMatchesInterval(int intervalStart, int intervalEnd, Packet &pckt, int mode, double granularity){
    double transmission_dur = calcTransmissionTimeMs(pckt.datasize, mode, pckt.stationId);
    if((pckt.arrival <= intervalStart) && ((intervalStart + transmission_dur*granularity) <= min((double)intervalEnd, pckt.deadline)) && (pckt.scheduled==false)) return true;
    return false;
}

// ----------------------------------------------------------
// -------------- Heuristic Algorithm -----------------------
// ----------------------------------------------------------

vector<int> masterconfig = {1,2,4,8,16,32,74}; // max of all possible RU configruations
int total_critical_packets = 0;
/*********************************
Given interval timestamps and the current available packets, figure out best packets for the master
config, which 
*********************************/
vector<vector<int>> selectIndicesForConfig(int itvl_start, int itvl_end, vector<int> &availablePackets, vector<int> &newAvailablePackets, vector<Packet>& packets, double granularity, double packetDropFactor, Config &userconfig){
    // re-evaluate master config
    // cout<<configs.size()<<" is configs size\n";
  
    priority_queue<pair<int, int>, vector<pair<int, int>>, myComparator> topPackets[userconfig.maxRU+1];
    // vector<int> retry(0);
    for(int& ii: availablePackets){
        Packet& pckt = packets[ii];
        bool assigned = false;
        for(int mode = RU_26; mode <= userconfig.maxRU; ++mode){
            if(packetMatchesInterval(itvl_start, itvl_end, pckt, mode, granularity) && masterconfig[mode] > 0){
                int modecpy = mode;
                if(topPackets[modecpy].size() < masterconfig[modecpy]){
                    topPackets[modecpy].push({pckt.penalty, ii});
                    assigned = true;
                    break;
                }
                Packet pckt2;
                bool assigned2 = true;
                int ii2=ii, ii3;

                if(topPackets[modecpy].top().first < pckt.penalty){
                    // pckt2 = topPackets[modecpy].top().first;
                    ii3 = topPackets[modecpy].top().second;
                    pckt2 = packets[ii3];
                    topPackets[modecpy].pop();
                    topPackets[modecpy].push({pckt.penalty, ii2});
                    ++modecpy;
                    pckt = pckt2;
                    ii2 = ii3;
                    assigned = true;
                    assigned2 = false;
                }
                while(!assigned2 && modecpy<=userconfig.maxRU){
                    if(masterconfig[modecpy] == 0) break;
                    if(topPackets[modecpy].size() < masterconfig[modecpy]){
                        topPackets[modecpy].push({pckt.penalty, ii2});
                        assigned2 = true;
                        break;
                    }else{
                        if(topPackets[modecpy].top().first < pckt.penalty){
                            // pckt2 = topPackets[modecpy].top().first;
                            ii3 = topPackets[modecpy].top().second;
                            pckt2 = packets[ii3];
                            topPackets[modecpy].pop();
                            topPackets[modecpy].push({pckt.penalty, ii2});
                            ++modecpy;
                            pckt = pckt2;
                            ii2=ii3;
                        }else{
                            break;
                        }
                    }
                }
                if(assigned && !assigned2){
                    newAvailablePackets.push_back(ii2);
                    break;
                }
            }
        }
        if(!assigned){
            newAvailablePackets.push_back(ii);
        }
    }
    vector<vector<int>> bestpckts(userconfig.maxRU+1);
    for(int mode = userconfig.maxRU; mode >= RU_26; --mode){
        while(!topPackets[mode].empty()){
            bestpckts[mode].push_back(topPackets[mode].top().second); topPackets[mode].pop();
        }
        reverse(bestpckts[mode].begin(), bestpckts[mode].end());
    }
    return bestpckts;
}

// The proposed heuristic algorithm
vector<IntervalData> DPMSS(vector<Packet>& packets, int start_time, int T, int stations_count, int criticalThreshold, double granularity, double packetDropFactor, Config &userconfig){
    const double alpha = 2;
    vector<vector<Packet>> scheduledPackets(stations_count);
    vector<int> availableIndices;  // to reference packets with integers
    cout<<"DPMSS started\n";
    for(int i = 0; i < packets.size(); ++i){
        availableIndices.push_back(i);
    }
    for(int mode = RU_26; mode <= userconfig.maxRU; ++mode){
        userconfig.masterconfig[mode] = 0;
        for(int i = 0; i < (int)configs.size(); ++i){
            userconfig.masterconfig[mode] = max(userconfig.masterconfig[mode], configs[i][mode]);
        }
    }
    vector<IntervalData> selectedIntervals;
    int delta = 5*granularity;
    vector<int> newAvailableIndices;
    // d and t are in nunits
    for(int d = 1; d <= delta; ++d){
        for(int t = start_time; t < T-d+start_time; ++t)
        {
            int prevscore = -1;
            // Interval Data creation
            int intervalStart = t, intervalEnd = t+d;         // diff = d nunits
            IntervalData currInterval(intervalStart, intervalEnd);

            newAvailableIndices.clear();   

            // greedy assignment of available packets
            vector<vector<int>> bestids = selectIndicesForConfig(intervalStart, intervalEnd, availableIndices, newAvailableIndices, packets, granularity, packetDropFactor, userconfig);
                // calc prefix sum
            vector<vector<int>> prefsum(bestids);
            for(int mode = userconfig.maxRU; mode >= RU_26; --mode){
                for(int ctr = 0; ctr < prefsum[mode].size(); ++ctr){
                    int currid = bestids[mode][ctr];
                    if(ctr == 0) prefsum[mode][ctr] = packets[currid].penalty;
                    else prefsum[mode][ctr] = prefsum[mode][ctr-1] + packets[currid].penalty;
                }
            }
            //select best configuration
            vector<int> bestconfig; int cfgscore = -1;
            for(vector<int>& config: configs){
                //calc score
                int curscore = 0;
                for(int mode = userconfig.maxRU; mode >= RU_26; --mode){
                    int till = min((int)bestids[mode].size(), config[mode]) - 1;
                    if(till < 0) continue;
                    curscore += prefsum[mode][till];
                }
                if(curscore > cfgscore){
                    cfgscore = curscore;
                    bestconfig = config;
                }
            }
            if(bestconfig.empty()){
                continue;
            }
            // fill the current interval
            currInterval.packets.clear();
            currInterval.setConfig(bestconfig);
            for(int mode = userconfig.maxRU; mode >= RU_26; --mode){
                int till = min((int)bestids[mode].size(), bestconfig[mode]) - 1;  // index till which to take packets
                if(till >= 0)
                    currInterval.updateFreeSlots(mode, bestconfig[mode] - till-1);
                int at = 0;
                for(; at <= till; ++at){
                    Packet &assignedPckt = packets[bestids[mode][at]];
                    schedulePacket(assignedPckt, mode, d);
                    // packets_scheduled2++;
                    currInterval.insert(assignedPckt);
                }
                for(; at < bestids[mode].size(); ++at){
                    newAvailableIndices.push_back(bestids[mode][at]);
                }
            }
            //overlapping intervals
            vector<IntervalData> overlapIntervals;
            vector<IntervalData> remainingIntervals;
            double overlapScore = 0;
            for(IntervalData& interval: selectedIntervals){
                if(!(interval.end <= currInterval.start || interval.start >= currInterval.end)){
                    overlapIntervals.push_back(interval);
                    overlapScore += interval.score;
                } else {
                    remainingIntervals.push_back(interval);
                }
            }
            if(alpha * (double)overlapScore < currInterval.score){
                //choose curr interval
                remainingIntervals.push_back(currInterval);
                swap(selectedIntervals, remainingIntervals);

                for(IntervalData& itvl: overlapIntervals){
                    for(Packet& pckt: itvl.packets){
                        newAvailableIndices.push_back(pckt.id);
                    }
                }
            } else {
                // reject curr interval
                for(Packet& pckt: currInterval.packets){
                    newAvailableIndices.push_back(pckt.id);
                }
            }
            swap(availableIndices, newAvailableIndices);
        }
    }

    int totalDropped = 0;
    int criticalDropped = 0;
    // final sweep (to schedule packets into free slots)
    for(int &ii: availableIndices){
        Packet& pckt = packets[ii];
        bool assigned = false;
        for(IntervalData& itvl: selectedIntervals){
            for(int mode = RU_26; mode <= userconfig.maxRU; ++mode){
                if(itvl.freeslots[mode] <= 0) continue;
                if(packetMatchesInterval(itvl.start, itvl.end, pckt, mode, granularity)){
                    itvl.freeslots[mode]--;
                    assigned = true;
                    schedulePacket(pckt, mode, itvl.end-itvl.start);
                    // packets_scheduled2++;
                    itvl.insert(pckt); break;
                }
            }
            if(assigned) break;
        }
        if(assigned == false){
            totalDropped++;
            if(pckt.penalty >= criticalThreshold){
                criticalDropped++;
            }
        }
    }
    int score = 0;

    // updated NDP stuff and score as well
    for(IntervalData& itvl: selectedIntervals){
        score += itvl.score;
        for(Packet& pckt: itvl.packets){
            transmitNDPacket(pckt, itvl.start, itvl.end);
            // coding interference:
            if(pckt.packetDropFactor < packetDropFactor){
                score -= pckt.penalty;
                totalDropped++;
                if(pckt.penalty >= critical_threshold) {
                    criticalDropped++;
                }
            }
        }
    }
    // sort intervals based on start times
	sort(selectedIntervals.begin(), selectedIntervals.end(), [](IntervalData& i1, IntervalData& i2){
		return i1.start < i2.start;
	});

    cout<<"Heuristic Score, total dropped, critical dropped:\n";
    cout<<score<<" ";
    cout<<totalDropped<<" ";
    cout<<criticalDropped<<"\n";

	return selectedIntervals;
}