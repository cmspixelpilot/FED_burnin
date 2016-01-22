#include "commandlinetool.hh"

int main(int argc, char* argv[] )
{
    tbm_index_error_ctr = 0;
    tbm_core_error_ctr = 0;
    payload_error_ctr = 0;

    cHWFile = argv[1];
    std::cout << "HW Description File: " << cHWFile << std::endl;

    // for logging
    /*
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%d-%m-%Y_%H:%M:%S", timeinfo);


    logfilename = "logfile_";
    logfilename += buffer;
    logfilename += ".txt";
    logger.open(logfilename);
    std::cout << "Dumping log to: " << logfilename << std::endl;
    uhal::setLogLevelTo(uhal::Debug());
    */
    // instantiate System Controller
    cSystemController;

    // initialize map of settings so I can know the proper number of acquisitions and TBMs
    cSystemController.InitializeSettings(cHWFile, std::cout);

    // initialize HWdescription from XML, beware, settings have to be read first
    cSystemController.InitializeHw(cHWFile, std::cout);

    // configure the HW
    cSystemController.ConfigureHw(std::cout );
    auto cSetting = cSystemController.fSettingsMap.find("NAcq");
    cNAcq = (cSetting != std::end(cSystemController.fSettingsMap)) ? cSetting->second : 10;

    //clear userinput
    userInput = "";
    
    //prepare for loop
    defineValidInput();


    while( userInput != "q" &&  userInput != "quit")
      {
	//show the user a promt to choose his actions from       
	if(splitUserInput.size() == 0)
	  {
	    printPromt();
	    //get user input
	    getline(std::cin,userInput);
	    //this is for testing purposes
	    std::cout << "You entred: " << userInput << std::endl;
	    //sanitize input
	    splitInput(userInput);
	  }

	//if invalid input do nothing and show promt again
	if(splitUserInput.size() == 0)
	  {
	    std::cout << "No arguments were provided, please go again" << std::endl; 
	  }
	else if(!checkInput(splitUserInput[0]))
	  {
	    std::cout << splitUserInput[0]<<" is an invalid command! Try again" << std::endl;
	    splitUserInput.clear();
	  }
	else if(splitUserInput[0] == "i" || splitUserInput[0] == "info")
	  {
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "DAQinfo()" <<  std::endl;
	    DAQinfo();
	  }
	else if(splitUserInput[0] == "s" || splitUserInput[0] == "start")
	  {
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "startDAQ()" <<  std::endl;
	    startDAQ();
	  }
	else if(splitUserInput[0] == "x" || splitUserInput[0] == "stop")
	  {
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "stopDAQ()" <<  std::endl;
	    stopDAQ();
	  }
	else if(splitUserInput[0] == "p" || splitUserInput[0] == "pause")
	  {
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "pauseDAQ()" <<  std::endl;
	    pauseDAQ();
	  }
	else if(splitUserInput[0] == "r" || splitUserInput[0] == "resume")
	  {
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "resumeDAQ()" <<  std::endl;
	    resumeDAQ();
	  }
	else if(splitUserInput[0] == "read")
	  {
	    // READ data from DAQ
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "readDAQ()" <<  std::endl;
	    readData();
	  }
	else if(splitUserInput[0] == "a")
	  {
	    //check if a value for loops was supplied
	    std::string::size_type sz;
	    int loops;
	    if(splitUserInput.size() > 1 && !(checkInput(splitUserInput[1])))
	      {
		try
		  {
		    loops = stoi(splitUserInput[1], &sz);
		  }
		catch(std::invalid_argument&)
		  {
		    std::cout << "There must have been an error in the command string. Try again!" << std::endl;
		    splitUserInput.clear();
		    continue;
		  }

		if(splitUserInput[1].substr(sz) == "\0")
		  {
		    cNAcq = loops;
		    splitUserInput.erase(splitUserInput.begin(), splitUserInput.begin()+1);
		  }
		else
		  {
		    std::cout << "There must have been an error in the command string. Try again!" << std::endl;
		    splitUserInput.clear();
		    continue;
		  }
	      }
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "loopDAQ() || cNAcq =" << cNAcq <<  std::endl;
	    loopDAQ(cNAcq);
	  }
	else if(splitUserInput[0] == "c" || splitUserInput[0] == "conf" || splitUserInput[0] == "configure" )
	  {
	    if(splitUserInput.size() > 1 && !checkInput(splitUserInput[1]))
	      {
		if(fileexists(splitUserInput[1]))
		  {
		    //the configuration is loaded from a valid file path
		    const char* newHWFile = splitUserInput[1].c_str();
	    	    splitUserInput.erase(splitUserInput.begin(),splitUserInput.begin()+1);
		    std::cout << "configDAQ() with " << newHWFile <<  std::endl;
		    configDAQ(newHWFile);
		  }
		else
		  {
		    //neither the command after the configure is valid,
		    //nor can an existing file be found.
		    //we terminate the loop for now
		    std::cout << splitUserInput[0] << " " <<  splitUserInput[1] <<" Error: HWdescription file does not exist" << std::endl;
		    splitUserInput.clear();
		  }
	      }
	    //no additional argument is presented
	    else
	      {
		splitUserInput.erase(splitUserInput.begin());
		std::cout << "configDAQ() with " << cHWFile <<  std::endl;
		configDAQ(cHWFile);
	      }
	  }
	else if(splitUserInput[0] == "list" )
	  {
	    // LIST firmware images
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "listFW()" <<  std::endl;
	    listFW();
	  }
	else if(splitUserInput[0] == "flash" )
	  {
	    // FLASH FPGA
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "flashFPGA()" <<  std::endl;	    
	    flashFPGA();
	  }
	else if(splitUserInput[0] == "fpgaconf" )
	  {
	    // jump to an FPGA configuration (this could be a bit trickier)
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "switchFW()" <<  std::endl;	    
	    switchFW();
	  }
	else if(splitUserInput[0] == "trans")
	  {
	    // TRANSPARENT buffer dump
	    //	    getTransparent();
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "getTransparent()" <<  std::endl;
	    getTransparent();
	  }
	else if(splitUserInput[0] == "spy")
	  {
	    // SPY FIFO dump
	    //	    getSpy();
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "getSpy()" <<  std::endl;	    
	    getSpy();
	  }
	else if(splitUserInput[0] == "fifo1" || splitUserInput[0] == "one")
	  {
	    // FIFO1 dump
	    //	    getFIFO1();
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "getFIFO1()" <<  std::endl;	    
	    getFIFO1();
	  }
	else if(splitUserInput[0] == "dumpallfifo" || splitUserInput[0] == "dump")
	  {
	    // dump all FIFOs and transparent buffer
	    //	    dumpAll();
	    splitUserInput.erase(splitUserInput.begin());
	    std::cout << "dumpAll()" <<  std::endl;	    
	    dumpAll();
	  }
	else if(splitUserInput[0] == "q" || splitUserInput[0] == "quit")
	  {
	    // QUIT/END program
	    break;
	  }

      }
    //    logger.close();

}

void defineValidInput(){

  // board info
  validInput.push_back("i");      
  validInput.push_back("info");   
  // quit/end program
  validInput.push_back("q");      
  validInput.push_back("quit");   
  // start DAQ
  validInput.push_back("s");      
  validInput.push_back("start");  
  // stop DAQ
  validInput.push_back("x");      
  validInput.push_back("stop");      
  // pause DAQ
  validInput.push_back("p");      
  validInput.push_back("pause");
  // resume DAQ
  validInput.push_back("r");      
  validInput.push_back("resume");
  // acquisition loop 
  validInput.push_back("a");      
  // (re)configure FED
  validInput.push_back("c");      
  validInput.push_back("conf");
  validInput.push_back("configure");
  // find phases
  validInput.push_back("findPhase");
  // read data
  validInput.push_back("read");
  // read transparent FIFO
  validInput.push_back("trans");
  // read spy FIFO
  validInput.push_back("spy");
  // read FIFO1
  validInput.push_back("fifo1");
  validInput.push_back("one");
  // dump all FIFOs
  validInput.push_back("dumpallfifo");
  validInput.push_back("dump");
  // flash PROM
  validInput.push_back("flash");
  // Jump to FPGA Config
  validInput.push_back("fpgaconf");
  // List all firmware images
  validInput.push_back("list");
 
}

void splitInput(std::string userInput){

  //split into different words
  std::istringstream iss (userInput);
  while(!iss.eof())
    {
      std::string uI;
      getline(iss,uI,' ');
      //      std::cout << uI << std::endl;
      splitUserInput.push_back(uI);
    }
}

bool checkInput(std::string userInput){

  for ( auto& s : validInput)
    {
      if( s.compare(userInput) == 0)
	{
	  // we have a valid input
	  return true;
	}
    }
  // the user input does not match any valid input string
  return false;
}

void printPromt(){

  std::cout << "Please choose an operation:" << std::endl;
  std::cout << "\t [i/info] for board info" << std::endl;
  //DAQ control
  std::cout << "DAQ control:" << std::endl;
  std::cout << "\t [s/start] to start DAQ on all FEDs" << std::endl;
  std::cout << "\t [x/stop] to stop DAQ on all FEDs" << std::endl;
  std::cout << "\t [p/pause] to pause DAQ on all FEDs" << std::endl;
  std::cout << "\t [r/resume] to resume DAQ on all FEDs" << std::endl;
  std::cout << "\t [read] to read data from DAQ" << std::endl;
  std::cout << "\t [a] for data acquisition" << std::endl;
  std::cout << "\t \t or [a xxx] for data acquisition of xxx loops" << std::endl;
  //DAQ config
  std::cout << "DAQ configuration" << std::endl;
  std::cout << "\t [c/conf] to re-load configuration from disk" << std::endl;
  std::cout << "\t [list] to show all available firmware versions on all boards" << std::endl; //perhaps here it would be better to also ask for a board nummer or something to identify the correct FED
  std::cout << "\t [flash] to execute phase finding" << std::endl;
  std::cout << "\t [fpgaconf] to execute phase finding" << std::endl;
  //DQM
  std::cout << "\t [trans] to dump transparent buffer data" << std::endl;
  std::cout << "\t [spy] to dump spy FIFO data" << std::endl;
  std::cout << "\t [fifo1/one] to dump FIFO1 data" << std::endl;
  std::cout << "\t [dumpallfifo] to dump all three FIFOs" << std::endl;

  std::cout << "\t [q/quit] to to quit" << std::endl;

}


bool fileexists(std::string filename){
  std::ifstream ifile(filename);
  return ifile;
}

void DAQinfo(){
  // get the board INFO of all boards and start the acquistion
  std::cout << "DAQ info" << std::endl;
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      cSystemController.fFEDInterface->getBoardInfo(cFED);
    }
}

void startDAQ(){
  // START DAQ on all FEDs
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      cSystemController.fFEDInterface->Start(cFED);
    }
}

void stopDAQ(){
  // STOP DAQ on all FEDs
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      cSystemController.fFEDInterface->Stop(cFED);
    }
}

void pauseDAQ(){
  // PAUSE DAQ on all FEDs
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      cSystemController.fFEDInterface->Pause(cFED);
    }
}

void resumeDAQ(){
  // RESUME DAQ on all FEDs
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      cSystemController.fFEDInterface->Resume(cFED);
    }
}


void readData(){
  //READ a single data acquisition
  Data cData;
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      cData.add(0, cSystemController.fFEDInterface->ReadData(cFED));
    }
}


void loopDAQ(int loops){
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      cSystemController.fFEDInterface->Start(cFED);
    }

  // loop over the number of acquisitions
  uint32_t iAcq = 0;
  bool running = true;
  while ( running )
    {
      //std::cout << std::endl << BOLDRED << "Acquisition: " << iAcq << RESET << "\r";
      Data cData;
      for (auto& cFED : cSystemController.fPixFEDVector)
	{
	  //std::cout << BOLDGREEN << "Data for FED " << +cFED->getBeId() << RESET << std::endl;
	  cData.add(iAcq, cSystemController.fFEDInterface->ReadData(cFED));
	}
      iAcq++;
      if (iAcq < cNAcq && cNAcq > 0 )running = true;
      else if (cNAcq == 0 ) running = true;
      else running = false;
    }
  std::cout << std::endl << "Finished recording " << iAcq << " events!" << std::endl;
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      cSystemController.fFEDInterface->Stop(cFED);
      //std::cout << "Finished reading Data!" << std::endl;
    }
}

void configDAQ(const char* configfile){
  // CONFIGURE DAQ
  std::cout << "Loading new DAQ calibration from " << *configfile << std::endl;
  // we delete the old settings
  cSystemController.fPixFEDVector.clear();
  //do these have to go as well?
  cSystemController.fSettingsMap.clear();
  cSystemController.fFWMap.clear();
	    
  // load new settings

  // initialize map of settings so I can know the proper number of acquisitions and TBMs
  cSystemController.InitializeSettings(configfile, std::cout);
	    
  // initialize HWdescription from XML, beware, settings have to be read first
  cSystemController.InitializeHw(configfile, std::cout);
	    
  // configure the HW
  cSystemController.ConfigureHw(std::cout );
  auto cSetting = cSystemController.fSettingsMap.find("NAcq");
  int cNAcq = (cSetting != std::end(cSystemController.fSettingsMap)) ? cSetting->second : 10;
}

void flashFPGA(){}

void switchFW(){}

void listFW(){
  // list all saved firmware images for all FEDs

  //TODO: this is a dirty bug fix and needs to be removed at some point
  cSystemController.InitializeHw("settings/FWUpload.xml");

  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      // I print the board info until I know a better way to identify the FED
      cSystemController.fFEDInterface->getBoardInfo(cFED);
      std::vector<std::string> FWNames;
      FWNames = cSystemController.fFEDInterface->getFpgaConfigList(cFED);
      std::cout << FWNames.size() << " firmware images on SD card:" << std::endl;
      for (auto &name : FWNames)
	{
	  std::cout <<"\t -" << name << std::endl;
	}
    }
  //restore the original settings
  cSystemController.InitializeHw(cHWFile);

}

void getTransparent(){
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      // read TRANSPARENT fifo
      cSystemController.fFEDInterface->readTransparentFIFO(cFED);
    }
}

void getSpy(){
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      // read SPY fifo
      cSystemController.fFEDInterface->readSpyFIFO(cFED);
    }
}

void getFIFO1(){
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      // read fifo 1
      cSystemController.fFEDInterface->readFIFO1(cFED);
    }
}

void dumpAll(){
  Data cData;
  for (auto& cFED : cSystemController.fPixFEDVector)
    {
      // read TRANSPARENT fifo
      cSystemController.fFEDInterface->readTransparentFIFO(cFED);
      // read SPY fifo
      cSystemController.fFEDInterface->readSpyFIFO(cFED);
      // read fifo 1
      cSystemController.fFEDInterface->readFIFO1(cFED);
      // read data
      cData.add(0, cSystemController.fFEDInterface->ReadData(cFED));
    }
}

void quit(){}
