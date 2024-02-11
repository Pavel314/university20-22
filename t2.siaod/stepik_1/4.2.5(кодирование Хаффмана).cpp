#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <limits>

using byte = unsigned char;

struct continue_node;
struct leaf_node;
struct node;

struct visitor {
    virtual void visit_continue_node(const continue_node& node) = 0;
    virtual void visit_leaf_node(const leaf_node& node) = 0;
    virtual void begin_visit(const node& node) = 0;
};

struct node {
    std::size_t weight;
    node(std::size_t weight):weight(weight){}
    virtual ~node() = default;
    virtual void visit(visitor& visitor) const = 0;
};
struct continue_node : public node {
    std::unique_ptr<node> left, right;
    
    continue_node(std::size_t weight, std::unique_ptr<node> left, std::unique_ptr<node> right)
        : node(weight), left(std::move(left)), right(std::move(right)){}
    void visit(visitor& visitor) const override
    {
        visitor.visit_continue_node(*this);
    }
};

struct leaf_node : public node {
    byte value;
    leaf_node(std::size_t weight, byte value): node(weight), value(value){}
    void visit(visitor& visitor) const override
    {
        visitor.visit_leaf_node(*this);
    }
};

struct visitor256bet : public visitor {
public:
    struct code{
        unsigned int value;
        unsigned int length;
        static std::string format_code(code code){
            std::string res;
            while(code.length--) {
                res.push_back((code.value & 1) == 1 ? '1' : '0');
                code.value >>= 1;
            };
            return res;
        }
    };
    
private:
    code cur_code;
    std::array<code, std::numeric_limits<byte>::max()> table;

public:
    void visit_continue_node(const continue_node& node) override
    {
        ++cur_code.length;
        const auto mask = 1 << (cur_code.length - 1);
        cur_code.value &= ~mask;
        node.left->visit(*this);
        cur_code.value |= mask;
        node.right->visit(*this);
        --cur_code.length;
    }
    void visit_leaf_node(const leaf_node& node) override
    {
        table[node.value] = cur_code;
    }
    void begin_visit(const node& node) override
    {
        table.fill(code{0,0});
        cur_code=code{0,dynamic_cast<const leaf_node*>(&node) != nullptr ? 1 : 0};
        node.visit(*this);
    }
    template <typename InputIt, typename Callback> void encode(InputIt first, InputIt last, Callback callback)
    {
        for(; first != last; ++first) {
            callback(table[(unsigned char)(*first)]);
        }
    }
    
    template <typename Callback>
    void get_table(Callback callback){
        for (int i=0;i<table.size();++i){
             code cd = table[static_cast<byte>(i)]; 
             if (cd.length!=0) callback(cd,i);}
    }
    
};

template <typename InputIt> std::unordered_map<byte, std::size_t> get_freq_map(InputIt first, InputIt last)
{
    std::unordered_map<byte, std::size_t> map;
    std::for_each(first, last, [&map](const byte ch) { ++map[ch]; });
    return map;
}

template <typename FreqMapIt> std::unique_ptr<node> build_huffman_tree(FreqMapIt first, FreqMapIt last)
{
    std::deque<std::unique_ptr<leaf_node>> leafs;
    std::for_each(first, last,
        [&leafs](const auto& m) { leafs.push_back(std::make_unique<leaf_node>(m.second, (byte)(m.first))); });

    switch(leafs.size()) {
    case 0:
        return nullptr;
    case 1:
        return std::move(leafs.front());
    default:
        break;
    }

    std::deque<std::unique_ptr<continue_node>> trees;
    std::sort(leafs.begin(), leafs.end(),
        [](const std::unique_ptr<leaf_node>& l, const std::unique_ptr<leaf_node>& r) { return l->weight > r->weight; });

    std::array<std::unique_ptr<node> ,2> nodes;
    while(!leafs.empty() || trees.size() > 1) {
        
        for(auto& node:nodes) {
            if(trees.empty() ||  (!leafs.empty() && leafs.back()->weight <= trees.back()->weight)) {
                node = std::move(leafs.back());
                leafs.pop_back();
            } else {
                node = std::move(trees.back());
                trees.pop_back();
            }
        }

        auto parent = std::make_unique<continue_node>(
            nodes[0]->weight + nodes[1]->weight, std::move(nodes[0]), std::move(nodes[1]));
        trees.push_front(std::move(parent));
    }
    return std::move(trees.front());
}

int main()
{
    std::ios::sync_with_stdio(false);

    std::string input;
    /*из-за особенностей проверяющей системы пришлось использовать std::istream_iterator,
    хотя предполгагалось std::istreambuf_iterator*/
    std::copy(std::istream_iterator<char>(std::cin), std::istream_iterator<char>(),std::back_inserter(input));
    std::unordered_map<byte, std::size_t> map = get_freq_map(input.cbegin(), input.cend());
    std::unique_ptr<node> root(build_huffman_tree(map.cbegin(), map.cend()));
    if(root == nullptr)
        return 1;

     visitor256bet vis;
     vis.begin_visit(*root);
    
    std::string encoded_str;
    vis.encode(input.cbegin(), input.cend(), [&encoded_str](const visitor256bet::code& code) {
        encoded_str.append(visitor256bet::code::format_code(code));});
    
    std::cout<<map.size()<<' '<<encoded_str.size()<<std::endl;
    
    vis.get_table([](const visitor256bet::code& code, byte index){
        std::cout<<static_cast<char>(index)<<": "<<visitor256bet::code::format_code(code)<< '\n';});
    std::cout<<encoded_str;
    return 0;
}