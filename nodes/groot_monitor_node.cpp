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

const auto logger = rclcpp::get_logger("groot");

bool loadFileToString(const std::string &_filename, QString &_xml_text)
{
  if(_filename.empty())
    return false;
    
  QString fileName(_filename.c_str());

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
  {
     RCLCPP_ERROR(logger, "Cannot open file '%s'", _filename.c_str() );
     return 1;
  }

  // Read file to xml
  QString xml_text;
  QTextStream in(&file);
  while (!in.atEnd()) {
    xml_text += in.readLine();
  }
 
  _xml_text = xml_text;
  return true;     
}      

/**
 * @function main
 */
int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("groot_editor_node");

    node->declare_parameter("model_files", rclcpp::PARAMETER_STRING_ARRAY);
    node->declare_parameter("bt_file", rclcpp::PARAMETER_STRING);
        
    std::vector<std::string> model_files;
    std::string bt_xml_file;
    node->get_parameter("model_files", model_files);
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


    auto mode = GraphicMode::MONITOR;

    // Get the monitor options.
    const QString monitor_address("localhost");
    const QString monitor_pub_port("1666");
    const QString monitor_srv_port("1667");
    const bool monitor_autoconnect = false;

    // Start the main application.
    MainWindow win( mode, monitor_address, monitor_pub_port,
                    monitor_srv_port, monitor_autoconnect );

    // Model file
    RCLCPP_INFO(node->get_logger(), "Number of model files received: %lu", model_files.size());
    for(auto mi : model_files)
    {
      RCLCPP_INFO(node->get_logger(), "* Attempting to load Model: %s", mi.c_str());
      QString model_text;
      if( loadFileToString(mi, model_text) )
        win.loadFromXML(model_text);
    }
    
    // Open BT file
    if(!bt_xml_file.empty())
    {
      QString xml_text;
      if(loadFileToString(bt_xml_file, xml_text))
      {
        // Show xml
        RCLCPP_INFO(node->get_logger(), "Loading file %s", bt_xml_file.c_str());
        win.loadFromXML( xml_text );
      }  
    }
    
    win.show();
    return app.exec();
}


