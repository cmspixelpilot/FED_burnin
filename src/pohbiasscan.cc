
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
    std::cout << "Press [Enter] to read FIFOs ...";
    std::cin.get();
}

FILE *the_portal = 0;
void open_the_portal() {
  the_portal = fopen("the_portal", "wt");
  assert(the_portal);
}

void close_the_portal() {
  assert(the_portal);
  fclose(the_portal);
  assert(system("python ccubang.py") == 0);
  //mypause();
}

int main (int argc, char* argv[] )
{

    const char* cHWFile = argv[1];
    std::cout << "HW Description File: " << cHWFile << std::endl;

    uhal::setLogLevelTo (uhal::Debug() );

    // instantiate System Controller
    SystemController cSystemController;

    // initialize map of settings so I can know the proper number of acquisitions and TBMs
    cSystemController.InitializeSettings (cHWFile, std::cout);

    // initialize HWdescription from XML, beware, settings have to be read first
    cSystemController.InitializeHw (cHWFile, std::cout);

    //auto cSetting = cSystemController.fSettingsMap.find ("NAcq");
    //int cNAcq = (cSetting != std::end (cSystemController.fSettingsMap) ) ? cSetting->second : 10;
    //cSetting = cSystemController.fSettingsMap.find ("BlockSize");
    //int cBlockSize = (cSetting != std::end (cSystemController.fSettingsMap) ) ? cSetting->second : 2;

    auto cSetting = cSystemController.fSettingsMap.find ("ChannelOfInterest");
    int cChannelOfInterest = (cSetting != std::end (cSystemController.fSettingsMap) ) ? cSetting->second : 0;

    // turn it all off
    open_the_portal();
    const int i2c_chans[] = {0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 0x18, 0x30, 0x31, 0x32, 0x34, 0x35, 0x36, 0x38};
    for (int c : i2c_chans)
      fprintf(the_portal, "%x 0\n", c);
    fprintf(the_portal, "13 2a\n");
    fprintf(the_portal, "17 2a\n");
    fprintf(the_portal, "1b 2a\n");
    fprintf(the_portal, "33 2a\n");
    fprintf(the_portal, "37 2a\n");
    fprintf(the_portal, "3b 2a\n");
    close_the_portal();
    
    //configure FED & FITELS & SFP+
    cSystemController.ConfigureHw (std::cout );

    for (auto& cFED : cSystemController.fPixFEDVector)
    {
        cSystemController.fFEDInterface->getBoardInfo (cFED);
        cSystemController.fFEDInterface->setChannelOfInterest(cFED, cChannelOfInterest);
    }

    auto& cFED = cSystemController.fPixFEDVector[0];
    assert(cFED->fFitelVector.size() == 2);

    /*
    double last[2][12] = {0};
    for (int i2c_index = -1; i2c_index < 14; ++i2c_index) {
      const bool first = i2c_index == -1;
      if (!first) {
        open_the_portal();
        fprintf(the_portal, "%x 10\n", i2c_chans[i2c_index]);
        close_the_portal();
        usleep(100000);
      }
      for (size_t iFitel = 0; iFitel < 2; ++iFitel) {
        auto& cFitel = cFED->fFitelVector[iFitel];
        for (uint32_t cChannel = 0; cChannel < 12; cChannel++) {
          std::vector<double> v = cSystemController.fFEDInterface->ReadADC(cFitel, cChannel, true);
          const double rssi = 1000 * fabs(v[2] - v[3]) / 150;
          double& lst = last[iFitel][cChannel]; 
          printf("extract rssi ourselves %f lst %f\n", rssi, lst);
          if (!first) {
            const double diff = rssi - lst;
            if (fabs(diff) > 0.005)
              printf(BOLDRED "I2C CHAN %x CHANGED fitel %i channel %i : %f -> %f" RESET "\n", i2c_chans[i2c_index], iFitel, cChannel, lst, rssi);
          }
          lst = rssi;
        }
      }
    }
    */

    struct channelmap {
      int i2c;
      int fitel;
      int channel;
    };

//    const channelmap gaga[8] = {
//      {0x14, 0, 0},
//      {0x15, 1, 2},
//      {0x16, 1, 0},
//      {0x18, 0, 2},
//      {0x34, 0, 1},
//      {0x35, 1, 3},
//      {0x36, 1, 1},
//      {0x38, 0, 3}
//    };

    const channelmap gaga[1] = {
      {0x30, 0, 1}
    };

    for (channelmap a : gaga) {
      const int start = time(0);
      printf("start %i\n", start);
      auto& cFitel = cFED->fFitelVector[a.fitel];
      for (int gain : {0x2a}) { //{0, 0x15, 0x2a, 0x3f } ) {
        for (int bias = 0; bias <= 0x20; bias += 2) {
          open_the_portal();
          fprintf(the_portal, "%x %x\n", a.i2c, bias);
          close_the_portal();
          usleep(100000);

          std::vector<double> v = cSystemController.fFEDInterface->ReadADC(cFitel, a.channel, true);
          const double rssi = 1000 * fabs(v[2] - v[3]) / 150;
          printf("gggg chan0x%x %i %i %f\n", a.i2c, gain, bias, rssi);
        }
      }
      const int end = time(0);
      printf("start %i end %i elapsed %i\n", start, end, end-start);
    }
}
