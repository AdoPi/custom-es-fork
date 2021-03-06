#pragma once

#include <vector>
#include <utils/storage/HashMap.h>
#include <string>
#include <utils/os/fs/Path.h>
#include "MetadataDescriptor.h"
#include "ItemType.h"
#include "utils/cplusplus/Bitflags.h"

// Forward declarations
class SystemData;
class FolderData;
class RootFolderData;

// A tree node that holds information for a file.
class FileData
{
  public:
    typedef HashMap<std::string, FileData*> StringMap;
    typedef std::vector<FileData*> List;
    typedef std::vector<const FileData*> ConstList;
    typedef int (*Comparer)(const FileData& a, const FileData& b);

    //! Game filters
    enum class Filter
    {
      None     = 0, //!< Include nothing
      Normal   = 1, //!< Include normal files (not hidden, not favorite)
      Favorite = 2, //!< Include favorites
      Hidden   = 4, //!< Include hidden
      All      = 7, //!< Include all
    };

    //! Search attribute enumeration
    enum class SearchAttributes
    {
      ByName        = 1, //!< Search by name, excluding extension
      ByNameWithExt = 2, //!< Search by name, including extension
      ByHash        = 4, //!< Search by hash
      All           = 7, //!< All attributes
    };

  protected:
    //! Top ancestor (link to system)
    RootFolderData& mTopAncestor;
    //! Parent folder
    FolderData* mParent;
    //! Item type - Const ensure mType cannot be modified after being set by the constructor, so that it's alays safe to use c-style cast for FolderData sub-class.
    const ItemType mType;

  private:
    //! Item path on the filesystem
    Path mPath;
    //! Metadata
    MetadataDescriptor mMetadata;

  protected:
    /*!
     * Constructor for subclasses only
     * @param type
     * @param path Item path
     * @param system Parent system
     */
    FileData(ItemType type, const Path& path, RootFolderData& ancestor);

  public:
    /*!
     * Constructor
     * @param path Item path on filesystem
     * @param system system to attach to
     */
    FileData(const Path& path, RootFolderData& ancestor);

    /*
     * Getters
     */

    inline const std::string& getName() const { return mMetadata.Name(); }
    inline std::string getHash() const { return mMetadata.RomCrc32AsString(); }
    inline ItemType getType() const { return mType; }
    inline const Path& getPath() const { return mPath; }
    inline FolderData* getParent() const { return mParent; }
    inline RootFolderData& getTopAncestor() const { return mTopAncestor; }
    SystemData* getSystem() const;

    /*
     * Booleans
     */

    inline bool isEmpty() const { return mType == ItemType::Empty; }
    inline bool isGame() const { return mType == ItemType::Game; }
    inline bool isFolder() const { return mType == ItemType::Folder || mType == ItemType::Root; }
    inline bool isRoot() const { return mType == ItemType::Root; }
    inline bool isTopMostRoot() const { return mType == ItemType::Root && mParent == nullptr; }

    /*
     * Setters
     */

    inline void setParent(FolderData* parent) { mParent = parent; }

    /*!
     * Get Thumbnail path if there is one, or Image path.
     * @return file path (may be empty)
     */
    inline const Path& getThumbnailOrImagePath() const { return mMetadata.Thumbnail().IsEmpty() ? mMetadata.Image() : mMetadata.Thumbnail(); }

    /*!
     * Return true if at least one image is available (thumbnail or regular image)
     * @return Boolean result
     */
    inline bool hasThumbnailOrImage() const { return !(mMetadata.Thumbnail().IsEmpty() && mMetadata.Image().IsEmpty()); }

    /*!
     * const Metadata accessor for Read operations
     * @return const Metadata object
     */
    const MetadataDescriptor& Metadata() const { return mMetadata; }

    /*!
     * Metadata accessor for Write operations only
     * @return Writable Metadata object
     */
    MetadataDescriptor& Metadata() { return mMetadata; }

    /*!
     * Get clean default name, by reoving parenthesis and braqueted words
     * @return Clean name
     */
    std::string getScrappableName() const;

    /*!
     * Get smart default name, when available, depending of the file/folder name
     * Mainly used to get smart naming from arcade zipped roms
     * @return Smart name of the current item, or file/folder name
     */
    std::string getDisplayName() const;

    /*!
     * @brief Get Pad2Keyboard configuration file path
     * @return Pad2Keyboard configuration file path
     */
    Path P2KPath() const { return mPath.ChangeExtension(mPath.Extension() + ".p2k.cfg"); }

    /*!
     * @brief Check if Pad2Keyboard configuration file exists
     * @return Trie if the Pad2Keyboard configuration file exists
     */
    bool HasP2K() const;
};

DEFINE_BITFLAG_ENUM(FileData::Filter, int)
DEFINE_BITFLAG_ENUM(FileData::SearchAttributes, int)