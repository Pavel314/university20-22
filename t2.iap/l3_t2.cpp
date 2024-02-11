#include <cstdint>
#include <iostream>
#include <string>

template <typename NumT> NumT request_num(std::string text)
{
    NumT res;
    while(true) {
        std::cout << text << std::flush;
        std::cin >> res;
        if(!std::cin.good()) {
            std::cin.clear();
            std::string dummy;
            std::cin >> dummy;
        } else
            return res;
    }
}

int main()
{
    using ind_t = std::size_t;
    std::string sentence, word;
    std::cout << "Enter a sentence\n";
    std::getline(std::cin >> std::ws, sentence);
    std::cout << "Enter a world\n";
    std::getline(std::cin >> std::ws, word);
    ind_t target_ind = request_num<ind_t>("Enter the target index for this word(Indexing starts at zero)\n");
    ind_t j = 0;
    ind_t word_reps = 0;
    ind_t global_ind = 0;
    for(ind_t i = 0; i < sentence.size(); ++i) {
        const char c = sentence[i];
        if(c == word[j]) {
            if(j + 1 < word.size())
                ++j;
            else {
                global_ind = i;
                if(word_reps == target_ind)
                    break;
                j = 0;
                ++word_reps;
            }
        } else {

            j = c == word[0];
        }
    }
    if(j == word.size() - 1) {
        global_ind -= word.size() - 1;
        std::cout << sentence.erase(global_ind, word.size()) << '\n';
    } else {
        std::cout << "Target word not found\n";
    }
}