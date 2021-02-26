#include "biccameracontrol.h"
#include "globalfun.h"

BICCameraControl::BICCameraControl(Camera::CameraType type) : Camera(type)
{
    try {
        DShowLib::InitLibrary();
        atexit(DShowLib::ExitLibrary);
    } catch (DShowLib::Error &e) {
        qDebug() << "InitLibrary error: " + QString::fromStdString(e.toString());
    }

    m_timer.setInterval(GlobalValue::cam_itl);
    connect(&m_timer, &QTimer::timeout, this, &BICCameraControl::updateFrame);
}

void BICCameraControl::initCamera()
{
    try {
        m_Sink = DShowLib::FrameHandlerSink::create( FrameTypeInfoArray::createRGBArray(), NUM_BUFFERS );
        m_Sink->setSnapMode( true );

        // Apply the sink to the grabber.
        m_Grabber.setSinkType( m_Sink );
    } catch (DShowLib::Error &e) {
        qDebug() << "initCamera error: " + QString::fromStdString(e.toString());
    }
}

void BICCameraControl::destroyCamera()
{
    try {
        stopWork();
        closeCamera();
    } catch (DShowLib::Error &e) {
        qDebug() << "destroyCamera error: " + QString::fromStdString(e.toString());
    }
}

void BICCameraControl::openCamera()
{
    // Check if a device is open.
    if ( m_Grabber.isDevOpen() ) {
        return;
    }

    if (m_Grabber.getAvailableVideoCaptureDevices()->size() == 0) {
        return;
    }

    try {
        Grabber::tVidCapDevListPtr pVidCapDevList = m_Grabber.getAvailableVideoCaptureDevices();
        if( pVidCapDevList == 0 || pVidCapDevList->empty() )
        {
            qDebug() << "No device available !";
            return;
        }

        // Open camera from serial number. The device does not work when using USB connection in this way.
//        __int64 num;
//        pVidCapDevList->at(0).getSerialNumber(num);
//        m_Grabber.openDev(num);

        // Open camera from device name.
        std::string deviceName = pVidCapDevList->at(0).getName();
        m_Grabber.openDev(deviceName);

        m_Grabber.setVideoFormat(dstringa(GlobalValue::cam_fmt.toLatin1().data()));
        m_Grabber.setFPS(GlobalValue::cam_fps);

    } catch (DShowLib::Error &e) {
        qDebug() << "openCamera error: " + QString::fromStdString(e.toString());
    }
}

void BICCameraControl::closeCamera()
{
    // Check if a device is open.
    if ( !m_Grabber.isDevOpen() ) {
        return;
    }

    try {
        // Closes the currently active video capture device.
        m_Grabber.closeDev();
    } catch (DShowLib::Error &e) {
        qDebug() << "closeCamera error: " + QString::fromStdString(e.toString());
    }
}

void BICCameraControl::startWork()
{
    // Check if live mode is on.
    if ( m_Grabber.isLive() ) {
        return;
    }

    // Check if there is a valid device.
    if ( !m_Grabber.isDevValid() ) {
        return;
    }

    try {
        // Start the live video. Set true to enable the live video window, false to grab only.
        m_Grabber.startLive(false);
        m_timer.start();
    } catch (DShowLib::Error &e) {
        qDebug() << "startWork error: " + QString::fromStdString(e.toString());
    }
}

void BICCameraControl::stopWork()
{
    // Check if live mode is on.
    if ( !m_Grabber.isLive() ) {
        return;
    }

    try {
        // Stop live mode.
        m_Grabber.stopLive();
        m_timer.stop();
    } catch (DShowLib::Error &e) {
        qDebug() << "stopWork error: " + QString::fromStdString(e.toString());
    }
}

cv::Mat BICCameraControl::takeAPic()
{
    // Check if live mode is on.
    if ( !m_Grabber.isLive() ) {
        return cv::Mat();
    }

    QMutexLocker locker(&m_mutex);

    try {
        m_Sink->snapImages( 1, 1000 );
        smart_ptr<MemBuffer> pBuffer = m_Sink->getLastAcqMemBuffer();

        QImage image(static_cast< uchar* >( pBuffer->getPtr() ),
                     int( pBuffer->getSize().cx ),
                     int( pBuffer->getSize().cy ),
                     QImage::Format_RGB32);

        QImage retImage = image.mirrored(false, true);
        cv::Mat mat = GlobalFun::convertQImageToMat(retImage);
        return mat.clone();
    } catch (DShowLib::Error &e) {
        qDebug() << "takeAPic error: " + QString::fromStdString(e.toString());
        return cv::Mat();
    }
}

void BICCameraControl::saveConfigToFile(QString path)
{
    dstringa filepath = dstringa(path.toLatin1().data());
    m_Grabber.saveDeviceStateToFile(filepath);
}

void BICCameraControl::loadConfileFromFile(QString path)
{
    /* check if live mode is on */
    if(!m_Grabber.isLive()){
        return;
    }
    m_Grabber.stopLive();
    m_timer.stop();

    if ( GlobalFun::isFileExist(path) ) {
        dstringa filepath = dstringa(path.toLatin1().data());
        m_Grabber.loadDeviceStateFromFile(filepath, true);
    } else {
        // 首次加载先保存相机属性到本地
        saveConfigToFile(path);
    }

    m_Grabber.startLive(false);
    m_timer.start();
}

void BICCameraControl::showDialog()
{
    if ( m_Grabber.isDevOpen() && m_Grabber.isLive() ) {
        m_Grabber.stopLive();
        m_timer.stop();

        std::thread th([=](){ m_Grabber.showDevicePage(); });
        th.join();

        GlobalValue::cam_fps = m_Grabber.getFPS();
        GlobalValue::cam_fmt = QString::fromStdString(m_Grabber.getVideoFormat().toString());
        qDebug() << "FPS: " + QString::number(GlobalValue::cam_fps);
        qDebug() << "Format: " + GlobalValue::cam_fmt;

        m_Grabber.startLive(false);
        m_timer.start();

        m_Grabber.showVCDPropertyPage();

        // 更新相机属性
        saveConfigToFile("device.xml");

        GlobalValue::cam_exp = getExposure() * 1000;
        qDebug() << "Exposure: " + QString::number(GlobalValue::cam_exp);
    }
}

double BICCameraControl::getExposure() {
    tIVCDPropertyItemsPtr pItems = m_Grabber.getAvailableVCDProperties();
    if( pItems != 0 )
    {
        tIVCDPropertyItemPtr pExposureItem = pItems->findItem( VCDID_Exposure );
        tIVCDPropertyElementPtr pExposureValueElement = pExposureItem->findElement( VCDElement_Value );
        tIVCDAbsoluteValuePropertyPtr m_pExposureAbsoluteValue;

        if( pExposureValueElement != 0 )
        {
            pExposureValueElement->getInterfacePtr( m_pExposureAbsoluteValue );
        }

        if( m_pExposureAbsoluteValue != 0 )
        {
            return m_pExposureAbsoluteValue->getValue();
        }
    }

    return 0.033;
}

void BICCameraControl::updateFrame()
{
    QMutexLocker locker(&m_mutex);

    try {
        m_Sink->snapImages( 1, 1000 );
        smart_ptr<MemBuffer> pBuffer = m_Sink->getLastAcqMemBuffer();

        QImage image(static_cast< uchar* >( pBuffer->getPtr() ),
                     int( pBuffer->getSize().cx ),
                     int( pBuffer->getSize().cy ),
                     QImage::Format_RGB32);

        emit updateImage(image.mirrored(false, true));
    } catch (DShowLib::Error &e) {
        qDebug() << "updateFrame error: " + QString::fromStdString(e.toString());
    }
}
