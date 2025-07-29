#include <gtest/gtest.h>

#include "transfer_settings_widget.h"


TEST(TransferSettingsWidget, GetSetting) {
    TransferSettingsWidget wgt;
    auto settings = wgt.GetSettings();

    ASSERT_TRUE(settings.contains("port_name"));

    ASSERT_TRUE(settings.contains("baud_rate"));
    EXPECT_EQ(settings["baud_rate"],"110");

    ASSERT_TRUE(settings.contains("data_bits"));
    EXPECT_EQ(settings["data_bits"],"5");

    ASSERT_TRUE(settings.contains("parity"));
    EXPECT_EQ(settings["parity"],"None");

    ASSERT_TRUE(settings.contains("stop_bits"));
    EXPECT_EQ(settings["stop_bits"],"1");

    ASSERT_TRUE(settings.contains("flow_control"));
    EXPECT_EQ(settings["flow_control"],"None");
}

