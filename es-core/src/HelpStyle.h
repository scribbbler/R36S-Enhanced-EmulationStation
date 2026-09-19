#pragma once
#ifndef ES_CORE_HELP_STYLE_H
#define ES_CORE_HELP_STYLE_H

#include "math/Vector2f.h"
#include <memory>
#include <string>
#include <map>

class Font;
class ThemeData;

struct HelpStyle
{
	Vector2f position;
	Vector2f origin;
	unsigned int iconColor;
	unsigned int textColor;
	std::shared_ptr<Font> font;
	std::map<std::string, std::string> iconMap;
	std::map<std::string, std::string> labelMap;  // per-button label override (else system label)

	// extended, themeable look (all default to the classic behavior)
	float iconTextSpacing;          // px gap between an icon and its label
	float entrySpacing;             // px gap between entries
	float iconSize;                 // px icon height (<=0 => auto from font)
	bool  uppercase;                // force UPPERCASE labels
	unsigned int backgroundColor;   // rounded background behind the bar (alpha 0 => none)
	float backgroundRadius;         // px corner radius of that background
	Vector2f backgroundPadding;     // px padding (x,y) around the content inside the background
	float backgroundWidth;          // fixed pill width in px (<=0 => size to content)
	bool  visible;                  // hide the whole help bar for this view when false

	HelpStyle(); // default values
	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view);
};

#endif // ES_CORE_HELP_STYLE_H
