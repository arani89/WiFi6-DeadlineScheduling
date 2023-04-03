#include <iostream>
#include <set>
#include <vector>
#include <queue>
#include <cmath>
#include <stack>
#include <algorithm>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
  // mt19937 rng((uint64_t) new char);
int random(int range_from, int range_to) {
	if(range_from == range_to) return range_to;
	int temp = (rng())%(range_to - range_from + 1);
	temp += range_from;
	return temp;
}
struct Interval{
    int start; int end;
};
class Packet{
    public:
    int penalty, type, id, datasize;
    double arrival;
    double deadline;
    Packet(){
        datasize = penalty = type = -1;
        deadline = arrival = -1;
        id = 0;
    }
    Packet(int penalty, double arrival, int datasize, double deadline, int type, int id){
        this->penalty = penalty;
        this->arrival = arrival;
        this->datasize = datasize;
        this->deadline = deadline; 
        this->type = type;
        this->id = id;
    }
    void printstuff(){
        cout<<"arrival-deadline : "<< arrival<<" - "<<deadline<<"\n";
        cout<<"penalty: "<<penalty<<"\n\n";
    }
};

//------------------- Interval Generation -------------------

//global variables:
vector<vector<int>> configs;
double granularity = 1;
int stations_count = 0;
int criticalThreshold; 
bool printdetails = 0;

const int RU_484 = 0;
const int RU_242 = 1;
const int RU_106 = 2;
const int RU_52 = 3;
const int RU_26 = 4;

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
class IntervalData{
public:
    vector<int> config;
    vector<int> freeslots;
    int start; int end;
    vector<Packet> packets;
    int score;
    IntervalData(int start, int end):start(start),end(end),score(0){
        freeslots.assign(5,0);
        config.assign(5,0);
    }
    void updateFreeSlots(int mode, int val){
        freeslots[mode] = val;
    }
    void setConfig(vector<int>& config){
        for(int i = 0; i < 5; i++){
            this->config[i] = config[i];
        }
    }
    void insert(Packet& pckt){
        packets.push_back(pckt);
        score += pckt.penalty; 
    }
    int getavailfreq(){
        return freeslots[RU_484]*484 + 
            freeslots[RU_242]*242 + 
            freeslots[RU_106]*106 + 
            freeslots[RU_52]*52 +
            freeslots[RU_26]*26;
    }
    void printstuff(){
        int totfreeslots = 0;
        for(auto val: freeslots){
            totfreeslots += val;
        }
        if(totfreeslots == 0) return;
        cout<<"Free slots = "<<totfreeslots<<"\n";
        if(freeslots[RU_484] > 0) cout<<"RU_484 = "<<freeslots[RU_484]<<", ";
        if(freeslots[RU_242] > 0) cout<<"RU_242 = "<<freeslots[RU_242]<<", ";
        if(freeslots[RU_106] > 0) cout<<"RU_106 = "<<freeslots[RU_106]<<", ";
        if(freeslots[RU_52] > 0) cout<<"RU_52 = "<<freeslots[RU_52]<<", ";
        if(freeslots[RU_26] > 0) cout<<"RU_26 = "<<freeslots[RU_26]<<", ";
        cout<<"\n";
        cout<<"score = "<< score<<"\n";
        cout<<"Start - End = "<<start<<" - "<<end<<"\n";
        cout<<"Config : ";
        for(int i = 0; i < 5; ++i){
            cout<<config[i]<<" ";
        }
        cout<<"\n";
        // cout<<"Packets : \n";
        // for(Packet pckt: packets){
        //     cout<<pckt.arrival<<" "<<pckt.deadline<<" "<<pckt.penalty<<"\n";
        // }
    }
};

// this gives transmission time taken in ms 
vector<vector<double>> bitvalues = {{11.1,22.2,47.2,121.9,243.8}};
double calcTransmissionTime(double size, int mode){
    double bitrate;
    int bitmode = 0;
    if(mode == RU_26) bitrate = bitvalues[bitmode][0];
    else if(mode == RU_52) bitrate = bitvalues[bitmode][1];
    else if(mode == RU_106) bitrate = bitvalues[bitmode][2];
    else if(mode == RU_242) bitrate = bitvalues[bitmode][3];
    else if(mode == RU_484) bitrate = bitvalues[bitmode][4];
    else bitrate = -1;

    double res = (size * 8.0)/(bitrate * 1000.0);
    return res;
}

// Code to recursively generate all possible RU configurations
void createRUConfigs(int curr, vector<int>& config){
    //Print
    vector<int> tempfigs(config);
    configs.push_back(tempfigs);
    //try divisions
    for(int times = 1; times <= config[curr]; times++){
        config[curr] -= times;
        if(curr == RU_484){
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

// check for whether a packet can be scheduled in a given interval (intervalStart-intervalEnd)
bool packetMatchesInterval(int intervalStart, int intervalEnd, Packet& pckt, int mode){
    double transmission_dur = calcTransmissionTime(pckt.datasize, mode);
    if(pckt.arrival <= intervalStart && intervalStart + transmission_dur*granularity <= min((double)intervalEnd, pckt.deadline)) return true;
    return false;
}

// ----------------------------------------------------------
// -------------- Heuristic Algorithm -----------------------
// ----------------------------------------------------------

vector<int> masterconfig = {1,2,4,8,18}; // max of all possible RU configruations

/*********************************
Given interval timestamps and the current available packets, figure out best packets for the master
config, which 
*********************************/
vector<vector<int>> selectIndicesForConfig(int itvl_start, int itvl_end, vector<int>& availablePackets, vector<int>* newAvailablePackets, vector<Packet>& packets){
    priority_queue<pair<int, int>, vector<pair<int, int>>, myComparator> topPackets[5];
    // vector<int> retry(0);
    for(int& ii: availablePackets){
        Packet& pckt = packets[ii];
        bool assigned = false;
        for(int mode = RU_26; mode >= RU_484; --mode){
            if(packetMatchesInterval(itvl_start, itvl_end, pckt, mode) && masterconfig[mode] > 0){
                int modecpy = mode;
                if(topPackets[modecpy].size() < masterconfig[modecpy]){
                    topPackets[modecpy].push({pckt.penalty, ii});
                    assigned = true;
                    break;
                }
                // pair<Packet, int> jj;
                // if(!topPackets[modecpy].empty()){
                //     jj = topPackets[modecpy].top();
                // }
                Packet pckt2;
                bool assigned2 = true;
                int ii2=ii, ii3;

                if(topPackets[modecpy].top().first < pckt.penalty){
                    // pckt2 = topPackets[modecpy].top().first;
                    ii3 = topPackets[modecpy].top().second;
                    pckt2 = packets[ii3];
                    topPackets[modecpy].pop();
                    topPackets[modecpy].push({pckt.penalty, ii2});
                    --modecpy;
                    pckt = pckt2;
                    ii2 = ii3;
                    assigned = true;
                    assigned2 = false;
                }
                // Packet pckt3;
                // int ii3;
                while(!assigned2 && modecpy>=RU_484){
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
                            --modecpy;
                            pckt = pckt2;
                            ii2=ii3;
                        }else{
                            break;
                        }
                    }
                }
                if(assigned && !assigned2){
                    newAvailablePackets->push_back(ii2);
                    break;
                }
            }
        }
        if(!assigned){
            newAvailablePackets->push_back(ii);
        }
    }
    vector<vector<int>> bestpckts(5);
    for(int mode = RU_484; mode <= RU_26; ++mode){
        while(!topPackets[mode].empty()){
            bestpckts[mode].push_back(topPackets[mode].top().second); topPackets[mode].pop();
        }
        reverse(bestpckts[mode].begin(), bestpckts[mode].end());
    }
    return bestpckts;
}

// The proposed heuristic algorithm
void DPMSS(vector<Packet>& packets, int T){
    const double alpha = 2;
    vector<vector<Packet>> scheduledPackets(stations_count);
    vector<int> availableIndices;
    for(int i = 0; i < packets.size(); ++i){
        availableIndices.push_back(i);
    }
    vector<IntervalData> selectedIntervals;
    int delta = 5*granularity;
    vector<int> *newAvailableIndices;

    // d and t are in nunits
    for(int d = 1; d <= delta; ++d){
        for(int t = 0; t < T-d; ++t){
            int prevscore = -1;
            int intervalStart = t, intervalEnd = t+d; //diff = d nunits
            IntervalData currInterval(intervalStart, intervalEnd);
            newAvailableIndices = new vector<int>();
            vector<vector<int>> bestids = selectIndicesForConfig(intervalStart, intervalEnd, availableIndices, newAvailableIndices, packets);
            //calc prefix sum
            vector<vector<int>> prefsum(bestids);
            for(int mode = RU_484; mode <= RU_26; ++mode){
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
                for(int mode = RU_484; mode <= RU_26; ++mode){
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
            //fill the current interval
            currInterval.packets.clear();
            currInterval.setConfig(bestconfig);
            for(int mode = RU_484; mode <= RU_26; ++mode){
                int till = min((int)bestids[mode].size(), bestconfig[mode]) - 1;  // index till which to take packets
                if(till >= 0)
                    currInterval.updateFreeSlots(mode, bestconfig[mode] - till-1);
                int at = 0;
                for(; at <= till; ++at){
                    currInterval.insert(packets[bestids[mode][at]]);
                }
                for(; at < bestids[mode].size(); ++at){
                    newAvailableIndices->push_back(bestids[mode][at]);
                }
            }
            // for(int mode = RU_484; mode <= RU_26; ++mode){
            //     cout<<currInterval.config[mode]<<" ";
            // }
            // cout<<"\n";
            // for(int mode = RU_484; mode <= RU_26; ++mode){
            //     cout<<currInterval.freeslots[mode]<<" ";
            // }
            // cout<<"\n\n";
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
                        newAvailableIndices->push_back(pckt.id);
                    }
                }
            } else {
                // reject curr interval
                for(Packet& pckt: currInterval.packets){
                    newAvailableIndices->push_back(pckt.id);
                }
            }
            swap(availableIndices, *newAvailableIndices);
        }
    }

    int totalDropped = 0;
    int criticalDropped = 0;
    //final sweep:
    for(int &ii: availableIndices){
        Packet& pckt = packets[ii];
        bool assigned = false;
        for(IntervalData& itvl: selectedIntervals){
            for(int mode = RU_26; mode >= RU_484; --mode){
                if(itvl.freeslots[mode] <= 0) continue;
                if(packetMatchesInterval(itvl.start, itvl.end, pckt, mode)){
                    itvl.freeslots[mode]--;
                    assigned = true;
                    itvl.insert(pckt); break;
                }
            }
            if(assigned) break;
        }
        if(assigned == false){
            totalDropped++;
            if(pckt.penalty >= criticalThreshold)
                criticalDropped++;
        }
    }
    int score = 0;
    vector<int> graphAvailFreq(T, 484);
    for(IntervalData& itvl: selectedIntervals){
        // cout<<"---------------\n";
        // cout<<"Interval Data: \n";
        int avl = itvl.getavailfreq();
        for(int from = itvl.start; from <= itvl.end; from++){
            graphAvailFreq[from] = avl;
        }
        // itvl.printstuff();
        // if(itvl.)
        score += itvl.score;
    }
    cout<<"x=[";
    for(int i = 0; i < T; i++){
        cout<<i<<",";
    }
    cout<<"]\ny=[";
    for(int i = 0; i < T; i++){
        cout<<graphAvailFreq[i]<<",";
    }
    cout<<"]\n";
    cout<<"Heuristic Score, total dropped, critical dropped:\n";
    cout<<score<<" ";
    cout<<totalDropped<<" ";
    cout<<criticalDropped<<"\n";
    if(printdetails){
        sort(availableIndices.begin(), availableIndices.end());
        int x = 0;
        for(int i = 0; i < packets.size(); i++){
            if((x < availableIndices.size()) && (i == availableIndices[x])){
                x++;
            } else {
                Packet &pckt = packets[i];
                scheduledPackets[pckt.type].push_back(pckt);
            }
        }
        for(int i = 0; i < stations_count; i++){
            cout<<"Station "<<i<<" :\n";
            cout<<"-----------------\n";
            cout<<scheduledPackets[i].size()<<"\n";
        }
        cout<<"-------------------------------\n";
    }
}
// ----------------------------------------------------------
// -----------------------BASELINES-------------------------
bool edf(Packet& p1, Packet& p2){
    if(p1.deadline == p2.deadline){
        return p1.arrival < p2.arrival;
    }
    return p1.deadline < p2.deadline;
}
bool lrf(Packet& p1, Packet& p2){
    double wt1 = p1.deadline/(double)p1.penalty;
    double wt2 = p2.deadline/(double)p2.penalty;
    if(wt1 == wt2){
        return p1.arrival < p2.arrival;
    }
    return wt1 < wt2;
}

#define EDF 1
#define LRF 2

void baselineNLRF(vector<Packet> packets, int T){
    vector<int> packetsTransmitted(stations_count, 0);
    vector<int> packetsGenTillNow(stations_count, 0);
    vector<vector<Packet>> scheduledPackets(stations_count);
    // O(n^2 log n)
    int score = 0, totalDropped = 0, criticalDropped = 0;
    
    for(auto &pckt: packets){
        totalDropped++;
        if(pckt.penalty >= criticalThreshold)
            criticalDropped++;
    }

    int lenPackets = packets.size();
    double lastend = 0; //time
    for(int i = 0; i < lenPackets;){
        packetsGenTillNow.assign(stations_count, 0);
        for(auto &pckt: packets){
            if(pckt.arrival <= lastend) packetsGenTillNow[pckt.type]++;
        }
        //sort based on penalty / deadline 
        sort(packets.begin()+i, packets.end(), [&](const Packet& p1, const Packet& p2){
            return (double)(p1.penalty)*(double)(packetsGenTillNow[p1.type]+1)/(double)(p1.deadline * (packetsTransmitted[p1.type]+1)) > 
                    (double)(p2.penalty)*(double)(packetsGenTillNow[p2.type]+1)/(double)(p2.deadline * (packetsTransmitted[p2.type]+1));
        });
        while(lastend >= packets[i].deadline){
            ++i;
            if(i >= lenPackets) break;
        }
        if(i >= lenPackets) break;
        int maxscore = -1;
        vector<int> cfig;
        double newend = lastend;
        double newstart = max(lastend, packets[i].arrival);
        //iterate through all configurations
        for(auto copyconfig: configs){
            vector<int> config(copyconfig.begin(), copyconfig.end());
            int m = config[RU_484] + config[RU_242] + config[RU_106] + config[RU_52] + config[RU_26];
            // cout<<m<<" : ";
            int windowsize = 0;
            int curscore = 0;
            int curdrops = 0;
            //try to fit first m packets if possible
            for(int j = i; j < min(i+m, lenPackets); ++j){
                Packet& pckt = packets[j];
                if(pckt.arrival > newstart) continue;
                bool assigned = false;
                for(int mode = RU_26; mode >= RU_484; --mode){
                    if((config[mode]>0) && (calcTransmissionTime(pckt.datasize, mode)+newstart <= min(pckt.deadline, (double)T - 1))){
                        config[mode]--;
                        assigned = true;
                        windowsize = max(windowsize, (int)ceil(calcTransmissionTime(pckt.datasize, mode)));
                        break;
                    }
                }
                if(assigned){
                    curscore += pckt.penalty;
                }
            }
            //see if this score is better than the previous ones
            if( (curscore > maxscore) || ( (curscore == maxscore) && (newend > (newstart + windowsize)) )){
                cfig = copyconfig;
                newend = newstart + windowsize;
                maxscore = curscore;
            }
        }
        lastend = newend;
        if(newend >= (double)T) break;
        //update max score
        // cout<<newstart<<" "<<newend<<" "<<maxscore<<"\n";
        score += max(maxscore,0);
        // -----
        if(cfig.size() == 0) ++i;
        else {
            //figure out the packets that were assigned here
            int m = cfig[RU_484] + cfig[RU_242] + cfig[RU_106] + cfig[RU_52] + cfig[RU_26];
            //try to fit first m packets if possible
            for(int j = i; j < min(i+m, lenPackets); ++j){
                Packet& pckt = packets[j];
                if(pckt.arrival > newstart) continue;
                bool assigned = false;
                for(int mode = RU_26; mode >= RU_484; --mode){
                    if((cfig[mode]>0) && calcTransmissionTime(pckt.datasize, mode)+newstart <= min(pckt.deadline, (double)T - 1)){
                        cfig[mode]--;
                        assigned = true;
                        break;
                    }
                }
                if(assigned){
                    // cout<<(pckt.type/10)<<" ";
                    packetsTransmitted[pckt.type] += 1;
                    scheduledPackets[pckt.type].push_back(pckt);
                    //if assigned then remove from dropped packets
                    totalDropped--;
                    if(pckt.penalty >= criticalThreshold) 
                        criticalDropped--;
                }
            }
            i += m; 
        }
    }
    cout<<"Baseline NLRF Score, total dropped, critical dropped: \n";
    cout<<score<<" ";
    cout<<totalDropped<<" ";
    cout<<criticalDropped<<"\n";
    if(printdetails){
        for(int i = 0; i < stations_count; i++){
            cout<<"Station "<<i<<" :\n";
            cout<<"-----------------\n";
            cout<<scheduledPackets[i].size()<<"\n";
        }
        cout<<"-------------------------------\n";
    }
}

void newBaseline(vector<Packet> packets, int T, int opt){
    if(opt == EDF){
        sort(packets.begin(), packets.end(), edf);
    } else if(opt == LRF){
        sort(packets.begin(), packets.end(), lrf);
    } 
    int score = 0;
    int totalDropped = 0;
    vector<vector<Packet>> scheduledPackets(stations_count);
    int criticalDropped = 0;
    for(auto &pckt: packets){
        totalDropped++;
        if(pckt.penalty >= criticalThreshold)
            criticalDropped++;
    }
    double lastend = 0;
    int lenPackets = packets.size();
    for(int i = 0; i < lenPackets;){
        int currTotalAssigned = 0;
        int currCriticalAssigned = 0;
        //try all arrival packets at start
        if(lastend >= packets[i].deadline){
            ++i; continue;
        }
        int maxscore = -1;
        vector<int> cfig;
        double newend = lastend;
        double newstart = max(lastend, packets[i].arrival);
        vector<Packet> scheduledThisRound(0);
        //iterate through all configurations
        for(vector<int> copyconfig: configs){
            vector<Packet> tempScheduled(0);
            vector<int> config(copyconfig);
            int tempTotalAssigned = 0;
            int tempCriticalAssigned = 0;
            int m = config[RU_484] + config[RU_242] + config[RU_106] + config[RU_52] + config[RU_26];
            int windowsize = 0;
            int curscore = 0;
            //try to fit first m packets if possible
            for(int j = i; j < min(i+m, lenPackets); ++j){
                Packet& pckt = packets[j];
                if(pckt.arrival > newstart) continue;
                bool assigned = false;
                for(int mode = RU_26; mode >= RU_484; --mode){
                    if((config[mode]>0) && calcTransmissionTime(pckt.datasize, mode)+newstart <= min(pckt.deadline, (double)T - 1)){
                        config[mode]--;
                        assigned = true;
                        windowsize = max(windowsize, (int)ceil(calcTransmissionTime(pckt.datasize, mode)));
                        break;
                    }
                }
                if(assigned){
                    curscore += pckt.penalty;
                    tempScheduled.push_back(pckt);
                    tempTotalAssigned++;
                    if(pckt.penalty >= criticalThreshold)
                        tempCriticalAssigned++;
                }
            }
            //see if this score is better than the previous ones
            if( (curscore > maxscore) || ( (curscore == maxscore) && (newend > (newstart + windowsize)) )){
                scheduledThisRound = tempScheduled;
                cfig = copyconfig;
                newend = newstart + windowsize;
                maxscore = curscore;
                currTotalAssigned = tempTotalAssigned;
                currCriticalAssigned = tempCriticalAssigned;
            }
        }
        lastend = newend;
        if(newend >= T) break;

        //update max score and dropped packets
        for(auto &pckt: scheduledThisRound){
            scheduledPackets[pckt.type].push_back(pckt);
        }
        score += max(maxscore,0);
        totalDropped -= currTotalAssigned;
        criticalDropped -= currCriticalAssigned;
        // -----
        // -----
        if(cfig.size() == 0) ++i;
        else i += cfig[RU_484] + cfig[RU_242] + cfig[RU_106] + cfig[RU_52] + cfig[RU_26]; 
    }
    //output
    if(opt == EDF){
        cout<<"New EDF Score, total dropped, critical dropped: \n";
        cout<<score<<" ";
        cout<<totalDropped<<" ";
        cout<<criticalDropped<<"\n";
    } else if(opt == LRF){
        cout<<"New LRF Score, total dropped, critical dropped: \n";
        cout<<score<<" ";
        cout<<totalDropped<<" ";
        cout<<criticalDropped<<"\n";
    }
    if(printdetails){
        for(int i = 0; i < stations_count; i++){
            cout<<"Station "<<i<<" :\n";
            cout<<"-----------------\n";
            cout<<scheduledPackets[i].size()<<"\n";
        }
        cout<<"-------------------------------\n";
    }
}
// ----------------------------------------------------------
// double interval_for_usecase1[] = {1, 2, 4, 8, 16};
// int sizes_for_usecase2[] = {85, 58, 90, 400, 1400};

//does not support granularity yet.
void packetsGeneratorPoisson(int T, vector<Packet>& packets){
    int stations; cin>>stations;
    vector<vector<double>> producers(stations, vector<double>(5));//general
    vector<int> pcktsize(stations);
    vector<int> pcktdelay(stations);
    vector<int> pcktpenalty(stations);
    vector<int> pcktnodes(stations);
    //size | delay | penalty | nodes
    // cout<<"Size | Delay | penalty | nodes\n";
    for(int i = 0; i < stations; i++){ //general
        cin>>pcktsize[i]>>pcktdelay[i]>>pcktpenalty[i]>>pcktnodes[i];
    }

    std::random_device rd; // uniformly-distributed integer random number generator
    std::mt19937 rang (rd ()); // mt19937: Pseudo-random number generation

    double averageArrival = 1;  //in nunits
    double lamda = 1 / averageArrival;
    std::exponential_distribution<double> exp (lamda);
    int j = 0;
    for(int st = 0; st < stations; st++){
        double sumArrivalTimes=0;
        double newArrivalTime;
        while(sumArrivalTimes < T)
        {
            newArrivalTime=  exp.operator() (rang);// generates the next random number in the distribution 
            sumArrivalTimes  = sumArrivalTimes + newArrivalTime;  
            for(int i = 0; i < pcktnodes[st]; i++){
                Packet pckt(pcktpenalty[st], sumArrivalTimes, pcktsize[st], sumArrivalTimes+pcktdelay[st], j+i, (int)packets.size());
                packets.push_back(pckt);
            }
            // j += (int)pcktnodes.size();
        }
        j += (int) pcktnodes[st];
    }
    stations_count = j;
}
void uniformInput(int T, vector<Packet>& packets){
    int stations; cin>>stations;
    vector<vector<double>> producers(stations, vector<double>(5));//general
    // cout<<"Time Period | size | delay | profit | nodes\n";
    // Time period | size | delay | penalty | nodes(general)
    for(int i = 0; i < stations; ++i){ //general
        for(int j = 0; j < 5; ++j){
            cin>>producers[i][j];
        }
    }
    int i = 0;
    for(auto &pro: producers){
        //print pro
        double Tp = pro[0]; 
        int size = (int)pro[1]; //general
        double delay = pro[2]; //general
        int penalty = (int)pro[3]; //general
        int nodes = (int)pro[4]; //general 
        for(double arri = 0; arri < (double)T; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, size, arri + delay, i+ctr, (int)packets.size());
                packets.push_back(pckt);
            }
        }
        i += nodes;
    }
    stations_count = i;
}
void usecase1Input(int T, vector<Packet>& packets){
    int stations; cin>>stations;
    vector<vector<double>> producers(stations, vector<double>(6));  //usecase 1
    // cout<<"Time period | minframe size | maxframe size | delay | penalty | nodes\n";
    for(int i = 0; i < stations; ++i){ //usecase 1
        for(int j = 0; j < 6; ++j){
            cin>>producers[i][j];
        }
    }
    int i = 0;
    for(auto &pro: producers){
        //print pro
        double Tp = pro[0]; 
        int minsize = (int)pro[1]; //usecase 1
        int maxsize = (int)pro[2]; //usecase 1
        double delay = pro[3]; 
        int penalty = (int)pro[4], nodes = (int)pro[5];
        for(double arri = 0; arri < (double)T; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, random(minsize, maxsize), arri + delay, i+ctr, (int)packets.size());
                packets.push_back(pckt);
            }
        }
        i += nodes;
    }
    stations_count = i;
}
void usecase2Input(int T, vector<Packet>& packets){
    int stations; cin>>stations;

    // cout<<"mean arrival rate | number of packets/request 1, 2 | size | delay | penalty | nodes\n";
    vector<vector<double>> producers(stations, vector<double>(7));//usecase 2
    // mean arrival rate | number of packets | request 1, 2 | size | delay | penalty | nodes(usecase 2)

    for(int i = 0; i < stations; ++i){ //usecase 1
        for(int j = 0; j < 7; ++j){
            cin>>producers[i][j];
        }
    }
    int i = 0;
    for(auto &pro: producers){
        //print pro
        double meanArrRate = pro[0]; 
        int pcktPerReqlow = (int)pro[1]; //usecase 2
        int pcktPerReqhigh = (int)pro[2]; //usecase 2
        int size = (int)pro[3]; //usecase 2
        double delay = pro[4];
        double Tp = 1000/(meanArrRate*random(pcktPerReqlow, pcktPerReqhigh));
        int penalty = (int)pro[5], nodes = (int)pro[6];
        for(double arri = 0; arri < (double)T; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, 10, arri + delay, i+ctr, (int)packets.size());
                packets.push_back(pckt);
            }
        }
        i += nodes;
    }
    stations_count = i;
}
void usecase3Input(int T, vector<Packet>& packets){
    int stations; cin>>stations;
    // cout<<"bandwidth | size | delay | penalty | nodes\n";
    // bandwidth = bits/(ms*granularity), size = bytes, delay = ms * granularity
    vector<vector<double>> producers(stations, vector<double>(5));//usecase 2
    // // bandwidth | size | delay | penalty | nodes(usecase 3)
    for(int i = 0; i < stations; ++i){ //usecase 1
        for(int j = 0; j < 5; ++j){
            cin>>producers[i][j];
        }
    }
    int i = 0;
    for(auto &pro: producers){
        //print pro
        double bandwidth = pro[0]; 
        int size = (int)pro[1];
        double delay = pro[2];
        int penalty = (int)pro[3], nodes = (int)pro[4];
        double Tp = (size*8.0)/((double)bandwidth);
        // cout<<Tp<<"\n";
        for(double arri = 0; arri < (double)T; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, size, arri + delay, i+ctr, (int)packets.size());
                packets.push_back(pckt);
            }
        }
        i += nodes;
    } 
    stations_count = i;
}
int main(){
    // ios::sync_with_stdio(false);
    // cin.tie(0);
    int T; cin>>T;
    int inputcase; cin>>inputcase;
    vector<int> config = {1,0,0,0,0};
    createRUConfigs(0, config);
    while(T--){
        // cout<<"Time period and Granularity: \n";
        int timeperiod; cin>>timeperiod;
        cin>>granularity;
        cin>>criticalThreshold;
        vector<Packet> packets;
        // takeInput(timeperiod, packets);
        if(inputcase == 0)
            uniformInput(timeperiod, packets);
        else if(inputcase == 1)
            usecase1Input(timeperiod, packets);
        else if(inputcase == 2)
            usecase2Input(timeperiod, packets);
        else if(inputcase == 3)
            usecase3Input(timeperiod, packets);
        else if(inputcase == 4)
            packetsGeneratorPoisson(timeperiod, packets);
        else{
            uniformInput(timeperiod, packets);
        }
        
        // cout<<"---------\n";
        int maxScorePossible = 0;
        for(Packet& pckt: packets){
            maxScorePossible += pckt.penalty;
        }
        cout<<"Max score : "<<maxScorePossible<<"\n";
        cout<<"Total packets : "<< packets.size()<<"\n";
        // DPMSS
        auto start = high_resolution_clock::now();
        DPMSS(packets, timeperiod);
        auto t1 = high_resolution_clock::now();
        // EDF
        newBaseline(packets, timeperiod, EDF);
        // LRF
        newBaseline(packets, timeperiod, LRF);
        // NLRF
        baselineNLRF(packets, timeperiod);
        auto dur1 = duration_cast<microseconds>(t1 - start);
        cout<<"Heuristic algo time taken: \n"<<(dur1.count())/1000<<"\n"; //milliseconds
        // cout<<"---------\n";
    }
}

/*
Input:
n packets, x intervals
each interval corresponds to m RUs so one interval can take m packets (right?)
1 source, 1 sink, n packets, x intervals
source --1-- packets
packets --1-- intervals
intervals --m-- sink
the profits for all edges other than src --1-- packets are zero (no extra profit)
1
3 23
5
10 1 1 10
5 1 1 2
6 1 1 3
7 1 1 2
12 3 10 15
Meeting agenda:
- discuss how the packets are generated
onDemandApplication 
simulation packet generation band 
- how to handle non integral 
LRF
NLRF
EDF (earliest deadline first)


todo:
try on raspberry pi


*/
// interval    size      delay     penalty   number_of_packets
// 1
// total time
// 10
// 1000 30000 5000  300 3
// 100 500 500 500 4
// 60000 64 60000 450 1
// 500000  30 1000 500 2
// 1000 400 1000 300 2
// 60000 64 100 400 10
// 500 50 4000 100 6
// 1000 200 1000 150 20
// 360000 1000 100 450 10
// 20 24000 0 10 1