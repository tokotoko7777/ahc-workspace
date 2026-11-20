#include "../library/bounded-colored-path.hpp"
#include <array>
#include <iostream>
#include <vector>

int main() {
    const std::array<std::vector<int>, 4> edges{{{1, 2}, {3}, {1, 3}, {}}};
    const std::array<int, 4> color{0, 1, 1, 2};
    const std::array<int, 4> value{0, 4, 10, 0};
    std::vector<int> path, stack;
    std::vector<unsigned char> state(3);
    int score = bounded_colored_path::maximize(
        0, 3, 3, 3,
        [&](int v) -> const auto& { return edges[v]; },
        [&](int v) { return color[v]; },
        [&](int v) { return value[v]; },
        [](int) { return false; }, [](int) { return 0; }, path, stack, state);
    if (score != 10 || path != std::vector<int>{2}) return 1;
    std::cout << score << '\n';
}
