#include <Settings.h>
#include <recalbox/RecalboxSystem.h>
#include "components/MenuComponent.h"
#include "components/ButtonComponent.h"
#include "utils/locale/LocaleHelper.h"
#include "MenuThemeData.h"

#define BUTTON_GRID_VERT_PADDING Math::max(Renderer::getDisplayHeightAsFloat() * 0.008f, 2.0f)
#define BUTTON_GRID_HORIZ_PADDING Math::max(Renderer::getDisplayWidthAsFloat() * 0.01f, 3.0f)

#define TITLE_HEIGHT (mTitle->getFont()->getLetterHeight() + TITLE_VERT_PADDING)

MenuComponent::MenuComponent(Window&window, const char* title, const std::shared_ptr<Font>& titleFont)
  : Component(window),
    mBackground(window),
    mGrid(window, Vector2i(1, 3))
{
  (void)titleFont;

	addChild(&mBackground);
	addChild(&mGrid);

	auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();

	mBackground.setImagePath(menuTheme->menuBackground.path);
	mBackground.setCenterColor(menuTheme->menuBackground.color);
	mBackground.setEdgeColor(menuTheme->menuBackground.color);

	// set up title
	mTitle = std::make_shared<TextComponent>(mWindow);
	mTitle->setHorizontalAlignment(TextAlignment::Center);

	setTitle(title, menuTheme->menuTitle.font);
	mTitle->setColor(menuTheme->menuTitle.color);

	mGrid.setEntry(mTitle, Vector2i(0, 0), false);



	if (title == _("MAIN MENU") ) {

		auto headerGrid = std::make_shared<ComponentGrid>(mWindow, Vector2i(3, 1));

		std::string arch = Settings::Instance().Arch();
		if (arch == "x86" || arch == "x86_64") {

			auto batt = RecalboxSystem::getSysBatteryInfo(); // TODO: Remove ascending core=>app
			if (batt.second != -1) {
				auto batDisplay = std::make_shared<TextComponent>(mWindow);
				batDisplay->setFont(menuTheme->menuText.font);
				if (batt.second <= 15)
					batDisplay->setColor(0xFF0000FF);
				else
					batDisplay->setColor(menuTheme->menuText.color);
				batDisplay->setText(" " + batt.first + " " + std::to_string(batt.second) + "%");
				batDisplay->setHorizontalAlignment(TextAlignment::Left);
				headerGrid->setEntry(batDisplay, Vector2i(0, 0), false);

			}
		}

		if (Settings::Instance().ShowClock()) {

			mDateTime = std::make_shared<DateTimeComponent>(mWindow);
			mDateTime->setDisplayMode(DateTimeComponent::Display::Time);
			mDateTime->setHorizontalAlignment(TextAlignment::Right);
			mDateTime->setFont(menuTheme->menuText.font);
			mDateTime->setColor(menuTheme->menuText.color);
			headerGrid->setEntry(mDateTime, Vector2i(2, 0), false);
		}

		mGrid.setEntry(headerGrid, Vector2i(0, 0), false);

	}


    // set up list which will never change (externally, anyway)
    mList = std::make_shared<ComponentList>(mWindow);
    mGrid.setEntry(mList, Vector2i(0, 1), true);

    /*mGrid.setUnhandledInputCallback([this](const InputCompactEvent& event) -> bool {
        if (event.DownPressed()) {
            mGrid.setCursorTo(mList);
            mList->setCursorIndex(0);
            return true;
        }
        if (event.UpPressed()) {
        	mList->setCursorIndex(mList->size() - 1);
            if(!mButtons.empty()) {
				mGrid.moveCursor(Vector2i(0, 1));
            } else {
                mGrid.setCursorTo(mList);
            }
            return true;
        }
        return false;
    });*/

    updateGrid();
    updateSize();

    mGrid.resetCursor();
}

bool MenuComponent::ProcessInput(const InputCompactEvent& event)
{
  if (Component::ProcessInput(event)) return true;

  if (event.DownPressed())
  {
    mGrid.setCursorTo(mList);
    mList->setCursorIndex(0);
    return true;
  }
  else if (event.UpPressed())
  {
    mList->setCursorIndex(mList->size() - 1);
    if(!mButtons.empty()) mGrid.moveCursor(Vector2i(0, 1));
    else                  mGrid.setCursorTo(mList);
    return true;
  }
  return false;
}


void MenuComponent::setTitle(const char* title, const std::shared_ptr<Font>& font)
{
    mTitle->setText(Strings::ToUpperASCII(title));
    mTitle->setFont(font);
}

float MenuComponent::getButtonGridHeight() const
{
    auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
    return (mButtonGrid ? mButtonGrid->getSize().y() : menuTheme->menuText.font->getHeight() + BUTTON_GRID_VERT_PADDING);
}

void MenuComponent::updateSize()
{
    const float maxHeight = Renderer::getDisplayHeightAsFloat() * 0.8f;
    float height = TITLE_HEIGHT + mList->getTotalRowHeight() + getButtonGridHeight() + 2;
    if(height > maxHeight)
    {
        height = TITLE_HEIGHT + getButtonGridHeight() + 2;
        int i = 0;
        while(i < mList->size())
        {
            float rowHeight = mList->getRowHeight(i);
            if(height + rowHeight < maxHeight)
                height += rowHeight;
            else
                break;
            i++;
        }
    }

    float width = Math::min(Renderer::getDisplayHeightAsFloat(), Renderer::getDisplayWidthAsFloat() * 0.90f);
    setSize(width, height);
}

void MenuComponent::onSizeChanged()
{
    mBackground.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

    // update grid row/col sizes
    mGrid.setRowHeightPerc(0, TITLE_HEIGHT / mSize.y());
    mGrid.setRowHeightPerc(2, getButtonGridHeight() / mSize.y());
    
    mGrid.setSize(mSize);
}

void MenuComponent::addButton(const std::string& name, const std::string& helpText, const std::function<void()>& callback)
{
    mButtons.push_back(std::make_shared<ButtonComponent>(mWindow, Strings::ToUpperASCII(name), helpText, callback));
    updateGrid();
    updateSize();
}

void MenuComponent::updateGrid()
{
    if(mButtonGrid)
        mGrid.removeEntry(mButtonGrid);

    mButtonGrid.reset();

    if(!mButtons.empty())
    {
        mButtonGrid = makeButtonGrid(mWindow, mButtons);
        mGrid.setEntry(mButtonGrid, Vector2i(0, 2), true, false);
    }
}

bool MenuComponent::getHelpPrompts(Help& help)
{
    return mGrid.getHelpPrompts(help);
}

std::shared_ptr<ComponentGrid> makeButtonGrid(Window&window, const std::vector< std::shared_ptr<ButtonComponent> >& buttons)
{
    std::shared_ptr<ComponentGrid> grid = std::make_shared<ComponentGrid>(window, Vector2i(buttons.size(), 2));

    float gridWidth = (float) BUTTON_GRID_HORIZ_PADDING * buttons.size(); // initialize to padding
    for (int i = 0; i < (int)buttons.size(); i++)
    {
        grid->setEntry(buttons.at(i), Vector2i(i, 0), true, false);
        gridWidth += buttons.at(i)->getSize().x();
    }
    for (int i = 0; i < (int)buttons.size(); i++)
    {
        grid->setColWidthPerc(i, (buttons.at(i)->getSize().x() + BUTTON_GRID_HORIZ_PADDING) / gridWidth);
    }
    
    grid->setSize(gridWidth, buttons.at(0)->getSize().y() + BUTTON_GRID_VERT_PADDING + 2);
    grid->setRowHeightPerc(1, 2 / grid->getSize().y()); // spacer row to deal with dropshadow to make buttons look centered

    return grid;
}

/**
 * Limitation: same number of button per line, same dimension per cell
 */

std::shared_ptr<ComponentGrid> makeMultiDimButtonGrid(Window&window, const std::vector< std::vector< std::shared_ptr<ButtonComponent> > >& buttons, const float outerWidth, const float outerHeight)
{

    const int sizeX = (int) buttons.at(0).size();
    const int sizeY = (int) buttons.size();
    const float buttonHeight = buttons.at(0).at(0)->getSize().y();
    const float gridHeight = (buttonHeight + BUTTON_GRID_VERT_PADDING + 2) * (float)sizeY;

    float horizPadding = (float) BUTTON_GRID_HORIZ_PADDING;
    float gridWidth, buttonWidth;

    do {
        gridWidth = outerWidth - horizPadding; // to get centered because size * (button size + BUTTON_GRID_VERT_PADDING) let a half BUTTON_GRID_VERT_PADDING left / right marge
        buttonWidth = (gridWidth / (float)sizeX) - horizPadding;
        horizPadding -= 2;
    } while ((buttonWidth < 100) && (horizPadding > 2));


    std::shared_ptr<ComponentGrid> grid = std::make_shared<ComponentGrid>(window, Vector2i(sizeX, sizeY));

    grid->setSize(gridWidth, gridHeight < outerHeight ? gridHeight : outerHeight);

    for (int x = 0; x < sizeX; x++)
        grid->setColWidthPerc(x, (float) 1 / (float)sizeX);

    for (int y = 0; y < sizeY; y++)
    {
        for (int x = 0; x < sizeX; x++)
        {
            const std::shared_ptr<ButtonComponent>& button = buttons.at(y).at(x);
            button->setSize(buttonWidth, buttonHeight);
            grid->setEntry(button, Vector2i(x, y), true, false);
        }
    }

    return grid;
}

std::shared_ptr<ImageComponent> makeArrow(Window&window)
{
    auto menuTheme = MenuThemeData::getInstance()->getCurrentTheme();
    auto bracket = std::make_shared<ImageComponent>(window);
    bracket->setImage(menuTheme->iconSet.arrow);
    bracket->setColorShift(menuTheme->menuText.color);
    bracket->setResize(0, round(menuTheme->menuText.font->getLetterHeight()));
    
    return bracket;
}
