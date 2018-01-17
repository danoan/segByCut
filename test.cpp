
#include "typeDefs.h"
#include "utils.h"
#include "Artist.h"

using namespace DGtal;
using namespace DGtal::Z2i;

using namespace SegCut;

/*Tests if the generated GluedCurves are connected*/
void testConnectdeness(std::string imgFilePath)
{
    KSpace KImage;
    setKImage(imgFilePath,KImage);

    Curve intCurve,extCurve;
    setCurves(imgFilePath,intCurve,extCurve);

    ConnectorSeedRangeType seedRange = getSeedRange(KImage,intCurve,extCurve);
    unsigned int gluedCurveLength = 10;
    SeedToGluedCurveRangeFunctor stgcF(gluedCurveLength);
    GluedCurveSetRange gcsRange( seedRange.begin(),
                                         seedRange.end(),
                                         stgcF);


    DGtal::functors::SCellToIncidentPoints<KSpace> myFunctor(KImage);
    int gluedCurveNumber =0;
    for(auto itgc=gcsRange.begin();itgc!=gcsRange.end();++itgc ){
        std::cout << "Glued Curve Start " << gluedCurveNumber << std::endl;

        SCellGluedCurveIterator begin = itgc->first;
        SCellGluedCurveIterator end = itgc->second;

        GluedCurveIncidentPointsRange gcipRange(begin,
                                                end,
                                                myFunctor );

        GridCurve<KSpace> gc;
        gc.initFromSCellsRange(begin,end);
        ++gluedCurveNumber;
    }

}

/*Evaluates curvature for each generated GluedCurve*/
void testCurvatureEvaluation(std::string imgFilePath)
{
    KSpace KImage;
    setKImage(imgFilePath,KImage);

    Curve intCurvePriorGS,extCurvePriorGS;
    setCurves(imgFilePath,intCurvePriorGS,extCurvePriorGS);

    Curve intCurve,extCurve;
    Patch::initializeCurveCurvatureEstimator(KImage,intCurvePriorGS,intCurve);
    Patch::initializeCurveCurvatureEstimator(KImage,extCurvePriorGS,extCurve);

    ConnectorSeedRangeType seedRange = getSeedRange(KImage,intCurve,extCurve);
    unsigned int gluedCurveLength = 10;
    SeedToGluedCurveRangeFunctor stgcF(gluedCurveLength);
    GluedCurveSetRange gcsRange( seedRange.begin(),
                                 seedRange.end(),
                                 stgcF);


    DGtal::functors::SCellToIncidentPoints<KSpace> myFunctor(KImage);
    int gluedCurveNumber=0;
    for(auto itgc=gcsRange.begin();itgc!=gcsRange.end();++itgc ){
        std::cout << "Glued Curve Start " << gluedCurveNumber << std::endl;

        SCellGluedCurveIterator begin = itgc->first;
        SCellGluedCurveIterator end = itgc->second;

        GluedCurveIncidentPointsRange gcipRange(begin,
                                                end,
                                                myFunctor);

        std::vector<double> estimations;
        if(Patch::useDGtal){
            Patch::estimationsDGtalMCMSECurvature(gcipRange.begin(),
                                                  gcipRange.end(),
                                                  estimations);
        }else{
            Patch::estimationsPatchMCMSECurvature(gcipRange.begin(),
                                                  gcipRange.end(),
                                                  estimations);
        }



        {
            auto it = gcipRange.begin();
            int i =0;
            do {
                std::cout << estimations[i] << std::endl;
                ++it;
                ++i;
            } while (it != gcipRange.end());
        }

        ++gluedCurveNumber;
    }
}


void runTests(const std::string& imgPath,
              const std::string& outputFolder)
{
    testConnectdeness(imgPath);
    testCurvatureEvaluation(imgPath);



    KSpace KImage;
    setKImage(imgPath,KImage);

    Board2D board;
    Artist EA(KImage,board);

    EA.setOptional(true);

    Curve intCurvePriorGS,extCurvePriorGS;
    setCurves(imgPath,intCurvePriorGS,extCurvePriorGS);

    Curve intCurve,extCurve;
    Patch::initializeCurveCurvatureEstimator(KImage,intCurvePriorGS,intCurve);
    Patch::initializeCurveCurvatureEstimator(KImage,extCurvePriorGS,extCurve);

    /*All Glued Curves*/
    EA.board.clear(DGtal::Color::White);
    EA.drawAllGluedCurves(imgPath,outputFolder+"/gluedCurves");


    /*Complete Curvature Map*/
    EA.board.clear(DGtal::Color::White);
    std::string outputFilePath = outputFolder + "/completeCurvatureMap.eps";
    EA.drawCurvesAndConnectionsCurvatureMap(imgPath,outputFilePath);


    /*Curvature Maps*/
    EA.board.clear(DGtal::Color::White);
    outputFilePath = outputFolder + "/curvatureMap.eps";
    {
        int i=2;
        double cmax=-100;
        double cmin=100;
        do{
            EA.drawCurvatureMap(intCurve,cmin,cmax);
            EA.drawCurvatureMap(extCurve,cmin,cmax);
            --i;
        }while(i>0);
    }
    EA.board.saveEPS(outputFilePath.c_str());



    /*Tangent Maps*/
    EA.board.clear(DGtal::Color::White);
    outputFilePath = outputFolder + "/tangentMap.eps";
    {
        int i=2;
        double cmax=-100;
        double cmin=100;
        do{
            EA.drawTangentMap(intCurve,cmin,cmax);
            EA.drawTangentMap(extCurve,cmin,cmax);
            --i;
        }while(i>0);
    }
    EA.board.saveEPS(outputFilePath.c_str());

}

void testSequence()
{
    std::string outputRootPath = "../output/tests";
    if(Patch::useDGtal)
        outputRootPath += "/no-Patch";
    else
        outputRootPath += "/Patch";

    runTests("../images/flow-evolution/last_image.pgm",
             outputRootPath + "/last_image");

//    runTests("../images/graph-weight-test/single_square.pgm",
//             outputRootPath + "/square");

//    runTests("../images/graph-weight-test/single_triangle.pgm",
//             outputRootPath + "/triangle");
//
//    runTests("../images/graph-weight-test/smallest_disk.pgm",
//             outputRootPath + "/disk");
}

namespace Patch{
    bool useDGtal;
};

namespace UtilsTypes
{
    std::function< double(double) > toDouble = [](double x){return x;};
};

int main()
{
    Patch::useDGtal = true;
    testSequence();
    Patch::useDGtal = false;
    testSequence();

}

