#define AHC066_JOINT_ORDER 2
#define main ahc066_submission_main
#include "../problems/ahc066/main.cpp"
#undef main

int main(){
    for(const auto& macro: {"FFFFF","FRFRF","FLFLF","FFRFFL","FFLFFR"}){
        Robot r;
        istringstream in("4 2 200\n000\n010\n000\n000\n0000\n0100\n1000\n0 1 2 2\n1 3 3 0\n");
        auto saved=cin.rdbuf(in.rdbuf());
        r.read();
        cin.rdbuf(saved);cin.clear();
        int u=0;
        for(char c:string(macro))u=r.pose[u][labels.find(c)].to;
        const auto deadline=Clock::now()+chrono::seconds(3);
        Robot::Matrix matrix(r,macro,u/4,deadline);
        for(vector<int> order: {vector<int>{0,1},vector<int>{1,0}}){
            Robot::Result brute={INT_MAX,INT_MAX};
            for(int heads=0;heads<256;++heads){
                auto cost=Robot::Result{};
                int a=u%4,k=0;
                for(int id:order)for(int stop:{2*id+1,2*id+2}){
                    int b=4*stop+((heads>>(2*k))&3);++k;
                    const auto e=matrix.entries[a][b];
                    cost.first+=1+e.keys;cost.second+=1+e.expanded;a=b;
                }
                brute=min(brute,cost);
            }
            assert(Robot::ordered_cost(matrix,u%4,order)==brute);
            int matrices=0,probes=0;
            auto [out,expanded]=r.joint_trial(macro,order,false,deadline,matrices,probes);
            assert(matrices==1&&probes==0);
            assert(out.size()==brute.first+strlen(macro)+2);
            assert(expanded==brute.second+int(strlen(macro)));
        }
    }
}
