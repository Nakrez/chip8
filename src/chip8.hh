#ifndef CHIP8_HH
# define CHIP8_HH

# include <iostream>

# include <SDL/SDL.h>

class Chip8
{
    public:
        Chip8();
        ~Chip8();

        void execute();

        int get_error();

    private:
        int error_;
};

#endif /* !CHIP8_HH */
