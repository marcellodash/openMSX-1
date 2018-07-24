#include "RPMCSlot.hh"
#include "MSXMotherBoard.hh"
#include "HardwareConfig.hh"
#include "XMLElement.hh"
#include "MSXException.hh"
#include "Math.hh"
#include "StringOp.hh"
#include "outer.hh"
#include "serialize.hh"
#include "stl.hh"
#include "MsxBusPi.h"
#include <iostream>
#include <fstream>
 
using namespace::std;

namespace openmsx {

static byte calcEngineMask(MSXMotherBoard& motherBoard)
{
	string_view  type = motherBoard.getMachineConfig()->getConfig().getChildData(
	                               "MapperReadBackBits", "largest");
	if (type == "5") {
		return 0xE0; // upper 3 bits always read "1"
	} else if (type == "largest") {
		return 0x00; // all bits can be read
	} else if (type == "RPMCSlot") {
		return 0x00;
	} else {
		throw FatalError("Unknown mapper type: \"", type, "\"."); 
	}
}

RPMCSlot::RPMCSlot(const DeviceConfig& config)
	: MSXDevice(config)
	, debuggable(getMotherBoard(), getName())
	, engineMask(calcEngineMask(getMotherBoard()))
{
	//updateMask();
	reset(EmuTime::dummy());
	cout << "RPMCSlost start" << endl;
    // open the file:
    std::streampos fileSize;
    std::ifstream file("/home/pi/msxslot/fdc.rom", std::ios::binary);

    // get its size:
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read the data:
	cout << "filesize:" << fileSize << endl;
    file.read((char*) &rom[0], fileSize);
	msxinit();
}

void RPMCSlot::updateMask()
{
	// unused bits always read "1"
	unsigned largest = (mapperSizes.empty()) ? 1 : mapperSizes.back();
	mask = ((256 - Math::powerOfTwo(largest)) & 255) | engineMask;
}

void RPMCSlot::registerMapper(unsigned blocks)
{
	auto it = upper_bound(begin(mapperSizes), end(mapperSizes), blocks);
	mapperSizes.insert(it, blocks);
	updateMask();
}

void RPMCSlot::unregisterMapper(unsigned blocks)
{
	mapperSizes.erase(rfind_unguarded(mapperSizes, blocks));
	updateMask();
}

void RPMCSlot::reset(EmuTime::param /*time*/)
{
	// TODO in what state is mapper after reset?
	// Zeroed is most likely.
	// To find out for real, insert an external memory mapper on an MSX1.
	for (auto& reg : registers) {
		reg = 0;
	}
}

byte RPMCSlot::readIO(word port, EmuTime::param time)
{
	byte b = msxreadio(port);
	printf("Rport%02x:%02x\n", 0xff & port, b);
	return b;
}

byte RPMCSlot::peekIO(word port, EmuTime::param /*time*/) const
{
	return getSelectedPage(port & 0x03) | mask;
}

void RPMCSlot::writeIO(word port, byte value, EmuTime::param /*time*/)
{
	msxwriteio(port, value);
	printf("Wport%02x:%02x\n", 0xff & port, value);
}

void RPMCSlot::write(unsigned address, byte value)
{
//	registers[address] = value;
//	invalidateMemCache(0x4000 * address, 0x4000);
}

byte RPMCSlot::peekMem(word address, EmuTime::param time)
{
	return 0xff;
}

void RPMCSlot::writeMem(word address, byte value, EmuTime::param time)
{
	msxwrite(1, address, value);
//	printf("M%04x:%02xw\n", address, value);
}

byte RPMCSlot::readMem(word address, EmuTime::param time)
{
	return msxread(1, address);
}


// SimpleDebuggable

RPMCSlot::Debuggable::Debuggable(MSXMotherBoard& motherBoard_,
                                    const std::string& name_)
	: SimpleDebuggable(motherBoard_, name_, "Memory mapper registers", 4)
{
}

byte RPMCSlot::Debuggable::read(unsigned address)
{
	auto& mapperIO = OUTER(RPMCSlot, debuggable);
	return mapperIO.getSelectedPage(address);
}

void RPMCSlot::Debuggable::write(unsigned address, byte value)
{
	auto& mapperIO = OUTER(RPMCSlot, debuggable);
	mapperIO.write(address, value);
}


template<typename Archive>
void RPMCSlot::serialize(Archive& ar, unsigned /*version*/)
{
	ar.serialize("registers", registers);
	// all other state is reconstructed in another way
}
INSTANTIATE_SERIALIZE_METHODS(RPMCSlot);
REGISTER_MSXDEVICE(RPMCSlot, "RPMC_Slot");
} // namespace openmsx
