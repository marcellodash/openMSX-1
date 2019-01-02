#ifndef REALDRIVE_HH
#define REALDRIVE_HH

#include "DiskDrive.hh"
#include "Clock.hh"
#include "Schedulable.hh"
#include "ThrottleManager.hh"
#include "outer.hh"
#include "serialize_meta.hh"
#include <bitset>
#include <memory>

namespace openmsx {

class MSXMotherBoard;
class DiskChanger;

/** This class implements a real drive, single or double sided.
 */
class RealDrive final : public DiskDrive
{
public:
	RealDrive(MSXMotherBoard& motherBoard, EmuDuration::param motorTimeout,
	          bool signalsNeedMotorOn, bool doubleSided);
	~RealDrive() override;

	// DiskDrive interface
	bool isDiskInserted() const override;
	bool isWriteProtected() const override;
	bool isDoubleSided() const override;
	bool isTrack00() const override;
	void setSide(bool side) override;
	void step(bool direction, EmuTime::param time) override;
	void setMotor(bool status, EmuTime::param time) override;
	bool indexPulse(EmuTime::param time) override;
	EmuTime getTimeTillIndexPulse(EmuTime::param time, int count) override;

	unsigned getTrackLength() override;
	void writeTrackByte(int idx, byte val, bool addIdam) override;
	byte  readTrackByte(int idx) override;
	EmuTime getNextSector(EmuTime::param time, RawTrack::Sector& sector) override;
	void flushTrack() override;
	bool diskChanged() override;
	bool peekDiskChanged() const override;
	bool isDummyDrive() const override;

	void applyWd2793ReadTrackQuirk() override;
	void invalidateWd2793ReadTrackQuirk() override;

	template<typename Archive>
	void serialize(Archive& ar, unsigned version);

private:
	struct SyncLoadingTimeout : Schedulable {
		friend class RealDrive;
		explicit SyncLoadingTimeout(Scheduler& s) : Schedulable(s) {}
		void executeUntil(EmuTime::param /*time*/) override {
			auto& drive = OUTER(RealDrive, syncLoadingTimeout);
			drive.execLoadingTimeout();
		}
	} syncLoadingTimeout;

	struct SyncMotorTimeout : Schedulable {
		friend class RealDrive;
		explicit SyncMotorTimeout(Scheduler& s) : Schedulable(s) {}
		void executeUntil(EmuTime::param time) override {
			auto& drive = OUTER(RealDrive, syncMotorTimeout);
			drive.execMotorTimeout(time);
		}
	} syncMotorTimeout;

	void execLoadingTimeout();
	void execMotorTimeout(EmuTime::param time);
	EmuTime::param getCurrentTime() const { return syncLoadingTimeout.getCurrentTime(); }

	void doSetMotor(bool status, EmuTime::param time);
	void setLoading(EmuTime::param time);
	unsigned getCurrentAngle(EmuTime::param time) const;

	void getTrack();
	void invalidateTrack();

	static const unsigned MAX_TRACK = 85;
	static const unsigned TICKS_PER_ROTATION = 200000;
	static const unsigned INDEX_DURATION = TICKS_PER_ROTATION / 50;

	MSXMotherBoard& motherBoard;
	LoadingIndicator loadingIndicator;
	const EmuDuration motorTimeout;

	using MotorClock = Clock<TICKS_PER_ROTATION * ROTATIONS_PER_SECOND>;
	MotorClock motorTimer;
	std::unique_ptr<DiskChanger> changer;
	unsigned headPos;
	unsigned side;
	unsigned startAngle;
	bool motorStatus;
	const bool doubleSizedDrive;
	const bool signalsNeedMotorOn;

	static const unsigned MAX_DRIVES = 26; // a-z
	using DrivesInUse = std::bitset<MAX_DRIVES>;
	std::shared_ptr<DrivesInUse> drivesInUse;

	RawTrack track;
	bool trackValid;
	bool trackDirty;
};
SERIALIZE_CLASS_VERSION(RealDrive, 6);

} // namespace openmsx

#endif
