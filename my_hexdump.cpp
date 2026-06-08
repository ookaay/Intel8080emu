#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        std::cout << argv[0] << '\n';

        std::ifstream file_ref{argv[1]};

        std::cout << static_cast<bool>(file_ref) << '\n';

         char block;
         char block2;
        // file_ref.seekg(0,std::ios::beg);

        std::cout << sizeof(char) << '\n';

        std::cout << std::hex << std::setw(8) << std::right << std::setfill('0');
        std::cout << 0 << "  ";

        while (file_ref)
        {
            // file_ref.seekg(i,std::ios::beg);
            // std::cout <<'\n' << '[' << file_ref.tellg() <<']' << '\n';
            file_ref.get(block);
            file_ref.get(block2);

            unsigned char u_block = block;
            unsigned char u_block2 = block2;

            std::cout << std::hex << std::setw(2) << std::right << std::setfill('0');
            std::cout << static_cast<int>(u_block2);

            std::cout << std::hex << std::setw(2) << std::right << std::setfill('0');
            std::cout << static_cast<int>(u_block) << ' ';

            if ((file_ref.tellg() % 0x10) == 0)
            {
                std::cout << '\n';
                std::cout << std::hex << std::setw(8) << std::right << std::setfill('0');

                std::cout << file_ref.tellg() << "  ";
            }
        }
        std::cout << '\n';

        // std::getline(file_ref,block);
        // std::cout << block << '\n';

        // std::cout << 077 << '\n';
        // std::cout << 77 << '\n';
        // std::cout << 0x77 << '\n';
        // std::cout << 0bi77 << '\n';
    }
    return 0;
}