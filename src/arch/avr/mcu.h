/** mcu.h
 *
 *  Copyright (C) 2025-2026 Leesoo Ahn <lsahn@ooseel.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <auvia/libs/types.h>
#include <auvia/libs/trap.h>
#include <auvia/osal/mmio.h>

namespace mcu {

enum class fsm {
    UNDEFINED = 0,
    RESET,
    READY,
    RUNNING,
    PAUSE,
    STOP,
    SLEEP,
    IOWAIT,
    IRQ,
};

enum core
{
     r0,  r1,  r2,  r3,
     r4,  r5,  r6,  r7,
     r8,  r9, r10, r11,
    r12, r13, r14, r15,
    r16, r17, r18, r19,
    r20, r21, r22, r23,
    r24, r25, r26, r27,
    r28, r29, r30, r31,

    NR_REGS,

    /* alias */
    xL = r26, xH = r27,
    yL = r28, yH = r29,
    zL = r30, zH = r31,
};

/**
 * @brief Represents Core Registers of the MCU.
 */
class regs final : public auvia::mmio
{
public:
    regs(const usize start, const usize size) noexcept
        : mmio(kind::MCU, start, size)
        { }
    ~regs() noexcept = default;

    auto operator[](const usize offs) override -> auvia::variant {
        if (range_of(offs) == false) {
            return auvia::trap::error(auvia::trap::kind::PAGE_FAULT);
        }
        return auvia::variant(&regs_[offs]);
    }

    inline auto get_x() noexcept -> addr_t {
        return static_cast<addr_t>((regs_[xH] << 8) | regs_[xL]);
    }

    inline auto get_y() noexcept -> addr_t {
        return static_cast<addr_t>((regs_[yH] << 8) | regs_[yL]);
    }

    inline auto get_z() noexcept -> addr_t {
        return static_cast<addr_t>((regs_[zH] << 8) | regs_[zL]);
    }

    auto setup() override -> bool {
        return true;
    }

    auto release() override -> bool {
        return true;
    }

private:
    u8 regs_[NR_REGS];
}; // class regs
} // namespace mcu

#define SREG_BIT_METHODS(m, lambda)                         \
    inline auto set_ ## m (void) noexcept -> void {         \
        set(MASK::m);                                       \
    }                                                       \
    inline auto get_ ## m (void) noexcept -> u8 {           \
        return !!is_set(MASK::m);                           \
    }                                                       \
    inline auto clear_ ## m (void) noexcept -> void {       \
        clear(MASK::m);                                     \
    }                                                       \
    inline auto is_ ## m ## _set(void) noexcept -> bool {   \
        return is_set(MASK::m);                             \
    }                                                       \
    inline auto adjust_ ## m lambda

/**
 * @brief Represents the State Register of the MCU.
 */
class SREG
{
public:
    struct BIT {
        static constexpr u8 R7 = (1u << 7);
        static constexpr u8 R6 = (1u << 6);
        static constexpr u8 R5 = (1u << 5);
        static constexpr u8 R4 = (1u << 4);
        static constexpr u8 R3 = (1u << 3);
        static constexpr u8 R2 = (1u << 2);
        static constexpr u8 R1 = (1u << 1);
        static constexpr u8 R0 = (1u << 0);
    };

    struct MASK {
        static constexpr u8 I = BIT::R7;
        static constexpr u8 T = BIT::R6;
        static constexpr u8 H = BIT::R5;
        static constexpr u8 S = BIT::R4;
        static constexpr u8 V = BIT::R3;
        static constexpr u8 N = BIT::R2;
        static constexpr u8 Z = BIT::R1;
        static constexpr u8 C = BIT::R0;
    };

    SREG() noexcept
        : state_(0)
        { }
    ~SREG() noexcept = default;

    inline auto set(const u8 mask) noexcept -> void {
        state_ = state_ | mask;
    }

    inline auto clear(const u8 mask) noexcept -> void {
        state_ = state_ & ~mask;
    }

    inline auto is_set(const u8 mask) noexcept -> bool {
        return (state_ & mask) != 0;
    }

    inline auto state() const noexcept -> u8 {
        return state_;
    }

    SREG_BIT_METHODS(C, (const bool set) noexcept -> void {
        set ? set_C() : clear_C();
    })

    SREG_BIT_METHODS(Z, (const bool clear) noexcept -> void {
        clear ? clear_Z() : set_Z();
    })

    SREG_BIT_METHODS(N, (const bool set) noexcept -> void {
        set ? set_N() : clear_N();
    })

    SREG_BIT_METHODS(V, (const bool set) noexcept -> void {
        set ? set_V() : clear_V();
    })

    SREG_BIT_METHODS(S, (const bool set) noexcept -> void {
        set ? set_S() : clear_S();
    })

    SREG_BIT_METHODS(H, (const bool set) noexcept -> void {
        set ? set_H() : clear_H();
    })

    SREG_BIT_METHODS(T, (const bool set) noexcept -> void {
        set ? set_T() : clear_T();
    })

    SREG_BIT_METHODS(I, (const bool set) noexcept -> void {
        set ? set_I() : clear_I();
    })

private:
    union {
        u8      state_;

        u32     __align;
    };

private:
    /* issue if this fails at compile-time */
    SREG(phys_t v) : state_(BUILD_BUG_ON(sizeof(SREG) % 4)) {}
}; // class SREG
#undef SREG_BIT_METHODS
