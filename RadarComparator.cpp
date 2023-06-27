#include "RadarComparator.h"
#include "ConfigProvider.h"

#include <utility>

RadarComparator::RadarComparator(std::vector<std::string> configPaths) {
    RadarComparator::configPaths = std::move(configPaths);
    auto configProvider = new ConfigProvider();
}