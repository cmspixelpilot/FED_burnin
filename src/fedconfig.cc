#include <ctime>
#include <cstring>
#include "uhal/uhal.hpp"
#include "../Utils/Utilities.h"
#include "../HWDescription/PixFED.h"
#include "../HWInterface/PixFEDInterface.h"
#include "../System/SystemController.h"
#include "../AMC13/Amc13Controller.h"
#include "../Utils/Data.h"
void mypause()
{
    std::cout << "Press [Enter] ...";
    std::cin.get();

}
int main (int argc, char* argv[] )
{

    const char* cHWFile = argv[1];
    std::cout << "HW Description File: " << cHWFile << std::endl;
    bool use_amc13 = true;
    bool do_phases = true;
    bool phases_forever = false;
    bool phases_forever_one = false;
    bool dump_fitel_regs = false;
    bool do_calib_mode = false;

    for (int i = 2; i < argc; ++i) {
      const std::string a(argv[i]);
      if (a == "no_amc13")
	use_amc13 = false;
      else if (a == "phases_forever_one")
	phases_forever_one = true;
      else if (a == "phases_forever")
	phases_forever = true;
      else if (a == "no_phases")
	do_phases = false;
      else if (a == "dump_fitel_regs")
	dump_fitel_regs = true;
      else if (a == "do_calib_mode")
	do_calib_mode = true;
    }

    uhal::setLogLevelTo (uhal::Debug() );

    // instantiate System Controller
    SystemController cSystemController;
    Amc13Controller cAmc13Controller;

    // initialize map of settings so I can know the proper number of acquisitions and TBMs
    cSystemController.InitializeSettings (cHWFile, std::cout);

    // initialize HWdescription from XML, beware, settings have to be read first
    if (use_amc13) cAmc13Controller.InitializeAmc13 ( cHWFile, std::cout );
    cSystemController.InitializeHw (cHWFile, std::cout);

    auto cSetting = cSystemController.fSettingsMap.find ("NAcq");
    int cNAcq = (cSetting != std::end (cSystemController.fSettingsMap) ) ? cSetting->second : 10;
    cSetting = cSystemController.fSettingsMap.find ("BlockSize");
    int cBlockSize = (cSetting != std::end (cSystemController.fSettingsMap) ) ? cSetting->second : 2;

    cSetting = cSystemController.fSettingsMap.find ("ChannelOfInterest");
    int cChannelOfInterest = (cSetting != std::end (cSystemController.fSettingsMap) ) ? cSetting->second : 0;

    cSetting = cSystemController.fSettingsMap.find ("ROCOfInterest");
    int cROCOfInterest = (cSetting != std::end (cSystemController.fSettingsMap) ) ? cSetting->second : 0;

    // configure the AMC13
    if (use_amc13) cAmc13Controller.ConfigureAmc13 ( std::cout );

    //configure FED & FITELS & SFP+
    cSystemController.ConfigureHw (std::cout );

    // get the board info of all boards and start the acquistion
    for (auto& cFED : cSystemController.fPixFEDVector)
    {
        cSystemController.fFEDInterface->setChannelOfInterest(cFED, cChannelOfInterest);
        cSystemController.fFEDInterface->getBoardInfo(cFED);
        if (do_phases) cSystemController.fFEDInterface->findPhases(cFED);
        if (phases_forever)
          while (true) {
            mypause();
            cSystemController.fFEDInterface->findPhases(cFED);
          }
    }

    if (phases_forever_one) {
      std::cout << "Monitoring Phases for selected Channel of Interest for 10 seconds ... " << std::endl << std::endl;
      std::cout << BOLDGREEN << "FIBRE CTRL_RDY CNTVAL_Hi CNTVAL_Lo   pattern:                     S H1 L1 H0 L0   W R" << RESET << std::endl;
      while (true)
        for (auto& cFED : cSystemController.fPixFEDVector)
	  cSystemController.fFEDInterface->monitorPhases(cFED, cChannelOfInterest);
    }

    std::cout << "FED Configured, SLink Enabled, pressing Enter will send an EC0 & start periodic L1As" << std::endl;
    mypause();
    if (use_amc13) {
      cAmc13Controller.fAmc13Interface->SendEC0();
      //cAmc13Controller.fAmc13Interface->StartL1A();
    }

    for (int i = 0; i < 10000; i++)
    {
         for (auto& cFED : cSystemController.fPixFEDVector)
         {
             cAmc13Controller.fAmc13Interface->BurstL1A();
             //cSystemController.fFEDInterface->WriteBoardReg(cFED, "fe_ctrl_regs.decode_reg_reset", 1);
             mypause();
             cSystemController.fFEDInterface->readTransparentFIFO(cFED);
             cSystemController.fFEDInterface->readSpyFIFO(cFED);
             cSystemController.fFEDInterface->readFIFO1(cFED);
             cSystemController.fFEDInterface->readOSDWord(cFED, cROCOfInterest, cChannelOfInterest);
             cSystemController.fFEDInterface->readErrorFIFO (cFED, true);
             cSystemController.fFEDInterface->readTTSState (cFED); //returns byte with state
             cSystemController.fFEDInterface->ReadData (cFED, 0 );
         }
    }
    
    if (use_amc13) cAmc13Controller.fAmc13Interface->StopL1A();

    for (auto& cFED : cSystemController.fPixFEDVector)
        cSystemController.fFEDInterface->Stop (cFED);

    //cSystemController.HaltHw();
    if (use_amc13) cAmc13Controller.HaltAmc13();
}
