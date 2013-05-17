#include "chip8.hh"

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

void Chip8::execute()
{
    int loop = 1;
    SDL_Event event;

    SDL_SetVideoMode(640, 320, 32, SDL_HWSURFACE);
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
    }
}

void Chip8::execute_next()
{
    // Fetch opcode
    uint16_t op;

    op = ram_[pc_];
    op = op << 0x8;
    op |= ram_[pc_ + 1];

    switch (op & 0xF000)
    {
        case 0x0000:
            switch (op & 0x00FF)
            {
                case 0x00E0: // CLS
                    fault(op);
                    break;
                case 0x00EE: // RET
                    fault(op);
                    break;
                default: // JP ADDR
                    // -1 to avoid +1 at the end of the switch
                    pc_ = (op & 0x0FFF) - 1;
                    break;
            }
        case 0x6000: // LD Vx, byte
            {
                uint16_t x = op & 0x0F00;
                uint16_t byte = op & 0x00FF;

                x = x >> 0x8;

                V_[x] = byte;

                break;
            }
        default:
            fault(op);
            break;
    }

    ++pc_;
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
