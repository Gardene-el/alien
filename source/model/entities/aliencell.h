#ifndef ALIENCELL_H
#define ALIENCELL_H

#include <QVector3D>

class AlienCellCluster;
class AlienEnergy;
class AlienGrid;
class AlienToken;

class AlienCell
{
public:

    virtual ~AlienCell() {}

    virtual ProcessingResult process (AlienToken* token, AlienCell* previousCell) = 0;
    struct ProcessingResult {
        bool decompose;
        AlienEnergy* newEnergyParticle;
    };

    virtual bool connectable (AlienCell* otherCell) = 0;
    virtual bool isConnectedTo (AlienCell* otherCell) = 0;
    virtual void resetConnections (int maxConnections) = 0;
    virtual void newConnection (AlienCell* otherCell) = 0;
    virtual void delConnection (AlienCell* otherCell) = 0;
    virtual void delAllConnection () = 0;
    virtual int getNumConnections () = 0;
    virtual int getMaxConnections () = 0;
    virtual void setMaxConnections (int maxConnections) = 0;
    virtual AlienCell* getConnection (int i) = 0;
    virtual QVector3D calcNormal (QVector3D outerSpace, QMatrix4x4& transform) = 0;

    virtual void activatingNewTokens () = 0;
    virtual const quint64& getId () = 0;
    virtual void setId (quint64 id) = 0;
    virtual const quint64& getTag () = 0;
    virtual void setTag (quint64 tag) = 0;
    virtual int getNumToken (bool newTokenStackPointer = false) = 0;
    virtual AlienToken* getToken (int i) = 0;
    virtual void addToken (AlienToken* token, bool activateNow = true, bool updateAccessNumber = true) = 0;
    virtual void delAllTokens () = 0;

    virtual void setCluster (AlienCellCluster* cluster) = 0;
    virtual AlienCellCluster* getCluster () = 0;
    virtual QVector3D calcPosition (bool topologyCorrection = false) = 0;
    virtual void setAbsPosition (QVector3D pos) = 0;
    virtual void setAbsPositionAndUpdateMap (QVector3D pos) = 0;
    virtual QVector3D getRelPos () = 0;
    virtual void setRelPos (QVector3D relPos) = 0;

    virtual int getTokenAccessNumber () = 0;
    virtual void setTokenAccessNumber (int i) = 0;
    virtual bool blockToken () = 0;
    virtual void setBlockToken (bool block) = 0;
    virtual qreal getEnergy() = 0;
    virtual qreal getEnergyIncludingTokens() = 0;
    virtual void setEnergy (qreal i) = 0;
    virtual QVector< quint8 >& getMemory () = 0;

    virtual void serialize (QDataStream& stream) = 0;

    virtual QVector3D getVel () = 0;
    virtual void setVel (QVector3D vel) = 0;
    virtual quint8 getColor () = 0;
    virtual void setColor (quint8 color) = 0;
};

#endif // ALIENCELL_H
