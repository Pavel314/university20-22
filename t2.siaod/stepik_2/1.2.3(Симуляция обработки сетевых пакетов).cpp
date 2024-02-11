#include <cstdint>
#include <iostream>
#include <queue>
#include <iterator>

template <typename TimeType = unsigned long,
    TimeType PackIgnored = (TimeType)(-1),
    typename Queue = std::queue<TimeType>>
class network_pack_simulator
{
public:
    using time_type = TimeType;
    using size_type= typename Queue::size_type;
    using queue_type = Queue;
    constexpr static TimeType pack_ignored = PackIgnored;
protected:
    Queue queue;
    time_type past_time = 0;
public:
    const size_type  queue_size;
    
    network_pack_simulator(const size_type queue_size_)
        : queue_size(queue_size_){}

    time_type simulate_pack(const time_type arrival, const time_type duraction)
    {
        while(!queue.empty() && arrival >= queue.front())
            queue.pop();
        
        if(arrival > past_time) 
            past_time = arrival;

        if(queue.size() < queue_size) {
            time_type start_tm = past_time;
            past_time += duraction;
            queue.push(past_time);
            return start_tm;
        }
        return pack_ignored;
    }
    void reset(const size_type queue_size_)
    {
        past_time = 0;
        queue_size=queue_size_;
        while(!queue.empty()) {
            queue.pop();
        }
    }    
};

using num_t = std::uint_fast32_t;

struct pack {
    num_t arrival, duraction;
};
std::istream& operator>> (std::istream& is, pack& pk)
{
    is>>pk.arrival>>pk.duraction;
    return is;
}

int main()
{
    using simulator_t=network_pack_simulator<num_t>;
    
    simulator_t::size_type buf_size,size;
    std::cin>>buf_size>>size;
    
    simulator_t simulator(buf_size);
    auto it=std::istream_iterator<pack>(std::cin);
    for (decltype(size) i = 0; i < size; ++it, ++i) {
         simulator_t::time_type tm=simulator.simulate_pack(it->arrival,it->duraction);
         if (tm!=simulator_t::pack_ignored) 
             std::cout<<tm<<std::endl;
         else 
             std::cout<<"-1"<<std::endl;    
    }
}