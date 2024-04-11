#pragma once

#include <vector>
#include <queue>
#include "AllConfigs.hpp"
#include "Packet.hpp"
#include "IntervalData.hpp"
#include "commonDefines.hpp"
#include "utils.hpp"

// min priority queue
class myComparator
{
public:
    int operator()(const pair<int, int> &p1, const pair<int, int> &p2)
    {
        if (p1.first != p2.first)
            return p1.first > p2.first;
        return p1.second > p2.second;
    }
};

// check for whether a packet can be scheduled in a given interval (intervalStart-intervalEnd)
bool packetMatchesInterval(int intervalStart, int intervalEnd, Packet &pckt, int mode, double granularity, Config &userconfig, int rtLoss)
{
    double transmission_dur = calcTransmissionTimeMs(pckt.datasize, mode, pckt.stationId, userconfig.mcs_range, userconfig.mcs_lowerval, rtLoss);

    if ((pckt.arrival <= intervalStart) && ((intervalStart + transmission_dur * granularity) <= min((double)intervalEnd, pckt.deadline)) && (pckt.scheduled == false))
        return true;
    return false;
}

// ----------------------------------------------------------
// -------------- Heuristic Algorithm -----------------------
// ----------------------------------------------------------

// vector<int> masterconfig = {1,2,4,8,16,32,74}; // max of all possible RU configruations
int total_critical_packets = 0;
/*********************************
Given interval timestamps and the current available packets, figure out best packets for the master
config, which
*********************************/
vector<vector<int>> selectIndicesForConfig(int itvl_start, int itvl_end, vector<int> &availablePackets, vector<int> &newAvailablePackets, vector<Packet> &packets, double granularity, Config &userconfig, int rtLoss)
{
    // re-evaluate master config
    // cout<<configs.size()<<" is configs size\n";

    priority_queue<pair<int, int>, vector<pair<int, int>>, myComparator> topPackets[userconfig.maxRU + 1];
    for (int &ii : availablePackets)
    {
        int ii2 = ii;
        Packet pckt = packets[ii2];
        bool assigned = false;
        for (int mode = RU_26; mode <= userconfig.maxRU; ++mode)
        {
            if (userconfig.masterconfig[mode] == 0)
                break;
            if (packetMatchesInterval(itvl_start, itvl_end, pckt, mode, granularity, userconfig, rtLoss))
            {
                if (topPackets[mode].size() < userconfig.masterconfig[mode])
                {
                    topPackets[mode].push({pckt.penalty, ii2});
                    assigned = true;
                    break;
                }
                else if (topPackets[mode].top().first < pckt.penalty)
                {
                    topPackets[mode].push({pckt.penalty, ii2});
                    ii2 = topPackets[mode].top().second;
                    pckt = packets[ii2];
                    topPackets[mode].pop();
                }
            }
        }
        if (assigned == false)
        {
            newAvailablePackets.push_back(ii2);
        }
    }

    vector<vector<int>> bestpckts(userconfig.maxRU + 1);

    // print topPackets for each mode
    // for (int mode = userconfig.maxRU; mode >= RU_26; --mode)
    // {
    //     cout << "Mode = " << mode << "\n";

    //     while (!topPackets2[mode].empty())
    //     {
    //         cout << topPackets2[mode].top().first << " " << topPackets2[mode].top().second << "\n";
    //         topPackets2[mode].pop();
    //     }
    // }

    for (int mode = userconfig.maxRU; mode >= RU_26; --mode)
    {
        while (!topPackets[mode].empty())
        {
            bestpckts[mode].push_back(topPackets[mode].top().second);
            topPackets[mode].pop();
        }
        reverse(bestpckts[mode].begin(), bestpckts[mode].end());
    }
    // drop packets according to configs_dropfactor array

    return bestpckts;
}

void createMasterConfig(Config &userconfig)
{
    for (int mode = RU_26; mode <= userconfig.maxRU; ++mode)
    {
        userconfig.masterconfig[mode] = 0;
        for (int i = 0; i < (int)configs.size(); ++i)
        {
            userconfig.masterconfig[mode] = max(userconfig.masterconfig[mode], configs[i][mode]);
        }
    }
}

// write a function to drop packetDropFactor percent of packets randomly from the interval
void dropPackets(IntervalData &itvl, double packetDropFactor, int critical_threshold, int &criticalDropped, int &totalDropped)
{
    int totalPackets = itvl.packets.size();
    if (packetDropFactor >= 0)
    {
        int packetsToDrop = (int)(packetDropFactor * (double)totalPackets);
        for (int i = 0; i < packetsToDrop; ++i)
        {
            int idx = rand() % totalPackets;
            Packet &pckt = itvl.packets[idx];
            // itvl.packets.erase(itvl.packets.begin() + idx);

            // write a custom erase function to remove the packet from the interval
            for (int i = idx; i < itvl.packets.size() - 1; i++)
            {
                itvl.packets[i] = itvl.packets[i + 1];
            }
            itvl.packets.pop_back();
            totalDropped++;
            if (pckt.penalty >= critical_threshold)
            {
                criticalDropped++;
            }
            itvl.score -= pckt.penalty;
        }
    }
}
// The proposed heuristic algorithm
vector<IntervalData> LSDS(vector<Packet> &packets, int start_time, int T, int stations_count, int criticalThreshold, double granularity, double packetDropFactor, Config &userconfig, int maxScorePossible, int fsi, int rtLoss)
{
    const double alpha = 2;
    vector<vector<Packet>> scheduledPackets(stations_count);
    vector<int> availableIndices; // to reference packets with integers

    for (int i = 0; i < packets.size(); ++i)
    {
        availableIndices.push_back(i);
    }
    createMasterConfig(userconfig);
    vector<vector<vector<int>>> ru_packets(userconfig.maxRU + 1, vector<vector<int>>());
    for (int i = RU_26; i <= userconfig.maxRU; ++i)
    {
        ru_packets[i].resize(userconfig.masterconfig[i]);
    }
    // for (int i = 0; i < userconfig.masterconfig.size(); ++i)
    // {
    //     cout << userconfig.masterconfig[i] << " ";
    // }
    // cout<<userconfig.maxRU<<"\n";
    vector<IntervalData> selectedIntervals;
    int delta = 5 * granularity;
    vector<int> newAvailableIndices;
    // d and t are in nunits
    for (int d = 1; d <= delta; ++d)
    {
        for (int t = start_time; t < T - d + start_time; ++t)
        {
            int prevscore = -1;
            // Interval Data creation
            int intervalStart = t, intervalEnd = t + d; // diff = d nunits
            IntervalData currInterval(intervalStart, intervalEnd, userconfig.maxRU + 1);
            currInterval.best_config_packets.resize(userconfig.maxRU + 1, vector<int>());

            newAvailableIndices.clear();

            // greedy assignment of available packets
            vector<vector<int>> bestids = selectIndicesForConfig(intervalStart, intervalEnd, availableIndices, newAvailableIndices, packets, granularity, userconfig, rtLoss);

            // calc prefix sum
            vector<vector<int>> prefsum(bestids);

            for (int mode = userconfig.maxRU; mode >= RU_26; --mode)
            {
                for (int ctr = 0; ctr < prefsum[mode].size(); ++ctr)
                {
                    int currid = bestids[mode][ctr];
                    if (ctr == 0)
                    {
                        prefsum[mode][ctr] = packets[currid].penalty;
                    }
                    else
                    {
                        prefsum[mode][ctr] = prefsum[mode][ctr - 1] + packets[currid].penalty;
                    }
                }
            }
            // select best configuration
            vector<int> bestconfig;
            vector<vector<int>> temp_config_packets(userconfig.maxRU + 1, vector<int>());
            vector<vector<int>> bestconfig_packets(userconfig.maxRU + 1, vector<int>());
            int cfgscore = -1;
            for (vector<int> &config : configs)
            {
                // calc score
                for (int i = RU_26; i <= userconfig.maxRU; ++i)
                {
                    temp_config_packets[i].clear();
                }
                int curscore = 0;
                for (int mode = userconfig.maxRU; mode >= RU_26; --mode)
                {
                    int till = min((int)bestids[mode].size(), config[mode]) - 1;
                    if (till < 0)
                        continue;
                    for (int i = 0; i <= till; ++i)
                    {
                        temp_config_packets[mode].push_back(bestids[mode][i]);
                    }
                    curscore += prefsum[mode][till];
                }
                if (curscore > cfgscore)
                {
                    cfgscore = curscore;
                    bestconfig = config;
                    for (int i = RU_26; i <= userconfig.maxRU; ++i)
                    {
                        bestconfig_packets[i] = temp_config_packets[i];
                    }
                }
            }
            if (bestconfig.empty())
            {
                continue;
            }

            // fill the current interval
            currInterval.packets.clear();
            currInterval.setConfig(bestconfig);
            for (int mode = userconfig.maxRU; mode >= RU_26; --mode)
            {
                int till = min((int)bestids[mode].size(), bestconfig[mode]) - 1; // index till which to take packets
                if (till >= 0)
                    currInterval.updateFreeSlots(mode, bestconfig[mode] - till - 1);
                int at = 0;
                for (; at <= till; ++at)
                {
                    Packet &assignedPckt = packets[bestids[mode][at]];
                    currInterval.best_config_packets[mode].push_back(bestids[mode][at]);
                    schedulePacket(assignedPckt, mode, d);
                    // packets_scheduled2++;
                    currInterval.insert(assignedPckt);
                }
                for (; at < bestids[mode].size(); ++at)
                {
                    newAvailableIndices.push_back(bestids[mode][at]);
                }
            }
            // overlapping intervals
            vector<IntervalData> overlapIntervals;
            vector<IntervalData> remainingIntervals;
            double overlapScore = 0;
            for (IntervalData &interval : selectedIntervals)
            {
                if (!(interval.end <= currInterval.start || interval.start >= currInterval.end))
                {
                    overlapIntervals.push_back(interval);
                    overlapScore += interval.score;
                }
                else
                {
                    remainingIntervals.push_back(interval);
                }
            }
            if (alpha * (double)overlapScore < currInterval.score)
            {
                // choose curr interval
                remainingIntervals.push_back(currInterval);
                swap(selectedIntervals, remainingIntervals);
                for (IntervalData &itvl : overlapIntervals)
                {
                    for (Packet &pckt : itvl.packets)
                    {
                        newAvailableIndices.push_back(pckt.id);
                    }
                }
            }
            else
            {
                // reject curr interval
                for (Packet &pckt : currInterval.packets)
                {
                    newAvailableIndices.push_back(pckt.id);
                }
            }
            swap(availableIndices, newAvailableIndices);
        }
    }

    int totalDropped = 0;
    int criticalDropped = 0;
    // final sweep (to schedule packets into free slots)
    for (int &ii : availableIndices)
    {
        Packet &pckt = packets[ii];
        bool assigned = false;
        for (IntervalData &itvl : selectedIntervals)
        {
            for (int mode = RU_26; mode <= userconfig.maxRU; ++mode)
            {
                if (itvl.freeslots[mode] <= 0)
                    continue;
                if (packetMatchesInterval(itvl.start, itvl.end, pckt, mode, granularity, userconfig, rtLoss))
                {
                    itvl.freeslots[mode]--;
                    itvl.best_config_packets[mode].push_back(ii);
                    assigned = true;
                    schedulePacket(pckt, mode, itvl.end - itvl.start);
                    // packets_scheduled2++;
                    itvl.insert(pckt);
                    break;
                }
            }
            if (assigned)
                break;
        }
        if (assigned == false)
        {
            totalDropped++;
            if (pckt.penalty >= criticalThreshold)
            {
                criticalDropped++;
            }
        }
    }
    int score = 0;

    // updated NDP stuff and score as well

    for (IntervalData &itvl : selectedIntervals)
    {
        dropPackets(itvl, packetDropFactor, criticalThreshold, criticalDropped, totalDropped);
        for (int i = RU_26; i <= userconfig.maxRU; ++i)
        {
            for (int j = 0; j < itvl.best_config_packets[i].size(); ++j)
            {
                ru_packets[i][j].push_back(itvl.best_config_packets[i][j]);
            }
        }
        score += itvl.score;
        // write a function to drop 10% of packets randomly from the interval
        for (Packet &pckt : itvl.packets)
        {
            transmitNDPacket(pckt, itvl.start, itvl.end);
        }
    }

    if (fsi)
    {
        double drop_factor = 0;
        for (int i = RU_26; i <= userconfig.maxRU; ++i)
        {
            for (int j = 0; j < ru_packets[i].size(); ++j)
            {
                drop_factor = configs_dropfactor[i][j];
                int packetsToDrop = (int)((drop_factor * ru_packets[i][j].size())+0.5);
                for (int k = 0; k < packetsToDrop; ++k)
                {
                    int idx = rand() % ru_packets[i][j].size();
                    Packet &pckt = packets[ru_packets[i][j][idx]];
                    for (int l = idx; l < ru_packets[i][j].size() - 1; l++)
                    {
                        ru_packets[i][j][l] = ru_packets[i][j][l + 1];
                    }
                    ru_packets[i][j].pop_back();
                    totalDropped++;
                    if (pckt.penalty >= criticalThreshold)
                    {
                        criticalDropped++;
                    }
                    score -= pckt.penalty;
                }
            }
        }
    }
    // sort intervals based on start times
    sort(selectedIntervals.begin(), selectedIntervals.end(), [](IntervalData &i1, IntervalData &i2)
         { return i1.start < i2.start; });

    // cout << "LSDS Score, total dropped, critical dropped:\n";
    // cout << score << " ";
    // cout << ((double)(totalDropped - criticalDropped)) / total_packets << " ";
    // cout << ((double)criticalDropped) / total_packets << " ";
    cout << (double)score / (double)maxScorePossible << " ";

    return selectedIntervals;
}