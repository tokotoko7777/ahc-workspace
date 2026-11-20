#pragma GCC optimize("O3")
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

struct Box {int a,b,c,d;};
struct Layout {std::vector<int> ends,width;};
using Score=long long;
struct Solver {
    int w,d,n;
    std::vector<std::vector<int>> area;
    uint32_t random=1854671119;
    unsigned roll(unsigned bound) {
        random^=random<<13;random^=random>>17;random^=random<<5;
        return random%bound;
    }
    Score evaluate(const Layout& l,std::vector<std::vector<Box>>* out=nullptr) const {
        Score cost=1;
        int left=0,first=0;
        for(int g=0;g<int(l.ends.size());++g) {
            int end=l.ends[g],wide=l.width[g],count=end-first;
            int fixed[50]{}, heights[50], cuts[50], old[50];
            int fixed_sum=0;
            for(int j=0;j<count;++j) {
                int peak=0;
                for(int t=0;t<d;++t) peak=std::max(peak,area[t][first+j]);
                fixed[j]=(peak+wide-1)/wide;
                fixed_sum+=fixed[j];
            }
            for(int t=0;t<d;++t) {
                int sum=0;
                if(fixed_sum<=w) {
                    for(int j=0;j<count;++j) heights[j]=fixed[j];
                    sum=fixed_sum;
                } else {
                    for(int j=0;j<count;++j) {
                        const int v=area[t][first+j];
                        heights[j]=(v+wide-1)/wide;
                        sum+=heights[j];
                    }
                    // The first reduction of a nonintegral height costs less
                    // area than any subsequent full-width reduction.
                    while(sum>w) {
                        int jbest=-1,loss=wide+1;
                        for(int j=0;j<count;++j) if(heights[j]>1) {
                            int value=std::clamp(area[t][first+j]-(heights[j]-1)*wide,0,wide);
                            if(value<loss) loss=value,jbest=j;
                        }
                        if(loss==wide) {
                            int take=std::min(heights[jbest]-1,sum-w);
                            heights[jbest]-=take;sum-=take;
                        } else {
                            --heights[jbest];--sum;
                        }
                    }
                }
                heights[count-1]+=w-sum;
                int coordinate=0;
                for(int j=0;j<count;++j) {
                    int next=coordinate+heights[j];
                    cost+=100LL*std::max(0,area[t][first+j]-heights[j]*wide);
                    if(out) (*out)[t][first+j]={coordinate,left,next,left+wide};
                    cuts[j]=next;coordinate=next;
                }
                if(t) {
                    int i=0,j=0,difference=0;
                    while(i<count-1 && j<count-1) {
                        if(old[i]==cuts[j]) ++i,++j;
                        else if(old[i]<cuts[j]) ++i,++difference;
                        else ++j,++difference;
                    }
                    cost+=1LL*wide*(difference+2*(count-1)-i-j);
                }
                std::copy_n(cuts,count-1,old);
            }
            left+=wide;first=end;
        }
        return cost;
    }
    Layout construct(int m,int shape) {
        Layout result;
        std::vector<int> cuts;
        if(shape==0) for(int i=1;i<m;++i) cuts.push_back(i*n/m);
        else if(shape==1) for(int i=1;i<m;++i) cuts.push_back(n-m+i);
        else {
            for(int i=1;i<n;++i) cuts.push_back(i);
            for(int i=0;i<m-1;++i) std::swap(cuts[i],cuts[i+roll(n-1-i)]);
            cuts.resize(m-1);
            std::sort(cuts.begin(),cuts.end());
        }
        cuts.push_back(n);
        result.ends=cuts;
        std::vector<int> requests;
        int total=0,first=0;
        for(int e:cuts) {
            int peak=0;
            for(auto& row:area)
                peak=std::max(peak,std::accumulate(row.begin()+first,row.begin()+e,0));
            requests.push_back(peak);
            total+=peak;first=e;
        }
        int assigned=0;
        for(int i=0;i<m;++i) {
            auto width=1+int(1LL*(w-m)*requests[i]/total);
            result.width.push_back(width);
            assigned+=width;
        }
        for(int i=m-1;assigned<w;--i,++assigned) ++result.width[i];
        return result;
    }
};
int main() {
    std::ios::sync_with_stdio(false);std::cin.tie(nullptr);
    Solver s;
    if(!(std::cin>>s.w>>s.d>>s.n)) return 1;
    s.area.assign(s.d,std::vector<int>(s.n));
    for(auto& row:s.area) for(int& a:row) std::cin>>a;
    Layout answer{{s.n},{s.w}};
#ifndef AHC031_SINGLE_STRIP
    Score best=s.evaluate(answer);
    // Candidate counts are bounded; every candidate uses only known demands.
    for(int m=2;best>1 && m<=std::min(10,s.n);++m) for(int restart=0;restart<4 && best>1;++restart) {
        Layout current=s.construct(m,restart);
        Score score=s.evaluate(current);
        for(int step=0;step<48;++step) {
            if(score<best) best=score,answer=current;
            if(best==1) break;
            Layout next=current;
            if(step%4==0) {
                int i=s.roll(m-1);
                int j=next.ends[i]+(s.roll(2)==0?1:-1);
                if(j>=next.ends[i+1] || j<=(i?next.ends[i-1]:0)) continue;
                next.ends[i]=j;
            } else {
                int a=s.roll(m),b=s.roll(m-1);
                if(b>=a) ++b;
                int change=1+s.roll(step<24?36:13);
                if(next.width[a]<=change) continue;
                next.width[a]-=change;
                next.width[b]+=change;
            }
            Score cost=s.evaluate(next);
            if(cost<=score) current=std::move(next),score=cost;
        }
        if(score<best) best=score,answer=current;
    }
#endif
    std::vector<std::vector<Box>> boxes(s.d,std::vector<Box>(s.n));
    std::cerr<<"Model = "<<s.evaluate(answer,&boxes)<<'\n';
    for(auto& row:boxes) for(auto [a,b,c,d]:row)
        std::cout<<a<<' '<<b<<' '<<c<<' '<<d<<'\n';
}
