#include <cstdint>
#include <iostream>

int main(void) {
  std::uint_fast32_t n;
  std::cin >> n;
  std::uint_fast32_t prev=0,cur=1,next=1;
  
  while (n--){
      prev=cur;
      cur=next;
      next=(cur+prev)%10;
  } 
  std::cout<<prev;   
  return 0;
}