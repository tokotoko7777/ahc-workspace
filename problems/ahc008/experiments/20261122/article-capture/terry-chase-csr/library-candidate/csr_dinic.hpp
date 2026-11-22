#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>

namespace ahc {

// Fixed topology, one unrestricted solve. See README.md for capacity rules.
class CsrDinic {
public:
    using Capacity = std::uint64_t;
    struct Arc { int from, to; Capacity capacity; };

    explicit CsrDinic(int vertices, std::span<const Arc> arcs) {
        if (vertices < 2) throw std::invalid_argument("need at least two vertices");
        if (arcs.size() > std::numeric_limits<std::size_t>::max()/2)
            throw std::length_error("residual edge count exceeds size_t");
        n_ = vertices;
        const auto n = static_cast<std::size_t>(n_);
        row_.resize(n+1);
        next_.resize(n);
        level_.resize(n);
        queue_.resize(n);
        for (auto [from,to,capacity] : arcs) {
            check_vertex(from);
            check_vertex(to);
            if (capacity != 0 && from != to) {
                ++row_[static_cast<std::size_t>(from)+1];
                ++row_[static_cast<std::size_t>(to)+1];
            }
        }
        for (std::size_t v=0;v<n;++v) row_[v+1]+=row_[v];
        edges_.resize(row_.back());
        std::copy_n(row_.begin(),n,next_.begin());
        for (auto [from,to,capacity] : arcs) {
            if (capacity == 0 || from == to) continue;
            const auto j=next_[from]++;
            const auto k=next_[to]++;
            edges_[j]={k,capacity,to};
            edges_[k]={j,0,from};
        }
    }

    CsrDinic(const CsrDinic&) = delete;
    CsrDinic& operator=(const CsrDinic&) = delete;

    Capacity solve(int source, int sink) {
        check_vertex(source);
        check_vertex(sink);
        if (source == sink) throw std::invalid_argument("source and sink must differ");
        if (solved_) throw std::logic_error("unrestricted solve already completed");

        // Before any residual mutation, certify a representable upper bound.
        constexpr Capacity max = std::numeric_limits<Capacity>::max();
        Capacity upper = 0;
        for (std::size_t j=row_[source];j<row_[static_cast<std::size_t>(source)+1];++j) {
            const auto cap=edges_[j].cap;
            if (cap>max-upper) throw std::overflow_error("outgoing source capacity exceeds uint64_t");
            upper+=cap;
        }
        Capacity value = 0;
        while (build_levels(source,sink)) {
            std::copy_n(row_.begin(),n_,next_.begin());
            while (const auto pushed = push(source,sink,max)) value+=pushed;
        }
        solved_=true;
        return value;
    }

    [[nodiscard]] bool on_source_side(int vertex) const {
        check_vertex(vertex);
        if (!solved_) throw std::logic_error("min-cut requires a completed unrestricted solve");
        return level_[vertex]>=0;
    }

private:
    struct Residual { std::size_t reverse; Capacity cap; int to; };
    int n_ = 0;
    bool solved_ = false;
    std::vector<std::size_t> row_,next_;
    std::vector<int> level_,queue_;
    std::vector<Residual> edges_;

    void check_vertex(int v) const {
        if (v<0 || v>=n_) throw std::out_of_range("vertex endpoint out of range");
    }
    bool build_levels(int s,int t) {
        std::fill(level_.begin(),level_.end(),-1);
        level_[s]=0;
        queue_[0]=s;
        for (int i=0,count=1;i<count;++i) {
            const int u=queue_[i];
            for (std::size_t j=row_[u];j<row_[static_cast<std::size_t>(u)+1];++j) {
                const auto& e=edges_[j];
                if (e.cap && level_[e.to]<0) {
                    level_[e.to]=level_[u]+1;
                    queue_[count++]=e.to;
                }
            }
        }
        return level_[t]>=0;
    }
    Capacity push(int u,int t,Capacity limit) {
        if (u==t) return limit;
        for (auto& j=next_[u];j<row_[static_cast<std::size_t>(u)+1];++j) {
            auto& e=edges_[j];
            if (!e.cap || level_[e.to]!=level_[u]+1) continue;
            const auto sent=push(e.to,t,std::min(limit,e.cap));
            if (sent) {
                e.cap-=sent;
                edges_[e.reverse].cap+=sent;
                return sent;
            }
        }
        return 0;
    }
};

} // namespace ahc
