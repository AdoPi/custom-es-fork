//
// Created by bkg2k on 21/12/2019.
//

#include "Pad.h"
#include <input/InputDevice.h>
#include <SDL2/SDL.h>

Pad::Pad(const PadConfiguration& padConfiguration, const Configuration& configuration)
  : mConfiguration(&configuration),
    mPadConfiguration(padConfiguration),
    mSdlToRecalboxIndexex {},
    mItemOnOff {},
    mReady(false)
{
  Open();
}

Pad::Pad(const PadConfiguration& padConfiguration)
  : mConfiguration(nullptr),
    mPadConfiguration(padConfiguration),
    mSdlToRecalboxIndexex {},
    mItemOnOff {},
    mReady(false)
{
}

void Pad::Open(const OrderedDevices& orderedDevices)
{
  // Allow joystick event
  SDL_Init(SDL_INIT_JOYSTICK);
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
  SDL_JoystickEventState(SDL_ENABLE);
  SDL_JoystickUpdate();

  // Bitflag for assigned pads
  int Assigned = 0;
  // Reset
  for(int i = Input::sMaxInputDevices; --i >= 0; )
  {
    mSdlToRecalboxIndexex[i] = -1;
    mItemOnOff[i] = 0;
  }

  // Compute SDL to Recalbox indexes
  int count = SDL_NumJoysticks();
  for(int j = 0; j < count; ++j)
  {
    // Get joystick
    SDL_Joystick* joystick = SDL_JoystickOpen(j);
    // Get global index
    SDL_JoystickID joystickIndex = SDL_JoystickGetDeviceInstanceID(j);
    // Get informations
    const char* name = SDL_JoystickNameForIndex(j);
    char guid[64];
    memset(guid, 0, sizeof(guid));
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), guid, sizeof(guid));

    // SDL index too high?
    if (joystickIndex >= Input::sMaxInputDevices) continue;

    // Lookup
    for(int i = 0; i < orderedDevices.Count(); ++i)
    {
      const InputDevice& device = *(orderedDevices.Device(i));
      if (device.Identifier() == joystickIndex) // Joystick index match?
        if ((Assigned & (1 << i)) == 0)         // Not yet assigned?
        {
          Assigned |= (1 << i);
          mSdlToRecalboxIndexex[joystickIndex] = i;
          LOG(LogInfo) << "[Pad2Keyboard] " << name << " (" << guid << ") assigned as Pad #" << i;
          break;
        }
    }
  }

  mReady = (Assigned != 0);
  if (!mReady)
    LOG(LogWarning) << "[Pad2Keyboard] No Joystick assigned!";
}


void Pad::Open()
{
  // Allow joystick event
  SDL_Init(SDL_INIT_JOYSTICK);
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
  SDL_JoystickEventState(SDL_ENABLE);
  SDL_JoystickUpdate();

  // Bitflag for assigned pads
  int Assigned = 0;
  // Reset
  for(int i = Input::sMaxInputDevices; --i >= 0; )
  {
    mSdlToRecalboxIndexex[i] = -1;
    mItemOnOff[i] = 0;
  }

  // Compute SDL to Recalbox indexes
  int count = SDL_NumJoysticks();
  for(int j = 0; j < count; ++j)
  {
    // Get joystick
    SDL_Joystick* joystick = SDL_JoystickOpen(j);
    // Get global index
    SDL_JoystickID joystickIndex = SDL_JoystickGetDeviceInstanceID(j);
    // Get informations
    const char* name = SDL_JoystickNameForIndex(j);
    LOG(LogInfo) << name;
    char guid[64];
    SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), guid, sizeof(guid));
    LOG(LogInfo) << guid;

    // SDL index too high?
    if (joystickIndex >= Input::sMaxInputDevices) continue;

    // Lookup
    for(int i = 0; i < Input::sMaxInputDevices; ++i)
      if (mConfiguration->Valid(i))                          // Configuration valid?
        if (strcmp(name, mConfiguration->PadName[i]) == 0)   // Name matching?
          if (strcmp(guid, mConfiguration->PadGUID[i]) == 0) // Guid matching?
            if ((Assigned & (1 << i)) == 0)                 // Not yet assigned?
            {
              mSdlToRecalboxIndexex[joystickIndex] = i;
              break;
            }
  }

  mReady = true;
}

void Pad::Release()
{
  SDL_JoystickEventState(SDL_DISABLE);

  mReady = false;

  SDL_Event event;
  event.type = SDL_QUIT;
  SDL_PushEvent(&event);

  SDL_Quit();
}

bool Pad::GetEvent(Pad::Event& event)
{
  // Pop old events first
  if (!mEventQueue.Empty()) { event = mEventQueue.Pop(); return true; }

  SDL_Event sdl;
  for(;;)
  {
    bool ok = (SDL_WaitEvent(&sdl) == 1);
    if (ok)
    {
      switch (sdl.type)
      {
        case SDL_JOYAXISMOTION:
        {
          // Get pad mapping
          if ((unsigned int) sdl.jaxis.which >= Input::sMaxInputDevices) break;
          int index = mSdlToRecalboxIndexex[sdl.jaxis.which]; // Get Recalbox index
          if (index < 0) break;
          const PadConfiguration::PadAllItemConfiguration& pad = mPadConfiguration.Pad(index);

          // Get actual direction
          int dir = sdl.jaxis.value;
          dir = dir < -sJoystickDeadZone ? -1 : dir > sJoystickDeadZone ? 1 : 0;

          // Check mapping
          for (const PadConfiguration::PadItemConfiguration& item : pad.Items)
            if (item.Type == InputEvent::EventType::Axis)
              if (item.Id == sdl.jaxis.axis)
              {
                // Axis On?
                if ((dir == item.Value) && (mItemOnOff[index] & (1 << (int) item.Item)) == 0)
                {
                  mEventQueue.Push({ item.Item, (char) index, true });
                  mItemOnOff[index] |= (1 << (int) item.Item);
                }
                  // Axis off?
                else if ((dir == 0) && (mItemOnOff[index] & (1 << (int) item.Item)) != 0)
                {
                  mEventQueue.Push({ item.Item, (char) index, false });
                  mItemOnOff[index] &= ~(1 << (int) item.Item);
                }
              }
          break;
        }
        case SDL_JOYHATMOTION:
        {
          // Get pad mapping
          if ((unsigned int) sdl.jhat.which >= Input::sMaxInputDevices) break;
          int index = mSdlToRecalboxIndexex[sdl.jhat.which]; // Get Recalbox index
          if (index < 0) break;
          const PadConfiguration::PadAllItemConfiguration& pad = mPadConfiguration.Pad(index);

          // Check mapping
          for (const PadConfiguration::PadItemConfiguration& item : pad.Items)
            if (item.Type == InputEvent::EventType::Hat)
              if (item.Id == sdl.jhat.hat)
              {
                // Hat bit(s) On?
                if (((sdl.jhat.value & item.Value) == item.Value) && (mItemOnOff[index] & (1 << (int) item.Item)) == 0)
                {
                  mEventQueue.Push({ item.Item, (char) index, true });
                  mItemOnOff[index] |= (1 << (int) item.Item);
                }
                  // Hat bit(s) off?
                else if (((sdl.jhat.value & item.Value) != item.Value) &&
                         (mItemOnOff[index] & (1 << (int) item.Item)) != 0)
                {
                  mEventQueue.Push({ item.Item, (char) index, false });
                  mItemOnOff[index] &= ~(1 << (int) item.Item);
                }
              }

          break;
        }
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYBUTTONUP:
        {
          // Get pad mapping
          if ((unsigned int) sdl.jbutton.which >= Input::sMaxInputDevices) break;
          int index = mSdlToRecalboxIndexex[sdl.jbutton.which]; // Get Recalbox index
          if (index < 0) break;
          const PadConfiguration::PadAllItemConfiguration& pad = mPadConfiguration.Pad(index);

          for (const PadConfiguration::PadItemConfiguration& item : pad.Items)
            if (item.Type == InputEvent::EventType::Button)
              if (item.Id == sdl.jbutton.button)
              {
                // Button On?
                if ((sdl.jbutton.state == SDL_PRESSED) && (mItemOnOff[index] & (1 << (int) item.Item)) == 0)
                {
                  mEventQueue.Push({ item.Item, (char) index, true });
                  mItemOnOff[index] |= (1 << (int) item.Item);
                }
                  // Button off?
                else if ((sdl.jbutton.state == SDL_RELEASED) && (mItemOnOff[index] & (1 << (int) item.Item)) != 0)
                {
                  mEventQueue.Push({ item.Item, (char) index, false });
                  mItemOnOff[index] &= ~(1 << (int) item.Item);
                }
              }
          break;
        }
        case SDL_QUIT: return false;
        default: break;
      }
    }

    // Pop first of new events
    if (!mEventQueue.Empty())
    {
      event = mEventQueue.Pop();
      return true;
    }
  }
}

