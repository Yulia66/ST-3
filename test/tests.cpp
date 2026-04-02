// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <stdexcept>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Mock;
using ::testing::Invoke;

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    door = new TimedDoor(100);
  }

  void TearDown() override {
    delete door;
  }

  TimedDoor* door;
};

class DoorTimerAdapterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    timedDoor = new TimedDoor(100);
    adapter = new DoorTimerAdapter(*timedDoor);
  }

  void TearDown() override {
    delete adapter;
    delete timedDoor;
  }

  TimedDoor* timedDoor;
  DoorTimerAdapter* adapter;
};

class TimerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mockClient = new MockTimerClient();
    timer = new Timer();
  }

  void TearDown() override {
    delete timer;
    delete mockClient;
  }

  MockTimerClient* mockClient;
  Timer* timer;
};

TEST_F(TimedDoorTest, ConstructorSetsTimeoutCorrectly) {
  EXPECT_EQ(door->getTimeOut(), 100);
}

TEST_F(TimedDoorTest, InitiallyDoorIsClosed) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, ThrowStateThrowsException) {
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, MultipleUnlockAndLockOperations) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(DoorTimerAdapterTest, TimeoutThrowsExceptionWhenDoorIsOpened) {
  timedDoor->unlock();
  EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(DoorTimerAdapterTest, TimeoutDoesNotThrowExceptionWhenDoorIsClosed) {
  timedDoor->lock();
  EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(DoorTimerAdapterTest, TimeoutWithClosedDoorDoesNothing) {
  timedDoor->lock();
  adapter->Timeout();
  SUCCEED();
}

TEST_F(DoorTimerAdapterTest, TimeoutAfterUnlockThenLock) {
  timedDoor->unlock();
  timedDoor->lock();
  EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimerTest, TregisterCallsTimeoutOnClient) {
  EXPECT_CALL(*mockClient, Timeout()).Times(1);

  timer->tregister(1, mockClient);
}

TEST_F(TimerTest, TregisterWithNullClientDoesNotCrash) {
  EXPECT_NO_THROW(timer->tregister(1, nullptr));
}

TEST_F(TimerTest, TregisterWithLongTimeoutStillCallsTimeout) {
  EXPECT_CALL(*mockClient, Timeout()).Times(1);
  timer->tregister(10, mockClient);
}

TEST(IntegrationTest, FullDoorOperationScenario) {
  TimedDoor door(50);

  EXPECT_FALSE(door.isDoorOpened());

  door.unlock();
  EXPECT_TRUE(door.isDoorOpened());

  door.lock();
  EXPECT_FALSE(door.isDoorOpened());

  door.unlock();
  EXPECT_TRUE(door.isDoorOpened());
}

TEST(TimeoutExceptionTest, ExceptionThrownAfterTimeoutIfDoorStillOpen) {
  TimedDoor door(50);
  DoorTimerAdapter adapter(door);
  Timer timer;

  door.unlock();

  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(TimeoutExceptionTest, NoExceptionIfDoorClosedBeforeTimeout) {
  TimedDoor door(50);
  DoorTimerAdapter adapter(door);

  door.unlock();
  door.lock();

  EXPECT_NO_THROW(adapter.Timeout());
}

TEST(MultiAdapterTest, MultipleAdaptersWorkIndependently) {
  TimedDoor door1(100);
  TimedDoor door2(200);
  DoorTimerAdapter adapter1(door1);
  DoorTimerAdapter adapter2(door2);

  door1.unlock();
  door2.lock();

  EXPECT_THROW(adapter1.Timeout(), std::runtime_error);
  EXPECT_NO_THROW(adapter2.Timeout());
}

TEST_F(TimedDoorTest, GetTimeOutReturnsCorrectValue) {
  TimedDoor customDoor(300);
  EXPECT_EQ(customDoor.getTimeOut(), 300);
}

TEST(ChainTest, OpenDoorTimerThrowsException) {
  TimedDoor door(1);
  DoorTimerAdapter adapter(door);

  door.unlock();
  EXPECT_TRUE(door.isDoorOpened());
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}
