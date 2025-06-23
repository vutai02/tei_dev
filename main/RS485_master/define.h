// #pragma once
namespace RS485
{
    class parameter_ATS
    {
    public:
        /* special  */
        // static constexpr uint16_t COMMAND_IO1 = 0X0401;
        // static constexpr uint16_t COMMAND_IO2 = 0X0410;
        // static constexpr uint16_t COMMAND_OUT1 = 0X0752;
        // static constexpr uint16_t COMMAND_OUT2 = 0X0403;
        
        static constexpr uint16_t COMMAND_IO1 = 0x0000; 
        static constexpr uint16_t COMMAND_IO2 = 0x0001;  
        static constexpr uint16_t COMMAND_OUT1 = 0x0002; 
        static constexpr uint16_t COMMAND_OUT2 = 0x0003; 

        typedef struct io_parameter
        {
            int16_t gpio_in1;
            int16_t gpio_in2;
            int16_t gpio_out1;
            int16_t gpio_out2;
            bool io1_state; 
            bool io2_state;
        } io_parameter_t;

        struct io_parameter io_Vals;
    };
}
