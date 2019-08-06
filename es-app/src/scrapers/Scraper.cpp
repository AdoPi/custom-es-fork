#include "scrapers/Scraper.h"
#include "Log.h"
#include "Settings.h"
#include <FreeImage.h>
#include <boost/filesystem.hpp>
#include <boost/assign.hpp>
#include <RootFolders.h>

#include "GamesDBJSONScraper.h"
#include "MamedbScraper.h"
#include "ScreenscraperScraper.old.h"

#define SCRAPER_THEGAMEDB     "TheGamesDB"
#define SCRAPER_MAMEDB        "Mamedb"
#define SCRAPER_SCREENSCRAPER "Screenscraper"

static const std::map<std::string, generate_scraper_requests_func>& getScraperGeneratorList()
{
  static const std::map<std::string, generate_scraper_requests_func> scraperRequestFunction =
  {
    {SCRAPER_THEGAMEDB,     &thegamesdb_generate_json_scraper_requests},
    {SCRAPER_MAMEDB,        &mamedb_generate_scraper_requests},
    {SCRAPER_SCREENSCRAPER, &screenscraper_generate_scraper_requests},
  };
  return scraperRequestFunction;
}

std::unique_ptr<ScraperSearchHandle> startScraperSearch(const ScraperSearchParams& params)
{
  // Get prefered scrapper
	std::string name = Settings::getInstance()->getString("Scraper");

	// Control invalid scraper
	if (getScraperGeneratorList().find(name) == getScraperGeneratorList().end())
	  name = SCRAPER_SCREENSCRAPER;

	// Create the scraper
	std::unique_ptr<ScraperSearchHandle> handle(new ScraperSearchHandle());
  getScraperGeneratorList().at(name)(params, handle->mRequestQueue, handle->mResults);

	return handle;
}

std::vector<std::string> getScraperList()
{
	std::vector<std::string> list;
	for (const auto& scraper_request_func : getScraperGeneratorList())
	{
		list.push_back(scraper_request_func.first);
	}

	return list;
}

// ScraperSearchHandle
ScraperSearchHandle::ScraperSearchHandle()
{
	setStatus(ASYNC_IN_PROGRESS);
}

void ScraperSearchHandle::update()
{
	if(mStatus == ASYNC_DONE)
		return;

	while(!mRequestQueue.empty())
	{
		auto& req = mRequestQueue.front();
		AsyncHandleStatus status = req->status();

		if(status == ASYNC_ERROR)
		{
			// propegate error
			setError(req->getStatusString());

			// empty our queue
			while(!mRequestQueue.empty())
				mRequestQueue.pop();

			return;
		}

		// finished this one, see if we have any more
		if(status == ASYNC_DONE)
		{
			mRequestQueue.pop();
			continue;
		}

		// status == ASYNC_IN_PROGRESS
	}

	// we finished without any errors!
	if(mRequestQueue.empty())
	{
		setStatus(ASYNC_DONE);
		return;
	}
}



// ScraperRequest
ScraperRequest::ScraperRequest(std::vector<ScraperSearchResult>& resultsWrite) : mResults(resultsWrite)
{
}


// ScraperHttpRequest
ScraperHttpRequest::ScraperHttpRequest(std::vector<ScraperSearchResult>& resultsWrite, const std::string& url) 
	: ScraperRequest(resultsWrite)
{
	setStatus(ASYNC_IN_PROGRESS);
	mReq = std::unique_ptr<HttpReq>(new HttpReq(url));
}

void ScraperHttpRequest::update()
{
	HttpReq::Status status = mReq->status();
	if(status == HttpReq::REQ_SUCCESS)
	{
		setStatus(ASYNC_DONE); // if process() has an error, status will be changed to ASYNC_ERROR
		process(mReq, mResults);
		return;
	}

	// not ready yet
	if(status == HttpReq::REQ_IN_PROGRESS)
		return;

	// everything else is some sort of error
	LOG(LogError) << "ScraperHttpRequest network error (status: " << status << ") - " << mReq->getErrorMsg();
	setError(mReq->getErrorMsg());
}


// metadata resolving stuff

std::unique_ptr<MDResolveHandle> resolveMetaDataAssets(const ScraperSearchResult& result, const ScraperSearchParams& search)
{
	return std::unique_ptr<MDResolveHandle>(new MDResolveHandle(result, search));
}

MDResolveHandle::MDResolveHandle(const ScraperSearchResult& result, const ScraperSearchParams& search) : mResult(result)
{
	if(!result.imageUrl.empty())
	{
		std::string imgPath = getSaveAsPath(search, "images", result.imageUrl);
		mFuncs.push_back(ResolvePair(downloadImageAsync(result.imageUrl, imgPath), [this, imgPath]
		{
			mResult.mdl.SetImagePath(imgPath);
			mResult.imageUrl.clear();
		}));
	}
}

void MDResolveHandle::update()
{
	if(mStatus == ASYNC_DONE || mStatus == ASYNC_ERROR)
		return;
	
	auto it = mFuncs.begin();
	while(it != mFuncs.end())
	{
		if(it->first->status() == ASYNC_ERROR)
		{
			setError(it->first->getStatusString());
			return;
		}else if(it->first->status() == ASYNC_DONE)
		{
			it->second();
			it = mFuncs.erase(it);
			continue;
		}
		it++;
	}

	if(mFuncs.empty())
		setStatus(ASYNC_DONE);
}

std::unique_ptr<ImageDownloadHandle> downloadImageAsync(const std::string& url, const std::string& saveAs)
{
	return std::unique_ptr<ImageDownloadHandle>(new ImageDownloadHandle(url, saveAs, 
		Settings::getInstance()->getInt("ScraperResizeWidth"), Settings::getInstance()->getInt("ScraperResizeHeight")));
}

ImageDownloadHandle::ImageDownloadHandle(const std::string& url, const std::string& path, int maxWidth, int maxHeight)
  : mReq(new HttpReq(url)),
	  mSavePath(path),
	  mMaxWidth(maxWidth),
	  mMaxHeight(maxHeight)
{
}

void ImageDownloadHandle::update()
{
	if(mReq->status() == HttpReq::REQ_IN_PROGRESS)
		return;

	if(mReq->status() != HttpReq::REQ_SUCCESS)
	{
		std::stringstream ss;
		ss << "Network error: " << mReq->getErrorMsg();
		setError(ss.str());
		return;
	}

	// download is done, save it to disk
	std::ofstream stream(mSavePath, std::ios_base::out | std::ios_base::binary);
	if(stream.bad())
	{
		setError("Failed to open image path to write. Permission error? Disk full?");
		return;
	}

	const std::string& content = mReq->getContent();
	stream.write(content.data(), content.length());
	stream.close();
	if(stream.bad())
	{
		setError("Failed to save image. Disk full?");
		return;
	}

	// resize it
	if(!resizeImage(mSavePath, mMaxWidth, mMaxHeight))
	{
		setError("Error saving resized image. Out of memory? Disk full?");
		return;
	}

	setStatus(ASYNC_DONE);
}

//you can pass 0 for width or height to keep aspect ratio
bool resizeImage(const std::string& path, int maxWidth, int maxHeight)
{
	// nothing to do
	if(maxWidth == 0 && maxHeight == 0)
		return true;

	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* image = nullptr;
	
	//detect the filetype
	format = FreeImage_GetFileType(path.c_str(), 0);
	if(format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(path.c_str());
	if(format == FIF_UNKNOWN)
	{
		LOG(LogError) << "Error - could not detect filetype for image \"" << path << "\"!";
		return false;
	}

	//make sure we can read this filetype first, then load it
	if(FreeImage_FIFSupportsReading(format))
	{
		image = FreeImage_Load(format, path.c_str());
	}else{
		LOG(LogError) << "Error - file format reading not supported for image \"" << path << "\"!";
		return false;
	}

	float width = (float)FreeImage_GetWidth(image);
	float height = (float)FreeImage_GetHeight(image);

	if(maxWidth == 0)
	{
		maxWidth = (int)(((float)maxHeight / height) * width);
	}else if(maxHeight == 0)
	{
		maxHeight = (int)(((float)maxWidth / width) * height);
	}

	FIBITMAP* imageRescaled = FreeImage_Rescale(image, maxWidth, maxHeight, FILTER_BILINEAR);
	FreeImage_Unload(image);

	if(imageRescaled == nullptr)
	{
		LOG(LogError) << "Could not resize image! (not enough memory? invalid bitdepth?)";
		return false;
	}

	bool saved =  path.empty() ? false : FreeImage_Save(format, imageRescaled, path.c_str());
	FreeImage_Unload(imageRescaled);

    if(!saved) {
		LOG(LogError) << "Failed to save resized image!";
    }

	return saved;
}

std::string getSaveAsPath(const ScraperSearchParams& params, const std::string& suffix, const std::string& url)
{
	const std::string subdirectory = params.system->getName();
	const std::string name = params.game->getPath().stem().generic_string() + "-" + suffix;

	// default dir in rom directory
	std::string path = params.system->getRootFolder()->getPath().generic_string() + "/media/" + suffix + '/';
	if (!boost::filesystem::exists(path))
  {
    boost::filesystem::create_directories(path);
    if (!boost::filesystem::exists(path))
    {
      LOG(LogError) << "Cannot create " << path;
      return "";
    }
  }

	size_t dot = url.find_last_of('.');
	std::string ext;
	if(dot != std::string::npos)
		ext = url.substr(dot, std::string::npos);

	path += name + ext;
	return path;
}
