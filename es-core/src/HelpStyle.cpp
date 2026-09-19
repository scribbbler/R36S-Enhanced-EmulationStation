#include <string>
#include "HelpStyle.h"

#include "resources/Font.h"

HelpStyle::HelpStyle()
{
	position = Vector2f(Renderer::getScreenWidth() * 0.012f, Renderer::getScreenHeight() * 0.9515f);
	origin = Vector2f(0.0f, 0.0f);
	iconColor = 0x777777FF;
	textColor = 0x777777FF;
	font = nullptr;

	// classic-behavior defaults
	iconTextSpacing = 8.0f;
	entrySpacing = 16.0f;
	iconSize = 0.0f;               // auto (from font height)
	uppercase = true;
	backgroundColor = 0x00000000;  // no background
	backgroundRadius = 0.0f;
	backgroundPadding = Vector2f(0.0f, 0.0f);
	backgroundWidth = 0.0f;        // content width
	visible = true;

	if (FONT_SIZE_SMALL != 0)
		font = Font::get(FONT_SIZE_SMALL);
}

void HelpStyle::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view)
{
	auto elem = theme->getElement(view, "help", "helpsystem");
	if(!elem)
		return;

	if(elem->has("pos"))
		position = elem->get<Vector2f>("pos") * Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	if(elem->has("origin"))
		origin = elem->get<Vector2f>("origin");

	if(elem->has("textColor"))
		textColor = elem->get<unsigned int>("textColor");

	if(elem->has("iconColor"))
		iconColor = elem->get<unsigned int>("iconColor");

	if(elem->has("fontPath") || elem->has("fontSize"))
		font = Font::getFromTheme(elem, ThemeFlags::ALL, font);

	// extended look (fractions of the screen, like other theme sizes)
	if(elem->has("iconTextSpacing"))
		iconTextSpacing = elem->get<float>("iconTextSpacing") * Renderer::getScreenWidth();

	if(elem->has("entrySpacing"))
		entrySpacing = elem->get<float>("entrySpacing") * Renderer::getScreenWidth();

	if(elem->has("iconSize"))
		iconSize = elem->get<float>("iconSize") * Renderer::getScreenHeight();

	if(elem->has("textUppercase"))
		uppercase = elem->get<bool>("textUppercase");

	if(elem->has("backgroundColor"))
		backgroundColor = elem->get<unsigned int>("backgroundColor");

	if(elem->has("backgroundRadius"))
		backgroundRadius = elem->get<float>("backgroundRadius") * Renderer::getScreenHeight();

	if(elem->has("backgroundPadding"))
		backgroundPadding = elem->get<Vector2f>("backgroundPadding") *
			Vector2f((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());

	if(elem->has("backgroundWidth"))
		backgroundWidth = elem->get<float>("backgroundWidth") * Renderer::getScreenWidth();

	if(elem->has("visible"))
		visible = elem->get<bool>("visible");

	if (elem->has("iconUpDown"))
		iconMap["up/down"] = elem->get<std::string>("iconUpDown");

	if (elem->has("iconLeftRight"))
		iconMap["left/right"] = elem->get<std::string>("iconLeftRight");

	if (elem->has("iconUpDownLeftRight"))
		iconMap["up/down/left/right"] = elem->get<std::string>("iconUpDownLeftRight");

	if (elem->has("iconA"))
		iconMap["a"] = elem->get<std::string>("iconA");

	if (elem->has("iconB"))
		iconMap["b"] = elem->get<std::string>("iconB");

	if (elem->has("iconX"))
		iconMap["x"] = elem->get<std::string>("iconX");

	if (elem->has("iconY"))
		iconMap["y"] = elem->get<std::string>("iconY");

	if (elem->has("iconL"))
		iconMap["l"] = elem->get<std::string>("iconL");

	if (elem->has("iconR"))
		iconMap["r"] = elem->get<std::string>("iconR");

	if (elem->has("iconStart"))
		iconMap["start"] = elem->get<std::string>("iconStart");

	if (elem->has("iconSelect"))
		iconMap["select"] = elem->get<std::string>("iconSelect");

	// per-button label overrides (used verbatim; fall back to the system label)
	if (elem->has("labelA"))          labelMap["a"] = elem->get<std::string>("labelA");
	if (elem->has("labelB"))          labelMap["b"] = elem->get<std::string>("labelB");
	if (elem->has("labelX"))          labelMap["x"] = elem->get<std::string>("labelX");
	if (elem->has("labelY"))          labelMap["y"] = elem->get<std::string>("labelY");
	if (elem->has("labelL"))          labelMap["l"] = elem->get<std::string>("labelL");
	if (elem->has("labelR"))          labelMap["r"] = elem->get<std::string>("labelR");
	if (elem->has("labelStart"))      labelMap["start"] = elem->get<std::string>("labelStart");
	if (elem->has("labelSelect"))     labelMap["select"] = elem->get<std::string>("labelSelect");
	if (elem->has("labelUpDown"))     labelMap["up/down"] = elem->get<std::string>("labelUpDown");
	if (elem->has("labelLeftRight"))  labelMap["left/right"] = elem->get<std::string>("labelLeftRight");
	if (elem->has("labelUpDownLeftRight")) labelMap["up/down/left/right"] = elem->get<std::string>("labelUpDownLeftRight");
}
