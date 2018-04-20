#include "faceview.h"
#include "ui_faceview.h"


FaceView::FaceView(QWidget *parent) : QDialog(parent), ui(new Ui::FaceView)
{
    ui->setupUi(this);

    face = new Naie::Face();

    LBP_Para_Radius =1;
    LBP_Para_Neighbors=8;
    LBP_Para_GridX=8;
    LBP_Para_GridY=8;

    InitStatus();
}

FaceView::~FaceView()
{
    delete ui;
}

void FaceView::InitStatus()
{
    //参数设置
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), ui->spinBox, SLOT(setValue(int)));
    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), ui->spinBox_2, SLOT(setValue(int)));
    connect(ui->horizontalSlider_3, SIGNAL(valueChanged(int)), ui->spinBox_3, SLOT(setValue(int)));
    connect(ui->horizontalSlider_4, SIGNAL(valueChanged(int)), ui->spinBox_4, SLOT(setValue(int)));

    connect(ui->spinBox, SIGNAL(valueChanged(int)), ui->horizontalSlider, SLOT(setValue(int)));
    connect(ui->spinBox_2, SIGNAL(valueChanged(int)), ui->horizontalSlider_2, SLOT(setValue(int)));
    connect(ui->spinBox_3, SIGNAL(valueChanged(int)), ui->horizontalSlider_3, SLOT(setValue(int)));
    connect(ui->spinBox_4, SIGNAL(valueChanged(int)), ui->horizontalSlider_4, SLOT(setValue(int)));

    connect(ui->horizontalSlider, QOverload<int>::of(&QSlider::valueChanged), [=](int item)
    {
        LBP_Para_Radius = item;
    });
    connect(ui->horizontalSlider_2, QOverload<int>::of(&QSlider::valueChanged), [=](int item)
    {
        LBP_Para_Neighbors = item;
    });
    connect(ui->horizontalSlider_3, QOverload<int>::of(&QSlider::valueChanged), [=](int item)
    {
        LBP_Para_GridX = item;
    });
    connect(ui->horizontalSlider_4, QOverload<int>::of(&QSlider::valueChanged), [=](int item)
    {
        LBP_Para_GridY = item;
    });

    connect(ui->spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int item)
    {
        LBP_Para_Radius = item;
    });
    connect(ui->spinBox_2, QOverload<int>::of(&QSpinBox::valueChanged), [=](int item)
    {
        LBP_Para_Neighbors = item;
    });
    connect(ui->spinBox_3, QOverload<int>::of(&QSpinBox::valueChanged), [=](int item)
    {
        LBP_Para_GridX = item;
    });
    connect(ui->spinBox_4, QOverload<int>::of(&QSpinBox::valueChanged), [=](int item)
    {
        LBP_Para_GridY = item;
    });

    //算法选择
    LBP_Algorithm = ui->comboBox->currentIndex();
    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int item)
    {
        LBP_Algorithm = item;
    });

    //
    connect(ui->logtextEdit,SIGNAL(textChanged()), this, SLOT(onAutoScroll()));
}

//执行人脸检测算法
void FaceView::on_startButton_clicked()
{
    //clear log
    ui->logtextEdit->clear();
    face->setPara(LBP_Para_Radius, LBP_Para_Neighbors, LBP_Para_GridX, LBP_Para_GridY);
    face->setAlgorithm(LBP_Algorithm);
    face->doing();
    double rate = face->getRecognitionRate();

    ui->rateLabel->setText(QVariant(rate).toString());

    //logs
    vector<string> predictLogs = face->getLog();
    std::for_each(predictLogs.begin(), predictLogs.end(), [&](string &item){
        QString strLog = QString::fromStdString(item);
        ui->logtextEdit->append(strLog);
    });

}

void FaceView::onAutoScroll()
{
    QTextCursor cursor = ui->logtextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->logtextEdit->setTextCursor(cursor);
}
