#ifndef AHC_BOUNDED_EXACT_MEMO_HPP
#define AHC_BOUNDED_EXACT_MEMO_HPP
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

namespace ahc {
// Fixed-capacity direct mapping. The hash selects only a slot, never proves
// identity. Equal keys should have the same hash to maximize hit rate.
// Include every relevant input in Key, or clear() when fixed context changes.
template<class Key, class Value, std::size_t Capacity> class BoundedExactMemo {
    static_assert(std::has_single_bit(Capacity));
    struct Entry { Key key; Value value; };
    std::array<std::optional<Entry>,Capacity> slots{};
public:
    template<class Compute>
    std::pair<const Value*,bool> get_or_compute(Key key, std::uint64_t hash, Compute&& compute) {
        auto& slot=slots[hash&(Capacity-1)];
        if(slot && slot->key==key) return {&slot->value,true};
        // Compute before overwriting the current slot.
        Value result=std::forward<Compute>(compute)();
        slot.emplace(Entry{std::move(key),std::move(result)});
        return {&slot->value,false};
    }
    void clear() { for(auto& slot:slots)slot.reset(); }
};
}
#endif
