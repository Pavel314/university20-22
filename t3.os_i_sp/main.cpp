#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <pthread.h>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// TODO std :: filesystem :: perms
/*
 COMPILER FLAGS: -std=c++20; -lpthread; -D_REENTRANT
 LINKER FLAGS: -static; -static-libgcc; -static-libstdc++
*/

namespace fs = std::filesystem;

namespace utils
{
namespace implementation_details
{
    struct eng_wrap {
        std::mt19937 engine;
        eng_wrap()
        {
            std::random_device device;
            engine.seed(device());
        }
        std::mt19937& operator()()
        {
            return engine;
        }
    };
    eng_wrap rnd_eng;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
}

template <typename int_t, int_t a, int_t b> int_t random_int()
{
    static_assert(a <= b);
    static std::uniform_int_distribution<int_t> distr(a, b);
    return distr(implementation_details::rnd_eng());
}

void start_timer()
{
    implementation_details::start_time = std::chrono::high_resolution_clock::now();
}

template <class ToDuration = std::chrono::milliseconds> ToDuration stop_timer()
{
    auto stop_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<ToDuration>(stop_time - implementation_details::start_time);
}

template <class TException = std::runtime_error> void exception_assert(bool cond, const char* msg)
{
    if(!cond)
        throw TException(msg);
}
}

template <char first_letter = 'a', char last_letter = 'z'>
std::size_t create_random_file(const fs::path& path, std::size_t bytes)
{
    static_assert(first_letter <= last_letter);
    constexpr int letters = last_letter - first_letter;
    std::array<char, 4096> buffer;
    std::size_t lines = 0;
    std::ofstream ofs(path, std::ios_base::out | std::ios_base::trunc);
    std::size_t ind = 0;
    while(bytes--) {
        const int val = utils::random_int<int, 0, last_letter - first_letter + 1>(); //+1 for new line
        if(val != letters + 1) {
            buffer[ind] = static_cast<char>(first_letter + val);
        } else {
            ++lines;
            buffer[ind] = '\n';
        }
        ++ind;
        if(ind == buffer.size()) {
            ind = 0;
            ofs.write(buffer.data(), buffer.size());
        }
    }
    ofs.write(buffer.data(), ind);
    return lines > 0 ? lines + 1 : 0;
}

std::size_t calculate_lines(const fs::path& path)
{
    std::ifstream ifs(path);
    if(ifs.peek() == std::ifstream::traits_type::eof())
        return 0;
    return std::count(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(), '\n') + 1;
}

std::size_t remove_calculate_lines(const fs::path& p)
{
    const std::size_t lines = calculate_lines(p);
    fs::remove(p);
    return lines;
}

constexpr std::size_t default_files_count = 100;
static_assert(default_files_count > 0);
constexpr std::size_t min_size = 1024 * 10;
constexpr std::size_t max_size = 1024 * 1024;
static_assert(min_size<=max_size);

bool create_confirm = true;
fs::path temp_path = fs::temp_directory_path();

struct file_info {
    fs::path path;
    std::size_t size;
};
std::vector<file_info> files;
decltype(files)::size_type files_count = default_files_count;

void writer_message(const file_info& file, std::size_t lines)
{
    std::cout << "writer: File " << file.path.filename() << " Lines=" << lines << ", Size=" << file.size << '\n';
}

void reader_message(const file_info& file, std::size_t lines)
{
    std::cout << "reader: File " << file.path.filename() << " Lines " << lines << ", Size=" << file.size << '\n';
}

void singlethread_work()
{
    for(const auto& file : files) {
        writer_message(file, create_random_file(file.path, file.size));
        reader_message(file, remove_calculate_lines(file.path));
    }
}

std::queue<const file_info*> queue;
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cout_lock = PTHREAD_MUTEX_INITIALIZER;
bool queue_bottom = false;
/*We should consider the variable queue_bottom as a member of the queue.
Accordingly, mutex queue_lock also applies to this variable */
pthread_cond_t queue_push_cond;

void* thread_reader(void*)
{
    while(true) {
        pthread_mutex_lock(&queue_lock);
        while(queue.empty()) {
            if(queue_bottom)
                return nullptr;
            pthread_cond_wait(&queue_push_cond, &queue_lock);
        }
        const file_info* file = queue.front();
        queue.pop();
        pthread_mutex_unlock(&queue_lock);

        std::size_t lines = remove_calculate_lines(file->path);

        pthread_mutex_lock(&cout_lock);
        reader_message(*file, lines);
        pthread_mutex_unlock(&cout_lock);
    }
    return nullptr;
}

void dualthread_work()
{
    pthread_t reader;
    utils::exception_assert(pthread_create(&reader, nullptr, thread_reader, nullptr) == 0, "pthread_create failed");
    utils::exception_assert(pthread_cond_init(&queue_push_cond, nullptr) == 0, "pthread_cond_init failed");

    for(const auto& file : files) {
        std::size_t lines = create_random_file(file.path, file.size);

        pthread_mutex_lock(&cout_lock);
        writer_message(file, lines);
        pthread_mutex_unlock(&cout_lock);

        pthread_mutex_lock(&queue_lock);
        queue.push(&file);
        pthread_cond_signal(&queue_push_cond);
        pthread_mutex_unlock(&queue_lock);
    }
    pthread_mutex_lock(&queue_lock);
    queue_bottom = true;
    pthread_mutex_unlock(&queue_lock);

    pthread_join(reader, nullptr);
    pthread_cond_destroy(&queue_push_cond);
}

namespace args_processor
{
bool arg_help(std::string_view)
{
    std::cout <<
        R"(=======================================
Welcome to SimplestThreadingTester 1.0!
========================================
Q. What is the program intended for?
A. To compare the speed of performing some work in single-threaded mode and dual-threaded mode.

Q. What are the settings?
A. We have a 'files' variable responsible for the number of files and
a 'path' variable responsible for the temporary location of files.

Q. How it work?
A. The program works in two phases, while measuring the amount of elapsed time.
The first phase performs work in single-threaded mode, the second - in dualthreaded mode.
After measuring the time, the user will see the ratio of the division of two quantities.

Q. How is the work in the first phase arranged?
A. In the single-threaded phase, a text file with random content and a size of no more than one megabyte is created
along the 'path' variable, further, this file is read along with the line count, after which it is deleted.
This is how one iteration works, the number of iterations is set by the 'files' variable

Q. How is the work in the second phase arranged?
A. In the second phase, similar work is performed, with the first thread responsible for creating the file
and push path to the queue. 
However, the second thread is using for counting the number of lines.

Q. What are the command line arguments?
A. '--help' - Get help about of using this program
   '--files=n' - The command line argument that controls the number of files being created. Where n is natural number
   '--path=n' - Specifies the path to place temporary files. Where n is correct directory path
   '--confirm=n' - Create a confirmation before running?. n is 0 or 1.
)";
    std::exit(0);
    return true;
}

std::stringstream ss;
bool arg_files_count(std::string_view arg)
{
    ss.clear();
    ss.str(std::string(arg));
    decltype(files_count) new_count;
    ss >> new_count;
    if(!ss.fail() && ss.eof() && new_count > 0) {
        files_count = new_count;
        return true;
    };
    return false;
}

bool arg_path(std::string_view arg)
{
    ss.clear();
    ss.str(std::string(arg));
    fs::path new_path;
    ss >> new_path;
    if(!ss.fail() && ss.eof() && std::filesystem::is_directory(new_path)) {
        temp_path = new_path;
        return true;
    };
    return false;
}

bool arg_confirm(std::string_view arg)
{
    ss.clear();
    ss.str(std::string(arg));
    bool new_confirm;
    ss >> new_confirm;
    if(!ss.fail() && ss.eof()) {
        create_confirm = new_confirm;
        return true;
    };

    return false;
}

struct arg_info {
    typedef bool argfunc(std::string_view arg);

    const std::string_view name;
    argfunc* const func;
    const bool generate_short = false;

    enum run_result { name_fail = -1, name_ok_logic_err, name_ok_logic_ok };
    run_result run(std::string_view arg) const
    {
        const bool is_const_arg = name.back() != '=';
        std::size_t min_len = is_const_arg ? 2 : 4;

        //    std::size_t max_len=is_const_arg?2:arg
        bool is_correct_short = generate_short
            //
            && name.size() >= 3 &&
            name.starts_with("--")
            //
            && arg.size() >= min_len && arg[0] == '-' && arg[1] == name[2] &&
            ((is_const_arg && arg.size() == 2) || (!is_const_arg && arg[2] == '='));

        if(is_correct_short) {
            return static_cast<run_result>(is_const_arg ? func(name) : func(arg.data() + 3));
        } else {
            if(is_const_arg) {
                if(arg == name)
                    return static_cast<run_result>(func(arg));
            } else if(arg.size() > name.size() && arg.starts_with(name))
                return static_cast<run_result>(func(arg.data() + name.size()));
        }
        return run_result::name_fail;
    }
    constexpr static bool name_matched(run_result r) noexcept
    {
        return r != name_fail;
    }
    constexpr static bool arg_successfull(run_result r) noexcept
    {
        return r == name_ok_logic_ok;
    }
};

void parse_args(int argc, char** argv)
{
    std::array<arg_info, 4> args_info { { { "--help", arg_help, true }, { "--files=", arg_files_count, true },
        { "--path=", arg_path, true }, { "--confirm=", arg_confirm, true } } };

    char** args_end = argv + argc;
    bool is_mb_program_location_arg = true;
    for(char** args_ptr = argv; args_ptr != args_end; ++args_ptr) {
        bool successfull_run = false;
        std::string_view arg(*args_ptr);
        for(const auto& inf : args_info) {
            auto res = inf.run(arg);
            if(arg_info::name_matched(res)) {
                successfull_run = arg_info::arg_successfull(res);
                break;
            }
        }
        if(!successfull_run && !is_mb_program_location_arg)
            std::cout << "Argument discard: " << arg << '\n';
        is_mb_program_location_arg = false;
    }
}
}

int main(int argc, char** argv)
{
    //------------------------------
    //----INITIALIZATION SECTION----
    //------------------------------
    args_processor::parse_args(argc, argv);

    files.reserve(files_count);

    std::size_t requare = 0;
    for(decltype(files)::size_type i = 0; i != files.capacity(); ++i) {
        files.emplace_back(temp_path / ("tst_file" + std::to_string(i) + ".txt"),
            utils::random_int<decltype(min_size), min_size, max_size>());
        requare += files.back().size;
    }

    const std::size_t available = std::filesystem::space(temp_path).available;
    std::cout << "Temporary path is " << temp_path << '\n'
              << "Files count is " << files.size() << '\n'
              << "No more than " << requare << " bytes of space will be spent\n";

    if(available < requare) {
        std::cout << "Not enough space. Available " << available << " bytes\n";
        return 0;
    }
    if(create_confirm) {
        char c;
        std::cout << "Do you want to continue?[y] ";
        std::cin >> c;
        if(std::cin.bad() || (c != 'y' && c != 'Y')) {
            std::cout << "Canceled by user\n";
            return 0;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    //------------------------------
    //----END OF INITIALIZATION-----
    //------------------------------
    std::cout << "Phase 1. Performing work in a single thread\n";
    utils::start_timer();
    singlethread_work();
    auto singlethread_time = utils::stop_timer().count();
    std::cout << "end of linear work. time=" << singlethread_time << "ms\n\n";

    std::cout << "Phase 2. Performing work in a dual thread\n";
    utils::start_timer();
    dualthread_work();
    auto dualthread_time = utils::stop_timer().count();
    std::cout << "end of dualthread work. time=" << dualthread_time << "ms\n\n";

    std::cout << "Dualthreading is " << singlethread_time / static_cast<double>(dualthread_time) << " times faster\n";

    //------------------------------
    //------DESTRUCTOR SECTION------
    //------------------------------
    // mutex with PTHREAD_MUTEX_INITIALIZER initialize by call pthread_mutex_lock or pthread_mutex_trylock()
    // https://www.ibm.com/docs/en/i/7.2?topic=ssw_ibm_i_72/apis/users_60.htm
    pthread_mutex_destroy(&queue_lock);
    pthread_mutex_destroy(&cout_lock);
    //------------------------------
    //--------END DESTRUCTOR--------
    //------------------------------
    return 0;
}