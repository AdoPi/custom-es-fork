#pragma once

#include "guis/Gui.h"
#include "HttpReq.h"
#include <functional>
#include <memory>

/* 
	Used to asynchronously run an HTTP request.
	Displays a simple animation on the UI to show the application hasn't frozen.  Can be canceled by the user pressing B.

	Usage example:
		std::shared_ptr<HttpReq> httpreq = std::make_shared<HttpReq>("cdn.garcya.us", "/wp-content/uploads/2010/04/TD250.jpg");
		AsyncReqComponent* req = new AsyncReqComponent(mWindow, httpreq,
			[] (std::shared_ptr<HttpReq> r)
		{
			LOG(LogInfo) << "Request completed";
			LOG(LogInfo) << "   error, if any: " << r->getErrorMsg();
		}, [] ()
		{
			LOG(LogInfo) << "Request canceled";
		});

		mWindow.pushGui(req);
		//we can forget about req, since it will always delete itself
*/

class AsyncReqComponent : public Gui
{
public:
	AsyncReqComponent(Window& window, std::shared_ptr<HttpReq> req, std::function<void(std::shared_ptr<HttpReq>)> onSuccess, std::function<void()> onCancel = nullptr);

	bool ProcessInput(const InputCompactEvent& event) override;
	void Update(int deltaTime) override;
	void Render(const Transform4x4f& parentTrans) override;

	bool getHelpPrompts(Help& help) override;
private:
	std::function<void(std::shared_ptr<HttpReq>)> mSuccessFunc;
	std::function<void()> mCancelFunc;

	unsigned int mTime;
	std::shared_ptr<HttpReq> mRequest;
};
