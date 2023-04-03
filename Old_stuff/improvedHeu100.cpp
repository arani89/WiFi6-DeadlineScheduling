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
    }
};

//------------------- Interval Generation -------------------

//global variables:
vector<vector<int>> configs;
double granularity = 1;
int stations_count = 0;
int criticalThreshold; 
bool printdetails = 0;
const int ditr = 100;

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
    void printstuff(){
        // cout<<"score = "<< score<<"\n";
        // cout<<"Start - End = "<<start<<" - "<<end<<"\n";
        // cout<<"Config : ";
        // for(int i = 0; i < 5; ++i){
        //     cout<<config[i]<<" ";
        // }
        // cout<<"\n";
        // cout<<"Packets : \n";
        // for(Packet pckt: packets){
        //     cout<<pckt.arrival<<" "<<pckt.deadline<<" "<<pckt.penalty<<"\n";
        // }
    }
};

// this gives transmission time taken in ms 
double calcTransmissionTime(double size, int mode){
    double bitrate;
    if(mode == RU_26) bitrate = 11.1;
    else if(mode == RU_52) bitrate = 22.2;
    else if(mode == RU_106) bitrate = 47.2;
    else if(mode == RU_242) bitrate = 121.9;
    else if(mode == RU_484) bitrate = 243.8;
    else bitrate = -1;

    double res = (size * 8.0)/(bitrate * 1000.0);
    return res;
}
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
bool packetMatchesInterval(int intervalStart, int intervalEnd, Packet& pckt, int mode){
    double transmission_dur = calcTransmissionTime(pckt.datasize, mode);
    if(pckt.arrival <= intervalStart && intervalStart + transmission_dur*granularity <= min((double)intervalEnd, pckt.deadline)) return true;
    return false;
}

// ----------------------------------------------------------
// -------------- Heuristic Algorithm -----------------------
// ----------------------------------------------------------

vector<int> masterconfig = {1,2,4,8,18};

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


int createInterval(vector<Packet>& packets, int T){
    const double alpha = 2;
    vector<vector<Packet>> scheduledPackets(stations_count);
    vector<int> availableIndices;
    for(int i = 0; i < packets.size(); ++i){
        availableIndices.push_back(i);
    }
    vector<IntervalData> selectedIntervals;
    int delta = 5*granularity;
    vector<int> *newAvailableIndices;

    // d and t are in n-units
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
                int till = min((int)bestids[mode].size(), bestconfig[mode]) - 1;
                currInterval.updateFreeSlots(mode, bestconfig[mode] - till+1);
                int at = 0;
                for(; at <= till; ++at){
                    currInterval.insert(packets[bestids[mode][at]]);
                }
                for(; at < bestids[mode].size(); ++at){
                    newAvailableIndices->push_back(bestids[mode][at]);
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
    for(IntervalData& itvl: selectedIntervals){
        score += itvl.score;
    }
    return score;
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

int baselineNLRF(vector<Packet> packets, int T){
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
    return score;
}

int newBaseline(vector<Packet> packets, int T, int opt){
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
        return score;
    } else if(opt == LRF){
        return score;
    }
    return -1;
}

void packetsGeneratorPoisson(int T, vector<vector<Packet>>& packets){
    // cout<<"Stations: "<<"\n";
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


    double averageArrival = 1;  //in nunits
    double lamda = 1 / averageArrival;
    std::exponential_distribution<double> exp (lamda);
    int j = 0;
	for(int ii = 0; ii < ditr; ii++) for(int st = 0; st < stations; st++){
		j = 0;
        double sumArrivalTimes=0;
        double newArrivalTime;
        while(sumArrivalTimes < T)
        {
            newArrivalTime=  exp.operator() (rng);// generates the next random number in the distribution 
            sumArrivalTimes  = sumArrivalTimes + newArrivalTime;  
            for(int i = 0; i < pcktnodes[st]; i++){
                Packet pckt(pcktpenalty[st], sumArrivalTimes, pcktsize[st], sumArrivalTimes+pcktdelay[st], j+i, (int)packets[ii].size());
                packets[ii].push_back(pckt);
            }
            // j += (int)pcktnodes.size();
        }
        j += (int) pcktnodes[st];
    }
    stations_count = j;
}
void uniformInput(int T, vector<vector<Packet>>& packets){
    // cout<<"Stations:\n";
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
    for(int ii = 0; ii < ditr; ii++) for(auto &pro: producers){
        //print pro
        double Tp = pro[0]; 
        int size = (int)pro[1]; //general
        double delay = pro[2]; //general
        int penalty = (int)pro[3]; //general
        int nodes = (int)pro[4]; //general 
        for(double arri = 0; arri < (double)T; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, size, arri + delay, i+ctr, (int)packets[ii].size());
                packets[ii].push_back(pckt);
            }
        }
        i += nodes;
    }
    stations_count = i;
}
void usecase1Input(int T, vector<vector<Packet>>& packets){
    int stations; cin>>stations;
    vector<vector<double>> producers(stations, vector<double>(6));  //usecase 1
    // cout<<"Time period | minframe size | maxframe size | delay | penalty | nodes\n";
    for(int i = 0; i < stations; ++i){ //usecase 1
        for(int j = 0; j < 6; ++j){
            cin>>producers[i][j];
        }
    }
    int i = 0;
	for(int ii = 0; ii < ditr; ii++) for(auto &pro: producers){
		i = 0;
        //print pro
        double Tp = pro[0]; 
        int minsize = (int)pro[1]; //usecase 1
        int maxsize = (int)pro[2]; //usecase 1
        double delay = pro[3]; 
        int penalty = (int)pro[4], nodes = (int)pro[5];
        for(double arri = 0; arri < (double)T; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
				int temp = random(minsize, maxsize);
				// cout<<temp<<endl;
				Packet pckt(penalty, arri, temp, arri + delay, i+ctr, (int)packets[ii].size());
                packets[ii].push_back(pckt);
            }
        }
        i += nodes;
    }
    stations_count = i;
}
void usecase2Input(int T, vector<vector<Packet>>& packets){
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
	for(int ii = 0; ii < ditr; ii++) for(auto &pro: producers){
		i = 0;
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
                Packet pckt(penalty, arri, 10, arri + delay, i+ctr, (int)packets[ii].size());
                packets[ii].push_back(pckt);
            }
        }
        i += nodes;
    }
    stations_count = i;
}
void usecase3Input(int T, vector<vector<Packet>>& packets){
    int stations; cin>>stations;
    // cout<<"bandwidth | size | delay | penalty | nodes\n";
    vector<vector<double>> producers(stations, vector<double>(5));//usecase 2
    // // bandwidth(bits/ms) | size(bytes) | delay | penalty | nodes(usecase 3)
    for(int i = 0; i < stations; ++i){ //usecase 1
        for(int j = 0; j < 5; ++j){
            cin>>producers[i][j];
        }
    }
    int i = 0;
    for(int ii = 0; ii < ditr; ii++) for(auto &pro: producers){
        //print pro
        double bandwidth = pro[0]; 
        int size = (int)pro[1];
        double delay = pro[2];
        int penalty = (int)pro[3], nodes = (int)pro[4];
        double Tp = (size*8.0)/(bandwidth);
        // cout<<Tp<<"\n";
        for(double arri = 0; arri < (double)T; arri += Tp){
            for(int ctr = 0; ctr < nodes; ++ctr){
                Packet pckt(penalty, arri, size, arri + delay, i+ctr, (int)packets[ii].size());
                packets[ii].push_back(pckt);
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
		vector<vector<Packet>> packets(ditr);
        // takeInput(timeperiod, packets);
        if(inputcase == 1)
            usecase1Input(timeperiod, packets);
        else if(inputcase == 2)
            usecase2Input(timeperiod, packets);
        else if(inputcase == 3)
            usecase3Input(timeperiod, packets);
        else if(inputcase == 4)
            packetsGeneratorPoisson(timeperiod, packets);
        else
            uniformInput(timeperiod, packets);
        
        //getting total profits
        vector<int> totalProfits(ditr, 0);
        for(int i = 0; i < ditr; i++) for(Packet& pckt: packets[i]){
            totalProfits[i] += pckt.penalty;
        }

		for(int i = 0; i < ditr; i++)
			cout<<((double) createInterval(packets[i], timeperiod)/(double) totalProfits[i])<<" ";
		cout<<"\n";
		for(int i = 0; i < ditr; i++)
        	cout<<((double) newBaseline(packets[i], timeperiod, EDF)/(double) totalProfits[i])<<" ";
		cout<<"\n";
		for(int i = 0; i < ditr; i++)
        	cout<<((double)newBaseline(packets[i], timeperiod, LRF)/(double) totalProfits[i])<<" ";
		cout<<"\n";
		for(int i = 0; i < ditr; i++)
        	cout<<((double)baselineNLRF(packets[i], timeperiod)/(double) totalProfits[i])<<" ";
		cout<<"\n";
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