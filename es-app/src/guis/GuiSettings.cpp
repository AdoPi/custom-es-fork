#include "guis/GuiSettings.h"
#include "WindowManager.h"
#include "Settings.h"
#include "utils/locale/LocaleHelper.h"

GuiSettings::GuiSettings(WindowManager&window, const std::string& title)
  : Gui(window),
    mMenu(window, title)
{
	addChild(&mMenu);

	mMenu.addButton(_("CLOSE"), _("CLOSE"), [this] { Close(); });

	setSize(Renderer::Instance().DisplayWidthAsFloat(), Renderer::Instance().DisplayHeightAsFloat());
}

GuiSettings::~GuiSettings()
{
	if(doSave) save();
}

void GuiSettings::save()
{
	if(mSaveFuncs.empty())
		return;

	for (auto& saveFunc : mSaveFuncs)
		saveFunc();

	Settings::Instance().saveFile();
}

bool GuiSettings::ProcessInput(const InputCompactEvent& event)
{
	if (event.APressed())
	{
		Close();
		return true;
	}

	if (event.StartPressed())
	{
		// close everything
	  mWindow.CloseAll();
		return true;
	}
	
	return Component::ProcessInput(event);
}

bool GuiSettings::getHelpPrompts(Help& help)
{
	mMenu.getHelpPrompts(help);

	help.Set(HelpType::A, _("BACK"))
	    .Set(HelpType::Start, _("CLOSE"));

	return true;
}
