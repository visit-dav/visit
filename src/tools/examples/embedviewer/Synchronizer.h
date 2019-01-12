#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H
#include <QObject>
#include <Observer.h>
#include <SyncAttributes.h>

class Synchronizer : public QObject, public Observer
{
   Q_OBJECT
public:
   Synchronizer(Subject *s, Subject *ds);
   virtual ~Synchronizer();

   void PostSynchronize();

signals:
   void synchronized();
private:
   virtual void Update(Subject *s);

   int syncCount;
   SyncAttributes *sync, *delayedSync;
};

#endif
