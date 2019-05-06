#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("PWM Inverter");
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->customPlot2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                    QCP::iSelectLegend | QCP::iSelectPlottables);
    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    // make bottom and left axes transfer their ranges to top and right axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    subconstructor();
}

MainWindow::~MainWindow()
{
    delete[] out;
    delete ui;
}

void MainWindow::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->customPlot->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->customPlot->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->customPlot->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->customPlot->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->customPlot->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<ui->customPlot->graphCount(); ++i)
  {
    QCPGraph *graph = ui->customPlot->graph(i);
    QCPPlottableLegendItem *item = ui->customPlot->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
    }
  }
}

void MainWindow::mousePress()
{
  // if an axis is selected, only allow the direction of that axis to be dragged
  // if no axis is selected, both directions may be dragged

  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::mouseWheel()
{
  // if an axis is selected, only allow the direction of that axis to be zoomed
  // if no axis is selected, both directions may be zoomed

  if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
  else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
    ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
  else
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void MainWindow::subconstructor()
{
    time.clear();
    vd.clear();
    sine.clear();
    il.clear();
    Vtri.clear();

    time.push_back(0);
    vd.push_back(vPeak);
    sine.push_back(0);
    il.push_back(0);
    Vtri.push_back(0);

    for(int i = 0; i < ITER_MAX; i++){
        double now = (i+1)*deltat;
        time.push_back(now);
        vd.push_back(myfuncVd(now));
        sine.push_back(sineAmpl*sin(2*PI*sineFreq*now));
        il.push_back(il[i] + rk4(vd[i], il[i]));
        Vtri.push_back(myfuncVtri(now));
    }

    ui->customPlot->clearGraphs();
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(time, vd);
    ui->customPlot->graph(0)->setName("Vd");
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setData(time, sine);
    ui->customPlot->graph(1)->setName("Sine");
    ui->customPlot->graph(1)->setPen(QPen(QColor(Qt::red)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(2)->setData(time, il);
    ui->customPlot->graph(2)->setName("Iload");
    ui->customPlot->graph(2)->setPen(QPen(QColor(Qt::black)));
    ui->customPlot->addGraph();
    ui->customPlot->graph(3)->setData(time, Vtri);
    ui->customPlot->graph(3)->setName("Vtri");
    ui->customPlot->graph(3)->setPen(QPen(QColor(Qt::green)));
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
    subconstructorFourier();
}

void MainWindow::subconstructorFourier()
{
    ui->customPlot2->clearGraphs();
    ui->customPlot2->addGraph();
    ui->customPlot2->graph(0)->setName("Harmonics");
    ui->customPlot2->graph(0)->setPen(QPen(QColor(Qt::black)));
    ui->customPlot2->graph(0)->setLineStyle(QCPGraph::lsImpulse);
    p = fftw_plan_dft_r2c_1d(ITER_MAX, vd.data(), out, FFTW_ESTIMATE);
    fftw_execute(p);
    for(int i = 0; i < ITER_MAX; i++){
        ui->customPlot2->graph(0)->addData(i, sqrt(pow(out[i][0], 2) + pow(out[i][1], 2))/ITER_MAX);
    }
    ui->customPlot2->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot2->xAxis2->setScaleType(QCPAxis::stLogarithmic);
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    ui->customPlot2->xAxis->setTicker(logTicker);
    ui->customPlot2->xAxis2->setTicker(logTicker);
    ui->customPlot2->xAxis->setNumberFormat("eb");
    ui->customPlot2->xAxis->setNumberPrecision(0);
    ui->customPlot2->legend->setVisible(true);
    ui->customPlot2->rescaleAxes();
    ui->customPlot2->xAxis->setRangeLower(1e-1);
    ui->customPlot2->replot();
    fftw_destroy_plan(p);
    fftw_cleanup();
}

void MainWindow::on_plotButton_clicked()
{
    subconstructor();
}


void MainWindow::on_controlRatioSlider_valueChanged(int value)
{
    controlRatio = value/100.0;
    sineAmpl = controlRatio * vPeak;
    ui->controlRatioLabel->setText("Vsine/Vd = " + QString::number(value) + "%");
}
