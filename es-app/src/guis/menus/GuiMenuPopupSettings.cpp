//
// Created by bkg2k on 31/10/2020.
//

#include <utils/locale/LocaleHelper.h>
#include "GuiMenuPopupSettings.h"
#include "guis/MenuMessages.h"

GuiMenuPopupSettings::GuiMenuPopupSettings(WindowManager& window)
  : GuiMenuBase(window, _("POPUP SETTINGS"))
{
  auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();

  //help popup time 0=no popup
  mHelp = std::make_shared<SliderComponent>(mWindow, 0.f, 10.f, 1.f, "s");
  mHelp->setSlider((float) RecalboxConf::Instance().GetPopupHelp());
  mHelp->setSelectedChangedCallback(SetHelpPopupTime);
  mMenu.addWithLabel(mHelp, _("HELP POPUP DURATION"), _(MENUMESSAGE_UI_HELP_POPUP_DURATION_HELP_MSG));

  //music popup time 0=no popup
  mMusic = std::make_shared<SliderComponent>(mWindow, 0.f, 10.f, 1.f, "s");
  mMusic->setSlider((float) RecalboxConf::Instance().GetPopupMusic());
  mMusic->setSelectedChangedCallback(SetMusicPopupTime);
  mMenu.addWithLabel(mMusic, _("MUSIC POPUP DURATION"), _(MENUMESSAGE_UI_MUSIC_POPUP_DURATION_HELP_MSG));

  //netplay popup time 0=no popup
  mNetplay = std::make_shared<SliderComponent>(mWindow, 0.f, 10.f, 1.f, "s");
  mNetplay->setSlider((float) RecalboxConf::Instance().GetPopupNetplay());
  mNetplay->setSelectedChangedCallback(SetNetplayPopupTime);
  mMenu.addWithLabel(mNetplay, _("NETPLAY POPUP DURATION"), _(MENUMESSAGE_UI_NETPLAY_POPUP_DURATION_HELP_MSG));
}

void GuiMenuPopupSettings::SetHelpPopupTime(float time)
{
  if (time != RecalboxConf::Instance().GetPopupHelp())
  {
    RecalboxConf::Instance().SetPopupHelp((int) time);
    RecalboxConf::Instance().Save();
  }
}

void GuiMenuPopupSettings::SetMusicPopupTime(float time)
{
  if (time != RecalboxConf::Instance().GetPopupMusic())
  {
    RecalboxConf::Instance().SetPopupMusic((int) time);
    RecalboxConf::Instance().Save();
  }
}

void GuiMenuPopupSettings::SetNetplayPopupTime(float time)
{
  if (time != RecalboxConf::Instance().GetPopupNetplay())
  {
    RecalboxConf::Instance().SetPopupNetplay((int) time);
    RecalboxConf::Instance().Save();
  }
}

