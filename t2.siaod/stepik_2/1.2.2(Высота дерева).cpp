#include <iostream>
#include <unordered_set>
#include <vector>
#include <iterator>

using ind_t = std::size_t;
using len_t = std::size_t;

template <typename InputIt> len_t tree_height(InputIt first, InputIt last, ind_t count)
{
    constexpr ind_t root_mark=static_cast<ind_t>(-1);
    struct node {ind_t child_ind;len_t path=0;};
    std::vector<node> parents(count);
    std::unordered_set<ind_t> parents_set;
    node* root;

    for (auto& parent:parents){
        parent.child_ind=*first;
        parents_set.insert(*first);
        if(*first++ == root_mark)
            root = &parent;
    }
    for(ind_t i = 0; i != parents.size(); ++i) {
        if(parents_set.find(i)==parents_set.end()) {
            ind_t ind = i;
            len_t path = 1;
            while(ind != root_mark && parents[ind].path < path) {
                parents[ind].path = path++;
                ind = parents[ind].child_ind;
            }
        }
    }
    return root->path;
}

int main()
{
    ind_t count;
    std::cin>>count;
    std::cout << tree_height(std::istream_iterator<ind_t>(std::cin),std::istream_iterator<ind_t>(),count);
    return 0;
}