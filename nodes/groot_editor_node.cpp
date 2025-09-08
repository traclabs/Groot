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
    rclcpp::init(argc, argv);
    std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("groot_editor_node");
    node->declare_parameter("bt_file", "");
    
    std::string bt_xml_file;
    node->get_parameter("bt_file", bt_xml_file);

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


    // Open file
    if(!bt_xml_file.empty())
    {
    QString fileName(bt_xml_file.c_str());
    //std::cout << "Loading file: " << fileName.toStdString() << std::endl;

      QFile file(fileName);
      if (!file.open(QIODevice::ReadOnly))
      {
         RCLCPP_ERROR(node->get_logger(), "Cannot open file '%s'", bt_xml_file.c_str() );
         return 1;
      }

      // Read file to xml
      QString xml_text;
      QTextStream in(&file);
      while (!in.atEnd()) {
            xml_text += in.readLine();
      }

      // Show xml
      RCLCPP_INFO(node->get_logger(), "Loading file %s", bt_xml_file.c_str());
      win.loadFromXML( xml_text );
    }
    
    win.show();
    return app.exec();
}
