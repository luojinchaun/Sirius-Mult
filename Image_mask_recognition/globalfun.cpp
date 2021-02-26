#include "globalfun.h"
#include "windows.h"
#include<QDebug>
GlobalFun::GlobalFun()
{

}

pair<double,double> GlobalFun::getScaleFactor(int width,int height)
{
    return make_pair(floor((width*pow(10,3)+0.5))/pow(10,3)/1440,
                     floor((height*pow(10,3)+0.5))/pow(10,3)/900);//保留三位小数
}

QString GlobalFun::getProperty(const QString &source, QString section, QString key)
{
    QString path = section + "/" + key;
    QSettings settings(source, QSettings::IniFormat);
    return settings.value(path, "").toString();
}

void GlobalFun::setProperty(const QString &source, QString section, QString key, QString value)
{
    QString path = section + "/" + key;
    QSettings settings(source, QSettings::IniFormat);
    settings.setValue(path, value);
}

QJsonObject GlobalFun::getJsonObj(const QString &source)
{
    QFile file(source);

    if( !file.open(QFile::ReadOnly) ) {
        QJsonDocument doc = QJsonDocument::fromJson("", nullptr);
        return doc.object();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data, nullptr);
    return doc.object();
}

QString GlobalFun::getCurrentTime(int type)
{
    QDateTime time(QDateTime::currentDateTime());
    switch (type)
    {
    case 1: return time.toString("yyyy-MM-dd-hh_mm_ss");
    case 2: return time.toString("yyyy-MM-dd_hh:mm:ss.zzz");
    case 3: return time.toString("yyyy-MM-dd");
    case 4: return time.toString("yyyy-MM");
    case 5: return time.toString("hh:mm:ss.zzz");
    case 6: return time.toString("yyyy-MM-dd hh:mm");
    default: return "";
    }
}

QString GlobalFun::getCurrentPath()
{
    return QDir::currentPath();
}

QString GlobalFun::getCurrentThreadID()
{
    QString str = "";
    str.sprintf("%p", QThread::currentThread());
    return str;
}

void GlobalFun::bsleep(int second)
{
    QTime timer;
    timer.start();
    while ( timer.elapsed() < second )
    {
        QCoreApplication::processEvents();
    }
}

bool GlobalFun::equals(qreal a, qreal b)
{
    return abs(a-b) < pow(2, -52);
}

bool GlobalFun::isDirExist(QString &path)
{
    QDir dir(path);

    if( dir.exists() ) {
        return true;
    } else {
        return dir.mkdir(path);  // 只创建一级子目录，即必须保证上级目录存在
    }
}

bool GlobalFun::isFileExist(QString &fileName)
{
    QFileInfo fileInfo(fileName);
    return fileInfo.isFile();
}

bool GlobalFun::removeFolder(const QString &path)
{
    if ( path.isEmpty() ) {
        return false;
    }

    QDir dir(path);
    if( !dir.exists() ) {
        return true;
    }

    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);     // 设置过滤
    QFileInfoList fileList = dir.entryInfoList();               // 获取所有的文件信息
    foreach (QFileInfo file, fileList)                          // 遍历文件信息
    {
        if ( file.isFile() ) {                                  // 是文件，删除
            file.dir().remove(file.fileName());
        } else {                                                // 递归删除
            removeFolder(file.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath());                      // 删除文件夹
}

bool GlobalFun::removeFile(const QString &fileName)
{
    if( QFile::exists(fileName) ) {
        return QFile(fileName).remove();
    } else {
        return true;
    }
}

void GlobalFun::exportCSV(QString path, QTableWidget *widget)
{
    QFile file(path);
    if ( file.open(QFile::WriteOnly | QFile::Truncate) )
    {
        QTextStream data(&file);
        QStringList linelist;
        for (int i = 0; i != widget->columnCount(); ++i) {
            linelist.append(widget->horizontalHeaderItem(i)->text());
        }
        data << linelist.join(",") + "\n";

        for (int i = 0; i != widget->rowCount(); ++i) {
            linelist.clear();
            for (int j = 0; j != widget->columnCount(); ++j) {
                QString str = widget->item(i, j)->text();
                linelist.append(str);
            }
            data << linelist.join(",") + "\n";
        }
        file.close();
    }
}

bool GlobalFun::judVecAvailable(std::vector<cv::Mat> vec)
{
    if ( vec.size() == 0 ) {
        return false;
    }

    for ( auto &temp: vec )
    {
        if ( temp.empty() ) {
            return false;
        }
    }

    return true;
}

void GlobalFun::showMessageBox(int type, QString info)
{
    switch ( type )
    {
    case 1: QMessageBox::question(nullptr, "Question", info, QMessageBox::Ok); break;       // 在正常操作中提出问题
    case 2: QMessageBox::information(nullptr, "Information", info, QMessageBox::Ok); break; // 用于报告有关正常操作的信息
    case 3: QMessageBox::warning(nullptr, "Warning", info, QMessageBox::Ok); break;         // 用于报告非关键错误
    case 4: QMessageBox::critical(nullptr, "Error", info, QMessageBox::Ok); break;          // 用于报告关键错误
    default: break;
    }
}

//------------------------------------------------------------------------------

void GlobalFun::changeColorToRed(QImage& image)
{
    if ( image.format() == QImage::Format_RGB888 ) {
        cv::Mat3b mat = cv::Mat(image.height(), image.width(), CV_8UC3, image.bits());
        std::vector<cv::Mat1b> rgbChannels;
        cv::split(mat, rgbChannels);
        cv::Mat1b red = rgbChannels[0];
        mat.setTo(cv::Vec3b{255, 0, 0}, red >= 254);
    }
}

void GlobalFun::overExposure(QImage& image)
{
//    DWORD start_time = GetTickCount();

    unsigned char *data = image.bits();
    int width = image.width();
    int height = image.height();

    if ( image.format() == QImage::Format_RGB32 )
    {
        for ( int i = 0; i < width; ++i )
        {
            for ( int j = 0; j < height; ++j )
            {
                if ( *data >= 254 && *(data + 1) >= 254 && *(data + 2) >= 254 )
                {
                    *data = 0;
                    *(data + 1) = 0;
                }
                data += 4;
            }
        }
    }
    else if ( image.format() == QImage::Format_RGB888 )
    {
        for ( int i = 0; i < width; ++i )
        {
            for ( int j = 0; j < height; ++j )
            {
                if ( *data >= 254 && *(data + 1) >= 254 && *(data + 2) >= 254 )
                {
                    *(data + 1) = 0;
                    *(data + 2) = 0;
                }
                data += 3;
            }
        }
    }

//    DWORD end_time = GetTickCount();
//    std::cout << "times = " << end_time - start_time << std::endl;
}

std::vector<cv::Mat> GlobalFun::cvtBGR2GRAY(const std::vector<cv::Mat> &vec, cv::Rect rect, bool state)
{
    std::vector<cv::Mat> ret;

    for ( auto &temp : vec )
    {
        if ( temp.empty() ) {
            qDebug() << "Image is empty !";
            ret.push_back(cv::Mat());
        } else {
            cv::Mat gray;
            cvtColor(temp, gray, cv::COLOR_BGR2GRAY);   // 转换为灰度图像
            gray.convertTo(gray, CV_32FC1);             // C1数据类型转换为32F
            if ( state ) {
                ret.push_back(gray(rect));
            } else {
                ret.push_back(gray);
            }
        }
    }

    return ret;
}

cv::Mat GlobalFun::convertQImageToMat(QImage &image)
{
    cv::Mat mat;

    switch( image.format() )
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    default: mat = cv::Mat(); break;
    }

    return mat.clone();
}

QImage GlobalFun::convertMatToQImage(const cv::Mat mat)
{
    if( mat.type() == CV_8UC1 )
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for( int i = 0; i < 256; i++ )
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for( int row = 0; row < mat.rows; row++ )
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }

    else if( mat.type() == CV_8UC3 )
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }

    else if( mat.type() == CV_8UC4 )
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }

    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

//******************************************************************************************************************

QString GlobalString::menu_file = "";
QString GlobalString::menu_camera = "";
QString GlobalString::menu_shifter = "";
QString GlobalString::menu_language = "";

QString GlobalString::action_load = "";
QString GlobalString::action_save = "";
QString GlobalString::action_screenshot = "";
QString GlobalString::action_calibration = "";
QString GlobalString::action_aim = "";
QString GlobalString::action_chinese = "";
QString GlobalString::action_english = "";
QString GlobalString::action_calculate= "";
QString GlobalString::action_analysis= "";
QString GlobalString::action_setting= "";
QString GlobalString::action_Loopcheck= "";
QString GlobalString::action_download= "";
QString GlobalString::action_get= "";
QString GlobalString::action_lock="";

QString GlobalString::graphics_circle = "";
QString GlobalString::graphics_ellipse = "";
QString GlobalString::graphics_concentric_circle = "";
QString GlobalString::graphics_rectangle = "";
QString GlobalString::graphics_square = "";
QString GlobalString::graphics_polygon = "";
QString GlobalString::graphics_pill = "";
QString GlobalString::graphics_chamfer = "";

QString GlobalString::graphics_auto_circle = "";
QString GlobalString::graphics_auto_ellipse = "";
QString GlobalString::graphics_auto_pill = "";
QString GlobalString::graphics_auto_chamfer = "";
QString GlobalString::graphics_auto_roundEdgeRec = "";
QString GlobalString::graphics_auto_rotateRec = "";

QString GlobalString::graphics_lock = "";
QString GlobalString::graphics_unlock = "";
QString GlobalString::graphics_live = "";
QString GlobalString::graphics_showMask = "";
QString GlobalString::graphics_manual = "";
QString GlobalString::graphics_auto = "";
QString GlobalString::graphics_reset = "";
QString GlobalString::graphics_hole_num = "";
QString GlobalString::graphics_sure = "";
QString GlobalString::graphics_change = "";

QString GlobalString::contextMenu_clear = "";
QString GlobalString::contextMenu_delete = "";
QString GlobalString::contextMenu_export = "";
QString GlobalString::contextMenu_show = "";
QString GlobalString::contextMenu_unit = "";
QString GlobalString::contextMenu_sure="";
QString GlobalString::contextMenu_fitting="";
QString GlobalString::contextMenu_residual="";
QString GlobalString::contextMenu_fillSpikes="";

//******************************************************************************************************************

int GlobalValue::cam_tp = 0;
int GlobalValue::cam_itl = 0;
double GlobalValue::cam_exp = 0;
int GlobalValue::cam_exp_dly = 0;
QString GlobalValue::cam_fmt = "";
double GlobalValue::cam_fps = 0;
double GlobalValue::cam_p_cal = 0;

int GlobalValue::com_h_num = 0;
int GlobalValue::com_auto_itl=0;
int GlobalValue::com_tp = 0;
double GlobalValue::com_p_sle = 0;
int GlobalValue::com_s_p = 0;
double GlobalValue::com_value = 0;
QString GlobalValue::com_log_path = "";
QString GlobalValue::com_pic_path = "";

int GlobalValue::atm_t_i = 0;
int GlobalValue::atm_bte = 0;
int GlobalValue::atm_dbt = 0;
int GlobalValue::atm_cbt = 0;
int GlobalValue::atm_sbt = 0;
int GlobalValue::atm_fcl = 0;
QString GlobalValue::atm_ip = "";
QString GlobalValue::atm_port = "";
QString GlobalValue::atm_sts = "";
QString GlobalValue::atm_c_id = "";
QString GlobalValue::atm_all_ret = "";
int GlobalValue::atm_spe = 0;
int GlobalValue::atm_use_spe = 0;

int GlobalValue::lgn_tp = 0;

int GlobalValue::sft_ch_num = 0;
double GlobalValue::sft_s_vol = 0;
double GlobalValue::sft_i_vol = 0;
double GlobalValue::sft_v_cal = 0;
int GlobalValue::sft_s_rest = 0;
int GlobalValue::sft_i_rest = 0;
int GlobalValue::sft_a_s_t = 0;

double GlobalValue::gra_c_rad = 0;
double GlobalValue::gra_e_wid = 0;
double GlobalValue::gra_e_hei = 0;
double GlobalValue::gra_cc_rad_1 = 0;
double GlobalValue::gra_cc_rad_2 = 0;
double GlobalValue::gra_r_wid = 0;
double GlobalValue::gra_r_hei = 0;
double GlobalValue::gra_s_len = 0;
double GlobalValue::gra_p_wid = 0;
double GlobalValue::gra_p_hei = 0;
double GlobalValue::gra_ch_wid = 0;
double GlobalValue::gra_ch_hei = 0;
double GlobalValue::gra_ch_rad = 0;
int GlobalValue::gra_def_size = 0;

int GlobalValue::zer_pis = 0;
int GlobalValue::zer_tilt = 0;
int GlobalValue::zer_pow = 0;
int GlobalValue::zer_ast = 0;
int GlobalValue::zer_coma = 0;
int GlobalValue::zer_sph = 0;

int GlobalValue::ret_pv = 0;
int GlobalValue::ret_pv_x = 0;
int GlobalValue::ret_pv_y = 0;
int GlobalValue::ret_pv_res = 0;
int GlobalValue::ret_rms = 0;
int GlobalValue::ret_rms_res = 0;
int GlobalValue::ret_tilt = 0;
int GlobalValue::ret_power = 0;
int GlobalValue::ret_pwr_x = 0;
int GlobalValue::ret_pwr_y = 0;
int GlobalValue::ret_ast = 0;
int GlobalValue::ret_coma = 0;
int GlobalValue::ret_sph = 0;
int GlobalValue::ret_ttv = 0;
int GlobalValue::ret_fringe = 0;

int GlobalValue::chk_check = 0;
int GlobalValue::chk_cts = 0;
double GlobalValue::chk_p_sh_t1 = 0;
double GlobalValue::chk_p_s_t1 = 0;

int GlobalValue::par_psi = 0;
int GlobalValue::par_unw = 0;
int GlobalValue::com_unit = 0;
int GlobalValue::par_ztm = 0;
double GlobalValue::par_i_s_f = 0;
double GlobalValue::par_t_w = 0;
double GlobalValue::par_i_w = 0;
double GlobalValue::par_hp_fc=0;
int GlobalValue::par_use_fs = 0;

//******************************************************************************************************************
