//
// Created by bkg2k on 02/11/2020.
//
#pragma once

#include <input/InputCompactEvent.h>
#include <hardware/messaging/HardwareMessageSender.h>

class IBoardInterface
{
  public:
    //! CPU Speed governance
    enum class CPUGovernance
    {
        PowerSave, //! Save has much power as possible
        OnDemand,  //! Adjust frequencies on demand
        FullSpeed, //! Real full spead or "on demand"
    };

    /*!
     * @brief Constructor
     * @param notified Hardware event Notification interface
     */
    explicit IBoardInterface(HardwareMessageSender& messageSender)
      : mSender(messageSender)
    {
    }

    //! Default virtual constructor
    virtual ~IBoardInterface() = default;

    /*!
     * @brief Start optional global background processes
     * This method is called when ES starts
     */
    virtual void StartGlobalBackgroundProcesses() = 0;

    /*!
     * @brief Stop optional global background processes
     * This method is called when ES stops
     */
    virtual void StopGlobalBackgroundProcesses() = 0;

    /*!
     * @brief Start optional in-game background processes.
     * This method is called when a game starts
     */
    virtual void StartInGameBackgroundProcesses() = 0;

    /*!
     * @brief Stop optional in-game background processes.
     * This method is called when a game stops
     */
    virtual void StopInGameBackgroundProcesses() = 0;

    /*!
     * @brief Has Battery?
     */
    virtual bool HasBattery() = 0;

    /*!
     * @brief Has CPU governance? (and is it worth the use)
     */
    virtual bool HasCPUGovernance() = 0;

    /*!
     * @brief Has physical volume buttons?
     */
    virtual bool HasPhysicalVolumeButtons() = 0;

    /*!
     * @brief Has physical brightness buttons?
     */
    virtual bool HasPhysicalBrightnessButtons() = 0;

    /*!
     * @brief Has hardware suspend/resume?
     */
    virtual bool HasSuspendResume() = 0;

    /*!
     * @brief Has physical brightness buttons?
     */
    virtual bool HasBrightnessSupport() = 0;

    /*!
     * @brief Set lowest brightess possible, including black screen
     */
    virtual void SetLowestBrightness() = 0;

    /*!
     * @brief Set brightness
     * @param brighness brightness step from 0 to 8 included
     */
    virtual void SetBrightness(int brighness) = 0;

    /*!
     * @brief Set new CPU governance
     * @param governance cpu governance
     */
    virtual void SetCPUGovernance(CPUGovernance governance) = 0;

    /*!
     * @brief Get battery charge in percent
     * @return Battery charge (-1 = no battery)
     */
    virtual int BatteryChargePercent() = 0;

    /*!
     * @brief Check if the battery is charging
     * @return True = charging, False = discharging or no battery
     */
    virtual bool IsBatteryCharging() = 0;

    /*!
     * @brief Process special input if any
     * @param inputEvent Input to process
     * @return True if the input has been processed, false otherwise
     */
    virtual bool ProcessSpecialInputs(InputCompactEvent& inputEvent) = 0;

    /*!
     * @brief Suspend!
     */
    virtual void Suspend() = 0;

  protected:
    //! Hardware event Notification interface
    HardwareMessageSender& mSender;
};