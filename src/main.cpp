// Copyright 2025 UNN-IASR
#include "TimedDoor.h"

int main() {
    TimedEntrance entry(5);
    entry.secure();
    entry.release();
    return 0;
}
