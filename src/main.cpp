#include <algorithm>
#include <array>
#include <iostream>
#include <z3++.h>

std::vector<std::vector<z3::expr>> board_const(z3::context& c, std::size_t w, std::size_t h, std::string name) {
    std::vector<std::vector<z3::expr>> result;
    for(std::size_t y = 0; y < h; y++) {
        std::vector<z3::expr> row;
        std::string y_str = std::to_string(y);
        for(std::size_t x = 0; x < w; x++) {
            std::string x_str = std::to_string(x);
            std::string name_str = name + "[" + y_str + "][" + x_str + "]";
            row.push_back(c.int_const(name_str.c_str()));
        }
        result.push_back(row);
    }
    return result;
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    std::vector<std::vector<std::uint32_t>> data {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 0, 0},
        {0, 0, 0, 0, 0, 0, 4, 5, 0, 0, 8, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 9, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 8, 10, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 11, 12, 0, 0},
        {0, 3, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0},
        {0, 14, 0, 0, 0, 14, 0, 10, 0, 0, 13, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 12, 0, 0, 0},
        {0, 0, 15, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0},
        {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    const std::size_t w = data[0].size();
    const std::size_t h = data.size();

    for(std::size_t y = 0; y < h; y++) {
        for(std::size_t x = 0; x < w; x++) {
            std::cout << data[y][x] << "\t";
        }
        std::cout << std::endl;
    }

    z3::context c;
    
    std::vector<std::vector<z3::expr>> board = board_const(c, w, h, "board");

    z3::solver s(c);

    for(std::size_t y = 0; y < h; y++) {
        for(std::size_t x = 0; x < w; x++) {
            z3::expr here = board[y][x];
            bool is_endpoint = data[y][x] != 0;

            // If we are an endpoint, add that constraint
            if(is_endpoint) {
                // FIXME numeric cast
                s.add(here == ((int) data[y][x]));
            }

            // Add neighbor constraint
            z3::expr_vector neighbor_equality_constraints(c);
            if(y > 0) neighbor_equality_constraints.push_back(here == board[y - 1][x]);
            if(y < h - 1) neighbor_equality_constraints.push_back(here == board[y + 1][x]);
            if(x > 0) neighbor_equality_constraints.push_back(here == board[y][x - 1]);
            if(x < w - 1) neighbor_equality_constraints.push_back(here == board[y][x + 1]);

            const int num_neighbors_here = is_endpoint ? 1 : 2;
            std::vector<int> vector_of_ones(neighbor_equality_constraints.size(), 1);

            z3::expr neighbor_constraint = z3::pbeq(neighbor_equality_constraints, vector_of_ones.data(), num_neighbors_here);
            s.add(neighbor_constraint);
        }
    }

    std::cout << s.check() << "\n";

    z3::model m = s.get_model();
    // std::cout << m << "\n";
    // traversing the model
    for(std::size_t y = 0; y < h; y++) {
        for(std::size_t x = 0; x < w; x++) {
            std::cout << m.eval(board[y][x]) << "\t";
        }
        std::cout << std::endl;
    }

    return 0;
}
