#ifndef SEGBYCUT_CONNECTORSEED_H
#define SEGBYCUT_CONNECTORSEED_H

enum ConnectorType{
    internToExtern,
    externToIntern,
    makeConvex
};

template<typename T1,
         typename T2,
        typename T3>
struct ConnectorSeed{
    typedef T1 ConnectorElementType;
    typedef T2 SCellCirculatorType;
    typedef T3 KSpace;


    typedef typename SCellCirculatorType::value_type SCell;
    typedef DGtal::Dimension Dimension;


    typedef typename std::vector<ConnectorElementType>::const_iterator SCellIteratorType;


    ConnectorSeed(){}

    ConnectorSeed(ConnectorElementType cet,
                  SCellCirculatorType& fit,
                  SCellCirculatorType& sit,
                  ConnectorType ct): firstCirculator(fit),
                                     secondCirculator(sit),
                                     cType(ct)
    {
        connectors.push_back(cet);
    };

    ConnectorSeed(SCellIteratorType citB,
                  SCellIteratorType citE,
                  SCellCirculatorType fit,
                  SCellCirculatorType sit): firstCirculator(fit),
                                             secondCirculator(sit),
                                             cType(makeConvex)
    {
        for(auto it = citB;it!=citE;++it){
            connectors.push_back(*it);
        }

    }

    std::vector<ConnectorElementType> connectors;
    bool isValid;
    KSpace KImage;
    ConnectorType cType;

    SCellCirculatorType firstCirculator;
    SCellCirculatorType secondCirculator;
};

template< typename X >
struct ConnectorSeedConcept {
    typedef typename X::ConnectorElementType ConnectorElementType;
    typedef typename X::SCellCirculatorType SCellCirculatorType;
};

#endif //SEGBYCUT_CONNECTORSEED_H
