#include <iostream>

/*

    FileName :                    RegManager.cc
    Content :                     RegManager class, permit connection & r/w registers
    Programmer :                  Nicolas PIERRE
    Version :                     1.0
    Date of creation :            06/06/14
    Support :                     mail to : nico.pierre@icloud.com

 */
#include <uhal/uhal.hpp>
#include "RegManager.h"
#include "../Utils/Utilities.h"

#define DEV_FLAG    0
//#define JMTPRINTS

RegManager::RegManager( const char* puHalConfigFileName, uint32_t pBoardId ) :
    fThread( [ = ]
{
    uhal::setLogLevelTo(uhal::Warning());
    StackWriteTimeOut();
} ),
fDeactiveThread( false )
{
    // Loging settings
    uhal::disableLogging();
    //uhal::setLogLevelTo(uhal::Error()); //Raise the log level

    fUHalConfigFileName = puHalConfigFileName;

    uhal::ConnectionManager cm( fUHalConfigFileName ); // Get connection
    char cBuff[7];
    sprintf( cBuff, "board%d", pBoardId );

    fBoard = new uhal::HwInterface( cm.getDevice( ( cBuff ) ) );

    fThread.detach();

}

RegManager::RegManager( const char* pId, const char* pUri, const char* pAddressTable ) :
    fThread( [ = ]
{
    StackWriteTimeOut();
} ),
fDeactiveThread( false )
{
    // Loging settings
    uhal::disableLogging();
    //uhal::setLogLevelTo(uhal::Error()); //Raise the log level

    uhal::ConnectionManager cm( "file://HWInterface/dummy.xml" ); // Get connection

    fBoard = new uhal::HwInterface( cm.getDevice( pId, pUri, pAddressTable ) );

    fThread.detach();

}

RegManager::~RegManager()
{
    fDeactiveThread = true;
    if ( fBoard ) delete fBoard;
}


bool RegManager::WriteReg( const std::string& pRegNode, const uint32_t& pVal )
{
#ifdef JMTPRINTS
  std::cout << "JMT WriteReg " << pRegNode << " 0x" << std::hex << pVal << std::dec << std::endl;
#endif

    fBoardMutex.lock();
    fBoard->getNode( pRegNode ).write( pVal );
    fBoard->dispatch();
    fBoardMutex.unlock();

    // Verify if the writing is done correctly
    if ( DEV_FLAG )
    {
        fBoardMutex.lock();
        uhal::ValWord<uint32_t> reply = fBoard->getNode( pRegNode ).read();
        fBoard->dispatch();
        fBoardMutex.unlock();

        uint32_t comp = ( uint32_t ) reply;

        if ( comp == pVal )
        {
            std::cout << "Values written correctly !" << comp << "=" << pVal << std::endl;
            return true;
        }

        std::cout << "\nERROR !!\nValues are not consistent : \nExpected : " << pVal << "\nActual : " << comp << std::endl;
    }

    return false;
}


bool RegManager::WriteStackReg( const std::vector< std::pair<std::string, uint32_t> >& pVecReg )
{
#ifdef JMTPRINTS
  std::cout << "JMT WriteStackReg:\n";
#endif

    fBoardMutex.lock();
    for ( auto const& v : pVecReg )
    {
#ifdef JMTPRINTS
      std::cout << "\tJMT " << v.first << " 0x" << std::hex << v.second << std::dec << std::endl;
#endif
        fBoard->getNode( v.first ).write( v.second );
        //std::cout << v.first << "  :  " << v.second << std::endl;
    }
    fBoard->dispatch();
    fBoardMutex.unlock();

    if ( DEV_FLAG )
    {
        int cNbErrors = 0;
        uint32_t comp;

        for ( auto const& v : pVecReg )
        {
            fBoardMutex.lock();
            uhal::ValWord<uint32_t> reply = fBoard->getNode( v.first ).read();
            fBoard->dispatch();
            fBoardMutex.unlock();

            comp = static_cast<uint32_t>( reply );

            if ( comp ==  v.second )
                std::cout << "Values written correctly !" << comp << "=" << v.second << std::endl;
        }

        if ( cNbErrors == 0 )
        {
            std::cout << "All values written correctly !" << std::endl;
            return true;
        }

        std::cout << "\nERROR !!\n" << cNbErrors << " have not been written correctly !" << std::endl;
    }

    return false;
}


bool RegManager::WriteBlockReg( const std::string& pRegNode, const std::vector< uint32_t >& pValues )
{
#ifdef JMTPRINTS
  std::cout << "JMT WriteBlockReg " << pRegNode << ":\n";
  for (auto v : pValues)
    std::cout << "  JMT 0x" << std::hex << v << std::dec << std::endl;
#endif

    fBoardMutex.lock();
    fBoard->getNode( pRegNode ).writeBlock( pValues );
    fBoard->dispatch();
    fBoardMutex.unlock();

    bool cWriteCorr = true;

    //Verifying block
    if ( DEV_FLAG )
    {
        int cErrCount = 0;

        fBoardMutex.lock();
        uhal::ValVector<uint32_t> cBlockRead = fBoard->getNode( pRegNode ).readBlock( pValues.size() );
        fBoard->dispatch();
        fBoardMutex.unlock();

        //Use size_t and not an iterator as op[] only works with size_t type
        for ( std::size_t i = 0; i != cBlockRead.size(); i++ )
        {
            if ( cBlockRead[i] != pValues.at( i ) )
            {
                cWriteCorr = false;
                cErrCount++;
            }
        }

        std::cout << "Block Write finished !!\n" << cErrCount << " values failed to write !" << std::endl;
    }

    return cWriteCorr;
}

bool RegManager::WriteBlockAtAddress( uint32_t uAddr, const std::vector< uint32_t >& pValues, bool bNonInc )
{
#ifdef JMTPRINTS
  std::cout << "JMT WriteBlockReg(NonInc=" << bNonInc << ") @ 0x" << std::hex << uAddr << std::dec << ":\n";
  for (auto v : pValues)
    std::cout << "  JMT 0x" << std::hex << v << std::dec << std::endl;
#endif

    fBoardMutex.lock();
    fBoard->getClient().writeBlock( uAddr, pValues, bNonInc ? uhal::defs::NON_INCREMENTAL : uhal::defs::INCREMENTAL );
    fBoard->dispatch();
    fBoardMutex.unlock();

    bool cWriteCorr = true;

    //Verifying block
    if ( DEV_FLAG )
    {
        int cErrCount = 0;

        fBoardMutex.lock();
        uhal::ValVector<uint32_t> cBlockRead = fBoard->getClient().readBlock( uAddr, pValues.size(), bNonInc ? uhal::defs::NON_INCREMENTAL : uhal::defs::INCREMENTAL );
        fBoard->dispatch();
        fBoardMutex.unlock();

        //Use size_t and not an iterator as op[] only works with size_t type
        for ( std::size_t i = 0; i != cBlockRead.size(); i++ )
        {
            if ( cBlockRead[i] != pValues.at( i ) )
            {
                cWriteCorr = false;
                cErrCount++;
            }
        }

        std::cout << "BlockWriteAtAddress finished !!\n" << cErrCount << " values failed to write !" << std::endl;
    }

    return cWriteCorr;
}


uhal::ValWord<uint32_t> RegManager::ReadReg( const std::string& pRegNode )
{
    fBoardMutex.lock();
    uhal::ValWord<uint32_t> cValRead = fBoard->getNode( pRegNode ).read();
    fBoard->dispatch();
    fBoardMutex.unlock();

    if ( DEV_FLAG )
    {
        uint32_t read = ( uint32_t ) cValRead;
        std::cout << "\nValue in register ID " << pRegNode << " : " << read << std::endl;
    }

#ifdef JMTPRINTS
    std::cout << "JMT ReadReg " << pRegNode << " valid? " << cValRead.valid();
    if (cValRead.valid())
      std::cout << "  JMT 0x" << std::hex << cValRead << std::dec;
    std::cout << std::endl;
#endif

    return cValRead;
}

uhal::ValWord<uint32_t> RegManager::ReadAtAddress( uint32_t uAddr, uint32_t uMask )
{
    fBoardMutex.lock();
    uhal::ValWord<uint32_t> cValRead = fBoard->getClient().read( uAddr, uMask );
    fBoard->dispatch();
    fBoardMutex.unlock();

    if ( DEV_FLAG )
    {
        uint32_t read = ( uint32_t ) cValRead;
        std::cout << "\nValue at address " << std::hex << uAddr << std::dec << " : " << read << std::endl;
    }

#ifdef JMTPRINTS
    std::cout << "JMT ReadAtAddress @ 0x" << std::hex << uAddr << " mask 0x" << uMask << std::dec << " valid? " << cValRead.valid();
    if (cValRead.valid())
      std::cout << "  JMT 0x" << std::hex << cValRead << std::dec;
    std::cout << std::endl;
#endif

    return cValRead;
}


uhal::ValVector<uint32_t> RegManager::ReadBlockReg( const std::string& pRegNode, const uint32_t& pBlockSize )
{
    fBoardMutex.lock();
    uhal::ValVector<uint32_t> cBlockRead = fBoard->getNode( pRegNode ).readBlock( pBlockSize );
    fBoard->dispatch();
    fBoardMutex.unlock();

    if ( DEV_FLAG )
    {
        std::cout << "\nValues in register block " << pRegNode << " : " << std::endl;

        //Use size_t and not an iterator as op[] only works with size_t type
        for ( std::size_t i = 0; i != cBlockRead.size(); i++ )
        {
            uint32_t read = static_cast<uint32_t>( cBlockRead[i] );
            std::cout << " " << read << " " << std::endl;
        }
    }

#ifdef JMTPRINTS
    std::cout << "JMT ReadBlockReg " << pRegNode << " blocksize " << pBlockSize << " valid? " << cBlockRead.valid() << ":";
    if (cBlockRead.valid())
      for (auto v : cBlockRead)
        std::cout << "  JMT 0x" << std::hex << v << std::dec << std::endl;
    std::cout << std::endl;
#endif

    return cBlockRead;
}


void RegManager::StackReg( const std::string& pRegNode, const uint32_t& pVal, bool pSend )
{
#ifdef JMTPRINTS
  std::cout << "JMT StackReg\n";
#endif

    for ( std::vector< std::pair<std::string, uint32_t> >::iterator cIt = fStackReg.begin(); cIt != fStackReg.end(); cIt++ )
    {
        if ( cIt->first == pRegNode )
            fStackReg.erase( cIt );
    }

    std::pair<std::string, uint32_t> cPair( pRegNode, pVal );
    fStackReg.push_back( cPair );

    if ( pSend || fStackReg.size() == 100 )
    {
        WriteStackReg( fStackReg );
        fStackReg.clear();
    }
}


void RegManager::StackWriteTimeOut()
{
#ifdef JMTPRINTS
  std::cout << "JMT StackWriteTimeOut\n";
#endif

    uint32_t i = 0;

    while ( !fDeactiveThread )
    {
        std::this_thread::sleep_for( std::chrono::seconds( TIME_OUT ) );
        //std::cout << "Ping ! \nThread ID : " << std::this_thread::get_id() << "\n" << std::endl;

        if ( fStackReg.size() != 0 && i == 1 )
        {
            WriteStackReg( fStackReg );
            fStackReg.clear();
        }
        else if ( i == 0 )
            i = 1;

    }
}

const uhal::Node& RegManager::getUhalNode( const std::string& pStrPath )
{
    return fBoard->getNode( pStrPath );
}
