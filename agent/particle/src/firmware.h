enum SystemState {
  Sleep,
  CheckPowerState,
  MaintainCellularConnection,
  CheckTimeSync,
  LogData,
  SendData,
  CollectPeriodicInformation,
  ServiceWatchdog,
  ServiceLED
};

enum PowerState {
  Unknown,
  Powered,
  Unpowered
};

enum CellularState {
  InitiateParticleConnection,
  ParticleConnecting,
  ParticleConnected,
};

enum WatchdogState {
  WatchdogHigh,
  WatchdogLow
};

enum CollectionState {
  WaitForCollection,
  CollectResults
};

enum SleepState {
  SleepToAwakeCheck,
  PrepareForWake,
  AwakeToSleepCheck,
  PrepareForSleep,
};

enum SendState {
  ReadyToSend,
  SendPaused
};

enum LEDFlashingState {
  Solid,
  Blinking,
  Breathing
};
