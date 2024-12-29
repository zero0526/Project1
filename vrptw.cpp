#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = filesystem;

int NUM_VEHICLE= 25;
int CAPACITY=1000;
#define NUM_CUSTOMER 100

struct Customer {
    double x, y;
    int demand;
    int readyTime, dueTime, serviceTime;
};

struct Route {
    vector<int> customerIds;
    map<int,int> comeAtTime;
    float cost = 0.0;
    int load = 0;
};

class VRPTW {
private:
    vector<Customer> customers;
    vector<bool> visitedSeed;
    int vehicleCapacity;
    vector<pair<double,double>> anpha ;
    double price;
    vector<Route> finnalRoute;
    double calculateDistance(Customer& a, Customer& b) {
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        return sqrt(dx * dx + dy * dy);
    }

    int findFarthestCustomer(vector<bool>& visited) {
        double maxDistance = -1.0;
        int seedIndex = -1;
        for (size_t i = 1; i < customers.size(); ++i) {
            if (!visited[i]) {
                double distance = calculateDistance(customers[0], customers[i]);
                if (distance > maxDistance) {
                    maxDistance = distance;
                    seedIndex = i;
                }
            }
        }
        return seedIndex;
    }
    bool checkInsert(int customerId, int prev, int next, Route r) {
        if (this->customers[customerId].demand + r.load > CAPACITY) return false;
        if (visitedSeed[customerId]) return false;

        int prevTimeService = prev == 0 ? 0 : this->customers[prev].serviceTime;
        double dis = this->calculateDistance(this->customers[prev],this->customers[ customerId]);
        int prevTime = max(r.comeAtTime[prev],this->customers[prev].readyTime) + prevTimeService + dis;
        if ( prevTime> this->customers[customerId].dueTime){
            return false;
        }
        dis = this->calculateDistance(this->customers[customerId], this->customers[r.customerIds[next]]);
        prevTime = max(prevTime,this->customers[customerId].readyTime)
                        + this->customers[customerId].serviceTime + dis;
        int nextIdx = next;
        int prevC = customerId;
        for (int i = nextIdx; i < r.customerIds.size(); i++) {
            int nextC = r.customerIds[i];
            if (prevTime > this->customers[nextC].dueTime) {
                return false;
            }
            dis = this->calculateDistance(this->customers[prevC], this->customers[nextC]);
            prevTime = max(prevTime, this->customers[nextC].readyTime) + this->customers[nextC].serviceTime + dis;
            prevC = nextC;
        }
        return true;
    }
    pair<double, int> CalculateC1(double anpha1, double anpha2, double muy, int customerId, Route r) {
        double c1 = numeric_limits<double>::max();
        int recordIdx = -1;

        for (int i = 1; i < r.customerIds.size(); i++) {
            int prevIdx = r.customerIds[i - 1], nextIdx = r.customerIds[i];
            if (this->checkInsert(customerId, prevIdx, i, r)) {
                double dpu = this->calculateDistance(this->customers[prevIdx], this->customers[customerId]);
                double dnu = this->calculateDistance(this->customers[customerId], this->customers[nextIdx]);
                double dpn = this->calculateDistance(this->customers[prevIdx], this->customers[nextIdx]);
                double c11 = dpu + dnu - muy * dpn;
            
                double prevTime = max(r.comeAtTime[prevIdx],this->customers[prevIdx].readyTime)
                                + this->customers[prevIdx].serviceTime + dpu;
                prevTime = max(prevTime,double(this->customers[customerId].readyTime))
                            + this->customers[customerId].serviceTime + dnu;
                double c12 = max(prevTime, double(this->customers[nextIdx].readyTime))
                            - max(r.comeAtTime[nextIdx], this->customers[nextIdx].readyTime);

                if (anpha1 * c11 + anpha2 * c12 < c1) {
                    c1 = anpha1 * c11 + anpha2 * c12;
                    recordIdx = i;
                }
            }
        }
        return make_pair(c1, recordIdx);
    }

    tuple<double, int, int> calculateC2Seed(double lamda, Route r) {
        int recordIdx = -1;
        double costC2 = -numeric_limits<double>::max();
        int clientIdx = -1;

        for (int i = 1; i < this->customers.size(); i++) {
            if (!visitedSeed[i]) {
                pair<double, int> temp = this->CalculateC1(1, 0, 1, i, r);
                if (temp.first == numeric_limits<double>::max()) continue;

                double costc2 = this->calculateDistance(this->customers[0], this->customers[i]) * lamda - temp.first;
                if (costc2 > costC2) {
                    costC2 = costc2;
                    recordIdx = temp.second;
                    clientIdx = i;
                }
            }
        }

        return make_tuple(costC2, clientIdx, recordIdx);
    }
    tuple<bool,double,int,int,int> calculateC2(vector<Route> rs,double anpha1,double anpha2){
        int pInsert,rInsert,cInsert;
        double optimizeC2 = -numeric_limits<double>::max();
        for(int i =1;i<this->customers.size();i++){
            if(!visitedSeed[i]){
                vector<double> c1Min;
                double optimizeC1 = 1e9;
                int recordIdx = -1;
                int route = -1;
                for(int j=0;j<rs.size();j++){
                    Route r = rs[j];
                    pair<double,int> temp = this->CalculateC1(anpha1,anpha2,1,i,r);
                    if (temp.first == numeric_limits<double>::max()){
                        c1Min.push_back(1e9);//no position to insert
                        continue;
                    }
                    if(optimizeC1>temp.first){
                        optimizeC1 = temp.first;
                        route = j;
                        recordIdx = temp.second;
                    }
                    c1Min.push_back(temp.first);
                }
                if(optimizeC1==1e9){
                    return make_tuple(true,-1.0,i,-1,-1);
                }
                double c2=0;
                for(double v:c1Min){
                    c2 += v - optimizeC1;
                }
                if(c2>optimizeC2){
                    optimizeC2 = c2;
                    pInsert = recordIdx;
                    rInsert = route;
                    cInsert = i;
                }
            }
        }
        return make_tuple(false,optimizeC2,cInsert,rInsert,pInsert);
    }

public:
    VRPTW(int capacity,vector<pair<double, double>> alpha){
        this->vehicleCapacity = capacity;
        this->anpha = alpha;
    }
    vector<Customer> getCustomer(){
        return this->customers;
    }
    bool loadInput(const string& path) {
        ifstream file(path);
        if (!file) {
            cerr << "Path not found: " << path << '\n';
            return false;
        }
        string line;
         if (getline(file, line)) {
            stringstream ss(line);
            ss >> NUM_VEHICLE >> CAPACITY;  
        } else {
            cerr << "File is empty or invalid format.\n";
            return false;
        }
        customers.resize(NUM_CUSTOMER + 1);
        this->visitedSeed.resize(NUM_CUSTOMER + 1);
        while (getline(file, line)) {
            stringstream ss(line);
            vector<int> temp(7);
            int i = 0;
            while (ss >> temp[i]) { ++i; }
            int idx = temp[0];
            customers[idx].x = temp[1];
            customers[idx].y = temp[2];
            customers[idx].demand = temp[3];
            customers[idx].readyTime = temp[4];
            customers[idx].dueTime = temp[5];
            customers[idx].serviceTime = temp[6];
        }

        return true;
    }
    vector<Route> buildSeed() {
        fill(this->visitedSeed.begin(), this->visitedSeed.end(), false);
        vector<Route> routes;

        while (true) {
            Route route;
            int seedCus = findFarthestCustomer(this->visitedSeed);
            if (seedCus == -1) break;
            route.customerIds.push_back(0);
            visitedSeed[0] = true;
            route.comeAtTime[0] = 0;
            route.customerIds.push_back(seedCus);
            visitedSeed[seedCus] = true;
            route.comeAtTime[seedCus] = this->calculateDistance(this->customers[0],this->customers[seedCus]);
            route.load += this->customers[seedCus].demand;
            route.customerIds.push_back(0);

            while (true) {
                auto [cost, clientIdx, insertIdx] = this->calculateC2Seed(1, route);
                if (cost == -numeric_limits<double>::max()) {
                    routes.push_back(route);
                    break;
                } else {
                    route.customerIds.insert(route.customerIds.begin() + insertIdx, clientIdx);
                    visitedSeed[clientIdx] = true;
                    route.load += this->customers[clientIdx].demand;
                    int prevCus = route.customerIds[insertIdx-1];
                    double dis = this->calculateDistance(this->customers[prevCus],this->customers[clientIdx]);
                    route.comeAtTime[clientIdx] = max(route.comeAtTime[prevCus] ,this->customers[prevCus].readyTime)
                                                + this->customers[prevCus].serviceTime + dis;
                    for(int i = insertIdx +1;i<route.customerIds.size()-1;i++){
                        prevCus = route.customerIds[i-1];
                        int currCus = route.customerIds[i];
                        dis = this->calculateDistance(this->customers[prevCus],this->customers[currCus]);
                        route.comeAtTime[currCus] = max(route.comeAtTime[prevCus],this->customers[prevCus].readyTime)
                                                        + this->customers[prevCus].serviceTime + dis;
                    }
                }

            }
        }
        return routes;
    }
    vector<int> initial_seed(vector<Route> rs){
        vector<Route> sequenceSeed = rs;
        vector<int> seed;
        for(Route v:sequenceSeed){
            int recordIdx = -1;double maxDis = -1;
            for(int i=1;i<v.customerIds.size()-1;i++){
                double currDis = this->calculateDistance(this->customers[0],this->customers[v.customerIds[i]]);
                if(maxDis < currDis){
                    maxDis = currDis;
                    recordIdx = v.customerIds[i];
                }
            }
            seed.push_back(recordIdx);
        }
        return seed;
    }
    vector<int> initBetterSeed(vector<Route> rs){
        double minD = numeric_limits<double>::max();
        int recordIdx = -1;
        vector<int> seed = this->initial_seed(rs);
        for(int i = 1 ;i<seed.size();i++){
            for(int j=0;j<i;j++){
                double currentD = this->calculateDistance(this->customers[seed[i]],this->customers[seed[j]]);
                if(minD>currentD){
                    minD = currentD;
                    recordIdx = j;
                }
            }
        }
        seed.erase(seed.begin()+recordIdx);
        return seed;
    }
    vector<int> initWorseSeed(vector<int> sd){
        double maxD = -numeric_limits<double>::max();
        int record = -1;
        vector<int> seed = sd;
        map<int,bool> visited;
        for(int v:seed){
            visited[v] =true;
        }
        for(int i = 1; i<this->customers.size();i++){
            if(!visited[i]){
                for(int v:seed){
                    double currentD = this->calculateDistance(this->customers[i],this->customers[v]);
                    if(currentD>maxD){
                        maxD = currentD;
                        record = i;
                    }
                }
            }
        }
        seed.push_back(record);
        return seed;
    }
    vector<Route> parallelRouteBuilding(vector<int> seed,double anpha1,double anpha2) {
        fill(this->visitedSeed.begin(),this->visitedSeed.end(),false);
        visitedSeed[0] = true;
        vector<Route> routes;
        for(int v:seed){
            Route route;
            route.customerIds.push_back(0);
            route.customerIds.push_back(v);
            route.customerIds.push_back(0);
            route.load+=this->customers[v].demand;
            route.comeAtTime[v] = this->customers[0].readyTime + this->calculateDistance(this->customers[0],this->customers[v]);
            visitedSeed[v] =true;
            routes.push_back(route);
        }
        while(true){
            auto [confirm,costC2,cInsert,rInsert,pInsert] = this->calculateC2(routes,anpha1,anpha2);
            if(costC2== -numeric_limits<double>::max()){
                return routes;
            }else if(confirm){
                routes.clear();
                return routes;
            }
            routes[rInsert].customerIds.insert(routes[rInsert].customerIds.begin()+pInsert,cInsert);
            visitedSeed[cInsert] = true;
            routes[rInsert].load += this->customers[cInsert].demand;
            int prevCus = routes[rInsert].customerIds[pInsert-1];
            double dis = this->calculateDistance(this->customers[prevCus],this->customers[cInsert]);
            routes[rInsert].comeAtTime[cInsert] = max(routes[rInsert].comeAtTime[prevCus] ,this->customers[prevCus].readyTime)
                                                + this->customers[prevCus].serviceTime + dis;
            for(int i = pInsert +1;i<routes[rInsert].customerIds.size()-1;i++){
                prevCus = routes[rInsert].customerIds[i-1];
                int currCus = routes[rInsert].customerIds[i];
                dis = this->calculateDistance(this->customers[prevCus],this->customers[currCus]);
                routes[rInsert].comeAtTime[currCus] = max(routes[rInsert].comeAtTime[prevCus],this->customers[prevCus].readyTime)
                                                    + this->customers[prevCus].serviceTime;
            }
        }
    }
    void backtrack(vector<int> seed){
        for(pair<double,double> ap:this->anpha){
            vector<Route> routes = this->parallelRouteBuilding(seed,ap.first,ap.second);
            if(routes.size()>0){
                double cs = this->getPrice(routes);
                if(this->finnalRoute.size()==0){
                    this->finnalRoute = routes;
                    this->price = cs;
                }else if(this->finnalRoute.size()>routes.size()){
                    this->finnalRoute = routes;
                    this->price = cs;
                }else if(this->finnalRoute.size() == routes.size() && this->price>cs){
                    this->finnalRoute = routes;
                    this->price = cs;
                }
                vector<int> newSeed = this->initBetterSeed(routes);
                backtrack(newSeed);
            }
        }
        if(this->finnalRoute.size()==0){
            vector<int> newSeed = this->initWorseSeed(seed);
            backtrack(newSeed);
        }
    }
    vector<Route> getFinnalRoute(){
        return this->finnalRoute;
    }
    double getPrice(vector<Route> rs){
        double totalCostD = 0;
        if(rs.size()==0){
            return 0;
        }
         for(Route r:rs){
            for(int i =1;i<r.customerIds.size();i++){
                totalCostD += this->calculateDistance(this->customers[r.customerIds[i]],this->customers[r.customerIds[i-1]]);
            }
         }
        return totalCostD;
    }
    void show(vector<Route> rs){
        for(Route r: rs){
            for(int i:r.customerIds){
                cout<<i<< " ";
            }
            cout<<endl;
        //      for(int i:r.customerIds){
        //         cout<<r.comeAtTime[i]<< " ";
        //     }
        //     cout<<endl;
        }
        cout<<"num_routing :"<<rs.size()<<endl;
        cout<<"cost :"<<this->price;
    }
    
};
bool fully(){
    string path = "D:\\python\\norrmalPracticePy\\vrptwData";
    vector<string> paths;
    vector<pair<double,double>> alpha;
    alpha.push_back({0.5,0.5});
    alpha.push_back({0.75,0.25});
    alpha.push_back({1.0,0.0});


    for (const auto& entry : fs::directory_iterator(path)) {
        paths.push_back(entry.path().string());
    }
    int t = paths.size();
    while(t--){
        VRPTW solver(CAPACITY,alpha);

        if (!solver.loadInput(paths[t])) return false;
        cout<<paths[t].substr(38,5)<<endl;
        auto routes = solver.buildSeed();
        vector<int> seed = solver.initial_seed(routes);
        solver.backtrack(seed);
        vector<Route> rs = solver.getFinnalRoute();
        int cnt  = 1;
        for(Route r: rs){
            cout<<"#Route "<<cnt<<" : ";
            cnt++;
            for(int i:r.customerIds){
                cout<<i<< " ";
            }
            cout<<endl;
        }
        cout<<"num_routing :"<<rs.size()<<endl;
        cout<<"cost :"<<solver.getPrice(rs)<<endl;
    }
    return true;
}
bool single(string f){
    string path = "D:\\python\\norrmalPracticePy\\vrptwData\\" + f;
    vector<pair<double,double>> alpha;
    alpha.push_back({0.5,0.5});
    alpha.push_back({0.75,0.25});
    alpha.push_back({1.0,0.0});
    VRPTW solver(CAPACITY,alpha);
    if (!solver.loadInput(path)) false;
        cout<<path<<endl;
    auto routes = solver.buildSeed();
        vector<int> seed = solver.initial_seed(routes);
        solver.backtrack(seed);
        vector<Route> rs = solver.getFinnalRoute();

        solver.show(rs);
        return false;
}
int main() {
    if(fully());
   //single("RC208.txt");
    return 0;
}
