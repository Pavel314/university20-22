#include <cstdint>
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <iterator>

using num_t=uint_least64_t;

struct new_task {
    num_t id;
    num_t length;
};

struct completed_task {
    new_task src;
    num_t start_time;
    num_t processor;
    num_t get_end_time() const{
        return start_time + src.length;
    }
};

bool operator<(const completed_task& lhs, const completed_task& rhs)
{
    const num_t l_end = lhs.get_end_time();
    const num_t r_end = rhs.get_end_time();
    if(r_end != l_end)
        return r_end < l_end;
    return rhs.processor < lhs.processor;
}

template <typename TaskCompleted,
    typename CompletedTask = completed_task,
    typename Compare = std::less<CompletedTask>,
    typename PriorQueue = std::priority_queue<CompletedTask, std::vector<CompletedTask>, Compare>>
class parallel_simulator
{
public:
    using size_type = typename PriorQueue::size_type;
    using container_type = PriorQueue;
    using value_type = typename PriorQueue::value_type;
    using reference = typename PriorQueue::reference;
    using const_reference = typename PriorQueue::const_reference;

private:
    PriorQueue queue;

public:
    const size_type processors;
    const TaskCompleted& task_completed;

    explicit parallel_simulator(size_type processors_,
        const TaskCompleted& task_completed_,
        const Compare& comp = Compare())
        : queue(comp), processors(processors_)
        , task_completed(task_completed_){}
    void clear()
    {
        while(!queue.empty())
            queue.pop();
    }
    template <typename NewTask> void add_task(const NewTask& nt)
    {
        if(queue.size() >= processors) {
            CompletedTask ct(queue.top());
            task_completed(ct);
            queue.pop();
            queue.push(CompletedTask { nt, ct.get_end_time(), ct.processor });
        } else {
            const CompletedTask ct { nt, 0, queue.size() };
            if(nt.length != 0)
                queue.push(ct);
            else
                task_completed(ct);
        }
    }

    void extract_task()
    {
        task_completed(queue.top());
        queue.pop();
    }
    
    bool empty() const
    {
        return queue.empty();
    }
    size_type size() const
    {
        return queue.size();
    }
};

int main()
{
    num_t procs;
    std::cin>>procs;
    using tasks_container = std::vector<num_t>;
    tasks_container::size_type size;
    std::cin>>size;
    tasks_container tasks(size);
    std::copy(std::istream_iterator<tasks_container::value_type>(std::cin), 
              std::istream_iterator<tasks_container::value_type>(), 
              tasks.begin());
    
    std::vector<completed_task> result(tasks.size());    
    auto on_completed = [&result](const completed_task& ctask) { result[ctask.src.id] = ctask; };

    parallel_simulator<decltype(on_completed)> simulator(procs, on_completed);

    for(tasks_container::size_type i = 0; i != tasks.size(); ++i)
        simulator.add_task(new_task { i, tasks[i] });

    while(!simulator.empty())
        simulator.extract_task();

    for(const auto& task : result) {
        std::cout << task.processor << ' ' << task.start_time << std::endl;
    }
   return 0;
}