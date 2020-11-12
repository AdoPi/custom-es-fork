//
// Created by bkg2k on 11/11/2020.
//
#pragma once

#include <utils/locale/LocaleHelper.h>
#include "Wizard.h"

class WizardAGO2: public Wizard
{
  public:
    enum class Pages
    {
      Intro,
      SelectButton,
      StartButton,
      VolumeButtons,
      BrightnessButtons,
      Final,
      Count,
    };

    explicit WizardAGO2(Window& window)
      : Wizard(window, _("WELCOME TO RECALBOX!"), int(Pages::Count))
    {
    }

  private:
    static constexpr const char* sImagePath = "/recalbox/system/resources/wizard";
    static constexpr const char* sIntroImageFile = "intro.png";
    static constexpr const char* sSelectImageFile = "select.png";
    static constexpr const char* sStartImageFile = "start.png";
    static constexpr const char* sVolumeImageFile = "volume.png";
    static constexpr const char* sBrightnessImageFile = "brightness.png";
    static constexpr const char* sFinalImageFile = "final.png";

    //! Called when a page image is required
    Path OnImageRequired(int page) override;

    //! Called when a page text is required
    std::string OnTextRequired(int page) override;

    //! Called when rebuilding the help bar
    void OnHelpRequired(int page, Help& help) override;

    //! Called when z key event is received
    Move OnKeyReceived(int page, const InputCompactEvent& event) override;

    /*!
     * @brief Change current language
     * @param increment True to go forward in the language list, false to go backward
     */
    void ChangeLanguage(bool increment);
};

