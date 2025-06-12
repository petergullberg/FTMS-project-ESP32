void writeCharacteristic_hex(const char *cmd_str, const char *info, bool disable_log,
                                           bool wait_for_response) ;

#if 0 
class solef80treadmill : public treadmill {

  public:
    solef80treadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    double minStepInclination() override;

  private:
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const char *info, bool disable_log = false,
                             bool wait_for_response = false);
    void waitForAPacket();
    void startDiscover();
    void btinit();

    QTimer *refresh;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyService *gattFTMSService = nullptr;
    QLowEnergyCharacteristic gattWriteCharCustomService;
    QLowEnergyService *gattCustomService = nullptr;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = 0; //QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    double lastSpeed = 0.0;
    double lastInclination = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    enum _REQUEST_STATE { IDLE = 0, UP = 1, DOWN = 2 };
    _REQUEST_STATE requestSpeedState = IDLE;
    _REQUEST_STATE requestInclinationState = IDLE;

    typedef enum TYPE {
        F80 = 0,
        F63 = 1,
        TRX7_5 = 2,
    } TYPE;
    volatile TYPE treadmill_type = F80;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const uint8_t &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const uint8_t &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const uint8_t &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const uint8_t &newValue);
    void descriptorRead(const QLowEnergyDescriptor &descriptor, const uint8_t &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChangedint state);

    void changeInclinationRequested(double grade, double percentage);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};
#endif