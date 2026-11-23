#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <optional>

namespace observed_task {

enum class Kind { normal, repair, egress };
enum class Ack { applied, rejected, unknown };
struct Request {
    Kind kind;
    std::size_t task;   // ordinal; 0 is a placeholder for egress.
    friend bool operator==(const Request&, const Request&) = default;
};
struct Ticket {
    std::uint64_t token;
    bool fresh;         // false means already outstanding: do not transmit twice.
    friend bool operator==(const Ticket&, const Ticket&) = default;
};

class Cursor {
    struct Flight { Request req; std::uint64_t token; };
    std::size_t total_, capacity_, head_=0;
    std::deque<std::size_t> debt_;
    std::optional<Flight> pending_;
    std::uint64_t version_=0, sequence_=0;
    bool abort_=false, safe_=false;

public:
    explicit Cursor(std::size_t tasks, std::size_t debt_capacity=0,
                    std::uint64_t initial_version=0)
        : total_(tasks), capacity_(debt_capacity), version_(initial_version) {}

    std::size_t head() const { return head_; }
    std::size_t debt_size() const { return debt_.size(); }
    std::uint64_t version() const { return version_; }
    bool aborted() const { return abort_; }
    bool safe_confirmed() const { return safe_; }
    bool done() const { return !abort_ && head_==total_ && debt_.empty() && !pending_; }
    std::optional<std::uint64_t> pending_token() const {
        if(pending_) return pending_->token;
        return std::nullopt;
    }

    std::optional<Request> next() const {
        if(pending_ || safe_) return std::nullopt;
        if(abort_) return Request{Kind::egress,0};
        if(head_<total_) return Request{Kind::normal,head_};
        if(!debt_.empty()) return Request{Kind::repair,debt_.front()};
        return std::nullopt;
    }

    // Call only when submitting 'expected' for real, keyed to that observation.
    std::optional<Ticket> submit(Request expected, std::uint64_t version) {
        if(pending_) {
            if(pending_->req==expected && version>=version_)
                return Ticket{pending_->token,false};
            return std::nullopt;
        }
        if(version<version_ || next()!=expected || sequence_==std::numeric_limits<std::uint64_t>::max())
            return std::nullopt;
        version_=version;
        pending_=Flight{expected,++sequence_};
        return Ticket{pending_->token,true};
    }

    // true: this matching, newer observation was consumed, not necessarily resolved.
    bool observe(std::uint64_t token, std::uint64_t newer, Ack ack, bool attributed) {
        if(!pending_ || pending_->token!=token || newer<=version_) return false;
        version_=newer;
        if(!attributed || ack==Ack::unknown) return true;
        if(ack==Ack::applied) {
            if(pending_->req.kind==Kind::normal) ++head_;
            if(pending_->req.kind==Kind::repair) debt_.pop_front();
        }
        pending_.reset();
        return true;
    }

    // Caller certifies both skipping-now dependencies and future safe repair.
    // This operation advances the work cursor, not observed physical progress.
    bool defer_head(std::uint64_t version, bool certified) {
        if(version<version_ || !certified || pending_ || abort_ || head_==total_ || debt_.size()==capacity_)
            return false;
        debt_.push_back(head_);
        ++head_;
        version_=version;
        return true;
    }

    void request_abort() { if(!done()) abort_=true; }

    bool confirm_safe(std::uint64_t newer, bool certified) {
        if(!abort_ || safe_ || pending_ || newer<=version_ || !certified) return false;
        version_=newer;
        safe_=true;
        return true;
    }
};

} // namespace observed_task
