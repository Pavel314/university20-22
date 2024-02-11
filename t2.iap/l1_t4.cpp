#include <array>
#include <chrono>
#include <conio.h>
#include <iostream>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <vector>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace conio
{
int kbhit()
{
    return ::kbhit();
}
int getch()
{
    return ::getch();
}
void gotoxy(int x, int y)
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
}
void clrscr()
{
    system("cls");
}

}
struct pos {
    int x;
    int y;
    friend bool operator==(pos l, pos r)
    {
        return (l.x == r.x) && (l.y == r.y);
    }
    friend bool operator!=(pos l, pos r)
    {
        return !(l == r);
    }
};

template <typename ElemTp, std::size_t Width, std::size_t Height>
class matrix : public std::array<ElemTp, Width * Height>
{
public:
    using base_t = std::array<ElemTp, Width * Height>;
    constexpr static decltype(Width) width = Width;
    constexpr static decltype(Height) height = Height;
    constexpr typename base_t::reference operator[](pos ind) // override
    {
        return base_t::operator[](ind.y* width + ind.x);
    }
};

template <typename int_t> int_t get_random(int_t min, int_t max)
{
    struct eng_wrap {
        std::mt19937 engine;
        eng_wrap()
        {
            std::random_device device;
            engine.seed(device());
        }
    };
    static eng_wrap wrap;

    std::uniform_int_distribution<int_t> dist(min, max);
    return dist(wrap.engine);
}
enum element : char { empty = ' ', wall = '*', head = '0', tail = 'o', food = 'A' };
enum class direction { up, down, left, right };

constexpr int cx = 80;
constexpr int cy = 25;
constexpr int default_tails = 3;

std::vector<pos> snake;
matrix<char, (cx + 1), cy> field; // extra line for newline
bool end_game;
direction dir;
int scores;

template <typename Func> void visit_field(const Func& func)
{
    int y = 0;
    int x = 0;
    while(1) {
        if(x == cx) {
            x = 0;
            ++y;
        }
        if(y == cy)
            break;
        if(!func(x, y))
            break;
        ++x;
    }
}

void gen_food()
{
    int free_count = 0;
    visit_field([&free_count](int x, int y) {
        if(field[{ x, y }] == element::empty)
            ++free_count;
        return true;
    });

    int rnd = get_random(0, free_count);
    int ind = 0;
    visit_field([&ind, rnd](int x, int y) {
        if(field[{ x, y }] == element::empty) {
            if(ind == rnd) {
                field[{ x, y }] = element::food;
                return false;
            }
            ++ind;
        }
        return true;
    });
}

bool check_dir(direction cur, direction next)
{
    if(cur == direction::up && next == direction::down) {
        return false;
    }
    if(cur == direction::down && next == direction::up) {
        return false;
    }
    if(cur == direction::left && next == direction::right) {
        return false;
    }
    if(cur == direction::right && next == direction::left) {
        return false;
    }
    return true;
}

void move_snake(direction dir)
{
    auto next_pos = snake[0];
    switch(dir) {
    case direction::up:
        next_pos.y -= 1;
        break;
    case direction::down:
        next_pos.y += 1;
        break;
    case direction::left:
        next_pos.x -= 1;
        break;
    case direction::right:
        next_pos.x += 1;
        break;
    }

    const char collision = field[next_pos];

    field[snake[0]] = element::tail;

    field[next_pos] = element::head;
    ;
    field[snake.back()] = element::empty;

    for(int i = snake.size() - 2; i >= 0; --i) {
        snake[i + 1] = snake[i];
    }
    snake[0] = next_pos;

    if(collision != element::empty) {
        if(collision == element::food) {
            auto a = snake[snake.size() - 1];
            auto b = snake[snake.size() - 2];
            a.x += a.x - b.x;
            a.y += a.y - b.y;
            snake.push_back(a);
            field[a] = element::tail;
            ++scores;
            gen_food();
        } else {
            end_game = true;
        }
    }
}

void init()
{
    visit_field([](int x, int y) {
        if(x == cx - 1)
            field[{ x + 1, y }] = '\n';
        if(x == 0 || x == cx - 1 || y == 0 || y == cy - 1) { // ||(x==15 && y>1 && y<cy-2)
            field[{ x, y }] = element::wall;
        } else
            field[{ x, y }] = element::empty;
        return true;
    });
    snake.resize(default_tails + 1);

    int ox = cx / 2;
    int oy = cy / 2;
    snake[0] = { ox, oy };
    field[snake[0]] = element::head;

    for(int i = 1; i <= default_tails; ++i) {
        snake[i] = { ox, ++oy };
        field[snake[i]] = element::tail;
    }
    dir = direction::up;
    end_game = false;
    scores = 0;
    gen_food();
}

void draw()
{
    conio::gotoxy(0, 0);
    std::cout << "score=" << scores << '\n';
    std::cout.write(field.data(), field.size());
}

bool do_again()
{
    std::cout << "Game over. Press space to play again" << std::flush;
    while(true) {
        if(conio::kbhit() != 0) {
            if(conio::getch() == 32) {
                conio::clrscr();
                return true;
            }
        }
    }
}
// std::chrono::milliseconds(50)
auto get_pause(int scores)
{
    std::array<int, 5> levels { 200, 150, 120, 80, 75 };
    constexpr int changed=10;
    constexpr int max_scores = levels.size() * changed - 1;
    if(scores > max_scores)
        scores = max_scores;
    return std::chrono::milliseconds(levels[scores / changed]);
}

int main(int argc, char** argv)
{
    //  std::ios_base::sync_with_stdio(false);
    init();
    while(true) {
        direction next = dir;
        if(conio::kbhit() != 0) {
            if(conio::getch() == 224) {
                switch(conio::getch()) {
                case 75:
                    next = direction::left;
                    break;
                case 72:
                    next = direction::up;
                    break;
                case 77:
                    next = direction::right;
                    break;
                case 80:
                    next = direction::down;
                    break;
                }
            }
        }
        if(check_dir(dir, next))
            dir = next;
        move_snake(dir);
        draw();
        if(end_game) {
            do_again();
            init();
        }
        std::this_thread::sleep_for(get_pause(scores));
    }

    return 0;
}
