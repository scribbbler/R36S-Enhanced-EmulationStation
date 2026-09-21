#include <string>
#pragma once
#ifndef ES_APP_VIEWS_SYSTEM_VIEW_H
#define ES_APP_VIEWS_SYSTEM_VIEW_H

#include "components/IList.h"
#include "components/TextComponent.h"
#include "resources/Font.h"
#include "GuiComponent.h"
#include <memory>
#include <functional>

class ClockScreenSaver;

class AnimatedImageComponent;
class SystemData;
class VideoVlcComponent;

enum CarouselType : unsigned int
{
	HORIZONTAL = 0,
	VERTICAL = 1,
	VERTICAL_WHEEL = 2,
	HORIZONTAL_WHEEL = 3
};

struct SystemViewData
{	
	std::shared_ptr<GuiComponent> logo;
	std::vector<GuiComponent*> backgroundExtras;
};

struct SystemViewCarousel
{
	CarouselType type;
	Vector2f pos;
	Vector2f size;
	Vector2f origin;
	float logoScale;
	float logoRotation;
	Vector2f logoRotationOrigin;
	Alignment logoAlignment;
	unsigned int color;
	unsigned int colorEnd;
	bool colorGradientHorizontal;
	int maxLogoCount; // number of logos shown on the carousel
	Vector2f logoSize;
	Vector2f logoPos;
	float zIndex;
	float systemInfoDelay;

	unsigned int selectorColor;  // pill behind the selected system; alpha 0 = disabled
	unsigned int selectorColorEnd; // bottom color for a vertical gradient (== selectorColor -> solid)
	float selectorRadius;        // pill corner radius in px; <0 = auto (half height)
	float selectorHeight;        // pill height in px; <0 = auto (logoSize.y * logoScale)
	float selectorPadding;       // fit-content pill: horizontal pad around the text in px; <0 = auto (0.30 * pill height)
	bool  selectorFitContent;    // true = pill sized/positioned to the item box (left list); false = full-width centered
	bool  listScroll;            // true = gamelist-style navigation: cursor walks the slots, camera clamps at list ends
	float selectorWidth;         // fixed pill width in px; <0 = auto (fit-content or full-width)
	unsigned int logoColor;         // tint for unselected logos (used only if logoSelectedColor set)
	unsigned int logoSelectedColor; // tint for the selected logo; 0 = disabled (no tinting)

	std::string		defaultTransition;
	std::string		scrollSound;
};

class SystemView : public IList<SystemViewData, SystemData*>
{
public:
	SystemView(Window* window);
	~SystemView();

	virtual void onShow() override;
	virtual void onHide() override;

	void goToSystem(SystemData* system, bool animate);

	bool input(InputConfig* config, Input input) override;
	void showNavigationBar(const std::string& title, const std::function<std::string(SystemData* system)>& selector);
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

	void onThemeChanged(const std::shared_ptr<ThemeData>& theme);

	std::vector<HelpPrompt> getHelpPrompts() override;
	virtual HelpStyle getHelpStyle() override;

	// Clock screensaver (Select toggles it). While active every button is
	// locked except Select; ViewController enforces the lock so it also
	// catches Start (which it handles before delegating to the view).
	bool isClockSaverActive() const { return mClockSaverActive; }
	void setClockSaverActive(bool active);

protected:
	void onCursorChanged(const CursorState& state) override;

private:
	void	 activateExtras(int cursor, bool activate = true);	
	void	 updateExtras(const std::function<void(GuiComponent*)>& func);
	void	 clearEntries();
	void	 showQuickSearch();

	virtual void onScreenSaverActivate() override;
	virtual void onScreenSaverDeactivate() override;
	virtual void topWindow(bool isTop) override;

	void populate();
	void getViewElements(const std::shared_ptr<ThemeData>& theme);
	void getDefaultElements(void);
	void getCarouselFromTheme(const ThemeData::ThemeElement* elem);

	void renderCarousel(const Transform4x4f& parentTrans);
	void renderExtras(const Transform4x4f& parentTrans, float lower, float upper);
	void renderInfoBar(const Transform4x4f& trans);
	void renderFade(const Transform4x4f& trans);


	SystemViewCarousel	mCarousel;
	TextComponent		mSystemInfo;
	SystemData*			mLastSystem;
	ImageComponent*		mStaticBackground;
	VideoVlcComponent*	mStaticVideoBackground;

	// unit is list index
	float mCamOffset;
	float mExtrasCamOffset;
	float mExtrasFadeOpacity;
	int	  mExtrasFadeOldCursor;

	bool mViewNeedsReload;
	bool mShowing;
	bool mDisable;
	bool mScreensaverActive;

	int mLastCursor;

	ClockScreenSaver* mClockScreenSaver;
	bool mClockSaverActive;
};

#endif // ES_APP_VIEWS_SYSTEM_VIEW_H
