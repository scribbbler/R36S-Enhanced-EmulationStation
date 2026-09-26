#include <string>
#include "components/HelpComponent.h"

#include "components/ComponentGrid.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "resources/TextureResource.h"
#include "renderers/Renderer.h"
#include "utils/StringUtil.h"
#include "Log.h"
#include "Settings.h"

#define OFFSET_X 12 // move the entire thing right by this amount (px)
#define OFFSET_Y 12 // move the entire thing up by this amount (px)

#define ICON_TEXT_SPACING 8 // space between [icon] and [text] (px)
#define ENTRY_SPACING 16 // space between [text] and next [icon] (px)

static const std::map<std::string, const char*> ICON_PATH_MAP {
	{ "up/down", ":/help/dpad_updown.svg" },
	{ "left/right", ":/help/dpad_leftright.svg" },
	{ "up/down/left/right", ":/help/dpad_all.svg" },
	{ "a", ":/help/button_a.svg" },
	{ "b", ":/help/button_b.svg" },
	{ "x", ":/help/button_x.svg" },
	{ "y", ":/help/button_y.svg" },
	{ "l", ":/help/button_l.svg" },
	{ "r", ":/help/button_r.svg" },
	{ "lr", ":/help/button_lr.svg" },
	{ "start", ":/help/button_start.svg" },
	{ "select", ":/help/button_select.svg" }
};

HelpComponent::HelpComponent(Window* window) : GuiComponent(window)
{
}

void HelpComponent::clearPrompts()
{
	mPrompts.clear();
	updateGrid();
}

void HelpComponent::setPrompts(const std::vector<HelpPrompt>& prompts)
{
	mPrompts = prompts;
	updateGrid();
}

void HelpComponent::setStyle(const HelpStyle& style)
{
	mStyle = style;
	updateGrid();
}

// build one grid (icon+label row) from a subset of prompts; caller positions it
std::shared_ptr<ComponentGrid> HelpComponent::buildGrid(const std::vector<HelpPrompt>& prompts)
{
	std::shared_ptr<Font>& font = mStyle.font;

	auto grid = std::make_shared<ComponentGrid>(mWindow, Vector2i((int)prompts.size() * 4, 1));
	// [icon] [spacer1] [text] [spacer2]

	std::vector< std::shared_ptr<ImageComponent> > icons;
	std::vector< std::shared_ptr<TextComponent> > labels;

	float width = 0;
	const float rowHeight = Math::round(font->getLetterHeight() * 1.25f);
	const float iconH = (mStyle.iconSize > 0.0f) ? mStyle.iconSize : rowHeight; // themeable icon size
	const float gridH = Math::max(rowHeight, iconH);
	for(auto it = prompts.cbegin(); it != prompts.cend(); it++)
	{
		auto icon = std::make_shared<ImageComponent>(mWindow);

		// Ask for the display height BEFORE loading. setImage rasterises at
		// whatever size the component is when it runs, and TextureData's
		// setSourceSize only re-rasterises when the size GROWS -- so resizing
		// afterwards leaves a 96px icon at 96px and lets GL minify it to 32,
		// with GL_LINEAR and no mipmaps. That is what made the icons ragged,
		// and it happened to SVG and PNG alike.
		icon->setResize(0, iconH);

		if (mStyle.iconMap.find(it->first) != mStyle.iconMap.end() && Utils::FileSystem::exists(mStyle.iconMap[it->first]))
			icon->setImage(mStyle.iconMap[it->first], false, MaxSizeInfo(iconH, iconH));
		else
			icon->setImage(getIconTexture(it->first.c_str()));

		icon->setColorShift(mStyle.iconColor);
		icon->setResize(0, iconH);
		icons.push_back(icon);

		// theme label override (verbatim) if provided, else the system label (case-transformed)
		std::string labelText;
		auto lm = mStyle.labelMap.find(it->first);
		if(lm != mStyle.labelMap.end())
			labelText = lm->second;
		else
			labelText = mStyle.uppercase ? Utils::String::toUpper(it->second) : it->second;

		auto lbl = std::make_shared<TextComponent>(mWindow, labelText, font, mStyle.textColor);
		// A chip is one line, so the default 1.5 line spacing is pure leading --
		// and it is split unevenly around the glyphs, which left every label
		// sitting a pixel low once the grid centred the box rather than the ink.
		lbl->setLineSpacing(1.0f);
		labels.push_back(lbl);

		// entrySpacing separates one entry from the NEXT, so the last entry does
		// not get one -- counting it there padded the right of every pill by a
		// whole entry gap while the left kept only its background padding.
		width += icon->getSize().x() + lbl->getSize().x() + mStyle.iconTextSpacing;
		if (it + 1 != prompts.cend())
			width += mStyle.entrySpacing;
	}

	grid->setSize(width, gridH);
	for(unsigned int i = 0; i < icons.size(); i++)
	{
		const int col = i*4;
		grid->setColWidthPerc(col, icons.at(i)->getSize().x() / width);
		grid->setColWidthPerc(col + 1, mStyle.iconTextSpacing / width);
		grid->setColWidthPerc(col + 2, labels.at(i)->getSize().x() / width);
		grid->setColWidthPerc(col + 3, (i + 1 < icons.size() ? mStyle.entrySpacing : 0.0f) / width);

		grid->setEntry(icons.at(i), Vector2i(col, 0), false, false);
		grid->setEntry(labels.at(i), Vector2i(col + 2, 0), false, false);
	}
	return grid;
}

void HelpComponent::updateGrid()
{
	if(!Settings::getInstance()->getBool("ShowHelpPrompts") || !mStyle.visible || mPrompts.empty())
	{
		mGrid.reset();
		mGridRight.reset();
		return;
	}

	// Split into a left group (system / navigation) and a right group (action
	// buttons), each rendered as its own pill.
	std::vector<HelpPrompt> leftPrompts, rightPrompts;
	for(const auto& p : mPrompts)
	{
		const std::string& b = p.first;
		// v2 design: X/Y live in the LEFT pill; only A/B (and shoulder buttons)
		// go to the right pill.
		bool action = (b == "a" || b == "b" || b == "l" || b == "r");
		if(action)
			rightPrompts.push_back(p);
		else
			leftPrompts.push_back(p);
	}

	mGrid = leftPrompts.empty() ? nullptr : buildGrid(leftPrompts);
	mGridRight = rightPrompts.empty() ? nullptr : buildGrid(rightPrompts);

	// The two pills are placed from opposite edges and neither knows about the
	// other, so a long row runs them into each other and the left pill's last
	// label is cut mid-word. Drop trailing left-hand prompts - the ones the
	// view added last - until both pills fit side by side. Losing a prompt
	// reads better than half a word underneath another one.
	if(mGrid && mGridRight && mStyle.backgroundWidth <= 0.0f)
	{
		const float screenWidth = Renderer::getScreenWidth();
		const float sideMargin  = mStyle.position.x();
		const float minGap      = (mStyle.entrySpacing > 0.0f) ? mStyle.entrySpacing : ENTRY_SPACING;

		while(leftPrompts.size() > 1 &&
		      sideMargin + mGrid->getSize().x() + minGap
		        + mGridRight->getSize().x() + sideMargin > screenWidth)
		{
			leftPrompts.pop_back();
			mGrid = buildGrid(leftPrompts);
		}
	}

	// Explicit left-aligned positioning (origin 0) so nothing relies on grid
	// origin-anchoring. Left pill hugs the left margin; right pill hugs the
	// right margin. 'margin' (theme pos.x) is used on both sides.
	const float screenW = Renderer::getScreenWidth();
	const float margin  = mStyle.position.x();
	const float posY    = mStyle.position.y();
	const float padX    = mStyle.backgroundPadding.x();
	const float W       = mStyle.backgroundWidth;   // fixed pill width (px); 0 => content

	const float padY = mStyle.backgroundPadding.y();
	if(mGrid)
	{
		mGrid->setOrigin(0.0f, 0.0f);
		// fixed-pill mode: bottom-anchor so the pill's bottom edge sits at pos.y
		float y = (W > 0.0f) ? (posY - mGrid->getSize().y() - padY) : posY;
		mGrid->setPosition(Vector3f((W > 0.0f) ? (margin + padX) : margin, y, 0.0f));
	}
	if(mGridRight)
	{
		mGridRight->setOrigin(0.0f, 0.0f);
		float x = (W > 0.0f) ? (screenW - margin - W + padX)
		                     : (screenW - margin - mGridRight->getSize().x());
		float y = (W > 0.0f) ? (posY - mGridRight->getSize().y() - padY) : posY;
		mGridRight->setPosition(Vector3f(x, y, 0.0f));
	}
}

std::shared_ptr<TextureResource> HelpComponent::getIconTexture(const char* name)
{
	auto it = mIconCache.find(name);
	if(it != mIconCache.cend())
		return it->second;

	auto pathLookup = ICON_PATH_MAP.find(name);
	if(pathLookup == ICON_PATH_MAP.cend())
	{
		LOG(LogError) << "Unknown help icon \"" << name << "\"!";
		return nullptr;
	}
	if(!ResourceManager::getInstance()->fileExists(pathLookup->second))
	{
		LOG(LogError) << "Help icon \"" << name << "\" - corresponding image file \"" << pathLookup->second << "\" misisng!";
		return nullptr;
	}

	std::shared_ptr<TextureResource> tex = TextureResource::get(pathLookup->second);
	mIconCache[std::string(name)] = tex;
	return tex;
}

void HelpComponent::setOpacity(unsigned char opacity)
{
	GuiComponent::setOpacity(opacity);

	std::shared_ptr<ComponentGrid> grids[2] = { mGrid, mGridRight };
	for(auto& g : grids)
	{
		if(!g)
			continue;
		for(unsigned int i = 0; i < g->getChildCount(); i++)
			g->getChild(i)->setOpacity(opacity);
	}
}

void HelpComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();

	const float screenW = Renderer::getScreenWidth();
	const float margin  = mStyle.position.x();
	const float W       = mStyle.backgroundWidth;
	const float padX    = mStyle.backgroundPadding.x();
	const float padY    = mStyle.backgroundPadding.y();
	const bool  haveBg  = (mStyle.backgroundColor & 0xFF) != 0;

	// pill color faded with the bar's own opacity
	unsigned int a  = (mStyle.backgroundColor & 0xFF) * getOpacity() / 255;
	unsigned int bg = (mStyle.backgroundColor & 0xFFFFFF00) | (a & 0xFF);

	// isRight == false -> left pill hugs the left margin; true -> right margin
	auto drawGroup = [&](std::shared_ptr<ComponentGrid>& g, bool isRight)
	{
		if(!g)
			return;
		if(haveBg)
		{
			Vector3f gp = g->getPosition();
			Vector2f gs = g->getSize();
			float by = gp.y() - padY;
			float bh = gs.y() + 2.0f * padY;
			float bx, bw;
			if(W > 0.0f)
			{
				bw = W;
				bx = isRight ? (screenW - margin - W) : margin;
			}
			else
			{
				bw = gs.x() + 2.0f * padX;
				bx = gp.x() - padX;
			}
			Renderer::setMatrix(trans);
			Renderer::drawRoundRect(bx, by, bw, bh, mStyle.backgroundRadius, bg);
		}
		g->render(trans);
	};

	drawGroup(mGrid, false);
	drawGroup(mGridRight, true);
}
