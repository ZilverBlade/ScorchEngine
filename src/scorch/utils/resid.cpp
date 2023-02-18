#include "resid.h"
#include <xhash>
#include <algorithm>
namespace ScorchEngine {
	static std::hash<std::string> hasher;
	static std::string& convert(std::string& orig) { 
		std::replace(orig.begin(), orig.end(), '\\', '/'); 
		std::transform(orig.begin(), orig.end(), orig.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return orig; 
	}
	ResourceID::ResourceID(const std::string& asset) : 
		file(convert(const_cast<std::string&>(asset))), 
		id(hasher(convert(const_cast<std::string&>(asset)))) 
	{
	}
}