// Copyright 2025 UNN-IASR
#ifndef INCLUDE_TIMEDDOOR_H_
#define INCLUDE_TIMEDDOOR_H_

class SmartDoor;
class DoorBell;
class Entrance;
class TimedEntrance;

class AlarmListener {
 public:
  virtual void notifyTimeout() = 0;
  virtual ~AlarmListener() = default;
};

class Entrance {
 public:
  virtual void secure() = 0;
  virtual void release() = 0;
  virtual bool isOpen() = 0;
  virtual ~Entrance() = default;
};

class DoorBell : public AlarmListener {
 private:
  TimedEntrance& entry;
 public:
  explicit DoorBell(TimedEntrance&);
  void notifyTimeout() override;
};

class TimedEntrance : public Entrance {
 private:
  DoorBell* bell;
  int duration;
  bool opened;
 public:
  explicit TimedEntrance(int);
  ~TimedEntrance();
  bool isOpen() override;
  void release() override;
  void secure() override;
  int getDuration() const;
  void raiseAlert();
};

class Clock {
  AlarmListener* listener = nullptr;
  void wait(int);
 public:
  void setTimer(int, AlarmListener*);
};

#endif  // INCLUDE_TIMEDDOOR_H_
