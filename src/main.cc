#include <iostream>

#include "chip8.hh"

void usage()
{
    std::cout << "Chip8 Emulator: usage" << std::endl;

    std::cout << "chip8-emulator path-to-rom" << std::endl;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        usage();
        return 1;
    }

    Chip8 vm;

    vm.init();
    vm.load_bin(argv[1]);
    // vm.dump();

    if (!vm.get_error())
        vm.execute();

    return vm.get_error();
}
