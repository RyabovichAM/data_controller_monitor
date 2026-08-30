#include <gtest/gtest.h>

#include "data_storage_interface.h"

namespace {

using data_storage::SampleThinner;
using data_storage::TimePoint;

TEST(SampleThinner, FirstSampleIsAlwaysKept) {
    SampleThinner thinner{60'000};
    EXPECT_TRUE(thinner.ShouldKeep(TimePoint{}));
}

TEST(SampleThinner, ASampleWithinThePeriodIsDropped) {
    SampleThinner thinner{60'000};
    const TimePoint first{};

    ASSERT_TRUE(thinner.ShouldKeep(first));
    EXPECT_FALSE(thinner.ShouldKeep(first + std::chrono::seconds{30}));
}

TEST(SampleThinner, ASampleAfterThePeriodIsKept) {
    SampleThinner thinner{60'000};
    const TimePoint first{};

    ASSERT_TRUE(thinner.ShouldKeep(first));
    EXPECT_TRUE(thinner.ShouldKeep(first + std::chrono::seconds{61}));
}

TEST(SampleThinner, ZeroPeriodKeepsEverySample) {
    SampleThinner thinner{0};
    const TimePoint first{};

    EXPECT_TRUE(thinner.ShouldKeep(first));
    EXPECT_TRUE(thinner.ShouldKeep(first + std::chrono::milliseconds{1}));
}

TEST(SampleThinner, KeptSampleBecomesTheNewReference) {
    SampleThinner thinner{60'000};
    const TimePoint first{};

    ASSERT_TRUE(thinner.ShouldKeep(first));
    ASSERT_TRUE(thinner.ShouldKeep(first + std::chrono::seconds{61}));
    // Measured from the second kept sample, not the first.
    EXPECT_FALSE(thinner.ShouldKeep(first + std::chrono::seconds{90}));
}

}   //namespace
