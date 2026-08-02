/** atmega328p.h
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
#include <auvia/arch/harvard.h>

#include "mcu.h"

/**
 * @brief Represents Atmega328p MCU based on Harvard architecture.
 */
class atmega328p final : public auvia::arch::harvard
{
public:
    atmega328p() noexcept;
    ~atmega328p() noexcept = default;

    auto step() noexcept -> addr_t;
    auto decode() noexcept -> addr_t;

private:
    SREG        sreg_;
    addr_t      pc_{0};
    addr_t      sp_{0x45F};
    mcu::fsm    state_{mcu::fsm::RESET};

    /* aliases */
    auvia::bus::matu&   core_{data_bus_};
    auvia::bus::matu&   stack_{data_bus_};

private:
    /* issue if this fails at compile-time */
    atmega328p(const atmega328p &rhs)
        : pc_(BUILD_BUG_ON(sizeof(addr_t) - sizeof(u16)))
    { }
}; // class atmega328p
