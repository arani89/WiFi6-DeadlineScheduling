#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include "AllConfigs.hpp"
#include "Packet.hpp"
#include "commonDefines.hpp"
#include "utils.hpp"

using namespace std;

/*
Give the current packets, the time period, the stations_count
*/

void baselineNLRF(vector<Packet> &packets, int start_time, int T, int stations_count, int criticalThreshold, double packetDropFactor, Config &userconfig, int maxScorePossible, int rtLoss )
{
    vector<int> packetsTransmitted(stations_count, 0);
    vector<int> packetsGenTillNow(stations_count, 0);
    vector<vector<Packet>> scheduledPackets(stations_count);
    // O(n^2 log n)
    int score = 0, totalDropped = 0, criticalDropped = 0;

    for (auto &pckt : packets)
    {
        totalDropped++;
        if (pckt.penalty >= criticalThreshold)
            criticalDropped++;
    }

    int lenPackets = packets.size();
    double lastend = start_time; // time
    for (int i = 0; i < lenPackets;)
    {
        packetsGenTillNow.assign(stations_count, 0);
        for (auto &pckt : packets)
        {
            if (pckt.arrival <= lastend)
                packetsGenTillNow[pckt.stationId]++;
        }
        // cout<<"Baseline NLRF3\n";
        // sort based on penalty / deadline
        sort(packets.begin() + i, packets.end(), [&](const Packet &p1, const Packet &p2)
             { return (double)(p1.penalty) * (double)(packetsGenTillNow[p1.stationId] + 1) / (double)(p1.deadline * (packetsTransmitted[p1.stationId] + 1)) >
                      (double)(p2.penalty) * (double)(packetsGenTillNow[p2.stationId] + 1) / (double)(p2.deadline * (packetsTransmitted[p2.stationId] + 1)); });
        while (lastend >= packets[i].deadline)
        {
            ++i;
            if (i >= lenPackets)
                break;
        }
        if (i >= lenPackets)
            break;
        int maxscore = -1;
        vector<int> cfig;
        double newend = lastend;
        double newstart = max(lastend, packets[i].arrival);
        // iterate through all configurations
        for (auto copyconfig : configs)
        {
            vector<int> config(copyconfig.begin(), copyconfig.end());
            int m = 0;
            for (int j = RU_26; j <= userconfig.maxRU; ++j)
            {
                m += config[j];
            }
            // cout<<m<<" : ";
            int windowsize = 0;
            int curscore = 0;
            int curdrops = 0;
            // try to fit first m packets if possible
            for (int j = i; j < min(i + m, lenPackets); ++j)
            {
                Packet &pckt = packets[j];
                if (pckt.arrival > newstart)
                    continue;
                bool assigned = false;
                for (int mode = RU_26; mode <= userconfig.maxRU; ++mode)
                {
                    if ((config[mode] > 0) && (calcTransmissionTimeMs(pckt.datasize, mode, pckt.stationId, userconfig.mcs_range, userconfig.mcs_lowerval, rtLoss) + newstart <= min(pckt.deadline, (double)T - 1)))
                    {
                        config[mode]--;
                        assigned = true;
                        windowsize = max(windowsize, (int)ceil(calcTransmissionTimeMs(pckt.datasize, mode, pckt.stationId, userconfig.mcs_range, userconfig.mcs_lowerval, rtLoss)));
                        break;
                    }
                }
                if (assigned)
                {
                    curscore += pckt.penalty;
                }
            }
            // see if this score is better than the previous ones
            if ((curscore > maxscore) || ((curscore == maxscore) && (newend > (newstart + windowsize))))
            {
                cfig = copyconfig;
                newend = newstart + windowsize;
                maxscore = curscore;
            }
        }
        lastend = newend;
        if (newend >= (double)(T + start_time))
            break;
        // update max score
        //  cout<<newstart<<" "<<newend<<" "<<maxscore<<"\n";
        score += max(maxscore, 0);
        // -----
        if (cfig.size() == 0)
            ++i;
        else
        {
            // figure out the packets that were assigned here
            int m = 0;
            for (int j = RU_26; j <= userconfig.maxRU; ++j)
            {
                m += cfig[j];
            }
            // try to fit first m packets if possible
            for (int j = i; j < min(i + m, lenPackets); ++j)
            {
                Packet &pckt = packets[j];
                if (pckt.arrival > newstart)
                    continue;
                bool assigned = false;
                for (int mode = RU_26; mode <= userconfig.maxRU; ++mode)
                {
                    if ((cfig[mode] > 0) && calcTransmissionTimeMs(pckt.datasize, mode, pckt.stationId, userconfig.mcs_range, userconfig.mcs_lowerval, rtLoss) + newstart <= min(pckt.deadline, (double)T - 1))
                    {
                        cfig[mode]--;
                        assigned = true;
                        break;
                    }
                }
                if (assigned)
                {
                    // cout<<(pckt.stationId/10)<<" ";
                    packetsTransmitted[pckt.stationId] += 1;
                    scheduledPackets[pckt.stationId].push_back(pckt);
                    // if assigned then remove from dropped packets
                    totalDropped--;
                    if (pckt.penalty >= criticalThreshold)
                        criticalDropped--;
                }
            }
            i += m;
        }
    }
    //randomly remove packetDropFactor% packets from scheduledPackets
    for (int i = 0; i < stations_count; i++)
    {
        int len = scheduledPackets[i].size();
        int todrop = (int)(packetDropFactor * len);
        for (int j = 0; j < todrop; j++)
        {
            int index = rand() % scheduledPackets[i].size();
            Packet &pckt = scheduledPackets[i][index];
            totalDropped++;
            score -= pckt.penalty;
            if (pckt.penalty >= criticalThreshold)
                criticalDropped++;
            for(int k=index; k<len-1; k++){
                scheduledPackets[i][k] = scheduledPackets[i][k+1];
            }
            // itvl.packets.pop_back();
            scheduledPackets[i].pop_back();
        }
    }
    cout << "Baseline NLRF Score, total dropped, critical dropped: \n";
    cout << score << " ";
    cout << totalDropped << " ";
    cout << criticalDropped << "\n";
    // if(printdetails){
    //     for(int i = 0; i < stations_count; i++){
    //         cout<<"Station "<<i<<" :\n";
    //         cout<<"-----------------\n";
    //         cout<<scheduledPackets[i].size()<<"\n";
    //     }
    //     cout<<"-------------------------------\n";
    // }
}

bool edf(Packet &p1, Packet &p2)
{
    if (p1.deadline == p2.deadline)
    {
        return p1.arrival < p2.arrival;
    }
    return p1.deadline < p2.deadline;
}
bool lrf(Packet &p1, Packet &p2)
{
    double wt1 = p1.deadline / (double)p1.penalty;
    double wt2 = p2.deadline / (double)p2.penalty;
    if (wt1 == wt2)
    {
        return p1.arrival < p2.arrival;
    }
    return wt1 < wt2;
}

#define EDF 1
#define LRF 2

void newBaseline(vector<Packet> &packets, int start_time, int T, int opt, int stations_count, int criticalThreshold, double packetDropFactor, Config userconfig, int maxScorePossible, int rtLoss)
{
    if (opt == EDF)
    {
        sort(packets.begin(), packets.end(), edf);
    }
    else if (opt == LRF)
    {
        sort(packets.begin(), packets.end(), lrf);
    }
    int score = 0;
    int totalDropped = 0;
    vector<vector<Packet>> scheduledPackets(stations_count);
    int criticalDropped = 0;
    for (auto &pckt : packets)
    {
        totalDropped++;
        if (pckt.penalty >= criticalThreshold)
            criticalDropped++;
    }
    double lastend = start_time;
    int lenPackets = packets.size();
    for (int i = 0; i < lenPackets;)
    {
        int currTotalAssigned = 0;
        int currCriticalAssigned = 0;
        // try all arrival packets at start
        if (lastend >= packets[i].deadline)
        {
            ++i;
            continue;
        }
        int maxscore = -1;
        vector<int> cfig;
        double newend = lastend;
        double newstart = max(lastend, packets[i].arrival);
        vector<Packet> scheduledThisRound(0);
        // iterate through all configurations
        for (vector<int> copyconfig : configs)
        {
            vector<Packet> tempScheduled(0);
            vector<int> config(copyconfig);
            int tempTotalAssigned = 0;
            int tempCriticalAssigned = 0;
            int m = 0;
            for (int j = userconfig.maxRU; j >= RU_26; --j)
            {
                m += config[j];
            }
            int windowsize = 0;
            int curscore = 0;
            // try to fit first m packets if possible
            for (int j = i; j < min(i + m, lenPackets); ++j)
            {
                Packet &pckt = packets[j];
                if (pckt.arrival > newstart)
                    continue;
                bool assigned = false;
                for (int mode = RU_26; mode <= userconfig.maxRU; ++mode)
                {
                    if ((config[mode] > 0) && calcTransmissionTimeMs(pckt.datasize, mode, pckt.stationId, userconfig.mcs_range, userconfig.mcs_lowerval, rtLoss) + newstart <= min(pckt.deadline, (double)T - 1))
                    {
                        config[mode]--;
                        assigned = true;
                        windowsize = max(windowsize, (int)ceil(calcTransmissionTimeMs(pckt.datasize, mode, pckt.stationId, userconfig.mcs_range, userconfig.mcs_lowerval, rtLoss)));
                        break;
                    }
                }
                if (assigned)
                {
                    curscore += pckt.penalty;
                    tempScheduled.push_back(pckt);
                    tempTotalAssigned++;
                    if (pckt.penalty >= criticalThreshold)
                        tempCriticalAssigned++;
                }
            }
            // see if this score is better than the previous ones
            if ((curscore > maxscore) || ((curscore == maxscore) && (newend > (newstart + windowsize))))
            {
                scheduledThisRound = tempScheduled;
                cfig = copyconfig;
                newend = newstart + windowsize;
                maxscore = curscore;
                currTotalAssigned = tempTotalAssigned;
                currCriticalAssigned = tempCriticalAssigned;
            }
        }
        lastend = newend;
        if (newend >= T + start_time)
            break;

        // update max score and dropped packets
        for (auto &pckt : scheduledThisRound)
        {
            scheduledPackets[pckt.stationId].push_back(pckt);
        }
        score += max(maxscore, 0);
        totalDropped -= currTotalAssigned;
        criticalDropped -= currCriticalAssigned;
        // -----
        // -----
        if (cfig.size() == 0)
            ++i;
        else
        {
            for (int j = userconfig.maxRU; j >= RU_26; --j)
            {
                i += cfig[j];
            }
        }
    }
    for (int i = 0; i < stations_count; i++)
    {
        int len = scheduledPackets[i].size();
        int todrop = (int)(packetDropFactor * len);
        for (int j = 0; j < todrop; j++)
        {
            int index = rand() % scheduledPackets[i].size();
            Packet &pckt = scheduledPackets[i][index];
            totalDropped++;
            score -= pckt.penalty;
            if (pckt.penalty >= criticalThreshold)
                criticalDropped++;
            for(int k=index; k<len-1; k++){
                scheduledPackets[i][k] = scheduledPackets[i][k+1];
            }
            // itvl.packets.pop_back();
            scheduledPackets[i].pop_back();
        }
    }
    // output
    if (opt == EDF)
    {
        // cout<<"EDF Score, total dropped, critical dropped: \n";
        // cout<<score<<" ";
        cout << (double)score / (double)(maxScorePossible) << " ";
        // cout<<totalDropped<<" ";
        // cout<<criticalDropped<<"\n";
    }
    else if (opt == LRF)
    {
        // cout<<"LRF Score, total dropped, critical dropped: \n";
        // cout<<score<<" ";
        cout << (double)score / (double)(maxScorePossible) << " ";
        // cout<<totalDropped<<" ";
        // cout<<criticalDropped<<"\n";
    }
    // if(printdetails){
    //     for(int i = 0; i < stations_count; i++){
    //         cout<<"Station "<<i<<" :\n";
    //         cout<<"-----------------\n";
    //         cout<<scheduledPackets[i].size()<<"\n";
    //     }
    //     cout<<"-------------------------------\n";
    // }
}