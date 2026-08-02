/** atmega328p.cc
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

#include <xarch/arch/avr/avr8.h>

#include <auvia/libs/trap.h>
#include <auvia/libs/bitwise.h>
#include <auvia/modules/memory/bank.h>
#include <auvia/modules/storage/flash.h>

#include "atmega328p.h"

atmega328p::atmega328p() noexcept
{
    /* rom */
    auto* storage = new auvia::storage::flash(0x0, 0x1000);
    insn_bus_.attach(storage);

    /* ram */
    auto* cregs = new mcu::regs(0x0, 0x20);
    data_bus_.attach(cregs);
    /* a region of mmio is from 0x20 to 0x5F */
    auto* sram = new auvia::memory::bank(0x60, 1024);
    data_bus_.attach(sram);

    /* init attached devices */
    insn_bus_.setup_devices();
    data_bus_.setup_devices();
}

auto atmega328p::step() noexcept -> addr_t
{
    return 0;
}

#define nop(...) step_up_with_barrier(1)
#define step_up_with_barrier(n) \
    do {                        \
        __barrier__();          \
        step_up += (n);         \
    } while(false)

static inline auto rom_device(auvia::bus::mmu& bus) noexcept -> auto& {
    return *static_cast<auvia::storage::flash*>(
        bus.find_device(auvia::device::kind::STORAGE));
}

static inline auto mcu_device(auvia::bus::matu& bus) noexcept -> auto& {
    return *static_cast<mcu::regs*>(
        bus.find_device(auvia::device::kind::MCU));
}

static inline auto ram_device(auvia::bus::matu& bus) noexcept -> auto& {
    return *static_cast<auvia::memory::bank*>(
        bus.find_device(auvia::device::kind::MEMORY));
}

auto atmega328p::decode() noexcept -> addr_t
{
    u16* next = static_cast<u16*>(&insn_bus_[pc_]);
    u16 v;
    u8 step_up = 0;

    if ((v = avr8_NOP(next), v))
    {
        nop();
    }
    else if ((v = avr8_SPM(next), v))
    {
        /* TODO */
    }
    else if ((v = avr8_LPM1(next), v))
    {
        addr_t z = mcu_device(data_bus_).get_z();

        core_[mcu::core::r0] = static_cast<u8>(insn_bus_[z]);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_WDR(next), v))
    {
        /* TODO: reset watchdog timer */
    }
    else if ((v = avr8_BREAK(next), v))
    {
        state_ = mcu::fsm::STOP;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SLEEP(next), v))
    {
        state_ = mcu::fsm::SLEEP;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_RETI(next), v))
    {
        state_ = mcu::fsm::READY;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_ICALL(next), v))
    {
        /* TODO */
    }
    else if ((v = avr8_RET(next), v))
    {
        /* TODO */
    }
    else if ((v = avr8_CLI(next), v))
    {
        sreg_.clear_I();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_CLT(next), v))
    {
        sreg_.clear_T();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_CLH(next), v))
    {
        sreg_.clear_H();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_CLS(next), v))
    {
        sreg_.clear_S();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_CLV(next), v))
    {
        sreg_.clear_V();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_CLN(next), v))
    {
        sreg_.clear_N();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_CLZ(next), v))
    {
        sreg_.clear_Z();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_CLC(next), v))
    {
        sreg_.clear_C();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SEI(next), v))
    {
        sreg_.set_I();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SET(next), v))
    {
        sreg_.set_T();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SEH(next), v))
    {
        sreg_.set_H();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SES(next), v))
    {
        sreg_.set_S();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SEV(next), v))
    {
        sreg_.set_V();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SEN(next), v))
    {
        sreg_.set_N();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SEZ(next), v))
    {
        sreg_.set_Z();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_IJMP(next), v))
    {
        pc_ = mcu_device(data_bus_).get_z();
        step_up_with_barrier(0);
    }
    else if ((v = avr8_SEC(next), v))
    {
        sreg_.set_C();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SBRS(next), v))
    {
        u8 r = BIT_LSR(v, 4) & 0x1f;
        u8 bit_pos = v & 0x7;
        u8 rv = core_[r];

        step_up += ((rv >> bit_pos) & 1) ? 1 : 0;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SBRC(next), v))
    {
        u8 r = BIT_LSR(v, 4) & 0x1f;
        u8 bit_pos = v & 0x7;
        u8 rv = core_[r];

        step_up += ((rv >> bit_pos) & 1) ? 0 : 1;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BST(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 bit_pos = v & 0x7;
        u8 rv = core_[d];

        (rv >> bit_pos) & 1 ? sreg_.set_T() : sreg_.clear_T();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BLD(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 bit_pos = v & 0x7;
        u8 rv = core_[d];

        core_[d] = rv | (sreg_.state() & SREG::MASK::T);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRID(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_I_set() ? 0 : k);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRTC(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_T_set() ? 0 : k);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRHC(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_H_set() ? 0 : k);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRGE(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_S_set() ? 0 : k);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRVC(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_V_set() ? 0 : k);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRPL(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_N_set() ? 0 : k);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRNE(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_Z_set() ? 0 : k);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRCC(next), v) || (v = avr8_BRSH(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_C_set() ? 0 : k);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRBC(next), v))
    {
        u8 s = v & 0x7;
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += ((sreg_.state() >> s) & 1) ? 0 : k;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRIE(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_I_set() ? k : 0);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRTS(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_T_set() ? k : 0);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRHS(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_H_set() ? k : 0);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRLT(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_S_set() ? k : 0);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRVS(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_V_set() ? k : 0);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRMI(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_N_set() ? k : 0);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BREQ(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_Z_set() ? k : 0);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRCS(next), v) || (v = avr8_BRLO(next), v))
    {
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += (sreg_.is_C_set() ? k : 0);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_BRBS(next), v))
    {
        u8 s = v & 0x7;
        s8 k = BIT_LSR(v, 3) & 0x7f;

        step_up += ((sreg_.state() >> s) & 1) ? k : 0;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SER(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0xf;

        core_[d] = 0xff;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_LDI(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0xf;
        u8 K = ((v >> 4) & 0xf0) | (v & 0xf);

        core_[d] = K;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_RCALL(next), v))
    {
        /* TODO */
    }
    else if ((v = avr8_RJMP(next), v))
    {
        s16 k = v & 0xfff;

        step_up += k;
        step_up_with_barrier(1);
    }
    else if ((v = avr8_OUT(next), v))
    {
        /* TODO */
    }
    else if ((v = avr8_IN(next), v))
    {
        /* TODO */
    }
    else if ((v = avr8_STy4(next), v))
    {
        u16 r = BIT_LSR(v, 4) & 0x1f;
        u16 q = BIT_LSR((v & 0x2000), 8) |
                BIT_LSR((v & 0xc00), 7) |
                (v & 0x7);
        u16 y = mcu_device(data_bus_).get_y();

        data_bus_[y + q] = static_cast<u8>(core_[r]);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_STz4(next), v))
    {
        u16 r = BIT_LSR(v, 4) & 0x1f;
        u16 q = BIT_LSR((v & 0x2000), 8) |
                BIT_LSR((v & 0xc00), 7) |
                (v & 0x7);
        u16 z = mcu_device(data_bus_).get_z();

        data_bus_[z + q] = static_cast<u8>(core_[r]);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_LDy4(next), v))
    {
        u16 d = BIT_LSR(v, 4) & 0x1f;
        u16 q = BIT_LSR((v & 0x2000), 8) |
                BIT_LSR((v & 0xc00), 7) |
                (v & 0x7);
        u16 y = mcu_device(data_bus_).get_y();

        core_[d] = static_cast<u8>(data_bus_[y + q]);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_LDz4(next), v))
    {
        u16 d = BIT_LSR(v, 4) & 0x1f;
        u16 q = BIT_LSR((v & 0x2000), 8) |
                BIT_LSR((v & 0xc00), 7) |
                (v & 0x7);
        u16 z = mcu_device(data_bus_).get_z();

        core_[d] = static_cast<u8>(data_bus_[z + q]);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_MUL(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 r = BIT_LSR((v & 0x200), 5) | (v & 0xf);
        u16 rvd = static_cast<u8>(core_[r]) * static_cast<u8>(core_[d]);

        core_[mcu::core::r1] = static_cast<u8>((rvd & 0xff00) >> 8);
        core_[mcu::core::r0] = static_cast<u8>((rvd & 0xff));

        rvd ? sreg_.clear_Z() : sreg_.set_Z();
        static_cast<u8>(core_[mcu::core::r15]) ? sreg_.set_C()
                                               : sreg_.clear_C();
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SBIS(next), v))
    {
        u8 A = BIT_LSR(v, 3) & 0x1f;
        u8 b = (v & 0x7);
        u8 rvA = core_[A];

        step_up_with_barrier(((rvA >> b) & 1) ? 2 : 1);
    }
    else if ((v = avr8_SBI(next), v))
    {
        u8 A = BIT_LSR(v, 3) & 0x1f;
        u8 b = (v & 0x7);
        u8 rvA = core_[A];

        core_[A] = BIT_SET(static_cast<u8>(core_[A]), b);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SBIC(next), v))
    {
        u8 A = BIT_LSR(v, 3) & 0x1f;
        u8 b = (v & 0x7);
        u8 rvA = core_[A];

        step_up_with_barrier(((rvA >> b) & 1) ? 1 : 2);
    }
    else if ((v = avr8_CBI(next), v))
    {
        u8 A = BIT_LSR(v, 3) & 0x1f;
        u8 b = (v & 0x7);
        u8 rvA = core_[A];

        core_[A] = BIT_CLEAR(static_cast<u8>(core_[A]), b);
        step_up_with_barrier(1);
    }
    else if ((v = avr8_SBIW(next), v))
    {
        u8 K = BIT_LSR((v & 0xc0), 2) | (v & 0xf);
        u8 dd = 24 + 2 * ((v >> 4) & 0x3);
        u16 res = ((static_cast<u8>(core_[dd + 1]) << 8) |
                   (static_cast<u8>(core_[dd]))) - K;

        /* adjust SREG */
        u8 r15 = BIT_LSR(res & 0x8000, 15);
        u8 rdh7 = BIT_LSR(static_cast<u8>(core_[dd + 1]) & 0x80, 7);

        sreg_.adjust_C(r15 & ~rdh7);
        sreg_.adjust_Z(res != 0);
        sreg_.adjust_N(r15);
        __barrier__();
        sreg_.adjust_V(~r15 & rdh7);
        __barrier__();
        sreg_.adjust_S(sreg_.get_N() ^ sreg_.get_V());
        __barrier__();

        /* R[d+1]:Rd <- R[d+1]:Rd - K */
        core_[dd + 1] = static_cast<u8>((res >> 8) & 0xff);
        core_[dd] = static_cast<u8>(res & 0xff);

        step_up_with_barrier(1);
    }
    else if ((v = avr8_ADIW(next), v))
    {
        u8 K = BIT_LSR((v & 0xc0), 2) | (v & 0xf);
        u8 dd = 24 + 2 * ((v >> 4) & 0x3);
        u16 res = ((static_cast<u8>(core_[dd + 1]) << 8) |
                   (static_cast<u8>(core_[dd]))) + K;

        /* adjust SREG */
        u8 r15 = BIT_LSR(res & 0x8000, 15);
        u8 rdh7 = BIT_LSR(static_cast<u8>(core_[dd + 1]) & 0x80, 7);

        sreg_.adjust_C(~r15 & rdh7);
        sreg_.adjust_Z(res != 0);
        sreg_.adjust_N(r15);
        __barrier__();
        sreg_.adjust_V(~rdh7 & r15);
        __barrier__();
        sreg_.adjust_S(sreg_.get_N() ^ sreg_.get_V());
        __barrier__();

        /* R[d+1]:Rd <- R[d+1]:Rd + K */
        core_[dd + 1] = static_cast<u8>((res >> 8) & 0xff);
        core_[dd] = static_cast<u8>(res & 0xff);

        step_up_with_barrier(1);
    }
    else if ((v = avr8_CALL(next), v)) /* 32-bit */
    {
        pc_ += 2;
        stack_[sp_--] = static_cast<u8>((pc_ & 0xff00) >> 8);
        stack_[sp_--] = static_cast<u8>(pc_ & 0xff);
        __barrier__();

        u16 kh = BIT_LSR((v & 0x1f0), 3) | (v & 0x1);
        u16 kl = *reinterpret_cast<u16*>(next + 1);
        u32 k = BIT_LSL(kh, 16) | (kl);
        pc_ = k;
        __barrier__();

        goto skip;
    }
    else if ((v = avr8_JMP(next), v)) /* 32-bit */
    {
        u16 kh = BIT_LSR((v & 0x1f0), 3) | (v & 0x1);
        u16 kl = *reinterpret_cast<u16*>(next + 1);
        u32 k = BIT_LSL(kh, 16) | (kl);
        pc_ = k;
        __barrier__();

        goto skip;
    }
    else if ((v = avr8_DEC(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        core_[d] = static_cast<u8>(core_[d]) - 1;

        step_up_with_barrier(1);
    }
    else if ((v = avr8_BCLR(next), v))
    {
        u8 s = BIT_LSR(v, 4) & 0x7;
        sreg_.clear(BIT_LSL(1u, s));

        step_up_with_barrier(1);
    }
    else if ((v = avr8_BSET(next), v))
    {
        u8 s = BIT_LSR(v, 4) & 0x7;
        sreg_.set(BIT_LSL(1u, s));

        step_up_with_barrier(1);
    }
    else if ((v = avr8_ROR(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 oldv = core_[d];
        u8 newv = BIT_LSR(oldv, 1) | BIT_LSL(sreg_.get_C(), 7);
        __barrier__();

        /* adjust SREG */
        sreg_.adjust_C(oldv & SREG::BIT::R0);
        sreg_.adjust_N(newv & SREG::BIT::R7);
        sreg_.adjust_Z(newv & 0xff);
        __barrier__();
        sreg_.adjust_V(sreg_.get_N() ^ sreg_.get_C());
        __barrier__();
        sreg_.adjust_S(sreg_.get_N() ^ sreg_.get_V());
        __barrier__();

        core_[d] = newv;

        step_up_with_barrier(1);
    }
    else if ((v = avr8_LSR(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 oldv = core_[d];
        u8 newv = BIT_LSR(oldv, 1);
        __barrier__();

        /* adjust SREG */
        sreg_.adjust_C(oldv & SREG::BIT::R0);
        sreg_.adjust_N(false);
        sreg_.adjust_Z(newv & 0xff);
        __barrier__();
        sreg_.adjust_V(sreg_.get_N() ^ sreg_.get_C());
        __barrier__();
        sreg_.adjust_S(sreg_.get_N() ^ sreg_.get_V());
        __barrier__();

        core_[d] = newv;

        step_up_with_barrier(1);
    }
    else if ((v = avr8_ASR(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 oldv = core_[d];
        u8 newv = BIT_LSR(oldv, 1) | (oldv & 0x80);
        __barrier__();

        /* adjust SREG */
        sreg_.adjust_C(oldv & SREG::BIT::R0);
        sreg_.adjust_Z(newv != 0);
        sreg_.adjust_N(newv & 0x80);
        __barrier__();
        sreg_.adjust_V(sreg_.get_N() ^ sreg_.get_C());
        __barrier__();
        sreg_.adjust_S(sreg_.get_N() ^ sreg_.get_V());
        __barrier__();

        core_[d] = newv;

        step_up_with_barrier(1);
    }
    else if ((v = avr8_INC(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 oldv = core_[d];
        u8 newv = oldv + 1;
        __barrier__();

        /* adjust SREG */
        sreg_.adjust_Z(newv != 0);
        sreg_.adjust_N(newv & 0x80);
        __barrier__();
        sreg_.adjust_V(newv & 0x80);
        __barrier__();
        sreg_.adjust_S(sreg_.get_N() ^ sreg_.get_V());
        __barrier__();

        core_[d] = newv;

        step_up_with_barrier(1);
    }
    else if ((v = avr8_SWAP(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 oldv = core_[d];
        __barrier__();

        /* swap bits */
        core_[d] = BIT_LSL((oldv & 0x0f), 4) | BIT_LSR((oldv & 0xf0), 4);

        step_up_with_barrier(1);
    }
    else if ((v = avr8_NEG(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 oldv = core_[d];

        /* TODO */
    }
    else if ((v = avr8_COM(next), v))
    {
        u8 d = BIT_LSR(v, 4) & 0x1f;
        u8 oldv = core_[d];
        __barrier__();

        //core_[d] =

        /* adjust SREG */
        sreg_.adjust_C(true);
    }
    else if ((v = avr8_PUSH(next), v))
    { }
    else if ((v = avr8_STx3(next), v))
    { }
    else if ((v = avr8_STx2(next), v))
    { }
    else if ((v = avr8_STx1(next), v))
    { }
    else if ((v = avr8_STy3(next), v))
    { }
    else if ((v = avr8_STy2(next), v))
    { }
    else if ((v = avr8_STz3(next), v))
    { }
    else if ((v = avr8_STz2(next), v))
    { }
    else if ((v = avr8_STS(next), v)) /* 32-bit */
    {
        u16 k;

        next += 1;
        k = *reinterpret_cast<u16*>(next);
    }
    else if ((v = avr8_POP(next), v))
    { }
    else if ((v = avr8_LDx3(next), v))
    { }
    else if ((v = avr8_LDx2(next), v))
    { }
    else if ((v = avr8_LDx1(next), v))
    { }
    else if ((v = avr8_LDy3(next), v))
    { }
    else if ((v = avr8_LDy2(next), v))
    { }
    else if ((v = avr8_LPM3(next), v))
    { }
    else if ((v = avr8_LPM2(next), v))
    { }
    else if ((v = avr8_LDz3(next), v))
    { }
    else if ((v = avr8_LDz2(next), v))
    { }
    else if ((v = avr8_LDS(next), v)) /* 32-bit */
    {
        u16 k;

        next += 1;
        k = *reinterpret_cast<u16*>(next);
    }
    else if ((v = avr8_STy1(next), v))
    { }
    else if ((v = avr8_STz1(next), v))
    { }
    else if ((v = avr8_LDy1(next), v))
    { }
    else if ((v = avr8_LDz1(next), v))
    { }
    else if ((v = avr8_ANDI(next), v) || (v = avr8_CBR(next), v))
    { }
    else if ((v = avr8_SBR(next), v) || (v = avr8_ORI(next), v))
    { }
    else if ((v = avr8_SUBI(next), v))
    { }
    else if ((v = avr8_SBCI(next), v))
    { }
    else if ((v = avr8_CPI(next), v))
    { }
    else if ((v = avr8_MOV(next), v))
    { }
    else if ((v = avr8_OR(next), v))
    { }
    else if ((v = avr8_CLR(next), v) || (v = avr8_EOR(next), v))
    { }
    else if ((v = avr8_TST(next), v) || (v = avr8_AND(next), v))
    { }
    else if ((v = avr8_ROL(next), v) || (v = avr8_ADC(next), v))
    { }
    else if ((v = avr8_SUB(next), v))
    { }
    else if ((v = avr8_CP(next), v))
    { }
    else if ((v = avr8_CPSE(next), v))
    { }
    else if ((v = avr8_LSL(next), v) || (v = avr8_ADD(next), v))
    { }
    else if ((v = avr8_SBC(next), v))
    { }
    else if ((v = avr8_CPC(next), v))
    { }
    else if ((v = avr8_FMULSU(next), v))
    { }
    else if ((v = avr8_FMULS(next), v))
    { }
    else if ((v = avr8_FMUL(next), v))
    { }
    else if ((v = avr8_MULSU(next), v))
    { }
    else if ((v = avr8_MULS(next), v))
    { }
    else if ((v = avr8_MOVW(next), v))
    { }
    else
    {
        panic("Undefined instruction: %x", *next);
    }

    pc_ += step_up;

skip:
    return pc_;
}
