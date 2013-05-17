#include "chip8.hh"

Chip8::Chip8()
    : error_(0)
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        std::cerr << "Error cannot initialize SDL" << std::endl;
        error_ = 2;
    }
}

Chip8::~Chip8()
{
    SDL_Quit();
}

void Chip8::execute()
{
    int loop = 1;
    SDL_Event event;

    SDL_SetVideoMode(640, 320, 32, SDL_HWSURFACE);
    SDL_WM_SetCaption("Chip8 Emulator", NULL);

    while (!error_ && loop)
    {
        SDL_WaitEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                loop = 0;
        }
    }
}

int Chip8::get_error()
{
    return error_;
}
