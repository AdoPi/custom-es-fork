#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/OptionListComponent.h"
#include "FileSorts.h"

class IGameListView;

class GuiGamelistOptions : public GuiComponent
{
public:
	GuiGamelistOptions(Window&window, SystemData* system);
	~GuiGamelistOptions() override;

	void save();
	inline void addSaveFunc(const std::function<void()>& func) { mSaveFuncs.push_back(func); };

	bool ProcessInput(const InputCompactEvent& event) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	void openMetaDataEd();
	void jumpToLetter();
	
	MenuComponent mMenu;

	std::vector< std::function<void()> > mSaveFuncs;

	typedef OptionListComponent<char> LetterList;
	std::shared_ptr<LetterList> mJumpToLetterList;

	typedef OptionListComponent<int> SortList;
	std::shared_ptr<SortList> mListSort;

	bool mFavoriteState;
	bool mHiddenState;
	bool mReloading;

	SystemData* mSystem;
	IGameListView* getGamelist();
};
