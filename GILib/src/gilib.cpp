#include "gilib.h"

#include <sstream>

using namespace std;

std::vector<string> gi_lib::Split(const std::string& source, char delimiter) {

	std::stringstream source_stream(source);

	std::string item;

	vector<string> elements;

	while (std::getline(source_stream, item, delimiter)) {

		elements.push_back(item);

	}

	return elements;

}