#include <cassert>
#include "HttpReq.h"
#include "utils/Log.h"

CURLM* HttpReq::s_multi_handle = curl_multi_init();

std::map<CURL*, HttpReq*> HttpReq::s_requests;

std::string HttpReq::urlEncode(const std::string &s)
{
    const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

    std::string escaped;
    for (char i : s)
    {
        if (unreserved.find_first_of(i) != std::string::npos)
        {
            escaped.push_back(i);
        }
        else
        {
            escaped.append("%");
            char buf[3];
            sprintf(buf, "%.2X", (unsigned char)i);
            escaped.append(buf);
        }
    }
    return escaped;
}

bool HttpReq::isUrl(const std::string& str)
{
	//the worst guess
	return (!str.empty() && !Path(str).Exists() &&
		(str.find("http://") != std::string::npos || str.find("https://") != std::string::npos || str.find("www.") != std::string::npos));
}

HttpReq::HttpReq(const std::string& url)
	: mHandle(nullptr),
	  mStatus(Status::InProgress)
{
	mHandle = curl_easy_init();

	if(mHandle == nullptr)
	{
		mStatus = Status::IOError;
		onError("curl_easy_init failed");
		return;
	}

	//set the url
	CURLcode err = curl_easy_setopt(mHandle, CURLOPT_URL, url.c_str());
	if(err != CURLE_OK)
	{
    mStatus = Status::IOError;
    onError(curl_easy_strerror(err));
		return;
	}

	//tell curl how to write the data
	err = curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, &HttpReq::write_content);
	if(err != CURLE_OK)
	{
		mStatus = Status::IOError;
		onError(curl_easy_strerror(err));
		return;
	}

	//give curl a pointer to this HttpReq so we know where to write the data *to* in our write function
	err = curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, this);
	if(err != CURLE_OK)
	{
		mStatus = Status::IOError;
		onError(curl_easy_strerror(err));
		return;
	}

	//add the handle to our multi
	CURLMcode merr = curl_multi_add_handle(s_multi_handle, mHandle);
	if(merr != CURLM_OK)
	{
		mStatus = Status::IOError;
		onError(curl_multi_strerror(merr));
		return;
	}

	s_requests[mHandle] = this;
}

HttpReq::~HttpReq()
{
	if(mHandle != nullptr)
	{
		s_requests.erase(mHandle);

		CURLMcode merr = curl_multi_remove_handle(s_multi_handle, mHandle);

        if(merr != CURLM_OK) {
			LOG(LogError) << "Error removing curl_easy handle from curl_multi: " << curl_multi_strerror(merr);
        }

		curl_easy_cleanup(mHandle);
	}
}

HttpReq::Status HttpReq::status()
{
	if(mStatus == Status::InProgress)
	{
		int handle_count;
		CURLMcode merr = curl_multi_perform(s_multi_handle, &handle_count);
		if(merr != CURLM_OK && merr != CURLM_CALL_MULTI_PERFORM)
		{
			mStatus = Status::IOError;
			onError(curl_multi_strerror(merr));
			return mStatus;
		}

		int msgs_left;
		CURLMsg* msg;
        while((msg = curl_multi_info_read(s_multi_handle, &msgs_left)) != nullptr)
		{
			if(msg->msg == CURLMSG_DONE)
			{
				HttpReq* req = s_requests[msg->easy_handle];
				
				if(req == nullptr)
				{
					LOG(LogError) << "Cannot find easy handle!";
					continue;
				}

				if(msg->data.result == CURLE_OK)
				{
					req->mStatus = Status::Success;
				}else{
					req->mStatus = Status::IOError;
					req->onError(curl_easy_strerror(msg->data.result));
				}
			}
		}
	}

	return mStatus;
}

std::string HttpReq::getContent() const
{
	assert(mStatus == Status::Success);
	return mContent;
}

//used as a curl callback
//size = size of an element, nmemb = number of elements
//return value is number of elements successfully read
size_t HttpReq::write_content(void* buff, size_t size, size_t nmemb, void* req_ptr)
{
	std::string& ss = ((HttpReq*)req_ptr)->mContent;
	ss.append((char*)buff, size * nmemb);

	return nmemb;
}

//used as a curl callback
/*int HttpReq::update_progress(void* req_ptr, double dlTotal, double dlNow, double ulTotal, double ulNow)
{
	
}*/
