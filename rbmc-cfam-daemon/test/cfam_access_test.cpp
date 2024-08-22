/* SPDX-License-Identifier: Apache-2.0 */
#include "cfam_access.hpp"
#include "mock_sysfs.hpp"

#include <gtest/gtest.h>

using ::testing::Return;

// Test CFAMAccess::readScratchReg()
TEST(CFAMAccess, ReadTest)
{
    MockSysFS sysfs;

    std::expected<uint32_t, int> expected = 0x12345678;
    std::filesystem::path reg1{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};
    std::filesystem::path reg2{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch2"};

    // First read works, next one fails
    EXPECT_CALL(sysfs, read(reg1)).WillOnce(Return(expected));
    EXPECT_CALL(sysfs, read(reg2)).WillOnce(Return(std::unexpected<int>{2}));

    CFAMAccess cfam{0, sysfs};

    EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::one), 0x12345678);
    EXPECT_EQ(cfam.readScratchReg(cfam::ScratchPadReg::two),
              std::unexpected<int>{2});
}

// Test CFAMAccess::writeScratchReg()
TEST(CFAMAccess, WriteTest)
{
    MockSysFS sysfs;

    std::filesystem::path reg1{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};
    std::filesystem::path reg2{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch2"};

    // First write works, next one fails
    EXPECT_CALL(sysfs, write(reg1, 0x12345678)).WillOnce(Return(0));
    EXPECT_CALL(sysfs, write(reg2, 0)).WillOnce(Return(-1));

    CFAMAccess cfam{0, sysfs};

    EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::one, 0x12345678), 0);
    EXPECT_EQ(cfam.writeScratchReg(cfam::ScratchPadReg::two, 0), -1);
}

// Test CFAMAccess::writeScratchRegWithMask()
TEST(CFAMAccess, WriteWithMaskTest)
{
    MockSysFS sysfs;

    std::filesystem::path reg1{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch1"};
    std::filesystem::path reg2{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch2"};
    std::filesystem::path reg3{
        "/sys/class/fsi-master/fsi0/slave@00:00/scratch3"};

    std::expected<uint32_t, int> expected = 0x12345678;

    // A working read/write
    EXPECT_CALL(sysfs, read(reg1)).WillOnce(Return(expected));
    EXPECT_CALL(sysfs, write(reg1, 0x12AAAA78)).WillOnce(Return(0));

    // Read fails
    EXPECT_CALL(sysfs, read(reg2)).WillOnce(Return(std::unexpected<int>{2}));

    // Read works, write fails
    EXPECT_CALL(sysfs, read(reg3)).WillOnce(Return(expected));
    EXPECT_CALL(sysfs, write(reg3, 0x12AAAA78)).WillOnce(Return(3));

    CFAMAccess cfam{0, sysfs};

    cfam::ModifyOp op{cfam::ScratchPadReg::one, 0x00AAAA00, 0x00FFFF00};
    EXPECT_EQ(cfam.writeScratchRegWithMask(op), 0);

    // Read fails
    op.reg = cfam::ScratchPadReg::two;
    EXPECT_EQ(cfam.writeScratchRegWithMask(op), 2);

    // Write fails
    op.reg = cfam::ScratchPadReg::three;
    EXPECT_EQ(cfam.writeScratchRegWithMask(op), 3);
}
