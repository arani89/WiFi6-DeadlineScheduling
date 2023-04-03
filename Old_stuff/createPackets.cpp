#include <iostream>
#include <vector>
using namespace std;

double calcTxop(double size, int tonnes){
    double bitrate;
    if(tonnes == 26) bitrate = 11.1;
    else if(tonnes == 52) bitrate = 22.2;
    else if(tonnes == 106) bitrate = 47.2;
    else bitrate = -1;

    double res = (size * 8.0)/(bitrate * 1000);
    return res;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(0);
    int T; cin>>T;             //round time

    int m; cin>>m; 
    // int tonnes; cin>>tonnes;

    // int stations; cin>>stations;
    // vector<vector<double>> producers(stations, vector<double>(5));
    // vector<vector<int>> packets;
    // // Time period | size | delay | penalty | nodes
    // for(int i = 0; i < stations; i++){
    //     for(int j = 0; j < 5; j++){
    //         cin>>producers[i][j];
    //     }
    // }
    // vector<int> penalties, arrivals;
    // vector<double> txops, deadlines;
    // for(auto &pro: producers){
    //     int Tp = (int)pro[0];
    //     int size = (int)pro[1];
    //     double txop = calcTxop(size, tonnes);
    //     double delay = pro[2];
    //     int penalty = (int)pro[3];
    //     int nodes = (int)pro[4];
    //     for(int arri = 0; arri < T; arri += Tp){
    //         for(int ctr = 0; ctr < nodes; ctr++){
    //             penalties.push_back(penalty);
    //             arrivals.push_back(arri);
    //             txops.push_back(txop);
    //             deadlines.push_back( arri*1.0 + delay);
    //         }
    //     }
    // }
    // cout<<1<<"\n";
    // cout<<m<<" "<<T<<" "<<txops.size()<<"\n";
    // for(int i = 0; i < (int) txops.size(); i++){
    //     cout<<penalties[i]<<" ";
    //     cout<<arrivals[i]<<" ";
    //     cout<<txops[i]<<" ";
    //     cout<<deadlines[i]<<"\n";
    // }
}

200
5
0.25
0.5
1
2
4

/*

T 
number of stations, then station lines follow:
TimePeriod size delay profit nodes

200
7
1 400 0.5 90 2
2 10 1 100 10
100 500 500 20 1
2000 1000 100 30 10
33 6000 500 40 1
100 200 500 10 1
5 30 10 200 15

-----------------

200
7
1 400 0.5 90 2
2 10 1 100 10
100 500 500 20 1
2000 1000 100 30 10
33 6000 500 10 1
100 200 500 10 1
5 30 10 200 15

------------------

200
7
1000 100 100 120 2
2 10 1 100 15
100 500 500 20 1
2000 1000 100 30 10
33 6000 500 10 1
100 200 500 10 1
5 30 10 200 15

-------------------

200
8
1000 100 100 120 2
2 10 1 100 15
100 500 500 20 1
2000 1000 100 10 10
33 6000 500 40 1
100 200 500 10 1
5 30 10 200 15
33 20000 20 10 1


3
200
7
1 400 0.5 90 2
2 10 1 100 10
100 500 500 20 1
2000 1000 100 30 10
33 6000 500 40 1
100 200 500 10 1
5 30 10 200 15


200
7
1000 100 100 120 2
2 10 1 100 15
100 500 500 20 1
2000 1000 100 30 10
33 6000 500 10 1
100 200 500 10 1
5 30 10 200 15

200
8
1000 100 100 120 2
2 10 1 100 15
100 500 500 20 1
2000 1000 100 10 10
33 6000 500 40 1
100 200 500 10 1
5 30 10 200 15
33 20000 20 10 1

200
8
1 400 0.5 90 5
2 10 1 200 5
100 500 500 20 1
2000 1000 100 10 10
33 6000 500 40 1
100 200 500 10 1
5 30 10 200 15
33 20000 20 10 1

*/

/*
--
Test cases it performs better at:
--
T 
number of stations, then station lines follow:
TimePeriod size delay profit nodes
5
200
8
1000 100 100 120 2
1 400 0.5 90 2
2 10 1 200 8
100 500 500 20 1
2000 1000 100 10 10
33 6000 500 40 1
100 200 500 10 1
5 30 10 200 15
200
7
1 400 0.5 90 2
2 10 1 100 10
100 500 500 20 1
2000 1000 100 30 10
33 6000 500 40 1
100 200 500 10 1
5 30 10 200 15
200
7
1000 100 100 120 2
2 10 1 200 8
100 500 500 20 1
2000 1000 100 10 12
33 6000 500 30 1
100 200 500 10 1
5 30 10 200 15
200
7
1000 100 100 220 2
1 400 0.5 90 5
100 500 500 20 1
2000 1000 100 10 12
33 6000 500 40 2
100 200 500 10 2
5 30 10 400 15
200
8
1000 100 100 220 2
1 400 0.5 90 4
100 500 500 20 1
2000 1000 100 10 10
33 6000 500 40 2
100 200 500 10 2
5 30 10 400 15
33 20000 20 10 1
*/