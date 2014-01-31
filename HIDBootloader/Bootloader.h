#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "DeviceData.h"
#include "Device.h"
#include "ImportExportHex.h"
#include "Comm.h"

/*!
 * Provides HID bootloader communication.
 */
class Bootloader : public QObject {
    Q_OBJECT



signals:
   void writeLog(QString value);
   void setBootloadBusy(bool value);

protected:

    bool deviceFirmwareIsAtLeast101 = false;

    Comm::ExtendedQueryInfo extendedBootInfo;

    void IoWithDeviceStart(QString msg);
    void IoWithDeviceComplet(QString msg, Comm::ErrorCode result, double time);


    QString fileName, watchFileName;

    //void IoWithDeviceStarted(QString msg);
    //void IoWithDeviceCompleted(QString msg, Comm::ErrorCode, double time);

public:    

    Comm* comm;
    DeviceData* deviceData;
    DeviceData* hexData;
    Device* device;


    bool writeFlash;
    bool writeEeprom;
    bool writeConfig;
    bool eraseDuringWrite;
    bool hexOpen;

    bool wasBootloaderMode;

    void GetQuery(void);
    void EraseDevice(void);
    void BlankCheckDevice(void);
    void WriteDevice(void);
    void VerifyDevice(void);

    void LoadFile(QString fileName);

    explicit Bootloader();
    ~Bootloader();

};

#endif // BOOTLOADER_H

