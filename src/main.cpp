#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <memory>
#include <raylib.h>
#include <z3++.h>

struct FlowState {
    std::size_t width;
    std::size_t height;
    std::vector<std::vector<int>> board;

    FlowState(std::size_t width, std::size_t height, std::vector<std::vector<int>> board) : width(width), height(height), board(board) {}

    void resize(std::size_t new_width, std::size_t new_height) {
        width = new_width;
        height = new_height;
        std::for_each(board.begin(), board.end(), [new_width](std::vector<int>& row) {
            row.resize(new_width, 0);
        });
        board.resize(new_height, std::vector<int>(new_width, 0));
    }

    void increment(std::size_t x, std::size_t y) {
        board[y][x] = board[y][x] + 1;
    }

    void decrement(std::size_t x, std::size_t y) {
        if(board[y][x] > 0) board[y][x] = board[y][x] - 1;
    }
};

const FlowState initial_flow_state(14, 14, {
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
});

const Color colors[15] = {
    RED,
    GREEN,
    BLUE,
    YELLOW,
    PURPLE,
    ORANGE,
    SKYBLUE,
    BEIGE,
    DARKBLUE,
    GRAY,
    DARKGREEN,
    GOLD,
    MAROON,
    PINK,
    LIGHTGRAY,
};

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

std::vector<std::vector<int>> solve(std::shared_ptr<z3::context> c, FlowState state) {
    std::vector<std::vector<z3::expr>> board = board_const(*c, state.width, state.height, "board");

    z3::solver s(*c);

    for(std::size_t y = 0; y < state.height; y++) {
        for(std::size_t x = 0; x < state.width; x++) {
            z3::expr here = board[y][x];
            bool is_endpoint = state.board[y][x] != 0;

            // If we are an endpoint, add that constraint
            if(is_endpoint) {
                s.add(here == state.board[y][x]);
            }

            // Add neighbor constraint
            z3::expr_vector neighbor_equality_constraints(*c);
            if(y > 0) neighbor_equality_constraints.push_back(here == board[y - 1][x]);
            if(y < state.height - 1) neighbor_equality_constraints.push_back(here == board[y + 1][x]);
            if(x > 0) neighbor_equality_constraints.push_back(here == board[y][x - 1]);
            if(x < state.width - 1) neighbor_equality_constraints.push_back(here == board[y][x + 1]);

            const int num_neighbors_here = is_endpoint ? 1 : 2;
            std::vector<int> vector_of_ones(neighbor_equality_constraints.size(), 1);

            z3::expr neighbor_constraint = z3::pbeq(neighbor_equality_constraints, vector_of_ones.data(), num_neighbors_here);
            s.add(neighbor_constraint);
        }
    }

    // Return no results when the problem can't be satisfied
    if(s.check() == z3::check_result::unsat) return std::vector<std::vector<int>>(0);

    z3::model m = s.get_model();

    std::vector<std::vector<int>> result;
    for(std::size_t y = 0; y < state.height; y++) {
        std::vector<int> row;
        for(std::size_t x = 0; x < state.width; x++) {
            row.push_back(m.eval(board[y][x]).get_numeral_int());
        }
        result.push_back(row);
    }

    return result;
}

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    const int screen_width = 800;
    const int screen_height = 600;

    InitWindow(screen_width, screen_height, "flowsolver");
    SetTargetFPS(60);

    FlowState state = initial_flow_state;

    std::future<std::vector<std::vector<int>>> solve_status;
    std::shared_ptr<z3::context> solve_context = nullptr;
    std::vector<std::vector<int>> solution_data;

    bool state_changed = true;
    bool solution_dirty = true;

    while(!WindowShouldClose()) {
        // Do updating here
        if(IsKeyPressed(KEY_UP) && state.height > 1) {
            state.resize(state.width, state.height - 1);
            state_changed = true;
        }
        if(IsKeyPressed(KEY_DOWN)) {
            state.resize(state.width, state.height + 1);
            state_changed = true;
        }
        if(IsKeyPressed(KEY_LEFT) && state.width > 1) {
            state.resize(state.width - 1, state.height);
            state_changed = true;
        }
        if(IsKeyPressed(KEY_RIGHT)) {
            state.resize(state.width + 1, state.height);
            state_changed = true;
        }

        int grid_size = 500;

        int grid_width;
        int grid_height;

        if(state.width < state.height) {
            grid_height = grid_size;
            grid_width = (int) ((grid_height * state.width) / state.height);
        } else if (state.width > state.height) {
            grid_width = grid_size;
            grid_height = (int) ((grid_width * state.height) / state.width);
        } else {
            grid_width = grid_size;
            grid_height = grid_size;
        }

        int width_per_cell = grid_width / ((int) state.width);
        int height_per_cell = grid_height / ((int) state.height);

        int grid_start_x = (screen_width - grid_width) / 2;
        int grid_start_y = (screen_height - grid_height) / 2;

        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int press_x = GetMouseX();
            int press_y = GetMouseY();
            if(press_x > grid_start_x && press_x < (grid_start_x + grid_width) && press_y > grid_start_y && press_y < (grid_start_y + grid_height)) {
                int grid_x_offset = press_x - grid_start_x;
                int grid_x_cell = grid_x_offset / width_per_cell;
                int grid_y_offset = press_y - grid_start_y;
                int grid_y_cell = grid_y_offset / height_per_cell;
                state.increment(grid_x_cell, grid_y_cell);
                state_changed = true;
            }
        }
        if(IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            int press_x = GetMouseX();
            int press_y = GetMouseY();
            if(press_x > grid_start_x && press_x < (grid_start_x + grid_width) && press_y > grid_start_y && press_y < (grid_start_y + grid_height)) {
                int grid_x_offset = press_x - grid_start_x;
                int grid_x_cell = grid_x_offset / width_per_cell;
                int grid_y_offset = press_y - grid_start_y;
                int grid_y_cell = grid_y_offset / height_per_cell;
                state.decrement(grid_x_cell, grid_y_cell);
                state_changed = true;
            }
        }

        if(state_changed) {
            if(solve_context) solve_context->interrupt();

            solve_context = std::make_shared<z3::context>();
            solve_status = std::async(std::launch::async, solve, solve_context, state);
            
            state_changed = false;
            solution_dirty = true;
        }

        if(solution_dirty) {
            if(solve_status.valid()) {
                if(solve_status.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    try {
                        solution_data = solve_status.get();
                    } catch (const std::exception& e) {
                        std::cout << e.what() << std::endl;
                    }
                    solution_dirty = false;
                }
            }
        }

        BeginDrawing();

        ClearBackground(BLACK);

        Color grid_color = (!solution_dirty && solution_data.empty()) ? (Color { 80, 0, 0, 255 }) : DARKGRAY;

        int line_width = (6 * width_per_cell) / 10;
        int line_height = (6 * height_per_cell) / 10;
        int line_hthick = height_per_cell / 5;
        int line_vthick = width_per_cell / 5;
        int line_hxoffs = (4 * width_per_cell) / 10;
        int line_hyoffs = (4 * height_per_cell) / 10;
        int line_vxoffs = (4 * width_per_cell) / 10;
        int line_vyoffs = (4 * width_per_cell) / 10;

        // draw grid
        for(int y = 0; y < state.height; y++) {
            int ypos = (height_per_cell * y) + grid_start_y;
            for(int x = 0; x < state.width; x++) {
                int xpos = (width_per_cell * x) + grid_start_x;

                // Draw grid cell
                DrawRectangleLines(xpos, ypos, width_per_cell, height_per_cell, grid_color);

                // Draw endpoint
                int here = state.board[y][x];
                if(here > 0) {
                    DrawEllipse(xpos + (width_per_cell / 2), ypos + (height_per_cell / 2), (width_per_cell * 2.0f) / 5.0f, (height_per_cell * 2.0f) / 5.0f, colors[here - 1]);
                }

                // Draw solution data
                if(!solution_dirty && !solution_data.empty()) {
                    int solution_here = solution_data[y][x];
                    Color color_here = solution_here > 0 ? colors[solution_here - 1] : MAGENTA;

                    if(y > 0 && solution_here == solution_data[y - 1][x]) {
                        DrawRectangle(xpos + line_vxoffs, ypos, line_vthick, line_height, color_here);
                    }
                    if(y < (state.height - 1) && solution_here == solution_data[y + 1][x]) {
                        DrawRectangle(xpos + line_vxoffs, ypos + line_vyoffs, line_vthick, line_height, color_here);
                    }
                    if(x > 0 && solution_here == solution_data[y][x - 1]) {
                        DrawRectangle(xpos, ypos + line_hyoffs, line_width, line_hthick, color_here);
                    }
                    if(x < (state.width - 1) && solution_here == solution_data[y][x + 1]) {
                        DrawRectangle(xpos + line_hxoffs, ypos + line_hyoffs, line_width, line_hthick, color_here);
                    }
                }
            }
        }
        

        EndDrawing();
    }

    CloseWindow();
    
    if(solve_context) solve_context->interrupt();

    return 0;
}
