#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMath>
#include "fftw/api/fftw3.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void subconstructor();
    void subconstructorFourier();
    void on_plotButton_clicked();
    inline double myfuncVd(const double& var){
        return (sineAmpl*sin(2*PI*sineFreq*var) > myfuncVtri(var)) ? vPeak : -vPeak;
    }
    inline double myfuncVtri(const double& var){
        return (-2*vPeak/period)*(0.5*period - fabs(fmod(var + 0.5*period, 2*period) - period));
    }
    inline double dIl(const double& Vd, const double& Il){
        return (Vd - Il*resistance)/inductance;
    }//vd = il*R + L*dIl
    double rk4(const double& Vd, const double&Il){
        double delta1 = deltat * dIl(Vd, Il);
        double delta2 = deltat * dIl(Vd, Il + 0.5*delta1);
        double delta3 = deltat * dIl(Vd, Il + 0.5*delta2);
        double delta4 = deltat * dIl(Vd, Il + delta3);
        return (delta1 + 2*(delta2 + delta3) + delta4)/6;
    }

    void on_controlRatioSlider_valueChanged(int value);

private:
    Ui::MainWindow *ui;

    const double PI = 3.14159265358979323;

    const double frequency = 3e3;
    const double period = 1.0/frequency;
    double controlRatio = 0.75;
    const double sineFreq = 60;
    double sineAmpl = 7.5;

    const double vPeak = 10;
    const double inductance = 1e-2;
    const double resistance = 1;

    const long double deltat = 1e-6;
    const double timelength = 1.0/sineFreq;
    const int ITER_MAX = static_cast<int>(timelength/deltat);

    QVector<double> time, vd, il, sine, Vtri;

    fftw_complex *out = new fftw_complex[ITER_MAX];
    fftw_plan p;
};

#endif // MAINWINDOW_H
