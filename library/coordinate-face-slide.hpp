#pragma once
#include <algorithm>
#include <array>
#include <numeric>
#include <utility>
#include <vector>

// Moves touching rectangle faces as a group, keeping all other faces fixed.
// Face IDs are 4*i+d (d=0,1 lower x,y, d=2,3 upper x,y).
// get(i,d) reads a half-open rectangle coordinate. Input rectangles are
// legal and pairwise nonoverlapping. build() returns the components for one
// axis; refresh before each sweep because a move can form new contacts.
namespace coordinate_face_slide {
template<class Getter>
bool crosses(const Getter& get, int i, int j, int axis) {
    const int t = axis ^ 1;
    return get(i,t) < get(j,t+2) && get(j,t) < get(i,t+2);
}

template<class Getter>
std::vector<std::vector<int>> build(int n, int axis, const Getter& get) {
    std::vector<int> parent(2*n);
    std::iota(parent.begin(),parent.end(),0);
    auto root=[&](int i) {
        while(parent[i]!=i) { parent[i]=parent[parent[i]]; i=parent[i]; }
        return i;
    };
    for(int i=0;i<n;++i) for(int j=0;j<i;++j) {
        if(!crosses(get,i,j,axis)) continue;
        if(get(i,axis)==get(j,axis+2)) parent[root(2*i)]=root(2*j+1);
        if(get(j,axis)==get(i,axis+2)) parent[root(2*j)]=root(2*i+1);
    }
    std::vector<std::vector<int>> result(2*n);
    for(int i=0;i<n;++i) {
        result[root(2*i)].push_back(4*i+axis);
        result[root(2*i+1)].push_back(4*i+axis+2);
    }
    result.erase(std::remove_if(result.begin(),result.end(),
                                [](const auto& g){return g.empty();}),result.end());
    return result;
}

// Inclusive feasible coordinates for the entire group. extra(i,d) returns
// an inclusive domain that can encode containment of a required point.
// Mark every member before looking up currently adjacent fixed blockers.
template<class Getter, class Extra>
std::pair<int,int> limits(int n, int axis, const Getter& get, const Extra& extra,
                          const std::vector<int>& group, int lower, int upper,
                          std::vector<unsigned char>& membership) {
    membership.assign(4*n,0);
    for(int f:group) membership[f]=1;
    const int old=get(group.front()/4,group.front()%4);
    for(int f:group) {
        const int i=f/4,d=f%4;
        const auto [a,b]=extra(i,d);
        lower=std::max(lower,a); upper=std::min(upper,b);
        if(d<2) {
            upper=std::min(upper,get(i,d+2)-1);
            for(int j=0;j<n;++j) if(j!=i && !membership[4*j+axis+2]) {
                const int z=get(j,axis+2);
                if(z>lower && z<=old && crosses(get,i,j,axis)) lower=z;
            }
        } else {
            lower=std::max(lower,get(i,d-2)+1);
            for(int j=0;j<n;++j) if(j!=i && !membership[4*j+axis]) {
                const int z=get(j,axis);
                if(z<upper && z>=old && crosses(get,i,j,axis)) upper=z;
            }
        }
    }
    return {lower,upper};
}
}
