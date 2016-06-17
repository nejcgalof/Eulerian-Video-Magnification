#include "VideoProcessor.h"

VideoProcessor::VideoProcessor(QObject *parent)
  : QObject(parent)
  , delay(-1)
  , rate(0)
  , fnumber(0)
  , length(0)
  , stop(true)
  , modify(false)
  , curPos(0)
  , curIndex(0)
  , curLevel(0)
  , digits(0)
  , extension(".avi")
  , levels(4)
  , alpha(10)
  , lambda_c(80)
  , fl(0.05)
  , fh(0.4)
  , chromAttenuation(0.1)
  , delta(0)
  , exaggeration_factor(2.0)
  , lambda(0)
  , fileName("")
{
    connect(this, SIGNAL(revert()), this, SLOT(revertVideo()));
}
//nastavimo imefajla
void VideoProcessor::setFileName(QString n)
{
    fileName = n;
}

//nastavimo delay med frame-i
void VideoProcessor::setDelay(int d)
{
    delay = d;
}

//vrne stevilo procesiranih frameov
long VideoProcessor::getNumberOfProcessedFrames()
{
    return fnumber;
}

//vrne stevilo igranih frameov oziroma trenutno pozicijo
long VideoProcessor::getNumberOfPlayedFrames()
{
    return curPos;
}

//dobimo velikost od frame-a (visina,sirina)
cv::Size VideoProcessor::getFrameSize()
{
    int w = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    return cv::Size(w,h);
}

//dobimo index od naslednega frame-a
long VideoProcessor::getFrameNumber()
{
    long f = static_cast<long>(capture.get(CV_CAP_PROP_POS_FRAMES));
    return f;
}

//vrni pozicijo v MS
double VideoProcessor::getPositionMS()
{
    double t = capture.get(CV_CAP_PROP_POS_MSEC); //trenutna pozicija v videu
    return t;
}

//vrnemo frame rate - FPS.je
double VideoProcessor::getFrameRate()
{
    double r = capture.get(CV_CAP_PROP_FPS);
    return r;
}

//dobi stevilo frame-ov v videu
long VideoProcessor::getLength()
{
    return length;
}

//vrne dolzino videa v MS
double VideoProcessor::getLengthMS()
{
    double l = 1000.0 * length / rate;
    return l;
}

//spatial filtr. izberem - imam sliko in piramido
bool VideoProcessor::spatialFilter(const cv::Mat &src, std::vector<cv::Mat> &pyramid)
{
    return buildLaplacianPyramid(src, levels, pyramid);
}

//temporal filtriranje na sliki
void VideoProcessor::temporalFilter(const cv::Mat &src,cv::Mat &dst)
{
    temporalIIRFilter(src, dst);
    return;
}


//temporal IIR filter na sliko
//src = pyramid slika; dst= filtrirana slika
void VideoProcessor::temporalIIRFilter(const cv::Mat &src,cv::Mat &dst)
{
    //fh=0.4 hl=0.05
    //poracunamo
    cv::Mat temp1 = (1-fh)*lowpass1[curLevel] + fh*src; //low pass filter z high cutoff
    cv::Mat temp2 = (1-fl)*lowpass2[curLevel] + fl*src; //low pass filter z low cutoff
    //shranimo vmesne lowpasse
    lowpass1[curLevel] = temp1;
    lowpass2[curLevel] = temp2;
    //vrnemo rezultat kot razliko
    dst = lowpass1[curLevel] - lowpass2[curLevel];
}


//okrepitev gibanja
void VideoProcessor::amplify(const cv::Mat &src, cv::Mat &dst)
{
    float currAlpha;
    //alpha predefiniran na 10
    //izracunati spremenjen alfa za ta level piramide
    currAlpha = lambda/delta/8 - 1;
    currAlpha *= exaggeration_factor; //za boljso vizualizacijo
    if (curLevel==levels || curLevel==0){ //ignoriramo za najvisjo in najnizjo plast
        dst = src * 0;
    }
    else{
        dst = src * cv::min(alpha, currAlpha);
    }
}

//zmanjsamo I in Q kanale
void VideoProcessor::attenuate(cv::Mat &src, cv::Mat &dst)
{
    cv::Mat planes[3];
    cv::split(src, planes);//damo na 3 kanale
    //druga 2 kanala pomnozimo z chroma
    planes[1] = planes[1] * chromAttenuation;
    planes[2] = planes[2] * chromAttenuation;
    cv::merge(planes, 3, dst); //zdruzimo skupaj
}


//dobi kodek od videa
int VideoProcessor::getCodec(char codec[])
{
    union {
        int value;
        char code[4];
    } returned;

    returned.value = static_cast<int>(capture.get(CV_CAP_PROP_FOURCC));

    codec[0] = returned.code[0];
    codec[1] = returned.code[1];
    codec[2] = returned.code[2];
    codec[3] = returned.code[3];

    return returned.value;
}

//ime fajla kot vhod, VideoCapture (capture)odpre posnetek, ce je uspesno: TRUE, ce ne FALSE
bool VideoProcessor::setInput(const std::string &fileName)
{
    fnumber = 0;
    tempFile = fileName;

    //ce ze ima nekaj, spraznimo
    if (isOpened()){
        capture.release();
    }

    //odpremo video
    if(capture.open(fileName)){
        // read parameters
        length = capture.get(CV_CAP_PROP_FRAME_COUNT); //dobim stevilo frame-ov v posnetku
        rate = getFrameRate(); //CV_CAP_PROP_FPS znotraj, dobim frame rate
        cv::Mat input;
        // pokazem prvi frame, kot sliko videa
        getNextFrame(input); //najdem frame
        showFrame(input); //ga pokazem;
        updateBtn(); //osvezim gumbe- eneblam jih
        return true;
    } else {
        return false; //napaka
    }
}

//shranimo video - true ce je uspesno
bool VideoProcessor::setOutput(const std::string &filename, int codec, double framerate, bool isColor)
{
    outputFile = filename;
    extension.clear();

    if (framerate==0.0){
        framerate = getFrameRate(); //nastavimo isti frame rate
    }

    char c[4];
    //uporabimo isti kodek
    if (codec==0) {
        codec = getCodec(c);
    }

    //odpremo izhodno atoteko
    return writer.open(outputFile,codec, framerate,getFrameSize(),isColor);
}

//naredimo zacasni file - temp file; nastavimo codec, framerate, in barvo. True, če je uspešno
bool VideoProcessor::createTemp(double framerate, bool isColor)
{
    //ustvarimo enolicno ime - enolicno poskrbimo s poimenovanjem z timeom
    std::stringstream ss;
    ss << "temp_" << QDateTime::currentDateTime().toTime_t() << ".avi";
    tempFile = ss.str();
    //dodamo v listo temp fajlov
    tempFileList.push_back(tempFile);

    //ce se ni framerate.a
    if (framerate==0.0){
        framerate = getFrameRate(); //enak vhodnemu videu
    }

    //odpremo izhodno datoteko z videowriterju
    return tempWriter.open(tempFile, // ime datoteke
                       CV_FOURCC('M', 'J', 'P', 'G'), //kodek motion-JPEG kodek
                       framerate,      //FPS
                       getFrameSize(), //velikost frame.a
                       isColor); //barva videa - ja ali ne?
}

//prenega predvajati ali procesirati
void VideoProcessor::stopIt()
{
    stop = true;
    revert(); //ponastavi gumbe
}

//prejsni frame
void VideoProcessor::prevFrame()
{
    if(isStop()){
        pauseIt();
    }
    //ce smo ze na zacetku
    if (curPos >= 0){
        curPos -= 1;
        jumpTo(curPos);
    }
    updateProgressBar();
}

//naslednji frame
void VideoProcessor::nextFrame()
{
    if(isStop()){
        pauseIt();
    }
    curPos += 1;
    //ce smo ze na koncu
    if (curPos <= length){
        curPos += 1;
        jumpTo(curPos);
    }
    updateProgressBar();
}

//skocimo na pozicijo; true, ce je uspesno
bool VideoProcessor::jumpTo(long index)
{
    //ce pademo ven ->prevelik index
    if (index >= length){
        return 1;
    }

    cv::Mat frame;
    bool re = capture.set(CV_CAP_PROP_POS_FRAMES, index);//dobimo index - ce je znotraj je true

    //pokazemo frame od indexa
    if (re && !isStop()){
        capture.read(frame); //preberem naslednji frame
        showFrame(frame); //pokazemo frame
    }

    return re;
}

//zapremo video
void VideoProcessor::close()
{
    rate = 0;
    length = 0;
    modify = 0;
    capture.release();
    //writer.release();
    //tempWriter.release();
}


//true, ce ne spila/procesira
bool VideoProcessor::isStop()
{
    return stop;
}

//ce je video spremenjen
bool VideoProcessor::isModified()
{
    return modify;
}

//ce je capture odprt
bool VideoProcessor::isOpened()
{
    return capture.isOpened();
}

//najde naslednji frame - je true, ce najde frame
bool VideoProcessor::getNextFrame(cv::Mat &frame)
{
    return capture.read(frame);
}

//zapisemo naslednji frame
void VideoProcessor::writeNextFrame(cv::Mat &frame)
{
    if (extension.length()) { //zapisemo sliko
        std::stringstream ss;
        ss << outputFile << std::setfill('0') << std::setw(digits) << curIndex++ << extension;
        cv::imwrite(ss.str(),frame);
    }
    else { //ali pa video
        writer.write(frame);
    }
}

//predvajamo frame-e v zaporedju
void VideoProcessor::playIt()
{
    //mamo frame
    cv::Mat input;

    //ce ni capture odprt, ne nardimo nic
    if (!isOpened())
        return;

    //zdaj predvajamo
    stop = false;

    //updatamo gumbe
    updateBtn();

    //ce se predvaja capture
    while (!isStop()){
        //preberemo naslednji frame (true -> ce obstaja frame
        if (!getNextFrame(input)){//ce ni frame-a ne predvajamo vec
            break;
        }

        curPos = capture.get(CV_CAP_PROP_POS_FRAMES);//dobimo index frame-a ki more bit captured oziroma decoded

        //pokazemo frame
        showFrame(input);

        //posodobimo progressbar
        updateProgressBar();

        //delay
        sleep(delay);//poskrbimo za delay
    }
    if (!isStop()){
        revert();//ponastavimo - poskrbimo za gumbe
    }
}

//pauza
void VideoProcessor::pauseIt()
{
    stop = true;
    updateBtn();
}

// motionMagnify	-	eulerian motion magnification
void VideoProcessor::motionMagnify()
{
    //naredimo zacasno (temp) datoteko
    createTemp();

    //trenutni-vhodni frame
    cv::Mat input;
    //izhodni frame
    cv::Mat output;

    //motion slike
    cv::Mat motion;

    std::vector<cv::Mat> pyramid;
    std::vector<cv::Mat> filtered;

    //ce ni odprto
    if (!isOpened()){
        return;
    }

    //obdelujemo(spreminjamo)
    modify = true;

    //ker procesiramo-ni na stop
    stop = false;

    //shranimo trenutno pozicijo
    long pos = curPos;

    //skocimo na prvi frame
    jumpTo(0);

    //dokler ni konec
    while (!isStop()){
        //preberem naslednji frame
        if (!getNextFrame(input)){
            break; //ce pridemo do konca, prenehamo
        }
        //poskrbimo za pretvorbo v CV_32FC3(od 0.0-1.0 in mamo skalirni faktor zarad tega 1/255.0 namesto 1
        input.convertTo(input, CV_32FC3, 1.0/255.0f);

        // 1. pretvorimo v Lab color space (Lightness in a,b pravokotne osi 3d prostora
        // primerna za odbito in prepusceno svetlobo. L(0-100)
        //mamo 2 osi: od zelena-rdeca in modre-rumene
        //barvo transformira tako da bolje odslikavajo človeško zaznavanje barve
        //deluje na principu-če sta si 2 barvi podobni, želimo da sta si blizu tut v barvnem prostoru
        cv::cvtColor(input, input, CV_BGR2Lab);
         //cv::cvtColor(input, input, CV_BGR2XYZ);

        // 2. spatial filtering (prostorsko filtriranje) enega frame-a, recimo LAPLACE
        //uporabimo za izbolsanje komaj opaznih singalov. (princip piramid)
        cv::Mat s = input.clone();
        spatialFilter(s, pyramid);

        // 3. temporal filtering (one frame's pyramid)časovno filtriranje) enega frame-a piramide
        // za poudarjanje gibanja
        //
        if (fnumber == 0){      //ce je prvi frame
            lowpass1 = pyramid; //low pass filter for IIR
            lowpass2 = pyramid; //low pass filter for IIR
            filtered = pyramid;
        }
        else { //ce ni prvi
            for (int i=0; i<levels; ++i) { //za vsako plast piramide
                curLevel = i;
                //izvedemo temporal filtriranje na sliki in shranimo v filtered
                temporalFilter(pyramid.at(i), filtered.at(i));
            }

            cv::Size filterSize = filtered.at(0).size();
            int w = filterSize.width;
            int h = filterSize.height;

            //povečanje vsakega prostorskega(spatial) frekvenčnega pasa
            //v knjigi: sprememba alfa po prostorski frekvenci
            delta = lambda_c/8.0/(1.0+alpha);

            // faktor ki okrepi alfa nad mejo, za boljso vizualizacijo
            exaggeration_factor = 2.0;

            //izračun reprezentativne valovne dolžine lambda, za najnižjo plast laplacove piramide
            lambda = sqrt(w*w + h*h)/3;

            //interaktivno skozi plasti in aplifyat (krepiti gibanja)
            for (int i=levels; i>=0; i--){
                curLevel = i;
                //krepitev gibanja:
                amplify(filtered.at(i), filtered.at(i));

                //gremo en level dol
                //lambdo pomanjsamo za faktor 2 (polovicno zmanjsamo)
                lambda /= 2.0;
            }
        }

        // 4. rekonstrukcija gibanja iz laplacove piramide - shranimo v motion
        reconImgFromLaplacianPyramid(filtered, levels, motion);

        // 5. zmanjsamo 2 kanala slike z chromAttenuation.(2ga 2, Light ne)
        attenuate(motion, motion);

        // 6. zdruzimo originalni frame in motion frame
        if (fnumber > 0){ //ojačamo ne prvega frame-a
            s += motion;
        }

        // 7. pretvorimo nazaj v RGB in CV_8UC3
        output = s.clone();
        //cv::cvtColor(output, output, CV_XYZ2BGR);
        cv::cvtColor(output, output, CV_Lab2BGR);
        output.convertTo(output, CV_8UC3, 255.0, 1.0/255.0);

        //zapisemo frame v zacasno temp datoteko
        tempWriter.write(output);

        //osvezimo napredek
        std::string msg= "Procesiranje...";
        updateProcessProgress(msg, floor((fnumber++) * 100.0 / length));
    }
    if (!isStop()){
        revert(); //povrnemo stanje
    }
    closeProgressDialog();

    // release the temp writer
    //tempWriter.release();

    //spremenimo video v temp video - tega prikazujemo
    setInput(tempFile);

    //skocimo na pozicijo
    jumpTo(pos);
}

//damo rezultat
void VideoProcessor::writeOutput()
{
    cv::Mat input;

    //ce ni ne video odprt ne v writerju(da smo sprocesirali)
    if (!isOpened() || !writer.isOpened()){
        return;
    }

    //shranimo pozicijo
    long pos = curPos;
    
    //skocimo na zacetek
    jumpTo(0);

    //dokler ni konec
    while (getNextFrame(input)) {
        //zapisemo naslednji frame
        if (outputFile.length()!=0)
            writeNextFrame(input);
    }

    //modify damo na false
    modify = false;

    // release the writer
    //writer.release();

    //skocimo kjer smo bli
    jumpTo(pos);
}

//ponastavim video/stanje
void VideoProcessor::revertVideo()
{
    // pause the video
    jumpTo(0);    
    curPos = 0;
    pauseIt();
    updateProgressBar();
}
