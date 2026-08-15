#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "storage_registry.h"

namespace {

namespace fs = std::filesystem;

using data_storage::StorageRegistry;

class StorageRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        root_ = fs::temp_directory_path() /
                ("dcm-registry-test-" + std::to_string(::getpid()) + "-" +
                 ::testing::UnitTest::GetInstance()->current_test_info()->name());
        fs::create_directories(root_);
    }

    void TearDown() override {
        fs::remove_all(root_);
    }

    StorageRegistry MakeRegistry() const {
        return StorageRegistry{root_.string(), "text", "0"};
    }

    fs::path root_;
};

TEST_F(StorageRegistryTest, WritingSideCreatesTheCollectorDirectory) {
    StorageRegistry registry = MakeRegistry();

    ASSERT_NE(registry.ForCollector("collector-1"), nullptr);
    EXPECT_TRUE(fs::is_directory(root_ / "collector-1"));
}

TEST_F(StorageRegistryTest, TheSameCollectorGetsTheSameStorage) {
    StorageRegistry registry = MakeRegistry();

    EXPECT_EQ(registry.ForCollector("collector-1"), registry.ForCollector("collector-1"));
}

TEST_F(StorageRegistryTest, ReadingSideCreatesNothing) {
    StorageRegistry registry = MakeRegistry();

    // A DataLoad naming a collector nobody ever heard of must not leave an
    // empty directory behind — ListCollectors would then report it.
    EXPECT_EQ(registry.FindCollector("never-existed"), nullptr);
    EXPECT_FALSE(fs::exists(root_ / "never-existed"));
}

TEST_F(StorageRegistryTest, ReadingSideFindsWhatIsOnDisk) {
    // What a restarted service sees: the directory is there, the map is empty.
    fs::create_directories(root_ / "collector-1");

    StorageRegistry registry = MakeRegistry();

    EXPECT_NE(registry.FindCollector("collector-1"), nullptr);
}

TEST_F(StorageRegistryTest, KnownCollectorsComeFromDiskSorted) {
    fs::create_directories(root_ / "collector-2");
    fs::create_directories(root_ / "collector-1");

    StorageRegistry registry = MakeRegistry();

    EXPECT_EQ(registry.KnownCollectors(), (std::vector<std::string>{"collector-1", "collector-2"}));
}

TEST_F(StorageRegistryTest, AnEmptyRootIsNotAnError) {
    StorageRegistry registry = MakeRegistry();

    EXPECT_TRUE(registry.KnownCollectors().empty());
}

}   //namespace
