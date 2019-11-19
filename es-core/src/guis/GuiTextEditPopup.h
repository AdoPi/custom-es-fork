#include "GuiComponent.h"

#include "components/NinePatchComponent.h"
#include "components/ComponentGrid.h"
#include "components/TextEditComponent.h"
#include "components/TextComponent.h"

class GuiTextEditPopup : public GuiComponent
{
public:
	GuiTextEditPopup(Window&window, const std::string& title, const std::string& initValue,
			             const std::function<void(const std::string&)>& okCallback, bool multiLine,
			             const std::string& acceptBtnText);
  GuiTextEditPopup(Window&window, const std::string& title, const std::string& initValue,
                   const std::function<void(const std::string&)>& okCallback, bool multiLine)
    : GuiTextEditPopup(window, title, initValue, okCallback, multiLine, "OK")
  {
  }

	bool ProcessInput(const InputCompactEvent& event) override;
	void onSizeChanged() override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	NinePatchComponent mBackground;
	ComponentGrid mGrid;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextEditComponent> mText;
	std::shared_ptr<ComponentGrid> mButtonGrid;

	bool mMultiLine;
};
