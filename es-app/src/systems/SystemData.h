#pragma once

#include <string>
#include <utils/cplusplus/INoCopy.h>
#include <RecalboxConf.h>
#include "games/RootFolderData.h"
#include "WindowManager.h"
#include "PlatformId.h"
#include "themes/ThemeData.h"
#include "games/FileSorts.h"
#include "EmulatorList.h"
#include "SystemDescriptor.h"
#include "NetPlayData.h"
#include "EmulatorData.h"
#include "Settings.h"

class SystemManager;

class SystemData : private INoCopy
{
  public:
    //! System properties
    enum class Properties
    {
      None       =  0, //!< No properties
      Ports      =  1, //!< This system is the special "Ports" system
      Favorite   =  2, //!< This system is the special "Favorite" system
      Virtual    =  4, //!< This system is not a real system
      FixedSort  =  8, //!< This system has its own fixed sort
      AlwaysFlat = 16, //!< This system is presented always flat
      Searchable = 32, //!< We can search games in this system
    };

	private:
    //! CPU Governance file
    static constexpr const char* sGovernanceFile = "/recalbox/share_init/system/configs/.governances.cfg";

    // Allow manager to access this class
    friend class SystemManager;

    //! Global flag set to true when a game is running and the application
    //! is frozen until the game exits.
    static bool sIsGameRunning;
    //! Governance path
    static Path sGovernancePath;

    /*!
     * @brief Automatic Game running flag management
     */
    class GameRunner
    {
      public:
        //! Constructor - Set the flag
        GameRunner() { sIsGameRunning = true; }
        //! Destructor - Reset the flag
        ~GameRunner() { sIsGameRunning = false; }
    };

    //! Parent manager
    SystemManager& mSystemManager;

    //! Descriptor
    SystemDescriptor mDescriptor;
    //! Theme object
    ThemeData mTheme;
    //! Root folders - Children are top level visible game/folder of the system
    RootFolderData mRootOfRoot;
    //! Sorting index
    int mSortId;
    //! Is this system the favorite system?
    Properties mProperties;
    //! Fixed sort
    FileSorts::Sorts mFixedSort;

    /*!
     * @brief Populate the system using all available folder/games by gathering recursively
     * all files mathing the extension list
     * @param folder Root folder to recurse in
     * @param doppelgangerWatcher full path map to avoid adding a game more than once
     */
    void populateFolder(RootFolderData& folder, FileData::StringMap& doppelgangerWatcher);

    /*!
     * @brief Private constructor, called from SystemManager
     * @param System descriptor
     * @param childOwnership Type of children management
     * @param properties System properties
     */
    SystemData(SystemManager& systemManager, const SystemDescriptor& systemDescriptor, Properties properties, FileSorts::Sorts fixedSort = FileSorts::Sorts::FileNameAscending);

    /*!
     * @brief Get localized text inside a text. Look for [lg] tags to mark start/end of localized texts
     * if the current language is not found, the method looks for [en].
     * if still not found, the whole text is returned
     * @param source Source text
     * @return localized text
     */
    static std::string getLocalizedText(const std::string& source);

    /*!
     * @brief Looks for folder override files in the given folder.
     * If overriden images/texts are found, thay are loaded to override empty or gamelist information
     * The methods looks for:
     * - .folder.picture.svg or .folder.picture.png
     * - .folder.description.txt
     * @param folderdata folder data to override
     */
    static void overrideFolderInformation(FileData* folderdata);

    /*!
     * @brief Lookup an existing game entry (or create it) in the current system.
     * @param root Game root path (usually system root path)
     * @param path Game path
     * @param type Type (folder/game)
     * @param doppelgangerWatcher Maps to avoid duplicate entries
     * @return Existing or newly created FileData
     */
    FileData* LookupOrCreateGame(RootFolderData& topAncestor, const Path& rootPath, const Path& path, ItemType type, FileData::StringMap& doppelgangerWatcher) const;

    /*!
     * @brief Parse xml gamelist files and add games to the current system
     * @param root Root rom folder
     * @param doppelgangerWatcher Maps to avoid duplicate entries
     * @param forceCheckFile True to force to check if file exists
     */
    void ParseGamelistXml(RootFolderData& root, FileData::StringMap& doppelgangerWatcher, bool forceCheckFile);

    /*!
     * @brief Try to override command with the nearest one found in .system.cfg in folder or parent folders
     * @param romPath Rom path to fetch .system.cfg from
     * @param command Command to override
     */
    void OverrideCommand(const Path& romPath, std::string& command);

    /*!
     * @brief Get energy governance for a particular core/emulator
     * @param core Core/Emulateur
     * @return Governance
     */
    static IBoardInterface::CPUGovernance GetGovernance(const std::string& core);

    /*!
     * @brief Get root folder of the given type
     * @param type Type to lookup
     * @return Found root or nullptr
     */
    RootFolderData* GetRootFolder(RootFolderData::Types type);

    /*!
     * @brief Create new root folder
     * @param startpath Path
     * @param childownership Child ownership type
     * @param type Type of root
     * @return New root folder
     */
    RootFolderData& CreateRootFolder(const Path& startpath, RootFolderData::Ownership childownership, RootFolderData::Types type);

    /*!
     * @brief Lookup an existig root folder or create a new one using the given configuration
     * @param startpath Path
     * @param childownership Child ownership type
     * @param type Type of root
     * @return Existing or new root folder
     */
    RootFolderData& LookupOrCreateRootFolder(const Path& startpath, RootFolderData::Ownership childownership, RootFolderData::Types type);

  public:
    /*!
     * @brief Check if we must include adult games or not
     * @return True to include adult games in game lists
     */
    bool IncludeAdultGames() const
    {
      return !(RecalboxConf::Instance().AsBool("emulationstation.filteradultgames") ||
               RecalboxConf::Instance().AsBool("emulationstation." + mDescriptor.Name() + ".filteradultgames"));
    }

    static bool IncludeHiddenGames()
    {
      return Settings::Instance().ShowHidden();
    }

    /*!
     * @brief Get master root
     * @return Master root
     */
    RootFolderData& MasterRoot() { return mRootOfRoot; }
    /*!
     * @brief Get master root - const version
     * @return Master root
     */
    const RootFolderData& MasterRoot() const { return mRootOfRoot; }

    const std::string& getName() const { return mDescriptor.Name(); }
    const std::string& getFullName() const { return mDescriptor.FullName(); }
    const std::string& ThemeFolder() const { return mDescriptor.ThemeFolder(); }
    bool getHasFavoritesInTheme() const { return mTheme.getHasFavoritesInTheme(); }
    FileData::List getFavorites() const;
    FileData::List getGames() const;
    FileData::List getAllGames() const;
    FileData::List getFolders() const;
    FileData::List getTopGamesAndFolders() const;
    int getSortId() const { return mSortId; };
    void setSortId(const int sortId) { mSortId = sortId; };

    PlatformIds::PlatformId PlatformIds(int index) const { return mDescriptor.Platform(index); }
    int PlatformCount() const { return mDescriptor.PlatformCount(); }
    bool HasPlatform() const { return mDescriptor.PlatformCount() != 0; }
    bool hasPlatformId(PlatformIds::PlatformId id) const { return mDescriptor.HasPlatform(id); }

    inline const ThemeData& getTheme() const { return mTheme; }

    static Path getGamelistPath(const RootFolderData& root, bool forWrite);
    /*!
     * @brief Get list of writable Gamelists
     * @return List of writable gamelists
     */
    Path::PathList WritableGamelists();
    Path getThemePath() const;

    bool HasGame() const;
    bool HasVisibleGame() const;
    int GameCount() const;
    int GameAndFolderCount() const;
    int FavoritesCount() const;
    int HiddenCount() const;

    void RunGame(WindowManager& window, SystemManager& systemManager, FileData& game, const EmulatorData& emulator, const NetPlayData& netplay);

    // Load or re-load theme.
    void loadTheme();

    const EmulatorList& Emulators() const { return mDescriptor.EmulatorTree(); }

    static std::string demoInitialize(WindowManager& window);
    static void demoFinalize(WindowManager& window);
    bool DemoRunGame(const FileData& game, const EmulatorData& emulator, int duration, int infoscreenduration, const std::string& controlersConfig);

    //! Is this system the "Favorite" system?
    bool IsFavorite() const;

    //! Is this system the "Ports" system?
    bool IsPorts() const;

    //! Is this system virtual?
    bool IsVirtual() const;

    //! Is this system selt sorted
    bool IsSelfSorted() const;

    //! Is this system always flat?
    bool IsAlwaysFlat() const;

    //! Is this system always flat?
    bool IsSearchable() const;

    /*!
     * @brief Get or create pure virtual root - USE IT ONLY ON FAVORITE SYSTEM
     * @return Virtual root
     */
    FolderData& GetFavoriteRoot();

    FileSorts::Sorts FixedSort() const { return mFixedSort; }

    /*!
     * @brief Write modified games back to the gamelist xml file
     */
    void UpdateGamelistXml();

    /*!
     * @brief Update game list with a single game on top of the list
     * @param game game to insert or move
     */
    void UpdateLastPlayedGame(FileData& game);

    /*!
     * @brief Rebuild a complete map path/FileData recursively
     * @param doppelganger Map to fill in
     * @param includefolder Include folder or not
     */
    void BuildDoppelgangerMap(FileData::StringMap& doppelganger, bool includefolder) const;

    /*!
     * @brief Get parent system manager
     * @return
     */
    SystemManager& Manager() const { return mSystemManager; }

    /*!
     * @brief Run system command and capture output
     * @param cmd_utf8 Command to execute
     * @param debug log output?
     * @return Return code
     */
    static int Run(const std::string& cmd_utf8, bool debug);

    /*!
     * @brief Check if a game is running
     * @return True if a game is running
     */
    static bool IsGameRunning() { return sIsGameRunning; }

    /*!
     * @brief Search for all games containing 'text' and add them to 'result'
     * @param context Field in which to search text for
     * @param text Test to search for
     * @param results Result list
     * @param remaining Maximum results
     */
    void FastSearch(FolderData::FastSearchContext context, const std::string& text, FolderData::ResultList& results, int& remaining);
};

DEFINE_BITFLAG_ENUM(SystemData::Properties, int)