#ifndef MAGNIFYDIALOG_H
#define MAGNIFYDIALOG_H

#include <QDialog>
#include <VideoProcessor.h>

namespace Ui {
class MagnifyDialog;
}

class MagnifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MagnifyDialog(QWidget *parent = 0,
                         VideoProcessor *processor = 0);
    ~MagnifyDialog();

private slots:
    void on_alphaSlider_valueChanged(int value);

    void on_lambdaSlider_valueChanged(int value);

    void on_flSlider_valueChanged(int value);

    void on_fhSlider_valueChanged(int value);

    void on_chromSlider_valueChanged(int value);

private:
    Ui::MagnifyDialog *ui;
    VideoProcessor *processor;
    QString alphaStr, lambdaStr, flStr, fhStr, chromStr;
};

#endif // MAGNIFYDIALOG_H
