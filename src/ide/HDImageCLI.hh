// $Id$

#ifndef HDIMAGECLI_HH
#define HDIMAGECLI_HH

#include "CLIOption.hh"

namespace openmsx {

class CommandController;
class CommandLineParser;

class HDImageCLI : public CLIOption
{
public:
	explicit HDImageCLI(CommandLineParser& cmdLineParser);
	virtual bool parseOption(const std::string& option,
	                         std::deque<std::string>& cmdLine);
	virtual const std::string& optionHelp() const;

private:
	CommandController& commandController;
};

} // namespace openmsx

#endif
