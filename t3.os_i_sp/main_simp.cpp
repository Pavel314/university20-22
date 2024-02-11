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

constexpr std::size_t files_count = 100;
static_assert(files_count > 0);
constexpr std::size_t min_size = 1024 * 10;
constexpr std::size_t max_size = 1024 * 1024;

fs::path temp_path = fs::temp_directory_path();

struct file_info {
    fs::path path;
    std::size_t size;
};
std::array<file_info, files_count> files;

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

int main()
{
    //------------------------------
    //----INITIALIZATION SECTION----
    //------------------------------
    std::size_t requare = 0;
    for(decltype(files)::size_type i = 0; i != files.size(); ++i) {
        files[i] = file_info { temp_path / ("tst_file" + std::to_string(i) + ".txt"),
            utils::random_int<decltype(min_size), min_size, max_size>() };
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

    char c;
    std::cout << "Do you want to continue?[y] ";
    std::cin >> c;
    if(std::cin.bad() || (c != 'y' && c != 'Y')) {
        std::cout << "Canceled by user\n";
        return 0;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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