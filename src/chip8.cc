#include "chip8.hh"

static uint8_t default_char[80] =
{
    0xF0 ,0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8()
    : error_(0)
    , pc_(0x200)
    , sp_(0)
    , I_(0)
    , dt_(0)
    , st_(0)
{}

Chip8::~Chip8()
{
    SDL_FreeSurface(pixel_);
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
    memset(&ram_, 0, 4096);
    memset(&V_, 0, 16);
    memset(&stack_, 0, 16 * 2);
    memset(&vga_mem_, 0, 64 * 32);

    for (size_t i = 0; i < 80; ++i)
        ram_[i] = default_char[i];

    pixel_ = SDL_CreateRGBSurface(0, 10, 10, 32, 0, 0, 0, 0);

    SDL_FillRect(pixel_, NULL, SDL_MapRGB(pixel_->format, 255, 255, 255));
}

void Chip8::load_bin(const std::string& path)
{
    std::ifstream file;
    size_t file_size;

    file.open(path, std::ios::in | std::ios::binary);

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        file_size = file.tellg();

        // Test if the ROM will feet in RAM
        if (file_size > 0xFFF - 0x200)
        {
            std::cerr << "Error ROM cannot feet in RAM" << std::endl;
            error_ = 4;
        }
        else // OK
        {
            char* data_rom = (char* )(&(ram_[0x200]));

            file.seekg(0, std::ios::beg);
            file.read(data_rom, file_size);
        }

        file.close();
    }
    else
    {
        std::cerr << "Error while opening the ROM" << std::endl;
        error_ = 3;
    }
}

void Chip8::render(SDL_Surface* screen)
{
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    SDL_Rect pixel_pos;

    for (size_t x = 0; x < 64; ++x)
    {
        for (size_t y = 0; y < 32; ++y)
        {
            if (vga_mem_[y * 64 + x] == 1)
            {
                pixel_pos.x = x * 10;
                pixel_pos.y = y * 10;

                SDL_BlitSurface(pixel_, NULL, screen, &pixel_pos);
            }
        }
    }

    SDL_Flip(screen);
}

void Chip8::execute()
{
    int loop = 1;
    SDL_Event event;

    SDL_Surface* screen = SDL_SetVideoMode(640, 320, 32, SDL_HWSURFACE);
    SDL_WM_SetCaption("Chip8 Emulator", NULL);

    while (!error_ && loop)
    {
        SDL_PollEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                loop = 0;
        }

        // Execute next instruction
        execute_next();

        render(screen);
    }
}

void Chip8::execute_next()
{
    // Fetch opcode
    uint16_t op = (ram_[pc_] << 0x8) | ram_[pc_ + 1];

    switch (op & 0xF000)
    {
        case 0x0000:
            switch (op & 0x00FF)
            {
                case 0x00E0: // CLS
                    memset(&vga_mem_, 0, 64 * 32);
                    pc_ += 2;
                    break;
                case 0x00EE: // RET
                    fault(op);
                    break;
                default: // JP ADDR
                    // -1 to avoid +1 at the end of the switch
                    pc_ = op & 0x0FFF;
                    break;
            }
            break;
        case 0x2000: // Call addr
            {
                uint16_t addr = op & 0x0FFF;

                stack_[sp_++] = pc_;

                pc_ = addr;
            }
            break;
        case 0x4000: // NE Vx, byte
            {
                uint16_t x = op & 0x0F00;
                uint16_t byte = op & 0x00FF;

                if (V_[x] != byte)
                    pc_ += 2;

                pc_ += 2;
            }
            break;
        case 0x6000: // LD Vx, byte
            {
                uint16_t x = op & 0x0F00;
                uint16_t byte = op & 0x00FF;

                x = x >> 0x8;

                V_[x] = byte;

                pc_ += 2;
                break;
            }
            break;
        case 0x8000:
            {
                uint16_t x = (op & 0x0F00) >> 0x8;
                uint16_t y = (op & 0x00F0) >> 0x4;

                switch (op & 0x000F)
                {
                    case 0x0007: //SUBN Vx, Vy

                        if (V_[y] > V_[x])
                            V_[0xF] = 0x1;
                        else
                            V_[0xF] = 0x0;

                        V_[x] = V_[y] - V_[x];

                        pc_ += 2;
                        break;
                    default:
                        fault(op);
                        break;
                }
            }
            break;
        case 0xA000: // LD I, addr
            {
                I_ = (op & 0x0FFF);

                pc_ += 2;
            }
            break;
        case 0xD000: // DRW Vx, Vy, nibble
            {
                uint16_t x = V_[(op & 0x0F00) >> 0x8];
                uint16_t y = V_[(op & 0x00F0) >> 0x4];
                uint16_t nibble = (op & 0x000F);
                uint16_t pixel;

                V_[0xF] = 0;

                for (size_t i = 0; i < nibble; ++i)
                {
                    pixel = ram_[I_ + i];

                    for (size_t j = 0; j < 8; ++j)
                    {
                        if ((pixel & (0x80 >> j)) != 0)
                        {
                            if (vga_mem_[x + j + ((y + i) * 64)] != 0)
                                V_[0xF] = 1;

                            vga_mem_[x + j + ((y + i) * 64)] ^= 1;
                        }
                    }

                }

                pc_ += 2;
            }
            break;
        case 0xF000:
            switch (op & 0x00FF)
            {
                case 0x0033: // LD B, Vx
                    {
                        uint16_t x = (op & 0x0F00) >> 0x8;
                        uint8_t tmp = V_[x] % 100;

                        ram_[I_] = V_[x] / 100;
                        ram_[I_ + 1] = tmp / 10;
                        ram_[I_ + 2] = tmp % 10;

                        pc_ += 2;
                    }
                    break;
                case 0x0065: // LD [I], Vx
                    {
                        uint16_t x = (op & 0x0F00) >> 0x8;

                        for (size_t i = 0; i <= x; ++i)
                            ram_[I_ + i] = V_[i];

                        pc_ += 2;
                    }
                    break;
                default:
                    fault(op);
                    break;
            }
            break;
        default:
            fault(op);
            break;
    }

    if (dt_ > 0)
        --dt_;

    if (st_ > 0)
        --st_;
}

void Chip8::fault(uint16_t op)
{
    std::cerr << "CPU Fault" << std::endl;
    std::cerr << "Error unknown instruction "
              << std::hex << std::setfill('0') << std::setw(4)
              << std::uppercase << op << std::endl;
    dump();
    error_ = 5;
}

void Chip8::dump()
{
    std::cerr << "Dumping RAM" << std::endl;

    for (size_t i = 0; i < 2048; ++i)
    {
        if (i % 16 == 0)
            std::cerr << std::endl << "0x" << std::setfill('0') << std::setw(3)
                      << std::uppercase << std::hex << i << " | ";

        std::cerr << std::hex << std::setfill('0') << std::setw(2)
                  << std::uppercase << int(ram_[i])
                  << std::uppercase << std::setfill('0') << std::setw(2)
                  << int(ram_[i + 1]) << " ";
    }

    std::cerr << std::endl;

    std::cerr << std::endl << "Dumping Register" << std::endl;

    std::cerr << std::endl << "PC: 0x" << std::hex << std::setfill('0')
              << std::setw(4) << int(pc_) << std::endl;

    std::cerr << "I: 0x" << std::hex << std::setfill('0')
              << std::setw(4) << int(I_) << std::endl;

    std::cerr << "SP: 0x" << std::hex << std::setfill('0')
              << std::setw(2) << int(sp_) << std::endl;

    std::cerr << "DT: 0x" << std::hex << std::setfill('0')
              << std::setw(2) << int(dt_) << std::endl;

    std::cerr << "ST: 0x" << std::hex << std::setfill('0')
              << std::setw(2) << int(st_) << std::endl;

    for (size_t i = 0; i < 16; ++i)
    {
        std::cerr << "V" << i << ": 0x" << std::hex << std::setfill('0')
                  << std::setw(2) << int(V_[i]) << std::endl;
    }

    std::cerr << std::endl << "Dumping Stack" << std::endl;

    std::cerr << std::endl;

    for (size_t i = 0; i < 16; ++i)
    {
        std::cerr << "0x" << std::hex << std::setfill('0')
                  << std::setw(4) << int(stack_[i]) << std::endl;
    }
}

int Chip8::get_error()
{
    return error_;
}
