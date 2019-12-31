#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "HttpReq.h"

struct TheGamesDBJSONRequestResources
{
    TheGamesDBJSONRequestResources() = default;

    void prepare();

    void ensureResources();

    std::string getApiKey() const;

    std::unordered_map<int, std::string> gamesdb_new_developers_map;
    std::unordered_map<int, std::string> gamesdb_new_publishers_map;
    std::unordered_map<int, std::string> gamesdb_new_genres_map;

  private:
    bool checkLoaded();

    static bool saveResource(HttpReq* req, std::unordered_map<int, std::string>& resource, const std::string& resource_name,
                      const Path& file_name);

    std::unique_ptr<HttpReq> fetchResource(const std::string& endpoint);

    static int loadResource(std::unordered_map<int, std::string>& resource, const std::string& resource_name,
                     const Path& file_name);

    std::unique_ptr<HttpReq> gamesdb_developers_resource_request;
    std::unique_ptr<HttpReq> gamesdb_publishers_resource_request;
    std::unique_ptr<HttpReq> gamesdb_genres_resource_request;
};

Path getScrapersResouceDir();