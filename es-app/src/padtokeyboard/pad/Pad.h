//
// Created by bkg2k on 21/12/2019.
//
#pragma once

#include <utils/storage/Queue.h>
#include <utils/storage/HashMap.h>
#include "PadConfiguration.h"
#include "PadItems.h"

class Pad
{
  public:
    //! Pad event
    struct Event
    {
      PadItems Item; //!< Target item
      char     Pad;  //!< Pad number
      bool     On;   //!< Item set on/off (true/false)
    } __attribute__((packed));

  private:
    //! Joystick deadzone, in the 0-32767 range
    static constexpr int sJoystickDeadZone = 23000;

    //! Event queue
    Queue<Event> mEventQueue;

    //! Global configuration reference
    const Configuration* mConfiguration;
    //! Pad configurations references
    const PadConfiguration& mPadConfiguration;
    //! SDL Index to Recalbox Index
    HashMap<SDL_JoystickID, int> mSdlToRecalboxIndexex;
    //! SDL Index to Recalbox Index
    int mItemOnOff[Input::sMaxInputDevices];
    //! Devices readiness
    bool mReady;

  public:
    /*!
     * @brief Constructor
     * @param padConfiguration Pad Configuration
     * @param orderedDevices ordered devices
     */
    explicit Pad(const PadConfiguration& padConfiguration);

    /*!
     * @brief Get next pad event
     * @param event Output event
     * @return True if an event have been read. False if the devices have been released
     */
    bool GetEvent(Pad::Event& event);

    /*!
     * @brief Open all configured devices from the given ordered device list
     */
    void Open(const OrderedDevices& orderedDevices);

    /*!
     * @brief Check device readiness
     * @return True if configured devices have been opened successfully
     */
    bool Ready() const { return mReady; }

    /*!
     * @brief Release all devices
     */
    void Release();
};
