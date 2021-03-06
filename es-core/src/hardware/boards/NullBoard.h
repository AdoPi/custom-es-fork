//
// Created by bkg2k on 02/11/2020.
//

#include <hardware/IBoardInterface.h>

class NullBoard: public IBoardInterface
{
  public:
    explicit NullBoard(HardwareMessageSender& messageSender)
      : IBoardInterface(messageSender)
    {
    }

  private:
    void StartGlobalBackgroundProcesses() final {}

    void StopGlobalBackgroundProcesses() final {}

    void StartInGameBackgroundProcesses() final {}

    void StopInGameBackgroundProcesses() final {}

    bool HasBattery() final { return false; }

    bool HasCPUGovernance() final { return false; }

    bool HasPhysicalVolumeButtons() final { return false; }

    bool HasPhysicalBrightnessButtons() final { return false; }

    bool HasBrightnessSupport() final { return false; }

    bool HasSuspendResume() final { return false; }

    void SetCPUGovernance(CPUGovernance governance) final { (void)governance; }

    void SetLowestBrightness() final {}

    void SetBrightness(int brighness) final { (void)brighness; }

    int BatteryChargePercent() final { return -1; }

    bool IsBatteryCharging() final { return false; }

    bool ProcessSpecialInputs(InputCompactEvent& inputEvent) final { (void)inputEvent; return false; }

    void Suspend() final {}
};

