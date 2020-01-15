//
// Created by bkg2k on 28/12/2019.
//
#pragma once

#include <vector>
#include <utils/os/system/Thread.h>
#include <utils/sdl2/ISynchronousEvent.h>
#include <utils/sdl2/SyncronousEvent.h>
#include "BiosList.h"
#include "IBiosScanReporting.h"

class BiosManager : private Thread, public ISynchronousEvent
{
  private:
    //! Instance
    static BiosManager* sInstance;

    //! Path to bios.xml file
    static constexpr const char* sBiosFilePath = "system/.emulationstation/es_bios.xml";

    //! Bios list per system
    std::vector<BiosList> mSystemBiosList;
    //! Sync'ed event sender
    SyncronousEvent mSender;
    //! Current scan's reporting interface (also flag for an already running scan)
    IBiosScanReporting* mReporting;

    /*
     * Thread implementation
     */

    /*!
     * Threaded scan implementation
     */
    void Run() override;

    /*
     * ISynchronousInterface
     */

    /*!
     * @brief Receive synchronous SDL2 event
     * @param event SDL event with .user populated by the sender
     */
    void ReceiveSyncCallback(const SDL_Event& event) override;

  public:
    /*!
     * @brief Default constructor
     */
    BiosManager();

    /*!
     * @brief Default destructor
     */
    ~BiosManager() override;

    /*!
     * @brief Get unique instance
     * @return Instance
     */
    static BiosManager& Instance() { return *sInstance; }

    /*!
     * @brief Load all bios from bios.xml
     */
    void LoadFromFile();

    //! Get system count
    int SystemCount() const { return (int)mSystemBiosList.size(); }

    /*!
     * @brief Get bios list from the given system index
     * @param index System index
     * @return BiosList object
     */
    const BiosList& SystemBios(int index) const { return mSystemBiosList[index]; }

    /*!
     * @brief Start scanning all bios and report result using the given interface
     * @param reporting Reporting interface
     */
    void Scan(IBiosScanReporting* reporting);
};
