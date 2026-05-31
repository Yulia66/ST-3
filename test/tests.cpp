// Copyright 2025 UNN-IASR
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdexcept>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Exactly;
using ::testing::AtLeast;

class MockAlarmListener : public AlarmListener {
 public:
    MOCK_METHOD(void, notifyTimeout, (), (override));
};

class MockEntrance : public Entrance {
 public:
    MOCK_METHOD(void, secure, (), (override));
    MOCK_METHOD(void, release, (), (override));
    MOCK_METHOD(bool, isOpen, (), (override));
};

class TimedEntranceTest : public ::testing::Test {
 protected:
    void SetUp() override {
        testEntry = new TimedEntrance(100);
    }

    void TearDown() override {
        delete testEntry;
    }

    TimedEntrance* testEntry;
};

class DoorBellTest : public ::testing::Test {
 protected:
    void SetUp() override {
        timedEntry = new TimedEntrance(100);
        bellAdapter = new DoorBell(*timedEntry);
    }

    void TearDown() override {
        delete bellAdapter;
        delete timedEntry;
    }

    TimedEntrance* timedEntry;
    DoorBell* bellAdapter;
};

class ClockTest : public ::testing::Test {
 protected:
    void SetUp() override {
        mockClient = new MockAlarmListener();
        timerDevice = new Clock();
    }

    void TearDown() override {
        delete timerDevice;
        delete mockClient;
    }

    MockAlarmListener* mockClient;
    Clock* timerDevice;
};

TEST_F(TimedEntranceTest, ConstructorSetsDurationCorrectly) {
    EXPECT_EQ(testEntry->getDuration(), 100);
}

TEST_F(TimedEntranceTest, InitiallyEntranceIsSecured) {
    EXPECT_FALSE(testEntry->isOpen());
}

TEST_F(TimedEntranceTest, ReleaseOpensEntrance) {
    testEntry->release();
    EXPECT_TRUE(testEntry->isOpen());
}

TEST_F(TimedEntranceTest, SecureClosesEntrance) {
    testEntry->release();
    EXPECT_TRUE(testEntry->isOpen());
    testEntry->secure();
    EXPECT_FALSE(testEntry->isOpen());
}

TEST_F(TimedEntranceTest, RaiseAlertThrowsException) {
    EXPECT_THROW(testEntry->raiseAlert(), std::runtime_error);
}

TEST_F(TimedEntranceTest, MultipleReleaseAndSecureCycles) {
    testEntry->release();
    EXPECT_TRUE(testEntry->isOpen());
    testEntry->secure();
    EXPECT_FALSE(testEntry->isOpen());
    testEntry->release();
    EXPECT_TRUE(testEntry->isOpen());
    testEntry->secure();
    EXPECT_FALSE(testEntry->isOpen());
}

TEST_F(DoorBellTest, NotifyThrowsWhenEntranceIsOpen) {
    timedEntry->release();
    EXPECT_THROW(bellAdapter->notifyTimeout(), std::runtime_error);
}

TEST_F(DoorBellTest, NotifyDoesNotThrowWhenEntranceIsSecured) {
    timedEntry->secure();
    EXPECT_NO_THROW(bellAdapter->notifyTimeout());
}

TEST_F(DoorBellTest, NotifyWithSecuredEntranceNoEffect) {
    timedEntry->secure();
    bellAdapter->notifyTimeout();
    SUCCEED();
}

TEST_F(DoorBellTest, NotifyAfterReleaseThenSecure) {
    timedEntry->release();
    timedEntry->secure();
    EXPECT_NO_THROW(bellAdapter->notifyTimeout());
}

TEST_F(ClockTest, SetTimerCallsNotifyOnClient) {
    EXPECT_CALL(*mockClient, notifyTimeout()).Times(1);
    timerDevice->setTimer(1, mockClient);
}

TEST_F(ClockTest, SetTimerWithNullClientDoesNotCrash) {
    EXPECT_NO_THROW(timerDevice->setTimer(1, nullptr));
}

TEST_F(ClockTest, SetTimerWithLongerDelayStillCallsNotify) {
    EXPECT_CALL(*mockClient, notifyTimeout()).Times(1);
    timerDevice->setTimer(10, mockClient);
}

TEST(CombinedTest, FullEntranceOperationScenario) {
    TimedEntrance entry(50);

    EXPECT_FALSE(entry.isOpen());

    entry.release();
    EXPECT_TRUE(entry.isOpen());

    entry.secure();
    EXPECT_FALSE(entry.isOpen());

    entry.release();
    EXPECT_TRUE(entry.isOpen());
}

TEST(AlertTriggerTest, ExceptionTriggeredAfterDelayIfStillOpen) {
    TimedEntrance entry(50);
    DoorBell adapter(entry);
    Clock clock;

    entry.release();

    EXPECT_THROW(adapter.notifyTimeout(), std::runtime_error);
}

TEST(AlertTriggerTest, NoExceptionIfSecuredBeforeTimeout) {
    TimedEntrance entry(50);
    DoorBell adapter(entry);

    entry.release();
    entry.secure();

    EXPECT_NO_THROW(adapter.notifyTimeout());
}

TEST(MultipleAdapterTest, MultipleAdaptersWorkIndependently) {
    TimedEntrance entryA(100);
    TimedEntrance entryB(200);
    DoorBell bellA(entryA);
    DoorBell bellB(entryB);

    entryA.release();
    entryB.secure();

    EXPECT_THROW(bellA.notifyTimeout(), std::runtime_error);
    EXPECT_NO_THROW(bellB.notifyTimeout());
}

TEST_F(TimedEntranceTest, GetDurationReturnsCorrectValue) {
    TimedEntrance customEntry(300);
    EXPECT_EQ(customEntry.getDuration(), 300);
}

TEST(SequenceTest, OpenEntranceTimerTriggersAlert) {
    TimedEntrance entry(1);
    DoorBell adapter(entry);

    entry.release();
    EXPECT_TRUE(entry.isOpen());
    EXPECT_THROW(adapter.notifyTimeout(), std::runtime_error);
}

TEST_F(TimedEntranceTest, DefaultStateAfterConstruction) {
    TimedEntrance newEntry(200);
    EXPECT_FALSE(newEntry.isOpen());
    EXPECT_EQ(newEntry.getDuration(), 200);
}

TEST_F(DoorBellTest, NotifyAfterMultipleReleaseSecure) {
    timedEntry->release();
    timedEntry->secure();
    timedEntry->release();
    EXPECT_TRUE(timedEntry->isOpen());
    EXPECT_THROW(bellAdapter->notifyTimeout(), std::runtime_error);
}
