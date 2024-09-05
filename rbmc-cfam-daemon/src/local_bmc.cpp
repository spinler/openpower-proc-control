/* SPDX-License-Identifier: Apache-2.0 */
#include "local_bmc.hpp"

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/async.hpp>
#include <xyz/openbmc_project/State/BMC/Redundancy/common.hpp>
#include <xyz/openbmc_project/State/BMC/client.hpp>

constexpr uint8_t apiVersion{0x01};

sdbusplus::async::task<> LocalBMC::start()
{
    co_await waitForCFAM();

    writeApiVersion();
    writeFWVersion();
    writeBmcPosition();
    writeProvisioned();
    writeSiblingCommsNotOK();

    services = std::make_unique<Services>(
        ctx, [this](auto state) { bmcStateChanged(state); },
        [this](auto role) { roleChanged(role); },
        [this](auto enabled) { redEnabledChanged(enabled); });

    co_await writeRedundancyProps();
    co_await writeBMCState();

    ctx.spawn(watchHeartBeat());

    co_return;
}

sdbusplus::async::task<> LocalBMC::waitForCFAM()
{
    using namespace std::chrono_literals;
    bool tracedWait = false;
    const std::chrono::minutes timeout{5};
    auto startTime = std::chrono::steady_clock::now();

    while (!cfam.isReady())
    {
        co_await sdbusplus::async::sleep_for(ctx, 1s);

        if (!tracedWait)
        {
            lg2::info("Waiting for local CFAM");
            tracedWait = true;
        }

        // For now, crash so the BMC goes into Quiesce
        if ((std::chrono::steady_clock::now() - startTime) >= timeout)
        {
            // TODO: Create event log calling out hardware
            lg2::error("Timed out waiting for local CFAM to show up");
            throw std::runtime_error{"Local CFAM never showed up"};
        }
    }

    lg2::info("Local CFAM is ready");

    co_return;
}

sdbusplus::async::task<> LocalBMC::watchHeartBeat()
{
    using namespace sdbusplus::bus::match;
    using Redundancy =
        sdbusplus::common::xyz::openbmc_project::state::bmc::Redundancy;

    sdbusplus::async::match match(ctx, rules::interface(Redundancy::interface) +
                                           rules::member("Heartbeat"));

    while (!ctx.stop_requested())
    {
        co_await match.next<>();

        cfam.incHeartbeat();
    }

    co_return;
}

void LocalBMC::writeApiVersion()
{
    cfam.writeApiVersion(apiVersion);
}

void LocalBMC::writeFWVersion()
{
    const std::filesystem::path osRelease{"/etc/os-release"};

    auto versionID = services->getVersionID(osRelease);

    // Write the hash to the CFAM
    uint32_t version = std::hash<std::string>{}(versionID);
    cfam.writeFWVersion(version);
}

void LocalBMC::writeBmcPosition()
{
    cfam.writeBmcPosition(services->getBMCPosition());
}

void LocalBMC::writeSiblingCommsNotOK()
{
    cfam.writeSiblingCommsOK(false);
    siblingOK = false;
}

void LocalBMC::writeProvisioned()
{
    cfam.writeProvisioned(false);
}

void LocalBMC::bmcStateChanged(BMCState state)
{
    lg2::info("Local BMC state changed to {STATE}", "STATE", state);
    cfam.writeBMCState(state);
}

void LocalBMC::roleChanged(Role role)
{
    lg2::info("Local Role changed to {ROLE}", "ROLE", role);
    cfam.writeRole(role);
}
void LocalBMC::redEnabledChanged(bool enabled)
{
    lg2::info("Local Redundancy enabled changed to {ENABLED}", "ENABLED",
              enabled);
    cfam.writeRedundancyEnabled(enabled);
}

sdbusplus::async::task<> LocalBMC::writeRedundancyProps()
{
    try
    {
        auto [role, enabled] = co_await services->getRedundancyProps();
        lg2::info(
            "Initial values of local role and redEnabled: {ROLE} {ENABLED}",
            "ROLE", role, "ENABLED", enabled);

        cfam.writeRole(role);
        cfam.writeRedundancyEnabled(enabled);
        cfam.writeFailoversPaused(false); // TODO: get from D-Bus
    }
    catch (const sdbusplus::exception_t& e)
    {
        lg2::info("Redundancy interface not on D-Bus: {ERROR}", "ERROR", e);
    }

    co_return;
}

sdbusplus::async::task<> LocalBMC::writeBMCState()
{
    try
    {
        auto state = co_await services->getBMCState();
        lg2::info("Initial value of local BMC state is {STATE}", "STATE",
                  state);
        cfam.writeBMCState(state);
    }
    catch (const sdbusplus::exception_t& e)
    {
        lg2::info("Local BMC state not on D-Bus: {ERROR}", "ERROR", e);
    }

    co_return;
}
void LocalBMC::setSiblingCommsOK(bool ok)
{
    if (ok != siblingOK)
    {
        cfam.writeSiblingCommsOK(ok);
        siblingOK = ok;
    }
}
