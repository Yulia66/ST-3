// Copyright 2025 UNN-IASR
#include "TimedDoor.h"
#include <stdexcept>
#include <thread>
#include <chrono>

DoorBell::DoorBell(TimedEntrance& e) : entry(e) {}

void DoorBell::notifyTimeout() {
    if (entry.isOpen()) {
        entry.raiseAlert();
    }
}

TimedEntrance::TimedEntrance(int timeout) : duration(timeout), opened(false) {
    bell = new DoorBell(*this);
}

TimedEntrance::~TimedEntrance() {
    delete bell;
}

bool TimedEntrance::isOpen() {
    return opened;
}

void TimedEntrance::release() {
    opened = true;
}

void TimedEntrance::secure() {
    opened = false;
}

int TimedEntrance::getDuration() const {
    return duration;
}

void TimedEntrance::raiseAlert() {
    throw std::runtime_error("Entrance remains unsecured after timeout!");
}

void Clock::wait(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Clock::setTimer(int timeout, AlarmListener* client) {
    this->listener = client;
    wait(timeout);
    if (listener != nullptr) {
        listener->notifyTimeout();
    }
}
