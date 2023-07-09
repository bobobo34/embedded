#ifndef PsetT_1f76812a_cc4d_4c47_b317_4e376de899cd
#define PsetT_1f76812a_cc4d_4c47_b317_4e376de899cd

#ifndef __cplusplus
#error "<port> is a C++ header!"
#endif

#include <initializer_list>
#include <concepts>
#include <utility>

//************************************************************************************
//
// <port> is a c++ header that allows fast, easy-to-use functionality to interface
// with ports on the TM4C123GH6PM CPU (maybe more in the future)
//
// The datasheet can be found at: https://www.ti.com/lit/ds/symlink/tm4c123gh6pm.pdf?ts=1688732146028&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTM4C123GH6PM
//
// created by Evin Lodder, 7/5/2023
//
//************************************************************************************

namespace port {

//op is used to select the operation for use when setting/clearing registers
enum class op {
    set,
    clear,
};

}

namespace detail {
    //template for hardware register, with offset, data type, and data member
    template<typename T, uint32_t OFFSET>
        struct regbase {
            using value_type = T;
            value_type data;
            port::op operation;
            static constexpr uint32_t offset = OFFSET;
            constexpr regbase(value_type dt, port::op op) : data(dt), operation(op) {}
            constexpr regbase(value_type dt) : data(dt), operation(port::op::set) {}
            constexpr regbase() = delete;
        };
    //concept for sake of narrowing variadic function parameters (namely port_t.initialize and .reconfigure)
    template<typename T>
        concept regbase_t = std::is_class_v<T> && requires {
            typename T::value_type; //value_type is a member of type
            T::offset; //offset is static member of regbase
        };
}

namespace port {

#if defined(TARGET_IS_TM4C123_RB1)
#define BIT_SPECIFIC_ADDRESSING_RB1 1
// portname enum
// the underlying values are all of the base addresses of the ports
// according to the TI microcontroller datasheet
enum portname : uint32_t {
    A = 0x40004000,
    B = 0x40005000,
    C = 0x40006000,
    D = 0x40007000,
    E = 0x40024000,
    F = 0x40025000,
};

// there are three clock registers-
//  rcgcgpio- general-purpose run mode clock
//  scgcgpio- general-purpose sleep mode clock
//   -sleep mode stops processor clock and reduces power consumption
//  dcgcgpio- general-purpose deep-sleep mode clock
//   -deep-sleep mode stops system clock and switches off pll and flash
//  the underlying values are the base addresses of the registers according to the datasheet
enum clockreg: uint32_t {
    rcgcgpio = 0x400FE608,
    scgcgpio = 0x400FE708,
    dcgcgpio = 0x400FE808,
};

//direction register- 1 means pin is output, 0 means pin is input
using dir = detail::regbase<uint8_t, 0x400>;

//interrupt sense register- 1 means pin is level-sensitive, 0 means pin is edge-sensitive
using is = detail::regbase<uint8_t, 0x404>;

//interrupt both edges register- 1 means both edges on pin trigger an interrupt,
//0 means interrupt generation is controlled by GPIOIEV register
using ibe = detail::regbase<uint8_t, 0x408>;

//interrupt event register- 1 means high level on pin triggers interrupt (rising edge),
//0 means low level on pin triggers interrupt (falling edge)
using iev = detail::regbase<uint8_t, 0x40C>;

//interrupt mask register- 1 means interrupt from pin is sent to interrupt controller,
//0 means interrupt from pin is masked
using im = detail::regbase<uint8_t, 0x410>;

//raw interrupt status register- 1 means an interrupt condition has occured on the pin, 0 means one hasn't
//note- this is not a register you write to, this is an input register
using ris = detail::regbase<uint8_t, 0x414>;

//masked interrupt status register- 1 means this pin has triggered an interrupt to the interrupt controller,
//0 means the pin is masked or the interrupt condition has not occured
//note- this is not a register you write to, this is an input register
using mis = detail::regbase<uint8_t, 0x418>;

//interrupt clear register- 1 means the corresponding interrupt is cleared (only for edge-detec interrupts),
//writing 0 has no effect
using icr = detail::regbase<uint8_t, 0x41C>;

//alternate function select register- 1 means pin is functions as a peripheral signal and is controlled
//by the alternate hardware function, 0 means pin is GPIO and controlled by GPIO registers
//note- some pins have special consideration, see page 671 in the datasheet
using afsel = detail::regbase<uint8_t, 0x420>;

//2-mA drive select register- 1 means pin has 2-mA drive, 0 means drive for pin is controlled by dr4r or dr8r
using dr2r = detail::regbase<uint8_t, 0x500>;

//4-mA drive select register- 1 means pin has 4-mA drive, 0 means drive for pin is controlled by dr2r or dr8r
using dr4r = detail::regbase<uint8_t, 0x504>;

//8-mA drive select register- 1 means pin has 8-mA drive, 0 means drive for pin is controlled by dr2r or dr4r
using dr8r = detail::regbase<uint8_t, 0x508>;

//open drain select register- 1 means pin is configured as open drain, 0 means pin isn't
using odr = detail::regbase<uint8_t, 0x50C>;

//pull-up select register- 1 means pin's pull-up resistor is enabled, 0 means it's disabled
//note- some pins have special consideration, see page 677 in the datasheet
using pur = detail::regbase<uint8_t, 0x510>;

//pull-down select register- 1 means pin's pull-down resistor is enabled, 0 means it's disabled
//note- some pins have special consideration, see page 679 in the datasheet
using pdr = detail::regbase<uint8_t, 0x514>;

//slew rate control select register- 1 means slew rate control is enable, 0 means pin it's disabled
using slr = detail::regbase<uint8_t, 0x518>;

//digital enable register- 1 means digital functions for pin are enable, 0 means they're disabled
//note- some pins have special consideration, see page 682 in the datasheet
using den = detail::regbase<uint8_t, 0x51C>;

//lock register- writing 0x4C4F434B (lock_unlock) unlocks GPIOCR for write access.
//Writing anything else to this register or GPIOCR reapplies the lock
//For reading the register:
// 0x1 means the GPIOCR register is locked
// 0x0 means the GPIOCR register is unlocked
constexpr uint32_t lock_unlock {0x4C4F434B};
using lock = detail::regbase<uint32_t, 0x520>;

//commit register- 1 means corresponding GPIOAFSEL, GPIOPUR, GPIODPDR, or GPIODEN can be written,
//0 means they can't be written
using cr = detail::regbase<uint8_t, 0x524>;

//analog mode select register- 1 means analog function is enabled, isolation is disabled, and pin is
//capable of analog functions, 0 means analog function disabled, isolation enabled, and pin is capable
//of digital functions
using amsel = detail::regbase<uint8_t, 0x528>;

//port control register- used with GPIOAFSEL register to select specific peripheral signal for each GPIO pin
//when using alternate function mode. See page 1351 in the datasheet for defined encodings for the bit fields.
//note- bits 0-3 are used for pin 0, 4-7 for pin 1, and so on.
//note 2- some pins have special consideration, see page 688 in the datasheet
using pctl = detail::regbase<uint32_t, 0x52C>;

//ADC control register- 1 means pin is used to trigger the ADC, 0 means pin isn't
using adcctl = detail::regbase<uint8_t, 0x530>;

//DMA control register- 1 means pin is used to trigger the (mu)DMA register, 0 means pin isn't
using dmactl = detail::regbase<uint8_t, 0x534>;

//peripheral identification 4 register- bits bits 0-7 of this register are treated as bits 0-7 of the
//peripheral ID register
//note- periphid4, periphid5, periphid6, and periphid7 are used in conjunction and are treated as one
//32-bit register (peripheral ID register)
using periphid4 = detail::regbase<uint8_t, 0xFD0>;

//peripheral identification 5 register- bits 0-7 are treated as bits 8-15 of the peripheral ID register
//note above remarks about periphid3
using periphid5 = detail::regbase<uint8_t, 0xFD4>;

//peripheral identification 6 register- bits 0-7 are treated as bits 16-23 of the peripheral ID register
//note above remarks about periphid3
using periphid6 = detail::regbase<uint8_t, 0xFD8>;

//peripheral identification 7 register- bits 0-7 are treated as bits 24-31 of the peripheral ID register
//note above remarks about periphid3
using periphid7 = detail::regbase<uint8_t, 0xFDC>;

//peripheral identification 0 register- bits 0-7 of this register are treated as bits 0-7 of the
//peripheral ID register
//note- periphid0, periphid1, periphid2, and periphid3 are used in conjunction and are treated as one
//32-bit register (peripheral ID register), however these registers differ from 4-7 in that they can be used
//by software to identify the presence of this peripheral
using periphid0 = detail::regbase<uint8_t, 0xFE0>;

//peripheral identification 1 register- bits 0-7 are treated as bits 8-15 of the peripheral ID register
//note above remarks about periphid0
using periphid1 = detail::regbase<uint8_t, 0xFE4>;

//peripheral identification 2 register- bits 0-7 are treated as bits 16-23 of the peripheral ID register
//note above remarks about periphid0
using periphid2 = detail::regbase<uint8_t, 0xFE8>;

//peripheral identification 3 register- bits 0-7 are treated as bits 24-31 of the peripheral ID register
//note above remarks about periphid0
using periphid3 = detail::regbase<uint8_t, 0xFEC>;

//primecell identification 0 register- bits 0-7 of this register are treated as bits 0-7 of the
//primecell ID register
//note- pcellid0, pcellid1, pcellid2, and pcellid3 are used in conjunction and are treated as one
//32-bit register (primecell ID register). They provide software a standard cross-peripheral ID system.
using pcellid0 = detail::regbase<uint8_t, 0xFF0>;

//primecell identification 1 register- bits 0-7 are treated as bits 8-15 of the primecell ID register
//note above remarks about pcellid0
using pcellid1 = detail::regbase<uint8_t, 0xFF4>;

//primecell identification 1 register- bits 0-7 are treated as bits 16-23 of the primecell ID register
//note above remarks about pcellid0
using pcellid2 = detail::regbase<uint8_t, 0xFF8>;

//primecell identification 1 register- bits 0-7 are treated as bits 24-32 of the primecell ID register
//note above remarks about pcellid0
using pcellid3 = detail::regbase<uint8_t, 0xFFC>;
#endif

//port_t class to manage ports
// -provides data access
// -provides initialization
template<portname name>
class port_t {
public:
    //empty ctor
    constexpr port_t() = default;

#if defined(TARGET_IS_TM4C123_RB1)
    //gets data of the port (GPIODATA)
    //there will be two forms:
    // 1. no parameters- returns full 8-bit data
    // 2. multiple template parameters- returns bit-addressed data
    uint32_t& data() {
        return data(0, 1, 2, 3, 4, 5, 6, 7);
    }
    template<std::integral... Args>
    uint32_t& data(Args... nums) {
        if(!((nums >= 0 && nums < 8) && ...)) {
            return data(0, 1, 2, 3, 4, 5, 6, 7);
        }
        return *reinterpret_cast<uint32_t*>(m_port_base + offset(nums...));
    }

    //initializes the port
    //Given a list of registers,
    //initializes the port in that order.
    //You must also specify which clocks you want to program- normally just RCGCGPIO
    template<detail::regbase_t... Args>
    constexpr void initialize(const std::initializer_list<clockreg>& clocks,
                              Args&&... regs
                              ) {
        //set all clocks, delay at end to give time
        for(auto clock : clocks) {
            volatile uint32_t delay;
            switch(name) {
                case portname::A:
                    *(reinterpret_cast<uint32_t*>(clock)) |= 1;
                    break;
                case portname::B:
                    *(reinterpret_cast<uint32_t*>(clock)) |= 1 << 1;
                    break;
                case portname::C:
                    *(reinterpret_cast<uint32_t*>(clock)) |= 1 << 2;
                    break;
                case portname::D:
                    *(reinterpret_cast<uint32_t*>(clock)) |= 1 << 3;
                    break;
                case portname::E:
                    *(reinterpret_cast<uint32_t*>(clock)) |= 1 << 4;
                    break;
                case portname::F:
                    *(reinterpret_cast<uint32_t*>(clock)) |= 1 << 5;
                    break;
            }
            delay = *reinterpret_cast<uint32_t*>(clock); //allow time for the clock to start
        }

        //set all registers
        ([&](auto&& reg) {
            switch(reg.operation) {
                case op::set:
                    *reinterpret_cast<uint32_t*>(m_port_base + reg.offset) |= reg.data;
                    break;
                case op::clear:
                    *reinterpret_cast<uint32_t*>(m_port_base + reg.offset) &= ~reg.data;
                    break;
            }
        }(std::forward<Args>(regs)), ...);
    }

    //reconfigures the port
    //Given a list of registers,
    //reconfigures those registers in that order to their new value
    template<detail::regbase_t... Args>
    constexpr void reconfigure(Args&&... regs) {
        //set all registers
        ([&](auto&& reg) {
            switch(reg.operation) {
                case op::set:
                    *reinterpret_cast<uint32_t*>(m_port_base + reg.offset) |= reg.data;
                    break;
                case op::clear:
                    *reinterpret_cast<uint32_t*>(m_port_base + reg.offset) &= ~reg.data;
                    break;
            }
        }(std::forward<Args>(regs)), ...);    }
#endif

private:
    uint32_t m_port_base {name};
#if defined(BIT_SPECIFIC_ADDRESSING_RB1)
    //helper function to get offset sum of bit-addressed address.
    // For each bit, you add 4 * 2^n to the base address.
    // If you want multiple bits, like bits 2, 5, and 6, you just add the offsets
    template<typename... Args>
    constexpr uint16_t offset(Args... nums) {
        return (0 + ... + (4 * (1 << nums)));
    }
#endif
};


}

#endif
