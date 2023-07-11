#ifndef EFTDEM_GROUNDTRUTHCOMPARATOR_H
#define EFTDEM_GROUNDTRUTHCOMPARATOR_H

#include "Pipeline.h"
#include "IComparator.h"

#include <vector>
#include <string>

class GroundTruthComparator : public IComparator {
private:
public:
    explicit GroundTruthComparator(std::vector<std::string> configPaths);
    std::vector<rawPointCloud *> setupPointClouds() override;

    ~GroundTruthComparator() override;
};


#endif //EFTDEM_GROUNDTRUTHCOMPARATOR_H
