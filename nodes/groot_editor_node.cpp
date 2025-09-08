#include <QCommandLineParser>
#include <QApplication>
#include <QDialog>
#include <nodes/NodeStyle>
#include <nodes/FlowViewStyle>
#include <nodes/ConnectionStyle>
#include <nodes/DataModelRegistry>

#include <bt_editor/mainwindow.h>
#include <bt_editor/XML_utilities.hpp>
#include <bt_editor/startup_dialog.h>
#include <bt_editor/models/RootNodeModel.hpp>

#include <rclcpp/rclcpp.hpp>

using QtNodes::DataModelRegistry;
using QtNodes::FlowViewStyle;
using QtNodes::NodeStyle;
using QtNodes::ConnectionStyle;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Groot");
    app.setWindowIcon(QPixmap(":/icons/BT.png"));
    app.setOrganizationName("EurecatRobotics");
    app.setOrganizationDomain("eurecat.org");

    qRegisterMetaType<AbsBehaviorTree>();



    QFile styleFile( ":/stylesheet.qss" );
    styleFile.open( QFile::ReadOnly );
    QString style( styleFile.readAll() );
    app.setStyleSheet( style );


    auto mode = GraphicMode::EDITOR;

    // Get the monitor options.
    const QString monitor_address("localhost");
    const QString monitor_pub_port("1666");
    const QString monitor_srv_port("1667");
    const bool monitor_autoconnect = false;

    // Start the main application.
    MainWindow win( mode, monitor_address, monitor_pub_port,
                    monitor_srv_port, monitor_autoconnect );

    QString fileName = "";
    std::cout << "Loading file: " << fileName.toStdString() << std::endl;

    // Open file
    /*QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
       std::cout << "Cannot open file" << std::endl;
       return 1;
    }

    // Read file to xml
    QString xml_text;
    QTextStream in(&file);
    while (!in.atEnd()) {
          xml_text += in.readLine();
    }

    // Show xml
    win.loadFromXML( xml_text );*/

    win.show();
    return app.exec();
}
