#ifndef CHIP8_HH
# define CHIP8_HH

# include <iostream>
# include <iomanip>
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
        void dump();

        int get_error();

    private:
        void execute_next();
        int events();
        void fault(uint16_t op);
        void render(SDL_Surface* screen);

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

        // 16 bit I register
        uint16_t I_;

        // 8bit delay timer
        uint8_t dt_;

        // 8bit sound timer
        uint8_t st_;

        // Stack of 16 16bit slots
        uint16_t stack_[16];

        // Graphic memory
        uint8_t vga_mem_[64 * 32];

        // Keyboard key pressed
        uint8_t keyboard_[16];

        SDL_Surface *pixel_;

        SDL_Event event_;
};

#endif /* !CHIP8_HH */
