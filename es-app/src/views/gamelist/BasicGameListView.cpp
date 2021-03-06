#include <RecalboxConf.h>
#include "views/gamelist/BasicGameListView.h"
#include "views/ViewController.h"
#include "Renderer.h"
#include "themes/ThemeData.h"
#include "systems/SystemData.h"
#include "games/FileSorts.h"
#include "Settings.h"
#include "utils/locale/LocaleHelper.h"
#include "SystemIcons.h"
#include <recalbox/RecalboxSystem.h>
#include <systems/SystemManager.h>

BasicGameListView::BasicGameListView(WindowManager& window, SystemManager& systemManager, SystemData& system)
	: ISimpleGameListView(window, systemManager, system),
	  mList(window),
	  mHasGenre(false),
    mEmptyListItem(&system),
    mPopulatedFolder(nullptr)
{
	mList.setSize(mSize.x(), mSize.y() * 0.8f);
	mList.setPosition(0, mSize.y() * 0.2f);
	mList.setDefaultZIndex(20);

  addChild(&mList);

	mEmptyListItem.Metadata().SetName(_("EMPTY LIST"));
	populateList(system.MasterRoot());

  mList.setCursorChangedCallback([this](const CursorState& state)
                                 {
                                   (void) state;
                                   updateInfoPanel();
                                 });
}

void BasicGameListView::onThemeChanged(const ThemeData& theme)
{
	ISimpleGameListView::onThemeChanged(theme);
	mList.applyTheme(theme, getName(), "gamelist", ThemeProperties::All);
	// Set color 2/3 50% transparent of color 0/1
	mList.setColor(2, (mList.Color(0) & 0xFFFFFF00) | ((mList.Color(0) & 0xFF) >> 1));
  mList.setColor(3, (mList.Color(1) & 0xFFFFFF00) | ((mList.Color(1) & 0xFF) >> 1));
	sortChildren();
}

void BasicGameListView::onFileChanged(FileData* file, FileChangeType change)
{
	ISimpleGameListView::onFileChanged(file, change);

	if(change == FileChangeType::MetadataChanged)
	{
		// might switch to a detailed view
    ViewController::Instance().reloadGameListView(this);
		return;
	}
}

const char * BasicGameListView::getItemIcon(FileData* item)
{
  // Crossed out eye for hidden things
	if (item->Metadata().Hidden()) return "\uF070 ";
	// System icon, for Favorite games
	if ((item->isGame()) && (mSystem.IsVirtual() || item->Metadata().Favorite()))
		return SystemIcons::GetIcon(item->getSystem()->getName());
	// Open folder for folders
	if (item->isFolder())
	  return "\uF07C ";

	return nullptr;
}

void BasicGameListView::populateList(const FolderData& folder)
{
  mPopulatedFolder = &folder;
  mList.clear();
  mHeaderText.setText(mSystem.getFullName());

  // Default filter
  FileData::Filter filter = FileData::Filter::Normal | FileData::Filter::Favorite;
  // Add hidden?
  if (Settings::Instance().ShowHidden()) filter |= FileData::Filter::Hidden;
  // Favorites only?
  if (mFavoritesOnly) filter = FileData::Filter::Favorite;

  // Get items
  bool flatfolders = mSystem.IsAlwaysFlat() || (RecalboxConf::Instance().AsBool(mSystem.getName() + ".flatfolder"));
  FileData::List items;
  if (flatfolders) folder.getItemsRecursivelyTo(items, filter, false, mSystem.IncludeAdultGames());
  else             folder.getItemsTo(items, filter, true, mSystem.IncludeAdultGames());

  // Check emptyness
  if (items.empty()) items.push_back(&mEmptyListItem); // Insert "EMPTY SYSTEM" item

  // Sort
  FileSorts::Sorts sort =
    mSystem.IsSelfSorted() ? mSystem.FixedSort()
                           : FileSorts::AvailableSorts(mSystem.IsVirtual())[FileSorts::Clamp(mSystem.getSortId(), mSystem.IsVirtual())];
  FolderData::Sort(items, FileSorts::Comparer(sort), FileSorts::IsAscending(sort));

  // Region filtering?
  Regions::GameRegions currentRegion = Regions::Clamp((Regions::GameRegions)RecalboxConf::Instance().AsInt("emulationstation." + mSystem.getName() + ".regionfilter"));
  bool activeRegionFiltering = false;
  if (currentRegion != Regions::GameRegions::Unknown)
  {
    Regions::List availableRegion = AvailableRegionsInGames(items);
    // Check if our region is in the available ones
    for(Regions::GameRegions region : availableRegion)
    {
      activeRegionFiltering = (region == currentRegion);
      if (activeRegionFiltering) break;
    }
  }

  // Add to list
  mHasGenre = false;
  //mList.reserve(items.size()); // TODO: Reserve memory once
  for (FileData* fd : items)
	{
    // Select fron icon
    const char* icon = getItemIcon(fd);
  	// Get name
  	std::string name = icon != nullptr ? icon + fd->getName() : fd->getName();
  	// Region filtering?
  	int colorIndexOffset = 0;
  	if (activeRegionFiltering)
  	  if (!Regions::IsIn4Regions(fd->Metadata().Region(), currentRegion))
  	    colorIndexOffset = 2;
    // Store
		mList.add(name, fd, colorIndexOffset + (fd->isFolder() ? 1 : 0), false);
		// Attribuite analysis
		if (fd->isGame())
    {
      if (fd->Metadata().GenreId() != GameGenres::None)
        mHasGenre = true;
    }
	}
}

FileData::List BasicGameListView::getFileDataList()
{
	return mList.getObjects();
}

void BasicGameListView::setCursorIndex(int index)
{
  if (index >= mList.size()) index = mList.size() - 1;
  if (index < 0) index = 0;

	mList.setCursorIndex(index);
}

void BasicGameListView::setCursor(FileData* cursor)
{
	if(!mList.setCursor(cursor, 0))
	{
		populateList(mSystem.MasterRoot());
		mList.setCursor(cursor);

		// update our cursor stack in case our cursor just got set to some folder we weren't in before
		if(mCursorStack.empty() || mCursorStack.top() != cursor->getParent())
		{
			std::stack<FolderData*> tmp;
			FolderData* ptr = cursor->getParent();
			while((ptr != nullptr) && !ptr->isRoot())
			{
				tmp.push(ptr);
				ptr = ptr->getParent();
			}
			
			// flip the stack and put it in mCursorStack
			mCursorStack = std::stack<FolderData*>();
			while(!tmp.empty())
			{
				mCursorStack.push(tmp.top());
				tmp.pop();
			}
		}
	}
}

Regions::List BasicGameListView::AvailableRegionsInGames()
{
  bool regionIndexes[256];
  memset(regionIndexes, 0, sizeof(regionIndexes));
  // Run through all games
  for(int i = (int)mList.size(); --i >= 0; )
  {
    const FileData& fd = *mList.getSelectedAt(i);
    int fourRegions = fd.Metadata().Region();
    // Set the 4 indexes corresponding to all 4 regions (Unknown regions will all point to index 0)
    regionIndexes[(fourRegions >>  0) & 0xFF] = true;
    regionIndexes[(fourRegions >>  8) & 0xFF] = true;
    regionIndexes[(fourRegions >> 16) & 0xFF] = true;
    regionIndexes[(fourRegions >> 24) & 0xFF] = true;
  }
  // Rebuild final list
  Regions::List list;
  for(int i = 0; i < (int)sizeof(regionIndexes); ++i )
    if (regionIndexes[i])
      list.push_back((Regions::GameRegions)i);
  // Only unknown region?
  if (list.size() == 1 && regionIndexes[0])
    list.clear();
  return list;
}

Regions::List BasicGameListView::AvailableRegionsInGames(FileData::List& fdList)
{
  bool regionIndexes[256];
  memset(regionIndexes, 0, sizeof(regionIndexes));
  // Run through all games
  for(const FileData* fd : fdList)
  {
    int fourRegions = fd->Metadata().Region();
    // Set the 4 indexes corresponding to all 4 regions (Unknown regions will all point to index 0)
    regionIndexes[(fourRegions >>  0) & 0xFF] = true;
    regionIndexes[(fourRegions >>  8) & 0xFF] = true;
    regionIndexes[(fourRegions >> 16) & 0xFF] = true;
    regionIndexes[(fourRegions >> 24) & 0xFF] = true;
  }
  // Rebuild final list
  Regions::List list;
  for(int i = (int)sizeof(regionIndexes); --i >= 0; )
    if (regionIndexes[i])
      list.push_back((Regions::GameRegions)i);
  // Only unknown region?
  if (list.size() == 1 && regionIndexes[0])
    list.clear();
  return list;
}
