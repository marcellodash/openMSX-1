// $Id$

#ifndef __MSXMOTHERBOARD_HH__
#define __MSXMOTHERBOARD_HH__

#include <fstream>
#include <vector>
#include "MSXDevice.hh"
#include "MSXIODevice.hh"
#include "MSXMemDevice.hh"
#include "Scheduler.hh"
#include "EmuTime.hh"
#include "CPUInterface.hh"
#include "msxconfig.hh"


class MSXMotherBoard : public CPUInterface
{	
	public:
		/**
		 * Destructor
		 */
		virtual ~MSXMotherBoard();
		
		/**
		 * this class is a singleton class
		 * usage: MSXConfig::instance()->method(args);
		 */
		static MSXMotherBoard *instance();
	 
		/**
		 * Devices can register their In ports. 
		 * This should be done during in their init() method.
		 * Once device are registered, their readIO() method
		 * can get called.
		 */
		void register_IO_In(byte port,MSXIODevice *device);
		
		/**
		 * Devices can register their Out ports. 
		 * This should be done during in their init() method.
		 * Once device are registered, their writeIO() method
		 * can get called.
		 */
		void register_IO_Out(byte port,MSXIODevice *device);

		/**
		 * Devices can register themself in the MSX slotstructure.
		 * They should do this during their init() method. Once the
		 * devices are registered their readMem() / writeMem() methods
		 * can get called.
		 */
		void registerSlottedDevice(MSXMemDevice *device,int PrimSl,int SecSL,int Page);
		
		/**
		 * All MSXDevices should be registered by tyhe MotherBoard.
		 * This method should only be called at start-up
		 */
		void addDevice(MSXDevice *device);
		
		/**
		 * To remove a device completely from configuration
		 * fe. yanking a cartridge out of the msx
		 *
		 * TODO this method is unimplemented!!
		 */
		void removeDevice(MSXDevice *device);

		/**
		 * This will initialize all MSXDevices (the init() method of
		 * all registered MSXDevices is called)
		 */
		void InitMSX();

		/**
		 * This will start all MSXDevices (the start() method of
		 * all registered MSXDevices is called)
		 */
		void StartMSX();

		/**
		 * This will reset all MSXDevices (the reset() method of
		 * all registered MSXDevices is called)
		 * This also initiates the Scheduler.
		 */
		void ResetMSX();

		/**
		 * This will stop all MSXDevices (the stop() method of
		 * all registered MSXDevices is called)
		 */
		void StopMSX();

		/**
		 * This will destroy all MSXDevices (the destructor of
		 * all registered MSXDevices is called)
		 */
		void DestroyMSX();


		/**
		 * TODO
		 */
		void RestoreMSX();	// TODO unimplemented!!
		/**
		 * TODO
		 */
		void SaveStateMSX(std::ofstream &savestream);


		// CPUInterface //
		
		/**
		 * This reads a byte from the currently selected device
		 */
		byte readMem(word address, EmuTime &time);

		/**
		 * This writes a byte to the currently selected device
		 */
		void writeMem(word address, byte value, EmuTime &time);

		/**
		 * This read a byte from the given IO-port
		 */
		byte readIO(word port, EmuTime &time);

		/**
		 * This writes a byte to the given IO-port
		 */
		void writeIO(word port, byte value, EmuTime &time);

		/**
		 * This returns the current IRQ status
		 *    true ->    IRQ pending
		 *   false -> no IRQ pending
		 */
		bool IRQStatus();
		
		/**
		 * This method must we called _exactly_once_ by each device that
		 * wishes to raise an IRQ. The MSXDevice class offers helper methods
		 * to ensure this.
		 */
		void raiseIRQ();

		/**
		 * If a device wishes to lower the IRQ, it must call this method.
		 * This method may only be called (once) if the device had previously
		 * called raiseIRQ(). The MSXDevice class offers helper methods to
		 * ensure this.
		 */
		void lowerIRQ();


		/*
		 * Should only be used by PPI
		 *  TODO make friend
		 */
		void set_A8_Register(byte value);

	private:
		MSXMotherBoard();

		MSXIODevice* IO_In[256];
		MSXIODevice* IO_Out[256];
		std::vector<MSXDevice*> availableDevices;
		
		MSXMemDevice* SlotLayout[4][4][4];
		byte SubSlot_Register[4];
		byte A8_Register;
		byte PrimarySlotState[4];
		byte SecondarySlotState[4];
		bool isSubSlotted[4];
		MSXMemDevice* visibleDevices[4]; 
		
		int IRQLine;

		static MSXMotherBoard *oneInstance;

		MSXConfig::Config *config;
};
#endif //__MSXMOTHERBOARD_HH__
