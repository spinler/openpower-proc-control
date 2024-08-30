#pragma once

#include <cstdint>
#include <unordered_map>

namespace cfam
{

/**
 * @brief Enums for the CFAM scratchpad registers
 */
enum class ScratchPadReg
{
    one,
    two,
    three,
    four
};

constexpr auto numScratchPadRegs = 4;
using RegMap = std::unordered_map<ScratchPadReg, uint32_t>;

/**
 * @brief Information needed for a read/modify/write operation.
 */
struct ModifyOp
{
    ScratchPadReg reg;
    uint32_t data;
    uint32_t mask;
};

} // namespace cfam
