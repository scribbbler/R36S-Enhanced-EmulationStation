#include <string>
#pragma once
#ifndef ES_APP_SYSTEM_SCREEN_SAVER_H
#define ES_APP_SYSTEM_SCREEN_SAVER_H

#include "Window.h"
#include "GuiComponent.h"
#include "renderers/Renderer.h"
#include <random>

using namespace std;

class ImageComponent;
class Sound;
class VideoComponent;
class TextComponent;

class GameScreenSaverBase : public GuiComponent
{
public:
	GameScreenSaverBase(Window* window);
	~GameScreenSaverBase();

	virtual void setGame(FileData* mCurrentGame);

	void render(const Transform4x4f& transform) override;
	void update(int deltaTime) override;

	void setOpacity(unsigned char opacity) override;

protected:
	ImageComponent*		mMarquee;
	TextComponent*		mLabelGame;
	TextComponent*		mLabelSystem;
	TextComponent*		mLabelDate;
	TextComponent*		mLabelTime;

	ImageComponent*		mDecoration;

	Renderer::Rect		mViewport;

	int 				mDateTimeUpdateAccumulator;
	time_t				mDateTimeLastUpdate;
};

class ImageScreenSaver : public GameScreenSaverBase
{
public:
	ImageScreenSaver(Window* window);
	~ImageScreenSaver();

	void setImage(const std::string path);
	bool hasImage();

	void render(const Transform4x4f& transform) override;	

private:
	ImageComponent*		mImage;	
};

class VideoScreenSaver : public GameScreenSaverBase
{
public:
	VideoScreenSaver(Window* window);
	~VideoScreenSaver();

	void setVideo(const std::string path);
	void render(const Transform4x4f& transform) override;
	void update(int deltaTime) override;

private:
	VideoComponent*		mVideo;

	int mTime;
	float mFade;
};

// Clock screensaver class
class ClockScreenSaver : public GuiComponent
{
public:
	ClockScreenSaver(Window* window);
	~ClockScreenSaver();

	void render(const Transform4x4f& transform) override;
	void update(int deltaTime) override;

private:
	void refreshBattery();
	// Split the clock into the two cards and the AM/PM corner label.
	void applyTime(struct tm* t);
	// Center the charging line ("[battery] NN% Charged") on the date row.
	void layoutChargeLine();

	// The time is drawn as two flip-clock cards -- hours, then minutes -- with a
	// colon between them and a small AM/PM label in the first card's corner.
	TextComponent*		mLabelHour;
	TextComponent*		mLabelMinute;
	TextComponent*		mLabelColon;
	TextComponent*		mLabelMeridiem;
	TextComponent*		mLabelDate;
	int 				mDateTimeUpdateAccumulator;
	time_t				mDateTimeLastUpdate;

	// Small padlock icon centered at the top (the clock locks all input but Select).
	ImageComponent*		mLockImage;

	// While charging, the date line is replaced by a small battery icon + "NN%
	// Charged"; the large time stays visible below.
	ImageComponent*		mBattImage;
	TextComponent*		mBattLabel;
	std::string			mBattIconPath;
	bool				mCharging;
	int					mBattLevel;
	int					mBattCheckAccumulator;

	// cached layout geometry
	float				mScreenW;
	float				mScreenH;
	float				mDateY;
	float				mChargeIconW;
	float				mChargeGap;

	// flip-clock cards
	float				mCardX[2];
	float				mCardY;
	float				mCardW;
	float				mCardH;
	float				mCardRadius;
	float				mSplitH;
	float				mNotchW;
	float				mNotchH;
	unsigned int		mCardColor;
	unsigned int		mInkColor;
};

// Screensaver implementation for main window
class SystemScreenSaver : public Window::ScreenSaver
{
public:
	SystemScreenSaver(Window* window);
	virtual ~SystemScreenSaver();

	virtual void startScreenSaver();
	virtual void stopScreenSaver();
	virtual void nextVideo();
	virtual void renderScreenSaver();
	virtual bool allowSleep();
	virtual void update(int deltaTime);
	virtual bool isScreenSaverActive();

	virtual FileData* getCurrentGame();
	virtual void launchGame();
	inline virtual void resetCounts() { mVideosCounted = false; mImagesCounted = false; };

private:
	unsigned long countGameListNodes(const char *nodeName);
	void countVideos();
	void countImages();
    int curBrightnessLevel();

	std::string pickGameListNode(unsigned long index, const char *nodeName);
	std::string pickRandomVideo();
	std::string pickRandomGameListImage();
	std::string pickRandomCustomImage();

	enum STATE {
		STATE_INACTIVE,
		STATE_FADE_OUT_WINDOW,
		STATE_FADE_IN_VIDEO,
		STATE_SCREENSAVER_ACTIVE
	};

private:
	bool			mVideosCounted;
	unsigned long		mVideoCount;	
	bool			mImagesCounted;
	unsigned long		mImageCount;

	//VideoComponent*		mVideoScreensaver;
	std::shared_ptr<VideoScreenSaver>		mVideoScreensaver;

	std::shared_ptr<ImageScreenSaver>		mFadingImageScreensaver;
	std::shared_ptr<ImageScreenSaver>		mImageScreensaver;

	Window*			mWindow;
	STATE			mState;
	float			mOpacity;
	int				mTimer;
	FileData*		mCurrentGame;
	std::string		mGameName;
	std::string		mSystemName;
	int 			mVideoChangeTime;
	int             sysbrighttmp;

	//std::shared_ptr<Sound>	mBackgroundAudio;
	bool			mLoadingNext;

	random_device 	mRandomDevice;
	default_random_engine* mDefaultRandomEngine;

};

#endif // ES_APP_SYSTEM_SCREEN_SAVER_H
