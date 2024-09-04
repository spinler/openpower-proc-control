/* SPDX-License-Identifier: Apache-2.0 */
#pragma once

#include "local_cfam.hpp"
#include "services.hpp"
#include "sysfs.hpp"

#include <sdbusplus/async/context.hpp>

/**
 * @class LocalBMC
 *
 * Represents the local BMC.  This class handles writing
 * data about this BMC into its own CFAM so that the sibling
 * BMC can read it.
 */
class LocalBMC
{
  public:
    LocalBMC() = delete;
    ~LocalBMC() = default;
    LocalBMC(const LocalBMC&) = delete;
    LocalBMC& operator=(const LocalBMC&) = delete;
    LocalBMC(LocalBMC&&) = delete;
    LocalBMC& operator=(LocalBMC&&) = delete;

    /**
     * @brief Constructor
     *
     * @param ctx - The async context object
     * @param sysfs - The sysfs object
     */
    LocalBMC(sdbusplus::async::context& ctx, SysFS& sysfs) :
        ctx(ctx), cfam(0, sysfs)
    {}

    /**
     * @brief Kicks off initializing the local CFAM.
     *
     * This will write the fields into the CFAM, and start watches
     * for when D-Bus properties change that trigger more writes.
     */
    sdbusplus::async::task<> start();

  private:
    using Role =
        sdbusplus::common::xyz::openbmc_project::state::bmc::Redundancy::Role;
    using BMCState =
        sdbusplus::common::xyz::openbmc_project::state::BMC::BMCState;

    /**
     * @brief Writes the API version field into the CFAM
     */
    void writeApiVersion();

    /**
     * @brief Writes the FW version field into the CFAM
     */
    void writeFWVersion();

    /**
     * @brief Writes the BMC position field into the CFAM
     */
    void writeBmcPosition();

    /**
     * @brief Writes the Provisioned field into the CFAM
     */
    void writeProvisioned();

    /**
     * @brief Writes the Sibling Comms OK field into the CFAM
     */
    inline void writeSiblingCommsNotOK();

    /**
     * @brief Writes the Redundancy Enabled field into the CFAM
     */
    sdbusplus::async::task<> writeRedundancyProps();

    /**
     * @brief Writes the BMC state field into the CFAM
     */
    sdbusplus::async::task<> writeBMCState();

    /**
     * @brief Watches for heartbeat signals to increment the
     *        heartbeat field in the CFAM.
     */
    sdbusplus::async::task<> watchHeartBeat();

    /**
     * @brief Waits for the CFAM's sysfs files to show up so that
     *        the fields can be written.
     */
    sdbusplus::async::task<> waitForCFAM();

    /**
     * @brief Callback function for when the BMC state D-Bus property
     *        changes.
     *
     * Writes the new value into the CFAM.
     *
     * @param state - Value to write
     */
    void bmcStateChanged(BMCState state);

    /**
     * @brief Callback function for when the role D-Bus property
     *        changes.
     *
     * Writes the new value into the CFAM.
     *
     * @param role - Value to write
     */
    void roleChanged(Role role);

    /**
     * @brief Callback function for when the redundancy enabled
     * D-Bus property changes.
     *
     * Writes the new value into the CFAM.
     *
     * @param enabled - Value to write
     */
    void redEnabledChanged(bool enabled);

    /**
     * @brief The context object
     */
    sdbusplus::async::context& ctx;

    /**
     * @brief The LocalCFAM object to handle the writes
     */
    LocalCFAM cfam;

    /**
     * @brief Object to read data from the system.
     */
    std::unique_ptr<Services> services;
};
