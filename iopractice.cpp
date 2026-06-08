#include <iostream>
#include <fstream>
#include <iomanip>
#include <bitset>

int main(int argc, char *argv[]) {
    if(argc > 1) {
        std::ifstream file_p{argv[1]};

        u_int8_t num[10];

        file_p.read(reinterpret_cast<char*>(&num), 10);
        std::bitset<32> numB {num[5]};

        std::cout <<std::hex << num[5] << ' '<< sizeof(num)<< ' '<< numB <<'\n';
        
    }

    return 0;
}