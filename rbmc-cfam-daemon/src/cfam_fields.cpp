#include "cfam_fields.hpp"

#include <bitset>
#include <format>
#include <ranges>
#include <utility>

namespace cfam
{

uint32_t getFieldValue(const RegMap& regs, const FieldMetadata& meta)
{
    if (!regs.contains(meta.reg))
    {
        throw std::invalid_argument{
            std::format("getFieldValue: reg {} not in RegMap",
                        std::to_underlying(meta.reg))};
    }
    return getFieldValue(regs.at(meta.reg), meta.start, meta.numBits);
}

uint32_t getFieldValue(uint32_t regData, size_t start, size_t numBits)
{
    if ((start + numBits) > 32)
    {
        throw std::invalid_argument{
            std::format("getFieldValue: Start bit {} + numBits {} too big",
                        start, numBits)};
    }

    // First clear out the left most bits by shifting all the way left
    uint32_t data = regData << start;

    // Now right justify
    data = data >> (32 - numBits);

    return data;
}

ModifyOp createModifyOp(const FieldMetadata& meta, uint32_t fieldValue)
{
    uint32_t data = fieldValue << (32 - meta.start - meta.numBits);
    std::bitset<32> mask{};

    for (auto i : std::views::iota(meta.start, meta.start + meta.numBits))
    {
        // bitset uses the opposite numbering than CFAM regs
        mask.set(31 - i);
    }

    return {meta.reg, data, static_cast<uint32_t>(mask.to_ulong())};
}

} // namespace cfam
