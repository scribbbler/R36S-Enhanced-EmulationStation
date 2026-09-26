#include <string>
#include "components/SliderComponent.h"

#include "resources/Font.h"
#define MOVE_REPEAT_DELAY 500
#define MOVE_REPEAT_RATE 40

// Proportions taken from the Figma slider states, which are drawn in a 36x24
// box: a 2px track, a 10px knob, and the unfilled remainder at half strength.
#define KNOB_RATIO   (10.0f / 24.0f)
#define TRACK_RATIO  ( 2.0f / 24.0f)
#define TRACK_DIM    0.5f
#define SUFFIX_GAP   8.0f

SliderComponent::SliderComponent(Window* window, float min, float max, float increment, const std::string& suffix) : GuiComponent(window),
	mMin(min), mMax(max), mSingleIncrement(increment), mMoveRate(0), mKnob(window), mSuffix(suffix)
{
	assert((min - max) != 0);

	// some sane default value
	mValue = (max + min) / 2;

	auto menuTheme = ThemeData::getMenuTheme();
	mColor = menuTheme->Text.color;

	mKnob.setOrigin(0.5f, 0.5f);
	mKnob.setImage(ThemeData::getMenuTheme()->Icons.knob); // ":/slider_knob.svg");
	mKnob.setColorShift(mColor);

	// Width comes from the row (see wantsRowWidth); this is only a starting
	// value. Height matches the rest of the menu chrome.
	setSize(Renderer::getScreenWidth() * 0.15f, MENU_ICON_HEIGHT(menuTheme->Text.font));
}

void SliderComponent::setColor(unsigned int color) {
	mColor = color;
	mKnob.setColorShift(mColor);
	onValueChanged();
}

bool SliderComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedLike("left", input))
	{
		if(input.value)
			setValue(mValue - mSingleIncrement);

		mMoveRate = input.value ? -mSingleIncrement : 0;
		mMoveAccumulator = -MOVE_REPEAT_DELAY;
		return input.value;
	}
	if(config->isMappedLike("right", input))
	{
		if(input.value)
			setValue(mValue + mSingleIncrement);

		mMoveRate = input.value ? mSingleIncrement : 0;
		mMoveAccumulator = -MOVE_REPEAT_DELAY;
		return input.value;
	}

	return GuiComponent::input(config, input);
}

void SliderComponent::update(int deltaTime)
{
	if(mMoveRate != 0)
	{
		mMoveAccumulator += deltaTime;
		while(mMoveAccumulator >= MOVE_REPEAT_RATE)
		{
			setValue(mValue + mMoveRate);
			mMoveAccumulator -= MOVE_REPEAT_RATE;
		}
	}
	
	GuiComponent::update(deltaTime);
}

void SliderComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	// render suffix
	if(mValueCache)
		mFont->renderTextCache(mValueCache.get());

	// The track runs the width the row gave us, less whatever the value label
	// occupies, and is inset by the knob's radius at each end so the knob stays
	// inside the control at both extremes.
	const float knobW  = mKnob.getSize().x();
	const float radius = knobW / 2.0f;
	const float avail  = mSize.x() - (mValueCache ? mValueCache->metrics.size.x() + SUFFIX_GAP : 0);
	const float trackX = radius;
	const float trackW = Math::max(0.0f, avail - knobW);
	const float trackH = Math::max(2.0f, Math::round(mSize.y() * TRACK_RATIO));
	const float trackY = Math::round((mSize.y() - trackH) / 2.0f);

	// Two passes: the whole track dimmed, then the part behind the knob solid,
	// so the control reads its own value rather than relying on knob position.
	const float pct = (mMax - mMin) != 0 ? (mValue - mMin) / (mMax - mMin) : 0.0f;
	const unsigned int dim = (mColor & 0xFFFFFF00) | (unsigned char)((mColor & 0xFF) * TRACK_DIM);

	Renderer::drawRoundRect(trackX, trackY, trackW, trackH, trackH / 2.0f, dim);
	if (pct > 0.0f)
		Renderer::drawRoundRect(trackX, trackY, Math::round(trackW * pct), trackH, trackH / 2.0f, mColor);

	//render knob
	mKnob.render(trans);
	
	GuiComponent::renderChildren(trans);
}

void SliderComponent::setValue(float value)
{
	if (mValue == value)
		return;

	mValue = value;
	if(mValue < mMin)
		mValue = mMin;
	else if(mValue > mMax)
		mValue = mMax;

	onValueChanged();

	if (mValueChanged)
		mValueChanged(mValue);
}

float SliderComponent::getValue()
{
	return mValue;
}

void SliderComponent::onSizeChanged()
{
	if(!mSuffix.empty())
		// The theme's own menu font, so the value reads as part of the row.
		// This used to be Font::get(mSize.y(), FONT_PATH_LIGHT) -- a size taken
		// from the control's height and a light condensed face bundled with ES,
		// which is why "560Mb" came out bigger than the label beside it and in
		// a different weight.
		mFont = ThemeData::getMenuTheme()->Text.font;
	
	onValueChanged();
}

void SliderComponent::onValueChanged()
{
	// update suffix textcache
	if (mFont)
	{
		std::stringstream ss;
		ss << std::fixed;
		ss.precision(0);
		ss << mValue;
		ss << mSuffix;
		const std::string val = ss.str();

		ss.str("");
		ss.clear();
		ss << std::fixed;
		ss.precision(0);
		ss << mMax;
		ss << mSuffix;
		const std::string max = ss.str();

		Vector2f textSize = mFont->sizeText(max);
		mValueCache = std::shared_ptr<TextCache>(mFont->buildTextCache(val, mSize.x() - textSize.x(), (mSize.y() - textSize.y()) / 2, mColor));
		mValueCache->metrics.size[0] = textSize.x(); // fudge the width
	}

	// update knob position/size -- the same track the render uses, so the knob
	// sits on it. The old position was (value + min) / max, which only lands
	// correctly when min is zero.
	mKnob.setResize(0, Math::round(mSize.y() * KNOB_RATIO));
	const float pct = (mMax - mMin) != 0 ? (mValue - mMin) / (mMax - mMin) : 0.0f;
	const float avail = mSize.x() - (mValueCache ? mValueCache->metrics.size.x() + SUFFIX_GAP : 0);
	const float travel = Math::max(0.0f, avail - mKnob.getSize().x());
	mKnob.setPosition(Math::round(mKnob.getSize().x() / 2.0f + pct * travel), Math::round(mSize.y() / 2.0f));

}

std::vector<HelpPrompt> SliderComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("left/right", _("CHANGE")));
	return prompts;
}
