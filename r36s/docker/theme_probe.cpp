// Probe: load a theme with the real ThemeData code and dump the
// screen/batteryIndicator element the way Window::onThemeChanged sees it.
#include <iostream>
#include <map>
#include "ThemeData.h"
#include "resources/ResourceManager.h"

int main(int argc, char** argv)
{
	if (argc < 2) { std::cout << "usage: theme_probe <theme.xml>\n"; return 1; }

	std::map<std::string, std::string> sysData;
	sysData["system.theme"] = "psx";
	sysData["system.name"] = "psx";
	sysData["system.fullName"] = "PlayStation";

	ThemeData theme;
	try {
		theme.loadFile("psx", sysData, argv[1]);
	} catch (std::exception& e) {
		std::cout << "PARSE-ERROR: " << e.what() << "\n";
		return 2;
	}
	std::cout << "theme parsed OK\n";

	for (const char* view : { "screen" })
	{
		auto elem = theme.getElement(view, "batteryIndicator", "batteryIndicator");
		if (!elem) { std::cout << view << "/batteryIndicator: ELEMENT NOT FOUND\n"; continue; }
		std::cout << view << "/batteryIndicator: found, properties:\n";
		for (auto& p : elem->properties)
		{
			std::cout << "  " << p.first << " = ";
			if (!p.second.s.empty()) std::cout << p.second.s;
			std::cout << "\n";
			if (p.first == "incharge" || p.first == "full" || p.first == "empty"
				|| p.first == "at75" || p.first == "at50" || p.first == "at25")
			{
				bool ex = ResourceManager::getInstance()->fileExists(p.second.s);
				std::cout << "      fileExists: " << (ex ? "YES" : "NO") << "\n";
			}
		}
	}

	auto clock = theme.getElement("screen", "clock", "text");
	std::cout << "screen/clock: " << (clock ? "found" : "NOT FOUND") << "\n";
	return 0;
}
