// Independent AHC001 solver, derived from the problem statement only.
// No archived solution, winner submission, or third-party editorial consulted.
#if !defined(AHC_RUNTIME_VARIANT) && !defined(AHC_VARIANT)
#define AHC_VARIANT 64
#endif
#include <bits/stdc++.h>
using namespace std;

struct RNG {
    uint64_t s;
    explicit RNG(uint64_t seed) : s(seed + 0x9e3779b97f4a7c15ULL) {}
    uint32_t next() { s ^= s << 7; s ^= s >> 9; return uint32_t(s); }
    int integer(int n) { return int((uint64_t(next()) * n) >> 32); }
    double real() { return (next() + 0.5) / 4294967296.0; }
};
struct Rect {
    int q[4];
    int w() const { return q[2] - q[0]; }
    int h() const { return q[3] - q[1]; }
    int area() const { return w() * h(); }
};
bool overlap(const Rect& a, const Rect& b) {
    return (a.q[0] < b.q[2]) & (b.q[0] < a.q[2])
        & (a.q[1] < b.q[3]) & (b.q[1] < a.q[3]);
}
double quality(int area, double target) {
    double x = min(double(area), target) / max(double(area), target);
    return 1.0 - (1.0 - x) * (1.0 - x);
}

struct Solver {
    struct PartitionNode { Rect box; int id, count; };
    int n, x[205], y[205], desired[205];
    Rect state[205], best[205];
    double scale = 1, value = 0, best_value = -1, objective_sum = 0;
    RNG rng;
    double logs[16384];
    vector<Rect> empty_boxes[205];
    unordered_map<int,vector<Rect>> joint_boxes;
    vector<int> close_points[205];
    vector<int> axis_order[2];
    bool local_search=false,movable[205]{};
#ifdef AHC_VARIANT
    static constexpr int variant = AHC_VARIANT;
#else
    int variant = 0;
#endif
    double shape_weight = 0;
    double scaled_target[205], inverse_target[205];
    double empty_box_bound = 0;
    uint64_t iterations = 0, accepted = 0;
    int regional_trials=0,regional_improvements=0;
    explicit Solver(uint64_t seed) : rng(seed) {
        for (int k = 0; k < 16384; ++k) logs[k] = -log((k + .5) / 16384);
    }
    bool valid(int i, const Rect& a) const {
        return a.q[0] >= 0 && a.q[1] >= 0 && a.q[2] <= 10000 && a.q[3] <= 10000
            && a.q[0] <= x[i] && x[i] < a.q[2] && a.q[1] <= y[i] && y[i] < a.q[3];
    }
    void refresh_targets() {
        for(int i=0;i<n;++i) {scaled_target[i]=desired[i]*scale;inverse_target[i]=1/scaled_target[i];}
        if(variant>=52) objective_sum=evaluate();
    }
    double scaled_quality(int i,int area) const {
        if(variant<29) return quality(area,desired[i]*scale);
        double ratio=area<=scaled_target[i] ? area*inverse_target[i] : scaled_target[i]/area;
        return 1-(1-ratio)*(1-ratio);
    }
    double objective(int i, const Rect& a) const {
        double penalty = 0;
        if (shape_weight) {
            double ratio = log(double(a.w()) / a.h());
            penalty = shape_weight * ratio * ratio;
        }
        return scaled_quality(i,a.area()) - penalty;
    }
    void prepare_empty_boxes() {
        for(int axis=0;axis<2;++axis) {
            axis_order[axis].resize(n);iota(axis_order[axis].begin(),axis_order[axis].end(),0);
            const int* coordinates=axis ? y : x;
            sort(axis_order[axis].begin(),axis_order[axis].end(),[&](int a,int b){return coordinates[a]<coordinates[b];});
        }
        vector<int> order(n); iota(order.begin(), order.end(), 0);
        sort(order.begin(),order.end(),[&](int a,int b){return x[a]<x[b];});
        for (int i = 0; i < n; ++i) {
            vector<int> lefts{0};
            for (int j : order) if (x[j] < x[i]) lefts.push_back(x[j]+1);
            for (int l : lefts) {
                int b=0,t=10000;
                auto include_point = [&](int j) {
                    if (j==i || x[j]<l) return;
                    if (y[j]<=y[i]) b=max(b,y[j]+1); else t=min(t,y[j]);
                };
                int k=0;
                while (k<n && x[order[k]]<=x[i]) include_point(order[k++]);
                while (b<=y[i] && y[i]<t) {
                    int r=k==n ? 10000 : x[order[k]];
                    Rect a{{l,b,r,t}};
                    if(variant>=17) {
                        bool right_tight=(k==n),left_tight=(l==0);
                        for(int kk=k;kk<n && x[order[kk]]==r;++kk)
                            if(y[order[kk]]>=b && y[order[kk]]<t) right_tight=true;
                        if(right_tight && !left_tight) for(int j=0;j<n;++j)
                            if(x[j]+1==l && y[j]>=b && y[j]<t) {left_tight=true;break;}
                        if(left_tight && right_tight) empty_boxes[i].push_back(a);
                    } else if (a.area() > desired[i]*.15) empty_boxes[i].push_back(a);
                    if (k==n) break;
                    do { include_point(order[k++]); } while(k<n && x[order[k]]==r);
                }
            }
            sort(empty_boxes[i].begin(),empty_boxes[i].end(),[](const Rect& a,const Rect& b){return a.area()>b.area();});
            if (variant<17 && empty_boxes[i].size()>128) empty_boxes[i].resize(128);
            int largest = empty_boxes[i].empty() ? 1 : empty_boxes[i][0].area();
            empty_box_bound += quality(min(largest,desired[i]),desired[i]);
        }
    }
    Rect global_candidate(int i,bool allow_partial=false) {
        const auto& boxes = empty_boxes[i];
        if (boxes.empty()) return state[i];
        int count=boxes.size();
        if(variant>=33 && variant<=36 && (rng.next()&3)) {
            double threshold=min(desired[i]*scale,double(boxes[0].area()))*.98;
            if(variant==34) threshold=min(double(state[i].area()),threshold)*.95;
            int low=1,high=count;
            while(low<high) {
                int mid=(low+high)/2;
                if(boxes[mid].area()>=threshold) low=mid+1;else high=mid;
            }
            count=low;
        }
        int index = min(rng.integer(count),rng.integer(count));
        Rect c = boxes[index];
        double area = min(double(c.area()), desired[i]*scale);
        if(allow_partial && (rng.next()&1)) {
            double current=min(double(state[i].area()),area);
            area=variant==55 || variant>=60 ? current : current+rng.real()*(area-current);
        }
        int low_w = max(1,int(ceil(area/c.h()))), high_w = min(c.w(),max(1,int(area)));
        int w = clamp(int(round(exp(log(double(low_w)) + rng.real()*log(double(high_w)/low_w)))),low_w,high_w);
        int h = min(c.h(),max(1,int(round(area/w))));
        int lmin=max(c.q[0],x[i]-w+1), lmax=min(x[i],c.q[2]-w);
        int bmin=max(c.q[1],y[i]-h+1), bmax=min(y[i],c.q[3]-h);
        int l=lmin+rng.integer(lmax-lmin+1),b=bmin+rng.integer(bmax-bmin+1);
        return Rect{{l,b,l+w,b+h}};
    }
    const vector<Rect>& prepare_joint_boxes(int i,int j) {
        int key=min(i,j)*205+max(i,j);
        auto found=joint_boxes.find(key);
        if(found!=joint_boxes.end()) return found->second;
        auto& boxes=joint_boxes[key];
        Rect core{{min(x[i],x[j]),min(y[i],y[j]),max(x[i],x[j])+1,max(y[i],y[j])+1}};
        vector<int> order,lefts{0};
        for(int k=0;k<n;++k) if(k!=i && k!=j) {
            if(core.q[0]<=x[k] && x[k]<core.q[2] && core.q[1]<=y[k] && y[k]<core.q[3]) return boxes;
            order.push_back(k);
            if(x[k]<core.q[0]) lefts.push_back(x[k]+1);
        }
        sort(order.begin(),order.end(),[&](int a,int b){return x[a]<x[b];});
        for(int l:lefts) {
            int bottom=0,top=10000;bool possible=true;
            auto include=[&](int k) {
                if(x[k]<l) return;
                if(y[k]<core.q[1]) bottom=max(bottom,y[k]+1);
                else if(y[k]>=core.q[3]) top=min(top,y[k]);
                else possible=false;
            };
            int pos=0,count=order.size();
            while(pos<count && x[order[pos]]<core.q[2]) include(order[pos++]);
            while(possible) {
                int r=pos==count ? 10000 : x[order[pos]];
                bool left_tight=l==0,right_tight=pos==count;
                for(int k=pos;k<count && x[order[k]]==r;++k)
                    if(bottom<=y[order[k]] && y[order[k]]<top) right_tight=true;
                if(right_tight && !left_tight) for(int k:order)
                    if(x[k]+1==l && bottom<=y[k] && y[k]<top) {left_tight=true;break;}
                if(left_tight && right_tight) boxes.push_back(Rect{{l,bottom,r,top}});
                if(pos==count) break;
                do {include(order[pos++]);} while(pos<count && x[order[pos]]==r);
            }
        }
        sort(boxes.begin(),boxes.end(),[](Rect a,Rect b){return a.area()>b.area();});
        return boxes;
    }
    bool joint_change(int i,int j,double temperature) {
        const auto& boxes=prepare_joint_boxes(i,j);
        if(boxes.empty()) return false;
        Rect box=boxes[min(rng.integer(boxes.size()),rng.integer(boxes.size()))];
        int axis=rng.integer(2),ci=axis ? y[i] : x[i],cj=axis ? y[j] : x[j];
        if(ci==cj) return false;
        int first=ci<cj ? i : j,second=ci<cj ? j : i;
        int low=min(ci,cj)+1,high=max(ci,cj),width=axis ? box.w() : box.h();
        double ra=desired[first]*scale,rb=desired[second]*scale;
        double area=ra-(ra+rb-box.area())*ra*ra/(ra*ra+rb*rb);
        int cut=clamp(int(round(box.q[axis]+area/width)),low,high);
        if(rng.next()&1) cut=low+rng.integer(high-low+1);
        Rect a=box,b=box;a.q[axis+2]=cut;b.q[axis]=cut;
        a=fit_leaf(first,a,ra);b=fit_leaf(second,b,rb);
        Rect original[205];copy(state,state+n,original);double before=evaluate();
        state[first]=a;state[second]=b;
        if(!raw_push(first,a,second) || !raw_push(second,b,first)) {
            copy(original,original+n,state);return false;
        }
        for(int k=0;k<n;++k) if(k!=i && k!=j && state[k].area()<original[k].area()) {
            int begin=rng.integer(4);
            for(int d=0;d<4;++d) grow(k,(begin+d)&3);
        }
        double after=evaluate();
        if(after-before>=-temperature*logs[rng.next()&16383]) {
            value=0;for(int k=0;k<n;++k) value+=quality(state[k].area(),desired[k]);
            ++accepted;remember();return true;
        }
        copy(original,original+n,state);return false;
    }
    bool joint_shape_change(int i,Rect a,double temperature) {
        if(!valid(i,a)) return false;
        int j=-1;double largest_loss=-1;
        for(int k=0;k<n;++k) if(k!=i && overlap(a,state[k])) {
            double best_cut=-1e30;
            for(int d=0;d<4;++d) {
                Rect c=state[k];c.q[d]=a.q[d^2];
                if(valid(k,c)) best_cut=max(best_cut,objective(k,c));
            }
            double loss=objective(k,state[k])-best_cut;
            if(loss>largest_loss) {largest_loss=loss;j=k;}
        }
        if(j<0) return change(i,a,temperature,true);
        Rect b=(rng.next()&3) ? global_candidate(j) : state[j];
        if(overlap(a,b)) {
            Rect chosen_a{},chosen_b{};double highest=-1e30;
            for(int axis=0;axis<2;++axis) {
                int ci=axis ? y[i] : x[i],cj=axis ? y[j] : x[j];
                if(ci==cj) continue;
                int lo=min(ci,cj)+1,hi=max(ci,cj);
                auto separated=[&](int cut) {
                    Rect aa=a,bb=b;
                    if(ci<cj) {aa.q[axis+2]=min(aa.q[axis+2],cut);bb.q[axis]=max(bb.q[axis],cut);}
                    else {bb.q[axis+2]=min(bb.q[axis+2],cut);aa.q[axis]=max(aa.q[axis],cut);}
                    return pair<Rect,Rect>{aa,bb};
                };
                auto merit=[&](int cut) {
                    auto [aa,bb]=separated(cut);return objective(i,aa)+objective(j,bb);
                };
                while(hi-lo>5) {
                    int l=lo+(hi-lo)/3,r=hi-(hi-lo)/3;
                    if(merit(l)<merit(r)) lo=l;else hi=r;
                }
                for(int cut=lo;cut<=hi;++cut) {
                    auto [aa,bb]=separated(cut);double score=objective(i,aa)+objective(j,bb);
                    if(score>highest) {highest=score;chosen_a=aa;chosen_b=bb;}
                }
            }
            if(highest<-1e20) return false;
            a=chosen_a;b=chosen_b;
        }
        Rect original[205];copy(state,state+n,original);double before=evaluate();
        state[i]=a;state[j]=b;
        if(!raw_push(i,a,j) || !raw_push(j,b,i)) {copy(original,original+n,state);return false;}
        for(int k=0;k<n;++k) if(k!=i && k!=j && state[k].area()<original[k].area()) {
            int begin=rng.integer(4);
            for(int d=0;d<4;++d) grow(k,(begin+d)&3);
        }
        double after=evaluate();
        if(after-before>=-temperature*logs[rng.next()&16383]) {
            value=0;for(int k=0;k<n;++k) value+=quality(state[k].area(),desired[k]);
            ++accepted;remember();return true;
        }
        copy(original,original+n,state);return false;
    }
    Rect fit_leaf(int i, const Rect& box, double target=-1) const {
        if(target<0) target=desired[i];
        if (box.area() <= target) return box;
        Rect result = box; double best_score=-1;
        int low = max(1,int(ceil(target/box.h())));
        int middle = clamp(int(sqrt(target*box.w()/box.h())),low,box.w());
        int widths[] = {low, box.w(), middle, max(low,middle-1),min(box.w(),middle+1)};
        for (int w : widths) {
            int h = clamp(int(round(target/w)),1,box.h());
            int l = clamp((box.q[0]+box.q[2]-w)/2,max(box.q[0],x[i]-w+1),min(x[i],box.q[2]-w));
            int b = clamp((box.q[1]+box.q[3]-h)/2,max(box.q[1],y[i]-h+1),min(y[i],box.q[3]-h));
            Rect candidate{{l,b,l+w,b+h}};
            double score = quality(candidate.area(),target);
            if (score>best_score) {best_score=score;result=candidate;}
        }
        return result;
    }
    Rect free_candidate(int i) {
        vector<int> order;vector<int> lefts{0};
        for(int j=0;j<n;++j) if(j!=i) {
            order.push_back(j);
            if(state[j].q[2]<=x[i]) lefts.push_back(state[j].q[2]);
        }
        sort(order.begin(),order.end(),[&](int a,int b){return state[a].q[0]<state[b].q[0];});
        Rect chosen=state[i];
        auto merit=[&](const Rect& a){double ratio=log(double(a.w())/a.h());return objective(i,a)-1e-7*ratio*ratio;};
        double chosen_score=merit(chosen);
        auto consider=[&](Rect box) {
            if(box.area()<min(desired[i]*scale,double(chosen.area()))*.97) return;
            Rect c=fit_leaf(i,box,desired[i]*scale);
            double score=merit(c);
            if(score>chosen_score+1e-13) {
                int w=c.w(),h=c.h();
                int llo=max(box.q[0],x[i]-w+1),lhi=min(x[i],box.q[2]-w);
                int blo=max(box.q[1],y[i]-h+1),bhi=min(y[i],box.q[3]-h);
                int l=llo+rng.integer(lhi-llo+1),b=blo+rng.integer(bhi-blo+1);
                chosen=Rect{{l,b,l+w,b+h}};chosen_score=score;
            }
        };
        for(int l:lefts) {
            int b=0,t=10000;
            bool possible=true;
            auto include=[&](int j) {
                const Rect& a=state[j];
                if(a.q[2]<=l) return;
                if(a.q[3]<=y[i]) b=max(b,a.q[3]);
                else if(a.q[1]>y[i]) t=min(t,a.q[1]);
                else possible=false;
            };
            int k=0;
            while(k<int(order.size()) && state[order[k]].q[0]<=x[i]) include(order[k++]);
            while(possible) {
                int right=k==int(order.size()) ? 10000 : state[order[k]].q[0];
                consider(Rect{{l,b,right,t}});
                if(k==int(order.size())) break;
                do{include(order[k++]);}while(k<int(order.size()) && state[order[k]].q[0]==right);
            }
        }
        return chosen;
    }
    bool pair_change(int i,int j,double temperature) {
        Rect core{{min(x[i],x[j]),min(y[i],y[j]),max(x[i],x[j])+1,max(y[i],y[j])+1}};
        for(int k=0;k<n;++k) if(k!=i && k!=j && overlap(core,state[k])) return false;
        vector<int> order,lefts{0};
        for(int k=0;k<n;++k) if(k!=i && k!=j) {
            order.push_back(k);
            if(state[k].q[2]<=core.q[0]) lefts.push_back(state[k].q[2]);
        }
        sort(order.begin(),order.end(),[&](int a,int b){return state[a].q[0]<state[b].q[0];});
        Rect chosen_i=state[i],chosen_j=state[j];
        double before=objective(i,state[i])+objective(j,state[j]);
        auto merit=[&](int a,Rect ra,int b,Rect rb) {
            double ar=log(double(ra.w())/ra.h()),br=log(double(rb.w())/rb.h());
            return objective(a,ra)+objective(b,rb)-1e-9*(ar*ar+br*br);
        };
        double best_score=merit(i,state[i],j,state[j]);
        auto consider=[&](Rect box) {
            for(int axis=0;axis<2;++axis) {
                int ci=axis ? y[i] : x[i],cj=axis ? y[j] : x[j];
                if(ci==cj) continue;
                int first=ci<cj ? i : j,second=ci<cj ? j : i;
                int low=min(ci,cj)+1,high=max(ci,cj),width=axis ? box.w() : box.h();
                double ra=desired[first]*scale,rb=desired[second]*scale;
                double area=ra-(ra+rb-box.area())*ra*ra/(ra*ra+rb*rb);
                int cut=clamp(int(round(box.q[axis]+area/width)),low,high);
                for(int offset=-1;offset<=1;++offset) {
                    int boundary=clamp(cut+offset,low,high);
                    Rect a=box,b=box;a.q[axis+2]=boundary;b.q[axis]=boundary;
                    a=fit_leaf(first,a,ra);b=fit_leaf(second,b,rb);
                    double score=merit(first,a,second,b);
                    if(score>best_score+1e-13) {
                        best_score=score;chosen_i=first==i ? a : b;chosen_j=first==i ? b : a;
                    }
                }
            }
        };
        for(int l:lefts) {
            int bottom=0,top=10000;bool possible=true;
            auto include=[&](int k) {
                Rect a=state[k];
                if(a.q[2]<=l) return;
                if(a.q[3]<=core.q[1]) bottom=max(bottom,a.q[3]);
                else if(a.q[1]>=core.q[3]) top=min(top,a.q[1]);
                else possible=false;
            };
            int pos=0;
            while(pos<int(order.size()) && state[order[pos]].q[0]<core.q[2]) include(order[pos++]);
            while(possible) {
                int right=pos==int(order.size()) ? 10000 : state[order[pos]].q[0];
                consider(Rect{{l,bottom,right,top}});
                if(pos==int(order.size())) break;
                do{include(order[pos++]);}while(pos<int(order.size()) && state[order[pos]].q[0]==right);
            }
        }
        double after=objective(i,chosen_i)+objective(j,chosen_j);
        if(after-before < -temperature*logs[rng.next()&16383]) return false;
        value+=quality(chosen_i.area(),desired[i])+quality(chosen_j.area(),desired[j])
              -quality(state[i].area(),desired[i])-quality(state[j].area(),desired[j]);
        state[i]=chosen_i;state[j]=chosen_j;remember();return true;
    }
    void build_partition(vector<int> ids, Rect box, vector<PartitionNode>& nodes, double noise) {
        assert(!ids.empty() && ids.size()<=200);
        int count=ids.size();
        nodes.push_back({box,count==1 ? ids[0] : -1,count});
        if (count==1) return;
        double sum=0,sq=0;
        for (int i:ids) {sum+=desired[i];sq+=double(desired[i])*desired[i];}
        double best_cost=1e100;
        int chosen_axis=-1,chosen_count=0,chosen_cut=0;
        for (int axis=0;axis<2;++axis) {
            auto coordinate=[&](int i){return axis ? y[i] : x[i];};
            sort(ids.begin(),ids.end(),[&](int i,int j){return coordinate(i)<coordinate(j);});
            double left_sum=0,left_sq=0;
            int other_width=axis ? box.w() : box.h();
            for (int k=1;k<count;++k) {
                int i=ids[k-1];left_sum+=desired[i];left_sq+=double(desired[i])*desired[i];
                int lo=coordinate(i)+1,hi=coordinate(ids[k]);
                if (lo>hi) continue;
                double left_area=left_sum-(sum-box.area())*left_sq/sq;
                int cut=clamp(int(round(box.q[axis]+left_area/other_width)),lo,hi);
                double area=double(cut-box.q[axis])*other_width;
                double shortage_left=max(0.0,left_sum-area),shortage_right=max(0.0,sum-left_sum-(box.area()-area));
                double cost=shortage_left*shortage_left/left_sq+shortage_right*shortage_right/(sq-left_sq);
                cost+=noise*(-log(rng.real()));
                // Prefer balanced subproblems only when area estimates tie.
                cost+=1e-7*abs(count-2*k);
                if (cost<best_cost) {
                    best_cost=cost;chosen_axis=axis;chosen_count=k;chosen_cut=cut;
                }
            }
        }
        assert(chosen_axis>=0 && chosen_count>0 && chosen_count<count);
        sort(ids.begin(),ids.end(),[&](int a,int b){return chosen_axis ? y[a]<y[b] : x[a]<x[b];});
        Rect first=box,second=box;
        first.q[chosen_axis+2]=chosen_cut;second.q[chosen_axis]=chosen_cut;
        vector<int> first_ids(ids.begin(),ids.begin()+chosen_count);
        vector<int> second_ids(ids.begin()+chosen_count,ids.end());
        build_partition(move(first_ids),first,nodes,noise);
        build_partition(move(second_ids),second,nodes,noise);
    }
    void partition_search(double seconds) {
        auto start=chrono::steady_clock::now();
        vector<int> all(n);iota(all.begin(),all.end(),0);
        vector<PartitionNode> nodes;
        build_partition(all,Rect{{0,0,10000,10000}},nodes,.002);
        value=0;
        for (const auto& node:nodes) if(node.id>=0) {
            state[node.id]=fit_leaf(node.id,node.box);value+=quality(state[node.id].area(),desired[node.id]);
        }
        remember();
        uint64_t attempts=0;
        for (;;) {
            double p=chrono::duration<double>(chrono::steady_clock::now()-start).count()/seconds;
            if(p>=1) break;
            ++attempts;
            int index=rng.integer(nodes.size());
            if(nodes[index].count==1) continue;
            int count=nodes[index].count,end=index+2*count-1;
            vector<int> ids;
            double old_value=0;
            for(int j=index;j<end;++j) if(nodes[j].id>=0) {
                int i=nodes[j].id;ids.push_back(i);old_value+=quality(state[i].area(),desired[i]);
            }
            vector<PartitionNode> replacement;replacement.reserve(2*count-1);
            double noise=pow(10.0,-4+3*rng.real());
            build_partition(move(ids),nodes[index].box,replacement,noise);
            double new_value=0;
            for(const auto& node:replacement) if(node.id>=0)
                new_value+=quality(fit_leaf(node.id,node.box).area(),desired[node.id]);
            double temperature=.01*pow(.000001/.01,p);
            if(new_value-old_value >= -temperature*logs[rng.next()&16383]) {
                for(size_t j=0;j<replacement.size();++j) nodes[index+j]=replacement[j];
                for(const auto& node:replacement) if(node.id>=0) state[node.id]=fit_leaf(node.id,node.box);
                value+=new_value-old_value;remember();
            }
        }
        copy(best,best+n,state);value=best_value;
        cerr<<"partition_score="<<llround(best_value/n*1e9)<<" partition_attempts="<<attempts<<'\n';
    }
    void remember() {
        if (value > best_value + 1e-10) {
            best_value = value;
            copy(state, state + n, best);
        }
    }
    void grow(int i, int direction) {
        Rect candidate = state[i];
        int axis = direction & 1, width = axis ? candidate.w() : candidate.h();
        int target_width = max(1, int(round(desired[i] * scale / width)));
        int current_width = axis ? candidate.h() : candidate.w();
        if (target_width <= current_width) return;
        int distance = target_width - current_width;
        if (direction < 2) distance = min(distance, candidate.q[direction]);
        else distance = min(distance, 10000 - candidate.q[direction]);
        for (int j = 0; j < n && distance > 0; ++j) if (j != i) {
            const Rect& b = state[j];
            int other = axis ^ 1;
            if ((candidate.q[other] >= b.q[other+2]) | (b.q[other] >= candidate.q[other+2])) continue;
            if (direction < 2 && b.q[axis+2] <= candidate.q[axis])
                distance = min(distance, candidate.q[axis] - b.q[axis+2]);
            if (direction >= 2 && b.q[axis] >= candidate.q[axis+2])
                distance = min(distance, b.q[axis] - candidate.q[axis+2]);
        }
        candidate.q[direction] += direction < 2 ? -distance : distance;
        if (scaled_quality(i,candidate.area()) > scaled_quality(i,state[i].area()))
            state[i] = candidate;
    }
    Rect cap_expansion(int i, Rect c, int direction) const {
        int axis=direction&1,other=axis^1;
        if(direction<2 ? c.q[direction]>=state[i].q[direction] : c.q[direction]<=state[i].q[direction]) return c;
        if(variant>=52) {
            const int* along=axis ? y : x;
            const int* across=other ? y : x;
            const auto& order=axis_order[axis];
            int pos=lower_bound(order.begin(),order.end(),state[i].q[direction],
                                [&](int id,int coordinate){return along[id]<coordinate;})-order.begin();
            if(direction<2) {
                for(int k=pos-1;k>=0 && along[order[k]]>=c.q[direction];--k) {
                    int j=order[k];
                    if(c.q[other]<=across[j] && across[j]<c.q[other+2]) {c.q[direction]=along[j]+1;break;}
                }
            } else {
                for(int k=pos;k<n && along[order[k]]<c.q[direction];++k) {
                    int j=order[k];
                    if(c.q[other]<=across[j] && across[j]<c.q[other+2]) {c.q[direction]=along[j];break;}
                }
            }
            return c;
        }
        for(int j=0;j<n;++j) if(j!=i) {
            int along=axis ? y[j] : x[j],across=other ? y[j] : x[j];
            if(across<c.q[other] || across>=c.q[other+2]) continue;
            if(direction<2 && along<state[i].q[axis]) c.q[direction]=max(c.q[direction],along+1);
            if(direction>=2 && along>=state[i].q[axis+2]) c.q[direction]=min(c.q[direction],along);
        }
        return c;
    }
    void balance_edge(int i,int direction) {
        struct Neighbor {int gap;double slope,shortage;};
        Rect c=state[i];
        int axis=direction&1,other=axis^1;
        int width=c.q[other+2]-c.q[other];
        double shortage=1-c.area()/(desired[i]*scale);
        if(shortage<=0) return;
        int limit=min(direction<2 ? c.q[axis] : 10000-c.q[axis+2],
                      max(0,int(ceil((desired[i]*scale-c.area())/width))));
        vector<Neighbor> neighbors;
        for(int j=0;j<n;++j) if(j!=i) {
            Rect a=state[j];
            if(c.q[other]>=a.q[other+2] || a.q[other]>=c.q[other+2]) continue;
            int gap=direction<2 ? c.q[axis]-a.q[axis+2] : a.q[axis]-c.q[axis+2];
            if(gap<0 || gap>=limit) continue;
            int point=axis ? y[j] : x[j];
            limit=min(limit,direction<2 ? c.q[axis]-point-1 : point-c.q[axis+2]);
            double target=desired[j]*scale;
            neighbors.push_back({gap,(a.q[other+2]-a.q[other])/target,1-a.area()/target});
        }
        if(limit<=0) return;
        sort(neighbors.begin(),neighbors.end(),[](Neighbor a,Neighbor b){return a.gap<b.gap;});
        double slope=width/(desired[i]*scale),num=slope*shortage,den=slope*slope,lower=0;
        for(Neighbor a:neighbors) {
            if(num/den<=a.gap || a.gap>=limit) break;
            num+=a.slope*a.slope*a.gap-a.slope*a.shortage;
            den+=a.slope*a.slope;
            lower=a.gap;
        }
        int delta=clamp(int(round(max(lower,num/den))),0,limit);
        c.q[direction]+=direction<2 ? -delta : delta;
        change(i,c,0,false);
    }
    bool raw_push(int i, Rect c, int protected_id=-1) {
        if(!valid(i,c)) return false;
        state[i]=c;
        for(int j=0;j<n;++j) if(j!=i && overlap(c,state[j])) {
            if(j==protected_id) return false;
            if(variant>=60 && local_search && !movable[j]) return false;
            Rect before=state[j];
            Rect chosen{};double best_score=-1e30;
            for(int d=0;d<4;++d) {
                Rect cut=before;cut.q[d]=c.q[d^2];
                if(!valid(j,cut)) continue;
                if(variant==30 || variant==32) {
                    state[j]=cut;
                    int begin=rng.integer(4);
                    for(int dd=0;dd<4;++dd) grow(j,(begin+dd)&3);
                    cut=state[j];state[j]=before;
                }
                double score=objective(j,cut);
                if(score>best_score) {best_score=score;chosen=cut;}
            }
            if(best_score<-1e20) return false;
            state[j]=chosen;
        }
        return true;
    }
    bool chain_push(int i, int direction, bool active[], int& budget) {
        if(--budget<0 || active[i]) return false;
        active[i]=true;
        int axis=direction&1,other=axis^1,sign=direction<2 ? -1 : 1;
        for(int j=0;j<n;++j) if(j!=i && overlap(state[i],state[j])) {
            if(active[j]) {active[i]=false;return false;}
            Rect c=state[j];
            int point=axis ? y[j] : x[j];
            int need=direction<2 ? c.q[axis+2]-state[i].q[axis] : state[i].q[axis+2]-c.q[axis];
            int shift=direction<2 ? min(c.q[axis],c.q[axis+2]-point-1)
                                  : min(10000-c.q[axis+2],point-c.q[axis]);
            for(int k=0;k<n;++k) if(k!=j) {
                int along=axis ? y[k] : x[k],across=other ? y[k] : x[k];
                if(across<c.q[other] || across>=c.q[other+2]) continue;
                if(direction<2 && along<c.q[axis]) shift=min(shift,c.q[axis]-along-1);
                if(direction>=2 && along>=c.q[axis+2]) shift=min(shift,along-c.q[axis+2]);
            }
            shift=min(shift,need);
            c.q[axis]+=sign*shift;c.q[axis+2]+=sign*shift;
            if(overlap(state[i],c)) {
                Rect chosen{};double merit=-1e30;
                for(int d=0;d<4;++d) {
                    Rect cut=c;cut.q[d]=state[i].q[d^2];
                    if(!valid(j,cut)) continue;
                    double score=objective(j,cut);
                    if(score>merit) {merit=score;chosen=cut;}
                }
                if(merit<-1e20) {active[i]=false;return false;}
                c=chosen;
            }
            state[j]=c;
            if(!chain_push(j,direction,active,budget)) {active[i]=false;return false;}
        }
        active[i]=false;return true;
    }
    bool chain_change(int i,Rect c,int direction,double temperature) {
        if(!valid(i,c)) return false;
        Rect original[205];copy(state,state+n,original);
        double previous=evaluate();
        state[i]=c;bool active[205]{};int budget=256;
        if(!chain_push(i,direction,active,budget)) {copy(original,original+n,state);return false;}
        for(int j=0;j<n;++j) if(j!=i && state[j].area()<original[j].area()) {
            int begin=rng.integer(4);
            for(int d=0;d<4;++d) grow(j,(begin+d)&3);
        }
        double current=evaluate();
        if(current-previous>=-temperature*logs[rng.next()&16383]) {
            value=0;for(int j=0;j<n;++j) value+=quality(state[j].area(),desired[j]);
            ++accepted;remember();return true;
        }
        copy(original,original+n,state);return false;
    }
    double evaluate() const {
        double result=0;
        for(int i=0;i<n;++i) result+=objective(i,state[i]);
        return result;
    }
    double area_score_delta(const Rect* previous) const {
        double delta=0;
        for(int i=0;i<n;++i) {
            int before=previous[i].area(),after=state[i].area();
            if(before!=after) delta+=scaled_quality(i,after)-scaled_quality(i,before);
        }
        return delta;
    }
    bool repaired_change(int i, Rect c, double temperature) {
        if(!memcmp(&c,&state[i],sizeof(Rect))) return false;
        if(!valid(i,c)) return false;
        Rect original[205];copy(state,state+n,original);
        double previous=variant>=52 ? objective_sum : evaluate();
        if(!raw_push(i,c)) {copy(original,original+n,state);return false;}
        vector<pair<double,int>> damaged;
        for(int j=0;j<n;++j) if(j!=i && state[j].area()<original[j].area())
            damaged.emplace_back(objective(j,original[j])-objective(j,state[j]),j);
        sort(damaged.rbegin(),damaged.rend());
        if(damaged.size()>6) damaged.resize(6);
        double current=variant>=52 ? previous+area_score_delta(original) : evaluate();
        bool cascade=variant==50 || variant==51 || variant==54,fixed[205]{};fixed[i]=true;
        for(int repair_step=0;repair_step<(cascade ? 10 : int(damaged.size()));++repair_step) {
            int j;
            if(cascade) {
                double largest=1e-8;j=-1;
                for(int k=0;k<n;++k) if(!fixed[k]) {
                    double loss=objective(k,original[k])-objective(k,state[k]);
                    if(loss>largest) {largest=loss;j=k;}
                }
                if(j<0) break;
                fixed[j]=true;
            } else j=damaged[repair_step].second;
            if(variant==15 || variant==16) {
                state[j]=free_candidate(j);current=evaluate();continue;
            }
            Rect parent[205],chosen[205];copy(state,state+n,parent);copy(state,state+n,chosen);
            double chosen_score=current,best_merit=0;
            int repair_trials=variant==31 || variant==32 ? 16 :
                              ((variant>=23 && variant<=25) || variant>=27 ? 8 : 4);
            for(int d=0;d<repair_trials;++d) {
                copy(parent,parent+n,state);
                Rect next;
                if(d<4) {
                    next=state[j];
                    int width=(d&1) ? next.h() : next.w(),other=(d&1) ? next.w() : next.h();
                    int add=max(0,int(round(desired[j]*scale/other))-width);
                    next.q[d]=clamp(next.q[d]+(d<2 ? -add : add),0,10000);
                    next=cap_expansion(j,next,d);
                } else {
                    next=global_candidate(j);
                    if(overlap(next,state[i])) {
                        Rect chosen_cut{};double best_cut=-1e30;
                        for(int side=0;side<4;++side) {
                            Rect cut=next;cut.q[side]=state[i].q[side^2];
                            if(!valid(j,cut)) continue;
                            double score=objective(j,cut);
                            if(score>best_cut) {best_cut=score;chosen_cut=cut;}
                        }
                        if(best_cut<-1e20) continue;
                        next=chosen_cut;
                    }
                }
                if(cascade) for(int k=0;k<n;++k) if(k!=j && fixed[k] && overlap(next,state[k])) {
                    Rect chosen_cut{};double best_cut=-1e30;
                    for(int side=0;side<4;++side) {
                        Rect cut=next;cut.q[side]=state[k].q[side^2];
                        if(!valid(j,cut)) continue;
                        double score=objective(j,cut);
                        if(score>best_cut) {best_cut=score;chosen_cut=cut;}
                    }
                    if(best_cut<-1e20) {next=state[j];break;}
                    next=chosen_cut;
                }
                if(!raw_push(j,next,i)) continue;
                if(variant==57 && d>=4) {
                    int begin=rng.integer(4);
                    for(int dd=0;dd<4;++dd) grow(j,(begin+dd)&3);
                }
                for(int k=0;k<n;++k) if(k!=i && k!=j && state[k].area()<parent[k].area()) {
                    int begin=rng.integer(4);
                    for(int dd=0;dd<4;++dd) grow(k,(begin+dd)&3);
                }
                double score=variant>=52 ? current+area_score_delta(parent) : evaluate();
                double merit=score-current;
                if(cascade) {
                    double weight=variant==51 ? .75 : .5;
                    merit=weight*merit+(1-weight)*(objective(j,state[j])-objective(j,parent[j]));
                }
                if(merit>best_merit) {best_merit=merit;chosen_score=score;copy(state,state+n,chosen);}
            }
            copy(chosen,chosen+n,state);current=chosen_score;
        }
        if(current-previous>=-temperature*logs[rng.next()&16383]) {
            if(variant>=52) objective_sum=current;
            value=0;for(int j=0;j<n;++j) value+=quality(state[j].area(),desired[j]);
            ++accepted;remember();return true;
        }
        copy(original,original+n,state);return false;
    }
    bool change(int i, const Rect& candidate, double temperature, bool compensate) {
        if(!memcmp(&candidate,&state[i],sizeof(Rect))) return false;
        if (!valid(i, candidate)) return false;
        Rect old[205]; int ids[205], count = 1;
        old[0] = state[i]; ids[0] = i;
        state[i] = candidate;
        for (int j = 0; j < n; ++j) if (j != i && overlap(candidate, state[j])) {
            if(variant>=60 && local_search && !movable[j]) {
                for(int k=0;k<count;++k) state[ids[k]]=old[k];
                return false;
            }
            Rect chosen{}; double score = -1e30;
            Rect before = state[j];
            for (int d = 0; d < 4; ++d) {
                Rect cut = state[j];
                cut.q[d] = candidate.q[d ^ 2];
                if (!valid(j, cut)) continue;
                if (variant >= 5 && variant<=9 && compensate) {
                    state[j] = cut;
                    int start = rng.integer(4);
                    for (int dd=0; dd<4; ++dd) grow(j,(start+dd)&3);
                    cut = state[j]; state[j] = before;
                }
                double q = objective(j, cut);
                if (q > score) { score = q; chosen = cut; }
            }
            if (score < -1e20) {
                for (int k = 0; k < count; ++k) state[ids[k]] = old[k];
                return false;
            }
            old[count] = state[j]; ids[count++] = j;
            state[j] = chosen;
        }
        if (compensate) {
            for (int k = 1; k < count; ++k) {
                int start = rng.integer(4);
                for (int d = 0; d < 4; ++d) grow(ids[k], (d+start)&3);
            }
        }
        double delta = 0, actual_delta = 0;
        for (int k = 0; k < count; ++k) {
            int j = ids[k];
            double difference=objective(j, state[j]) - objective(j, old[k]);
            delta += difference;
            actual_delta += variant>=52 && scale==1 ? difference :
                            quality(state[j].area(), desired[j]) - quality(old[k].area(), desired[j]);
        }
        if (delta >= 0 || delta > -temperature*logs[rng.next()&16383]) {
            if(variant>=52) objective_sum+=delta;
            value += actual_delta; ++accepted; remember(); return true;
        }
        for (int k = 0; k < count; ++k) state[ids[k]] = old[k];
        return false;
    }
    void regional_search(int center,chrono::steady_clock::time_point start,double seconds) {
        if(quality(state[center].area(),desired[center])>.995) return;
        ++regional_trials;double original_value=value;
        vector<pair<double,int>> order;
        Rect proposed=global_candidate(center);
        for(int j=0;j<n;++j) if(j!=center) {
            int dx=max({0,state[center].q[0]-state[j].q[2],state[j].q[0]-state[center].q[2]});
            int dy=max({0,state[center].q[1]-state[j].q[3],state[j].q[1]-state[center].q[3]});
            double distance=dx+dy;
            if(overlap(proposed,state[j])) distance-=10000;
            distance+=((x[center]-x[j])*(x[center]-x[j])+(y[center]-y[j])*(y[center]-y[j]))*1e-8;
            order.emplace_back(distance,j);
        }
        sort(order.begin(),order.end());
        vector<int> ids{center};
        int desired_count=variant==61 ? 20 : 12;
        if(variant==64) {
            int collisions=0;for(auto [priority,j]:order) {(void)j;if(priority<0) ++collisions;}
            desired_count=max(12,min(24,collisions+1));
        }
        int count=min(n,desired_count);
        for(int k=0;k<count-1;++k) ids.push_back(order[k].second);
        fill(movable,movable+n,false);for(int i:ids) movable[i]=true;
        Rect chosen[205];copy(state,state+n,chosen);double chosen_value=value,old_scale=scale;
        bool destroyed[205]{};
        for(int k=0;k<int(ids.size());++k) {
            int i=ids[k];
            if(variant==63 && k>=4) continue;
            destroyed[i]=true;
            state[i]=variant==62 ? fit_leaf(i,state[i],state[i].area()*(.35+.3*rng.real()))
                                : Rect{{x[i],y[i],x[i]+1,y[i]+1}};
        }
        value=0;for(int i=0;i<n;++i) value+=quality(state[i].area(),desired[i]);
        scale=.75;refresh_targets();local_search=true;
        if(variant==64) {
            scale=1;refresh_targets();
            change(center,proposed,0,true);movable[center]=false;
        }
        sort(ids.begin(),ids.end(),[&](int a,int b){return desired[a]>desired[b];});
        for(int i:ids) if(destroyed[i] && (variant!=64 || i!=center))
            for(int attempt=0;attempt<10;++attempt) change(i,global_candidate(i),1e-7,true);
        if(value>chosen_value) {chosen_value=value;copy(state,state+n,chosen);}
        int steps=variant==61 || variant==64 ? 7000 : 4000;double temperature=.01;
        for(int step=0;step<steps;++step) {
            if((step&63)==0) {
                if(chrono::duration<double>(chrono::steady_clock::now()-start).count()>seconds-.005) break;
                double progress=double(step)/steps;
                scale=variant==64 ? 1.0 : min(1.0,.75+progress/.35*.25);refresh_targets();
                temperature=.01*pow(1-progress,4)+1e-7;
            }
            int i=ids[rng.integer(ids.size())],type=rng.integer(10),d=rng.integer(4);
            if(variant==64 && i==center) continue;
            Rect c=state[i];
            int delta=1+rng.integer(1<<rng.integer(11));
            if(type<4) {
                int width=(d&1) ? c.w() : c.h(),length=(d&1) ? c.h() : c.w();
                delta=max(0,int(round(desired[i]*scale/width))-length);
                c.q[d]=clamp(c.q[d]+(d<2 ? -delta : delta),0,10000);
                c=cap_expansion(i,c,d);
            } else if(type<6) {
                int axis=d&1,shift=d<2 ? -delta : delta;
                c.q[axis]+=shift;c.q[axis+2]+=shift;
            } else if(type==6) {
                c.q[d]+=d<2 ? delta : -delta;
            } else if(type==7) {
                int w=clamp(int(round(c.w()*exp(rng.real()-.5))),1,10000);
                int h=clamp(int(round(double(c.area())/w)),1,10000);
                int l=clamp(d&1 ? c.q[0] : c.q[2]-w,max(0,x[i]-w+1),min(x[i],10000-w));
                int b=clamp(d&2 ? c.q[1] : c.q[3]-h,max(0,y[i]-h+1),min(y[i],10000-h));
                c=Rect{{l,b,l+w,b+h}};
            } else c=global_candidate(i,type==9);
            change(i,c,temperature,(rng.next()&3)!=0);
            if(value>chosen_value) {chosen_value=value;copy(state,state+n,chosen);}
        }
        local_search=false;copy(chosen,chosen+n,state);value=chosen_value;scale=old_scale;refresh_targets();remember();
        if(value>original_value+1e-9) ++regional_improvements;
    }
    void reset_points() {
        value=0;
        for(int i=0;i<n;++i) {
            state[i]=Rect{{x[i],y[i],x[i]+1,y[i]+1}};
            value+=quality(1,desired[i]);
        }
    }
    void ordered_initialization() {
        vector<pair<double,int>> order;
        for(int i=0;i<n;++i) order.emplace_back(desired[i]*(.75+.5*rng.real()),i);
        sort(order.rbegin(),order.rend());
        scale=.55;
        refresh_targets();
        for(auto [priority,i]:order) {
            (void)priority;
            for(int attempt=0;attempt<12;++attempt) change(i,global_candidate(i),1e-8,true);
        }
    }
    void solve(double seconds, int selected_variant) {
#ifdef AHC_VARIANT
        if(selected_variant!=variant) throw invalid_argument("binary was compiled for a different variant");
#else
        variant = selected_variant;
#endif
        auto start = chrono::steady_clock::now();
        cin >> n;
        for (int i = 0; i < n; ++i) {
            cin >> x[i] >> y[i] >> desired[i];
            state[i] = Rect{{x[i],y[i],x[i]+1,y[i]+1}};
            value += quality(1, desired[i]);
            rng.s ^= uint64_t(x[i] + 10001*y[i] + desired[i]) * (i+1);
        }
        remember();
        refresh_targets();
        if (variant >= 3) prepare_empty_boxes();
        if(variant==41) for(int i=0;i<n;++i) {
            vector<pair<int,int>> order;
            for(int j=0;j<n;++j) if(j!=i) order.emplace_back((x[i]-x[j])*(x[i]-x[j])+(y[i]-y[j])*(y[i]-y[j]),j);
            sort(order.begin(),order.end());
            for(int k=0;k<min(8,int(order.size()));++k) close_points[i].push_back(order[k].second);
        }
        double preliminary=0;
        if (variant>=6 && variant<=9) {
            partition_search(seconds*(variant==6 ? .15 : variant==7 ? .4 : .98));
            preliminary=chrono::duration<double>(chrono::steady_clock::now()-start).count();
        }
        if(variant>=21 && variant!=26) ordered_initialization();
        double progress = 0, temperature = .004;
        bool directed=(variant>=12 && variant<=15) || variant==17 || variant>=19;
        int phases=variant==24 || variant==43 ? 2 : variant>=25 ? 1 : variant>=19 ? 4 : 1,phase=0;
        double polish_time=variant==27 ? seconds*.075 : variant==36 ? seconds*.025 : 0;
        double phase_seconds=(seconds-preliminary-polish_time)/phases;
        for (;; ++iterations) {
            if ((iterations & 1023) == 0) {
                double elapsed=chrono::duration<double>(chrono::steady_clock::now()-start).count();
                if (elapsed>=seconds-polish_time) break;
                int new_phase=min(phases-1,int((elapsed-preliminary)/phase_seconds));
                if(new_phase!=phase) {
                    phase=new_phase;
                    if(variant==20 || variant==22 || variant==43) {
                        reset_points();
                        if(variant==22 || variant==43) ordered_initialization();
                    } else {copy(best,best+n,state);value=best_value;}
                }
                progress = (elapsed-preliminary-phase_seconds*phase) / phase_seconds;
                if (progress >= 1) break;
                temperature = .004 * pow(.0000001/.004, progress);
                scale = variant == 1 ? 1.0 : min(1.0, .55 + progress/.65*.45);
                if (variant >= 4 && variant<6) {
                    temperature = .003 * pow(.000001/.003,progress);
                    scale = min(1.0, .65+progress/.35*.35);
                    shape_weight = .001*(1-progress)*(1-progress);
                }
                if (variant>=6 && variant<=9) scale=1;
                if (variant>=10) {
                    temperature=.03*pow(1-progress,4)+1e-7;
                    scale=variant==11 ? 1.0 : min(1.0,.55+progress/.45*.45);
                    if((variant==19 || variant==21 || (variant>=23 && variant!=43)) && phase>0) {
                        temperature=.01*pow(1-progress,4)+1e-7;
                        scale=min(1.0,.8+progress/.35*.2);
                    }
                }
                if(variant>=37 && variant<=40) {
                    double initial=(variant==37 || variant==39) ? .03 : .01;
                    double power=variant==39 ? 5 : variant==40 ? 4 : 3;
                    double ramp=variant==37 ? .35 : variant==40 ? .20 : .25;
                    temperature=initial*pow(1-progress,power)+1e-7;
                    scale=min(1.0,.55+progress/ramp*.45);
                }
                if(variant==42) scale=progress<.4 ? .55+progress/.4*.5 : 1.05-(progress-.4)/.6*.05;
                if(variant>=29) refresh_targets();
            }
            int i = rng.integer(n), type = rng.integer(10), d = rng.integer(4);
            if(variant==26 && (rng.next()&63)==0) {
                vector<int> neighbors;
                for(int j=0;j<n;++j) if(j!=i) {
                    int dx=max({0,state[i].q[0]-state[j].q[2],state[j].q[0]-state[i].q[2]});
                    int dy=max({0,state[i].q[1]-state[j].q[3],state[j].q[1]-state[i].q[3]});
                    if(dx+dy<=10) neighbors.push_back(j);
                }
                if(!neighbors.empty()) pair_change(i,neighbors[rng.integer(neighbors.size())],temperature);
                continue;
            }
            if(directed) {
                if(rng.next()&(variant==35 ? 7 : 1)) for(int k=0;k<(variant==35 ? 7 : 3);++k) {
                    int j=rng.integer(n);
                    auto shortage=[&](int id){
                        int capacity=empty_boxes[id].empty() ? desired[id] : min(desired[id],empty_boxes[id][0].area());
                        return scaled_quality(id,min(capacity,int(desired[id]*scale)))-scaled_quality(id,state[id].area());
                    };
                    if(shortage(j)>shortage(i)) i=j;
                }
                double ratio=state[i].area()/(desired[i]*scale);
                if(variant==48 && !empty_boxes[i].empty()) ratio=state[i].area()/min(desired[i]*scale,double(empty_boxes[i][0].area()));
                if(ratio>.98 && ratio<1.02 && (rng.next()&3)) type=7+rng.integer(3);
            }
            if(variant>=60 && progress>.55 && (rng.next()&8191)==0) {
                regional_search(i,start,seconds);continue;
            }
            if(variant==41 && (rng.next()&127)==0 && !close_points[i].empty()) {
                const auto& neighbors=close_points[i];
                int index=min(rng.integer(neighbors.size()),rng.integer(neighbors.size()));
                joint_change(i,neighbors[index],temperature);continue;
            }
            Rect c = state[i];
            int step = 1 + rng.integer(1 << rng.integer(11));
            if (type < 5) {
                if (type < 2) {
                    int other_width = (d&1) ? c.w() : c.h();
                    int width = (d&1) ? c.h() : c.w();
                    step = max(1, int(round(desired[i]*scale/other_width)) - width);
                    if(directed || variant==16) step=int(round(desired[i]*scale/other_width))-width;
                }
                c.q[d] += d < 2 ? -step : step;
                c.q[d] = clamp(c.q[d], 0, 10000);
                if(directed || variant==16) c=cap_expansion(i,c,d);
            } else if (type < 7) {
                c.q[d] += d < 2 ? step : -step;
            } else if (type < 9) {
                int axis = d&1, shift = d < 2 ? -step : step;
                c.q[axis] += shift; c.q[axis+2] += shift;
            } else if (variant>=14 && variant<=16 && (rng.next()&3)==0) {
                c=free_candidate(i);
            } else if (variant>=3 && (rng.next()&1)) {
                c = global_candidate(i,variant==55 || variant==56 || variant>=60);
            } else {
                int w = clamp(int(round(c.w()*exp((rng.real()-.5)*2))),1,10000);
                double target_area=desired[i]*scale;
                if((variant>=46 && variant<=49) || variant==53) {
                    if(variant!=47 || (rng.next()&1)) target_area=c.area();
                    else target_area=c.area()+rng.real()*(target_area-c.area());
                }
                int h = clamp(int(round(target_area/w)),1,10000);
                int l = clamp((c.q[0]+c.q[2]-w)/2, max(0,x[i]-w+1), min(x[i],10000-w));
                int b = clamp((c.q[1]+c.q[3]-h)/2, max(0,y[i]-h+1), min(y[i],10000-h));
                if(variant==49 || variant==53) {
                    l=clamp(d&1 ? c.q[0] : c.q[2]-w,max(0,x[i]-w+1),min(x[i],10000-w));
                    b=clamp(d&2 ? c.q[1] : c.q[3]-h,max(0,y[i]-h+1),min(y[i],10000-h));
                }
                c = Rect{{l,b,l+w,b+h}};
            }
            bool repair=(variant==13 || variant==15 || variant==16 || variant==17 || variant>=19)
                        && type==9 && (rng.next()&3)==0;
            if(((variant>=23 && variant<=25) || variant>=27) && type==9) {
                double gain=scaled_quality(i,c.area())-scaled_quality(i,state[i].area());
                repair=gain>1e-6 || (rng.next()&31)==0;
            }
            if((variant==44 || variant==45) && type==9 && (rng.next()&(variant==44 ? 15 : 3))==0)
                joint_shape_change(i,c,temperature);
            else if(variant==28 && type<5 && c.area()>state[i].area() && (rng.next()&3)==0)
                chain_change(i,c,d,temperature);
            else if(repair) repaired_change(i,c,temperature);
            else change(i, c, temperature, (variant >= 4 && variant<6) || (variant != 2 && (rng.next()&3) == 0));
        }
        if(polish_time>0) {
            copy(best,best+n,state);value=best_value;scale=1;
            refresh_targets();
            double before_polish=best_value;
            while(chrono::duration<double>(chrono::steady_clock::now()-start).count()<seconds) {
                int i=rng.integer(n);
                if(variant==36) {balance_edge(i,rng.integer(4));continue;}
                vector<int> neighbors;
                for(int j=0;j<n;++j) if(j!=i) {
                    int dx=max({0,state[i].q[0]-state[j].q[2],state[j].q[0]-state[i].q[2]});
                    int dy=max({0,state[i].q[1]-state[j].q[3],state[j].q[1]-state[i].q[3]});
                    if(dx+dy<=10) neighbors.push_back(j);
                }
                if(!neighbors.empty()) pair_change(i,neighbors[rng.integer(neighbors.size())],0);
            }
            cerr<<"polish_gain="<<llround((best_value-before_polish)/n*1e9)<<' ';
        }
        for (int i = 0; i < n; ++i) cout << best[i].q[0] << ' ' << best[i].q[1] << ' '
            << best[i].q[2] << ' ' << best[i].q[3] << '\n';
        cerr << "score=" << llround(best_value/n*1e9) << " iterations=" << iterations
             << " accepted=" << accepted << " all_anchors_bound=" << llround(empty_box_bound/n*1e9) << '\n';
        if(variant>=60) cerr<<"regional_trials="<<regional_trials<<" regional_improvements="<<regional_improvements<<'\n';
    }
};
int main(int argc, char** argv) {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    double seconds = argc > 1 ? stod(argv[1]) : 4.9;
    uint64_t seed = argc > 2 ? stoull(argv[2]) : 0;
    int variant = argc > 3 ? stoi(argv[3]) :
#ifdef AHC_VARIANT
        AHC_VARIANT;
#else
        64;
#endif
    Solver solver(seed); solver.solve(seconds, variant);
    return 0;
}
