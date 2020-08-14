//
// Created by bkg2k on 13/08/2020.
//
#pragma once

#include <utils/cplusplus/StaticLifeCycleControler.h>
#include <audio/IAudioController.h>
#include "AlsaCard.h"

class AlsaController: public IAudioController, public StaticLifeCycleControler<AlsaController>
{
  private:
    std::vector<AlsaCard> mPlaybacks;

    void Initialize();

  public:
    AlsaController()
      : StaticLifeCycleControler("AlsaController")
    {
      Initialize();
    }

    ~AlsaController() override = default;

    /*
     * IAudioController implementation
     */

    /*!
     * @brief Get playback list
     * @return Map opaque identifier : playback name
     */
    HashMap<int, std::string> GetPlaybackList() const final;

    /*!
     * @brief Set the default card/device
     * @param identifier opaque identifier from GetPlaybackList()
     */
    void SetDefaultPlayback(int identifier) final;

    /*!
     * @brief Set the default card/device
     * @param playbackName playback name from GetPlaybackList()
     * @return playbackName or default value if playbackName is invalid
     */
    std::string SetDefaultPlayback(const std::string& playbackName) final;

    /*!
     * @brief Get volume from the given playback
     * @return Volume percent
     */
    int GetVolume() final;

    /*!
     * @brief Set volume to the given playback
     * @param volume Volume percent
     */
    void SetVolume(int volume) final;
};
