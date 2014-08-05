#include "gifs2png_gui.h"
#include "ui_gifs2png_gui.h"
#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>

gifs2png_gui::gifs2png_gui(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::gifs2png_gui)
{
    ui->setupUi(this);
}

gifs2png_gui::~gifs2png_gui()
{
    delete ui;
}

void gifs2png_gui::on_BrowseInput_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Source Directory"),
                                                 QApplication::applicationDirPath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()) return;

    ui->inputDir->setText(dir);

}

void gifs2png_gui::on_BrowseOutput_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Target Directory"),
                                                 QApplication::applicationDirPath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()) return;

    ui->outputDir->setText(dir);
}

void gifs2png_gui::on_startTool_clicked()
{
    if(ui->inputDir->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Source directory is not set"), tr("Please, set the source directory"), QMessageBox::Ok);
        return;
    }

    QString command;

    #ifdef _WIN32
    command = QApplication::applicationDirPath()+"/GIFs2PNG.exe";
    #else
    command = QApplication::applicationDirPath()+"/GIFs2PNG";
    #endif

    if(!QFile(command).exists())
    {
        QMessageBox::warning(this, tr("Tool is not found"), tr("Can't run application: \n%1\nPlease, check the application directory.").arg(command), QMessageBox::Ok);
        return;
    }

    QStringList args;
    args << ui->inputDir->text();
    if(!ui->outputDir->text().isEmpty()) args << QString("-O%1").arg(ui->outputDir->text());

    if(ui->WalkSubDirs->isChecked()) args << "-W";
    if(ui->RemoveSource->isChecked()) args << "-R";

    QProcess::startDetached(command, args);
}

void gifs2png_gui::on_close_clicked()
{
    this->close();
}
