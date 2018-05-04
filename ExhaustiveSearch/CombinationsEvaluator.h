#ifndef SEGBYCUT_COMBINATIONSEVALUATOR_H
#define SEGBYCUT_COMBINATIONSEVALUATOR_H

#include <boost/filesystem/operations.hpp>
#include "DGtal/helpers/StdDefs.h"

#include "PropertyChecker/CheckableSeedPair.h"
#include "PropertyChecker/MarkedMapChecker/GluedIntersectionChecker.h"
#include "PropertyChecker/MarkedMapChecker/MinimumDistanceChecker.h"
#include "LazyCombinations.h"
#include "../utils.h"
#include "../FlowGraph/weightSettings.h"

template<int maxPairs>
class CombinationsEvaluator
{
protected:
    typedef DGtal::Z2i::Curve::ConstIterator SCellIterator;
    typedef DGtal::Circulator<SCellIterator> SCellCirculator;

    typedef DGtal::Z2i::Curve Curve;
    typedef DGtal::Z2i::KSpace KSpace;

    typedef ConnectorSeedRange<KSpace,SCellCirculator> MyConnectorSeedRange;
    typedef MyConnectorSeedRange::ConnectorSeedType ConnectorSeed;

public:
    void operator()(Curve& minCurve,
                    std::vector<CheckableSeedPair> &pairList,
                    KSpace &KImage,
                    int maxSimultaneousPairs,
                    std::string outputFolder="")
    {
        typedef std::vector<CheckableSeedPair> CheckableSeedList;
        bool saveEPS = outputFolder!="";

        GluedIntersectionChecker intersectionChecker;
        MinimumDistanceChecker minDistChecker(KImage);
        for (auto it = pairList.begin(); it != pairList.end(); ++it) {
            intersectionChecker.unmark(*it);
            minDistChecker.unmark(*it);
        }

        CheckableSeedPair seedCombination[10];

        double minEnergyValue = 100;
        double currentEnergyValue;

        LazyCombinations <CheckableSeedList, maxPairs> myCombinations(pairList);
        myCombinations.addConsistencyChecker(&intersectionChecker);
        myCombinations.addConsistencyChecker(&minDistChecker);

        int n = 0;

        Board2D board;
        if(saveEPS)
        {
            outputFolder += std::to_string(maxPairs);
            boost::filesystem::create_directories(outputFolder);
        }

        while (myCombinations.next(seedCombination)) {
            Curve curve;
            std::map<KSpace::SCell, double> weightMap;

            createCurve(curve, seedCombination, maxPairs);
            eliminateLoops(curve, KImage, curve);

            setGridCurveWeight(curve, KImage, weightMap);

            currentEnergyValue = energyValue(curve, weightMap);
            if (currentEnergyValue < minEnergyValue) {
                std::cout << "Updated min energy value: " << minEnergyValue << " -> " << currentEnergyValue
                          << std::endl;
                minEnergyValue = currentEnergyValue;

                minCurve = curve;

                if(saveEPS)
                {
                    board.clear();
                    board << curve;
                    board.saveEPS((outputFolder + "/" + std::to_string(n) + ".eps").c_str());
                }
            }

            ++n;
        }
        std::cout << maxPairs << "-Combination: " << n << std::endl;
        std::cout << "Min energy value: " << minEnergyValue << std::endl;

        CombinationsEvaluator<maxPairs - 1> next;
        next(pairList, KImage, maxSimultaneousPairs);
    }


    void operator()(std::vector<CheckableSeedPair> &pairList,
                    KSpace &KImage,
                    int maxSimultaneousPairs,
                    std::string outputFolder="")
    {
        Curve minCurve;
        this->operator()(minCurve,pairList,KImage,maxSimultaneousPairs,outputFolder);
    }

private:
    void addIntervalSCells(std::vector<KSpace::SCell>& vectorOfSCells,
                           SCellCirculator begin,
                           SCellCirculator end)
    {
        SCellCirculator it = begin;
        while (it != end)
        {
            vectorOfSCells.push_back(*it);
            ++it;
        }
        vectorOfSCells.push_back(*it);
    }

    void addSeedPairSCells(std::vector<KSpace::SCell>& vectorOfSCells,
                           CheckableSeedPair& currentPair,
                           CheckableSeedPair& nextPair)
    {
        ConnectorSeed inToExtSeed = currentPair.data().first;
        ConnectorSeed extToIntSeed = currentPair.data().second;

        if( currentPair.data().first.cType != ConnectorType::internToExtern)
        {
            std::cout << "ERROR" << std::endl;
        }

        if( currentPair.data().second.cType != ConnectorType::externToIntern)
        {
            std::cout << "ERROR" << std::endl;
        }


        vectorOfSCells.push_back(inToExtSeed.connectors[0]);
        addIntervalSCells(vectorOfSCells,inToExtSeed.secondCirculator,extToIntSeed.firstCirculator);


        ConnectorSeed nextIntToExtSeed = nextPair.data().first;
        vectorOfSCells.push_back(extToIntSeed.connectors[0]);
        addIntervalSCells(vectorOfSCells,extToIntSeed.secondCirculator,nextIntToExtSeed.firstCirculator);
    }

    void createCurve(Curve& curve,
                     CheckableSeedPair* seedPairs,
                     int totalPairs)
    {
        typedef KSpace::SCell SCell;
        std::vector<SCell> scells;

        CheckableSeedPair currentPair;
        CheckableSeedPair nextPair;
        for(int i=0;i<totalPairs;++i)
        {
            if(i==(totalPairs-1))
            {
                currentPair = seedPairs[totalPairs-1];
                nextPair = seedPairs[0];
            }else
            {
                currentPair = seedPairs[i];
                nextPair = seedPairs[i+1];
            }

            addSeedPairSCells(scells,currentPair,nextPair);
        }

        curve.initFromSCellsVector(scells);
    }

    double energyValue(Curve& curve, std::map<KSpace::SCell,double>& weightMap)
    {
        auto it = curve.begin();
        double v=0;
        do
        {
            v+=weightMap[*it];
            ++it;
        }while(it!=curve.end());

        return v;
    }
};

template<>
class CombinationsEvaluator<0>:public CombinationsEvaluator<1>
{
public:
    void operator()(std::vector<CheckableSeedPair> &pairList, KSpace& KImage, int maxSimultaneousPairs)
    {
        return;
    }
};

#endif //SEGBYCUT_COMBINATIONSEVALUATOR_H
