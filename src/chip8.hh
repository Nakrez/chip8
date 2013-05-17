#ifndef CHIP8_HH
# define CHIP8_HH

# include <iostream>
# include <fstream>
# include <cstdint>

# include <SDL/SDL.h>

class Chip8
{
    public:
        Chip8();
        ~Chip8();

        void init();
        void load_bin(const std::string& path);
        void execute();

        int get_error();

    private:
        int error_;

        // 4ko ram on chip8
        uint8_t ram_[4096];

        // 16 8bit register
        uint8_t V_[16];

        // 16bit Program counter (PC)
        uint16_t pc_;

        // 8bit Stack pointer (SP)
        uint8_t sp_;

        // Stack of 16 16bit slots
        uint16_t stack_[16];
};

#endif /* !CHIP8_HH */
