#pragma once

class IOBus
{
    public:
        void begin ();
        void end ();

    private:
        static void io_ez80_rd ();
        static void io_ez80_wr ();
};