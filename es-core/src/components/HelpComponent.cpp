#include <utils/Strings.h>
#include "components/HelpComponent.h"
#include "Renderer.h"
#include "Settings.h"
#include "utils/Log.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "components/ComponentGrid.h"

#define ICON_TEXT_SPACING Math::max(Renderer::getDisplayWidthAsFloat() * 0.004f, 2.0f) // space between [icon] and [text] (px)
#define ENTRY_SPACING Math::max(Renderer::getDisplayWidthAsFloat() * 0.008f, 2.0f) // space between [text] and next [icon] (px)

static const std::map<std::string, Path>& IconPathMap()
{
  static const std::map<std::string, Path> _IconPathMap =
  {
    {"up/down",            Path(":/help/dpad_updown.svg") },
    {"left/right",         Path(":/help/dpad_leftright.svg") },
    {"up/down/left/right", Path(":/help/dpad_all.svg") },
    {"a",                  Path(":/help/button_a.svg") },
    {"b",                  Path(":/help/button_b.svg") },
    {"x",                  Path(":/help/button_x.svg") },
    {"y",                  Path(":/help/button_y.svg") },
    {"l",                  Path(":/help/button_l.svg") },
    {"r",                  Path(":/help/button_r.svg") },
    {"start",              Path(":/help/button_start.svg") },
    {"select",             Path(":/help/button_select.svg") },
  };
  return _IconPathMap;
}

HelpComponent::HelpComponent(Window* window)
  : GuiComponent(window)
{
}

void HelpComponent::clearPrompts()
{
	mPrompts.clear();
	updateGrid();
}

void HelpComponent::setPrompts(const std::vector<HelpPrompt>& prompts)
{
	mPrompts = prompts;
	updateGrid();
}

void HelpComponent::setStyle(const HelpStyle& style)
{
	mStyle = style;
	updateGrid();
}

void HelpComponent::updateGrid()
{
	if(!Settings::getInstance()->getBool("ShowHelpPrompts") || mPrompts.empty())
	{
		mGrid.reset();
		return;
	}

	std::shared_ptr<Font>& font = mStyle.font;

	mGrid = std::make_shared<ComponentGrid>(mWindow, Vector2i((int)mPrompts.size() * 4, 1));
	// [icon] [spacer1] [text] [spacer2]
	
	std::vector< std::shared_ptr<ImageComponent> > icons;
	std::vector< std::shared_ptr<TextComponent> > labels;

	float width = 0;
	const float height = Math::round(font->getLetterHeight() * 1.25f);
	for (auto& mPrompt : mPrompts)
	{
		auto icon = std::make_shared<ImageComponent>(mWindow);
		
		if (mStyle.iconMap.find(mPrompt.first) != mStyle.iconMap.end())
			icon->setImage(mStyle.iconMap[mPrompt.first]);
		else
			icon->setImage(getIconTexture(mPrompt.first.c_str()));
					
		icon->setColorShift(mStyle.iconColor);
		icon->setResize(0, height);
		icons.push_back(icon);

		auto lbl = std::make_shared<TextComponent>(mWindow, Strings::ToUpperUTF8(mPrompt.second), font, mStyle.textColor);
		labels.push_back(lbl);

		width += icon->getSize().x() + lbl->getSize().x() + ICON_TEXT_SPACING + ENTRY_SPACING;
	}

	mGrid->setSize(width, height);
	for (int i = 0; i < (int)icons.size(); i++)
	{
		const int col = i*4;
		mGrid->setColWidthPerc(col, icons.at(i)->getSize().x() / width);
		mGrid->setColWidthPerc(col + 1, ICON_TEXT_SPACING / width);
		mGrid->setColWidthPerc(col + 2, labels.at(i)->getSize().x() / width);

		mGrid->setEntry(icons.at(i), Vector2i(col, 0), false, false);
		mGrid->setEntry(labels.at(i), Vector2i(col + 2, 0), false, false);
	}

	mGrid->setPosition(Vector3f(mStyle.position.x(), mStyle.position.y(), 0.0f));
	//mGrid->setPosition(OFFSET_X, Renderer::getScreenHeight() - mGrid->getSize().y() - OFFSET_Y);
}

std::shared_ptr<TextureResource> HelpComponent::getIconTexture(const char* name)
{
	auto it = mIconCache.find(name);
	if(it != mIconCache.end())
		return it->second;
	
	auto pathLookup = IconPathMap().find(name);
	if(pathLookup == IconPathMap().end())
	{
		LOG(LogError) << "Unknown help icon \"" << name << "\"!";
		return nullptr;
	}
	if(!ResourceManager::getInstance()->fileExists(pathLookup->second))
	{
		LOG(LogError) << "Help icon \"" << name << "\" - corresponding image file \"" << pathLookup->second.ToString() << "\" missing!";
		return nullptr;
	}

	std::shared_ptr<TextureResource> tex = TextureResource::get(pathLookup->second);
	mIconCache[std::string(name)] = tex;
	return tex;
}

void HelpComponent::setOpacity(unsigned char opacity)
{
	GuiComponent::setOpacity(opacity);

	for (unsigned int i = 0; i < mGrid->getChildCount(); i++)
	{
		mGrid->getChild(i)->setOpacity(opacity);
	}
}

void HelpComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	
	if(mGrid)
		mGrid->render(trans);
}
