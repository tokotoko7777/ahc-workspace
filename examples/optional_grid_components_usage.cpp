#include "optional-grid-components.hpp"
#include <array>
#include <cstdint>
#include <iostream>

int main() {
    using ahc::grid_cell;
    using Grid = ahc::optional_grid_components<2>;
    // Two synthetic independent quantities: stored stock, net demand.
    const std::array cells{grid_cell::fixed_open, grid_cell::optional,
                           grid_cell::fixed_open, grid_cell::blocked,
                           grid_cell::fixed_open};
    const std::array<Grid::weight_type, 5> weight{
        Grid::weight_type{2, -4}, {1, 0}, {4, 1}, {999, 999}, {7, 3}
    };
    const Grid prep(1, 5, cells, weight);
    const auto print = [](const Grid::summary_type& s) {
        std::cout << s.cells << ' ' << s.weight[0] << ' ' << s.weight[1] << '\n';
    };
    for (const auto& s : prep.unaffected()) print(s); // always 1 7 3
    for (const auto& s : prep.affected(std::array<std::uint8_t, 1>{1})) {
        print(s); // 3 7 -3
    }
}
