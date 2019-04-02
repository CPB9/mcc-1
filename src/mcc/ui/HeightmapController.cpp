#include "mcc/ui/HeightmapController.h"

#include <mutex>

namespace mccui {

HeightmapController::HeightmapController(const mcchm::HmReader* reader)
    : _reader(reader)
{
}

HeightmapController::~HeightmapController()
{
}

void HeightmapController::setHeightmapReader(const mcchm::HmReader* readerPtr)
{
    mcchm::Rc<const mcchm::HmReader> reader(readerPtr->clone());
    _lock.lock();
    _reader = reader;
    _lock.unlock();
    emit heightmapReaderChanged(reader);
}

mcchm::Rc<const mcchm::HmReader> HeightmapController::cloneHeightmapReader() const
{
    _lock.lock();
    mcchm::Rc<const mcchm::HmReader> reader(_reader);
    _lock.unlock();
    return reader->clone();
}

HmControllerPluginData::HmControllerPluginData(HeightmapController* hmReader)
    : mccplugin::PluginData(id)
    , _hmController(hmReader)
{
}

HmControllerPluginData::~HmControllerPluginData()
{
}

HeightmapController* HmControllerPluginData::hmReader()
{
    return _hmController.get();
}

const HeightmapController* HmControllerPluginData::hmReader() const
{
    return _hmController.get();
}
}
