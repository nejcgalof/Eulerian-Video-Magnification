#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    rateLabel = new QLabel; // Frame rate
    inputTip = "Odpri video datoteko"; //sporocilo za informacijo
    ui->videoLabel->setText(inputTip);
    QWidget::showMaximized();
    //Status bar
    rateLabel->setText("");
    ui->statusBar->addPermanentWidget(rateLabel);

    //progress dialog
    progressDialog =  0;

    //magnify dialog
    magnifyDialog = 0;

    updateStatus(false);

    video = new VideoProcessor;

    connect(video, SIGNAL(showFrame(cv::Mat)), this, SLOT(showFrame(cv::Mat)));
    connect(video, SIGNAL(sleep(int)), this, SLOT(sleep(int)));
    connect(video, SIGNAL(revert()), this, SLOT(revert()));
    connect(video, SIGNAL(updateBtn()), this, SLOT(updateBtn()));
    connect(video, SIGNAL(updateProgressBar()), this, SLOT(updateProgressBar()));
    connect(video, SIGNAL(updateProcessProgress(std::string, int)), this, SLOT(updateProcessProgress(std::string, int)));
    connect(video, SIGNAL(closeProgressDialog()), this, SLOT(closeProgressDialog()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


//osvezimo menu za predvajanje
void MainWindow::updateStatus(bool vi)
{
    for (int i = 0; i < ui->menuProcessor->actions().count(); ++i){
        ui->menuProcessor->actions().at(i)->setEnabled(vi);
    }

    ui->actionClose->setEnabled(vi);
    ui->actionSave_as->setEnabled(vi);
    ui->progressSlider->setEnabled(vi);
    ui->btnLast->setEnabled(vi);;
    ui->btnNext->setEnabled(vi);
    ui->btnPlay->setEnabled(vi);
    ui->btnStop->setEnabled(vi);
    ui->btnPause->setEnabled(vi);

    if(!vi){
        ui->progressSlider->setValue(0);
        rateLabel->setText("");
    }
}

//osvezim cas
void MainWindow::updateTimeLabel()
{
    QString curPos = QDateTime::fromMSecsSinceEpoch(video->getPositionMS()).toString("mm:ss");
    QString length = QDateTime::fromMSecsSinceEpoch(video->getLengthMS()).toString("mm:ss");
    ui->timeLabel->setText(tr("%1 / %2").arg(curPos, length));
}


//shraniti pred izhodom
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
        //clean();
    } else {
        event->ignore();
    }
}

//shranimo true, ce je uspesno oz false, ce uporabnik noce
bool MainWindow::maybeSave()
{
    if (video->isModified()){//ce je bil spremenjen
        //naredimo dialog - Qmessage box
        QMessageBox box;
        box.setWindowTitle(tr("Shranjevanje"));
        box.setIcon(QMessageBox::Warning);
        box.setText(tr("Trenutni video %1 je bil spremenjen. Shranimo?").arg(curFile));

        QPushButton *yesBtn = box.addButton(tr("JA"),QMessageBox::YesRole);
        box.addButton(tr("NE"), QMessageBox::NoRole);
        QPushButton *cancelBut = box.addButton(tr("Preklici"),QMessageBox::RejectRole);

        box.exec();
        if (box.clickedButton() == yesBtn){
            return saveAs();
        }
        if (box.clickedButton() == cancelBut){
            return false;
        }
    }
    return true;
}

//predvajanje videa
void MainWindow::play()
{
    video->setDelay(1000 / video->getFrameRate());
    video->playIt(); //predvajamo video
}


//shrani fajl. vrne true, ce je bilo uspesno
bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Shrani kot"),curFile);
    if (fileName.isEmpty()){
        return false;
    }
    return saveFile(fileName);
}


//shrani video na lokacijo, vrne true ce je blo uspesno
bool MainWindow::saveFile(const QString &fileName)
{
    video->setOutput(QFileInfo(fileName).filePath().toStdString());
    //spremenimo kurzor na cakanje
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    //shranimo
    video->writeOutput();

    //povrnemo kurzor
    QApplication::restoreOverrideCursor();

    //nastavimo trenutno pozicijo
    curFile = QFileInfo(fileName).canonicalPath();

    return true;
}



bool MainWindow::LoadFile(const QString &fileName) //nalaganje datoteke v video
{
    //najdem datoteko
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("VideoPlayer"),
                             tr("Napaka v nalaganju datoteke %1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return false;
    }

    // spremenim kurzor na cakanje
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    // dam v VideoProcessor: to je video ta vhod, ce ni uspesno nalozilo, koncamo
    // opencv_ffmpeg.dll rabimo v debug mapi, da lahko vhod nalozimo

    if (!video->setInput(fileName.toStdString())){
        QMessageBox::warning(this, tr("VideoPlayer"),
                             tr("Napaka v nalaganju datoteke v videoCapture %1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return false;
    }

    // povrnem kurzor
    QApplication::restoreOverrideCursor();

    // FPS.ji
    rateLabel->setText(tr("FPS: %1").arg(video->getFrameRate()));

    // osvezim cas
    updateTimeLabel();

    // dobim trenutno pot datoteke -  za kasnejse shranjevanje
    curFile = QFileInfo(fileName).canonicalPath();

    return true; //uspesno nalaganje
}


//prikazemo sliko
void MainWindow::showFrame(cv::Mat frame)
{
    cv::Mat tmp;
    cvtColor(frame, tmp, CV_BGR2RGB);//pretvorim
    QImage img = QImage((const unsigned char*)(tmp.data), tmp.cols, tmp.rows, QImage::Format_RGB888);//dobim sliko
    ui->videoLabel->setPixmap(QPixmap::fromImage(img));//nastavim
    ui->videoLabel->repaint();//prerisem
}

//ponastavimo stanje
void MainWindow::revert()
{
    updateBtn();
}

//delay med frame-i damo na pavzo za delay(msecs)
void MainWindow::sleep(int msecs)
{
    //helper->sleep(msecs);
    QTime dieTime = QTime::currentTime().addMSecs(msecs);
    while(QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

//posodobim gumbe video-predvajalnika
void MainWindow::updateBtn()
{
    bool isStop = video->isStop();
    ui->actionPause->setVisible(!isStop);
    ui->btnPause->setVisible(!isStop);
    ui->actionPlay->setVisible(isStop);
    ui->btnPlay->setVisible(isStop);
}

//osvezimo progress bar
void MainWindow::updateProgressBar()
{
    //slajder
    ui->progressSlider->setValue(video->getNumberOfPlayedFrames() * ui->progressSlider->maximum() / video->getLength() * 1.0);

    //čas
    updateTimeLabel();
}


//pokazatelj napredka v času procesiranja
void MainWindow::updateProcessProgress(const std::string &message, int value)
{
    if(!progressDialog){//ce se ni ustvarjen progressDialog
        progressDialog = new QProgressDialog(this);
        progressDialog->setLabelText(QString::fromStdString(message));
        progressDialog->setRange(0, 100);
        progressDialog->setModal(true);
        progressDialog->setCancelButtonText(tr("Preklici"));
        progressDialog->show();
        progressDialog->raise();
        progressDialog->activateWindow();
    }
    progressDialog->setValue(value + 1);
    qApp->processEvents(); //osvezi procese, da prikaze progressDialog
    if (progressDialog->wasCanceled()){ //ce smo preklicali
        video->stopIt(); //prekinemo
    }
}

//odpri video
void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Odpri video"),".",tr("Formati (*.avi *.mov *.mpeg *.mp4)"));
    if(!fileName.isEmpty()) {
        if(LoadFile(fileName)){
            updateStatus(true);//osvezimo stanje
            updateBtn(); // eneblamo gumbe
        }
    }
}

//Izhod
void MainWindow::on_actionQuit_triggered()
{
    //Close operacija pred zapiranjem
    on_actionClose_triggered();
    qApp->quit();
}

//close operacija
void MainWindow::on_actionClose_triggered()
{
    if (maybeSave()) {
        updateStatus(false);
        ui->videoLabel->setText(inputTip);
        video->close();
        //clean();
        ui->timeLabel->setText("");
        rateLabel->setText("");
    }
}

//Shrani kot triger
void MainWindow::on_actionSave_as_triggered()
{
    saveAs();
}

//stop gumb
void MainWindow::on_btnStop_clicked()
{
    video->stopIt();
}

//play gumb
void MainWindow::on_btnPlay_clicked()
{
    play();
}

//pauza gumb
void MainWindow::on_btnPause_clicked()
{
    video->pauseIt();
}

//gumb naslednji
void MainWindow::on_btnNext_clicked()
{
    video->nextFrame();
}

//gumb prejsni
void MainWindow::on_btnLast_clicked()
{
    video->prevFrame();
}

// slider premikanje z misko
void MainWindow::on_progressSlider_sliderMoved(int position)
{
    long pos = position * video->getLength() / ui->progressSlider->maximum(); //dobimo pozicijo
    video->jumpTo(pos); //skocimo na pravi frame
    updateTimeLabel(); //posodobimo cas
}

//prekličemo progres dialog
void MainWindow::closeProgressDialog()
{
    progressDialog->close(); //zapremo
    progressDialog = 0; //ponastavimo
}

// motion magnification
void MainWindow::on_motion_triggered()
{
    //ce se nismo pokazali dialoga
    if (!magnifyDialog)
        magnifyDialog = new MagnifyDialog(this, video);

    //prikazemo dialog za nastavitve
    magnifyDialog->show();
    magnifyDialog->raise();
    magnifyDialog->activateWindow();

    //ce smo potrdili parametre
    if (magnifyDialog->exec() == QDialog::Accepted) {
        //kurzor na cakanje
        QApplication::setOverrideCursor(Qt::WaitCursor);
        //zazenemo proces eulerian motion magnification
        video->motionMagnify();
        //vrnemo kurzor
        QApplication::restoreOverrideCursor();
    }
}


