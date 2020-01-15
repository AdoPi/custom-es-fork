#include "SwitchComponent.h"
#include "Renderer.h"
#include "resources/Font.h"
#include "Window.h"
#include "LocaleHelper.h"
#include "MenuThemeData.h"

SwitchComponent::SwitchComponent(Window&window)
  : Component(window),
    mImage(window),
    mState(false),
    mInitialState(false)
{
	auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
	mImage.setImage(mState ? menuTheme->iconSet.on : menuTheme->iconSet.off);
	mImage.setResize(0, Font::get(FONT_SIZE_MEDIUM)->getLetterHeight());
	mImage.setColorShift(menuTheme->menuText.color);
	mOriginColor = menuTheme->menuText.color;
	mSize = mImage.getSize();
}

void SwitchComponent::onSizeChanged()
{
	mImage.setSize(mSize);
}

void SwitchComponent::setColor(unsigned int color) {
	mImage.setColorShift(color);
}

bool SwitchComponent::ProcessInput(const InputCompactEvent& event)
{
	if(event.BPressed())
	{
		mState = !mState;
		onStateChanged();
		return true;
	}

	return false;
}

void SwitchComponent::Render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();

  mImage.Render(trans);

	renderChildren(trans);
}

void SwitchComponent::setState(bool state)
{
	mState = state;
	mInitialState = mState;
	onStateChanged();
}

void SwitchComponent::onStateChanged()
{
	mImage.setImage(mState ? Path(":/on.svg") : Path(":/off.svg"));
}

bool SwitchComponent::getHelpPrompts(Help& help)
{
	help.Set(HelpType::B, _("CHANGE"));
	return true;
}

std::string SwitchComponent::getValue() const {
  return mState ? "true" : "false";
}

bool SwitchComponent::changed() {
	return mInitialState != mState;
}
