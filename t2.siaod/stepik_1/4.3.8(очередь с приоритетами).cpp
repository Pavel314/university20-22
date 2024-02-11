#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>
using el_ty=int_fast32_t;

std::vector<el_ty> queue;

void insert(const el_ty val)
{
    queue.push_back(val);
    const std::size_t size = queue.size();
    if(size == 1)
        return;
    std::size_t cur_ind = size - 1;
    std::size_t parent_ind = size / 2 - 1; //(cur_ind + 1)/2-1
    while(queue[cur_ind] > queue[parent_ind]) {
        std::swap(queue[cur_ind], queue[parent_ind]);
        if(parent_ind == 0)
            break;
        cur_ind = parent_ind;
        parent_ind = (cur_ind + 1) / 2 - 1;
    }
}

el_ty extract_max()
{
    const el_ty res = queue[0];

    queue[0] = queue.back();
    queue.pop_back();
    const std::size_t size = queue.size();
    std::size_t cur_ind = 0;

    while(true) {
        const std::size_t child_ind1 = cur_ind * 2 + 1;
        const std::size_t child_ind2 = child_ind1 + 1;
        std::size_t max_ind;

        if(child_ind1 < size && child_ind2 < size)
            max_ind = queue[child_ind1] >= queue[child_ind2] ? child_ind1 : child_ind2;
        else {
            if(child_ind1 < size)
                max_ind = child_ind1;
            else
                break;
        }        

        if(queue[max_ind] > queue[cur_ind]) {
            std::swap(queue[cur_ind], queue[max_ind]);
            cur_ind = max_ind;
        } else
            break;
    }
    return res;
}
int main()
{
    std::size_t operations;
    std::cin >> operations;
    queue.reserve(operations / 2);
    std::string name;
    el_ty val;
    while(operations--) {
        std::cin >> name;
        if(name == "Insert") {
            std::cin >> val;
            insert(val);
        } else if(name == "ExtractMax") {
            std::cout << extract_max() << std::endl;
        }
    }
}