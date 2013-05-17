#include "chip8.hh"

Chip8::Chip8()
    : error_(0)
    , pc_(0x200)
    , sp_(0)
{}

Chip8::~Chip8()
{
    SDL_Quit();
}

void Chip8::init()
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        std::cerr << "Error cannot initialize SDL" << std::endl;
        error_ = 2;
    }

    // Initialise memory to 0
    memset(&ram_, 0, 0xFFF);
    memset(&V_, 0, 0xF);
    memset(&stack_, 0, 0xF);
}

void Chip8::load_bin(const std::string& path)
{
    std::ifstream file;
    size_t file_size;

    file.open(path, ios::in | ios::binary);
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
