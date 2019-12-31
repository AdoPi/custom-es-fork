//
// Created by bkg2k on 19/11/2019.
//
#pragma once

#include <components/base/Component.h>

class Gui : public Component
{
  private:
    //! When set, let the main processing delete safely this GUI
    bool mGuiClosePending;

  public:
    Gui(Window& window)
      : Component(window),
        mGuiClosePending(false)
    {
    }

    /*!
     * @brief Close the GUI
     */
    void Close() { mGuiClosePending = true; }

    /*!
     * @brief Get deletion status
     * @return True if the current GUI is pending for deletion
     */
    bool IsPendingForDeletion() const { return mGuiClosePending; }

    /*!
     * @brief Check if the current UI is on top of the screen
     * @return True if the current UI is the first visible
     */
    bool AmIOnTopOfScreen() const;

    /*
     * Partial IComponent implementation
     */
};