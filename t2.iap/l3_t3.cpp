#include <cctype>
#include <iostream>
#include <string>

char ctolower(char ch)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}

bool check_palindrome(const std::string& str)
{
    using ind_t = std::string::size_type;
    const ind_t size=str.size();
    for(ind_t i = 0; i != size / 2; ++i) {
        if(ctolower(str[i]) != ctolower(str[size-1- i ]))
            return false;
    }
    return true;
}

int main()
{
    constexpr int max_lines = 25;
    std::string text[max_lines];
    std::string max_palindrome;
    std::cout << "Enter a text (maximum="<<max_lines<< " sentences)\n";
    for(int i = 0; i < max_lines; ++i) {
        //std::cout << "Enter a sentence(" << i <<"/"<<max_lines-1<< ")\n";
        std::getline(std::cin, text[i]);
        if(text[i].empty())
            break;

        std::string word;
        for(char c : text[i]) {
            if(std::isalpha(static_cast<unsigned char>(c))) {
                word.push_back(c);
            } else {
                if(word.size() > max_palindrome.size() && check_palindrome(word)) {
                    max_palindrome = word;
                }
                word.clear();
            }
        }
        if(word.size() > max_palindrome.size() && check_palindrome(word)) {
            max_palindrome = word;
        }
    }

    if(max_palindrome.size()>1) {
        std::cout << "The following is a palindrome of maximum length:\n"
                  << max_palindrome << '\n'
                  << "Length=" << max_palindrome.size() << '\n';
    } else {
        std::cout << "This text does not contain any palindromes\n";
    }
}