/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include "cfam_access.hpp"
#include "cfam_fields.hpp"

#include <algorithm>
#include <map>
#include <set>

/**
 * @class BMCCFAM
 *
 * The base class representing a CFAM-S on a BMC card, where the four
 * scratchpad registers in the Gemini Mailbox block of the CFAM are
 * used to store redundant BMC information.
 *
 * Children of this base class are local CFAM and sibling CFAM classes.
 */
class BMCCFAM
{
  public:
    /**
     * @brief The fields stored in the mailbox scratchpad registers
     * in CFAM-S chips off of a BMC.
     */
    enum class Field
    {
        apiVersion,
        bmcPosition,
        role,
        redundancyEnabled,
        failoversPaused,
        provisioned,
        bmcState,
        siblingCommsOK,
        heartbeat,
        fwVersion
    };

    /**
     * @brief The register, starting bit, and length of the fields.
     */
    inline static const std::map<Field, cfam::FieldMetadata> cfamFields{
        {Field::apiVersion, {cfam::ScratchPadReg::one, 0, 8}},
        {Field::bmcPosition, {cfam::ScratchPadReg::one, 8, 1}},
        {Field::role, {cfam::ScratchPadReg::one, 9, 2}},
        {Field::redundancyEnabled, {cfam::ScratchPadReg::one, 11, 1}},
        {Field::failoversPaused, {cfam::ScratchPadReg::one, 12, 1}},
        {Field::provisioned, {cfam::ScratchPadReg::one, 13, 1}},
        {Field::bmcState, {cfam::ScratchPadReg::one, 14, 3}},
        {Field::siblingCommsOK, {cfam::ScratchPadReg::one, 17, 1}},
        {Field::heartbeat, {cfam::ScratchPadReg::one, 24, 8}},
        {Field::fwVersion, {cfam::ScratchPadReg::two, 0, 32}}};

    BMCCFAM() = delete;
    virtual ~BMCCFAM() = default;
    BMCCFAM(const BMCCFAM&) = delete;
    BMCCFAM& operator=(const BMCCFAM&) = delete;
    BMCCFAM(BMCCFAM&&) = delete;
    BMCCFAM& operator=(BMCCFAM&&) = delete;

    /**
     * @brief Constructor
     *
     * @param[in] link - The link the CFAM is on
     * @param[in] sysfs - The sysfs API object
     */
    BMCCFAM(size_t link, SysFS& sysfs) : cfamAccess(link, sysfs) {}

    /**
     * @brief Of the four possible scratchpad registers, returns the ones
     *        that are used for fields.
     *
     *  @return The set of registers
     */
    inline static std::set<cfam::ScratchPadReg> getUsedRegs()
    {
        std::set<cfam::ScratchPadReg> regs;

        std::ranges::for_each(cfamFields, [&regs](const auto& entry) {
            regs.insert(entry.second.reg);
        });

        return regs;
    }

  protected:
    /**
     * @brief The object to access the hardware.
     */
    CFAMAccess cfamAccess;
};
