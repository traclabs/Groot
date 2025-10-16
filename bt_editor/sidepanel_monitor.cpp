#include "sidepanel_monitor.h"
#include "ui_sidepanel_monitor.h"
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QLabel>
#include <QDebug>

#include "mainwindow.h"
#include "utils.h"
#include "zmq_addon.hpp"
#include "behaviortree_cpp/loggers/groot2_protocol.h"

SidepanelMonitor::SidepanelMonitor(QWidget *parent,
                                   const QString &address,
                                   const QString &publisher_port,
                                   const QString &server_port) :
    QFrame(parent),
    ui(new Ui::SidepanelMonitor),
    _zmq_context(1),
    _zmq_subscriber(_zmq_context, ZMQ_SUB),
    _connected(false),
    _msg_count(0),
    _parent(parent)
{
    ui->setupUi(this);
    this->set_load_tree_timeout_ms(_load_tree_default_timeout_ms);

    if ( !address.isEmpty() )
    {
        ui->lineEdit_address->setText(address);
    }
    if ( !publisher_port.isEmpty() )
    {
        ui->lineEdit_publisher->setText(publisher_port);
    }
    if ( !server_port.isEmpty() )
    {
        ui->lineEdit_server->setText(server_port);
    }

    _timer = new QTimer(this);
    connect( _timer, &QTimer::timeout, this, &SidepanelMonitor::on_timer );
}

SidepanelMonitor::~SidepanelMonitor()
{
    delete ui;
}

void SidepanelMonitor::clear()
{
    if( _connected ) this->on_Connect();
}

void SidepanelMonitor::on_timer()
{
    if( !_connected ) return;

    try {
        zmq::socket_t zmq_client(_zmq_context, ZMQ_REQ);
        zmq_client.connect(_connection_address_req.c_str());

        // Build request header (RequestType::STATUS)
        BT::Monitor::RequestHeader request(BT::Monitor::RequestType::STATUS);
        std::string header_str = BT::Monitor::SerializeHeader(request);
        zmq::message_t req_msg(header_str.data(), header_str.size());

        // Send request
        zmq_client.send(req_msg, zmq::send_flags::none);

        // Receive reply (multipart: header + status buffer)
        zmq::multipart_t multipart_msg;
        if (!multipart_msg.recv(zmq_client)) return;
        if (multipart_msg.size() < 2) return;

        const std::string header_reply = multipart_msg[0].to_string();
        const std::string status_str = multipart_msg[1].to_string();

        std::vector<std::pair<int, NodeStatus>> node_status;
        for (size_t offset = 0; offset + 3 <= status_str.size(); offset += 3)
        {
            uint16_t uid;
            memcpy(&uid, &status_str[offset], sizeof(uint16_t));
            uint8_t raw_status = static_cast<uint8_t>(status_str[offset + 2]);
            NodeStatus status = static_cast<NodeStatus>(raw_status);

            auto it = _uid_to_index.find(uid);
            if (it == _uid_to_index.end()) continue;

            int index = it->second;
            _loaded_tree.node(index)->status = status;
            node_status.push_back( {index, status} );
        }

        // update the graphic part
        emit changeNodeStyle( "BehaviorTree", node_status );

        // lock editing of nodes
        auto main_win = dynamic_cast<MainWindow*>( _parent );
        main_win->lockEditing(true);
    }
    catch( zmq::error_t& err)
    {
        qDebug() << "ZMQ receive failed: " << err.what();
    }
}

bool SidepanelMonitor::getTreeFromServer()
{
    try{
        zmq::multipart_t request;
        zmq::multipart_t reply;

        zmq::socket_t  zmq_client( _zmq_context, ZMQ_REQ );
        zmq_client.connect( _connection_address_req.c_str() );

        zmq_client.set(zmq::sockopt::rcvtimeo, _load_tree_timeout_ms);

        // Create FULLTREE request header
        BT::Monitor::RequestHeader req(BT::Monitor::RequestType::FULLTREE);
        request.addstr(BT::Monitor::SerializeHeader(req));
        request.send(zmq_client);

        // Receive reply
        reply.recv(zmq_client);
        if (reply.size() < 2) {
            return false;
        }

        // Parse XML into QDomDocument
        QDomDocument doc;
        QString xml_str = QString::fromStdString(reply[1].to_string());
        if (!doc.setContent(xml_str)) {
            return false;
        }

        QDomElement document_root = doc.documentElement();

        auto res_pair = BuildTreeFromGroot2Protocol(BuiltinNodeModels(), document_root);

        _loaded_tree  = std::move( res_pair.first );
        _uid_to_index = std::move( res_pair.second );

        // add new models to registry
        for(const auto& tree_node: _loaded_tree.nodes())
        {
            const auto& registration_ID = tree_node.model.registration_ID;
            if( BuiltinNodeModels().count(registration_ID) == 0)
            {
                addNewModel( tree_node.model );
            }
        }

        try {
            loadBehaviorTree( _loaded_tree, "BehaviorTree" );
        }
        catch (std::exception& err) {
            QMessageBox messageBox;
            messageBox.critical(this,"Error Connecting to remote server", err.what() );
            messageBox.show();
            return false;
        }

        std::vector<std::pair<int, NodeStatus>> node_status;
        node_status.reserve(_loaded_tree.nodesCount());

        //  qDebug() << "--------";

        for(size_t t=0; t < _loaded_tree.nodesCount(); t++)
        {
            node_status.push_back( { t, _loaded_tree.nodes()[t].status } );
        }
        emit changeNodeStyle( "BehaviorTree", node_status );
    }
    catch( zmq::error_t& err)
    {
        qDebug() << "ZMQ client receive failed: " << err.what();
        return false;
    }
    return true;
}

void SidepanelMonitor::on_Connect()
{
    if( !_connected )
    {
        QString address = ui->lineEdit_address->text();
        if( address.isEmpty() )
        {
            address = ui->lineEdit_address->placeholderText();
            ui->lineEdit_address->setText(address);
        }

        QString publisher_port = ui->lineEdit_publisher->text();
        if( publisher_port.isEmpty() )
        {
            publisher_port = ui->lineEdit_publisher->placeholderText();
            ui->lineEdit_publisher->setText(publisher_port);
        }

        QString server_port = ui->lineEdit_server->text();
        if( server_port.isEmpty() )
        {
          publisher_port = ui->lineEdit_server->placeholderText();
          ui->lineEdit_server->setText(publisher_port);
        }

        bool failed = false;
        if( !address.isEmpty() )
        {
            _connection_address_pub = "tcp://" + address.toStdString() + ":" + publisher_port.toStdString();
            _connection_address_req = "tcp://" + address.toStdString() + ":" + server_port.toStdString();

            try{
                _zmq_subscriber.connect( _connection_address_pub.c_str() );

                int timeout_ms = 1;
                _zmq_subscriber.set(zmq::sockopt::subscribe, "");
                _zmq_subscriber.set(zmq::sockopt::rcvtimeo, timeout_ms);

                if( !getTreeFromServer() )
                {
                    failed = true;
                    _connected = false;
                }
                // After we try get a tree on connect, reset to the default timeout.
                // This is done so that we only use the increased autoconnect timeout once.
                this->set_load_tree_timeout_ms(_load_tree_default_timeout_ms);
            }
            catch(zmq::error_t& err)
            {
                failed = true;
            }
        }
        else {
            failed = true;
        }

        if( !failed )
        {
            _connected = true;
            ui->lineEdit_address->setDisabled(true);
            ui->lineEdit_publisher->setDisabled(true);
            _timer->start(_timer_period_ms);
            connectionUpdate(true);
        }
        else{
            QMessageBox::warning(this,
                                 tr("ZeroMQ connection"),
                                 tr("Was not able to connect to [%1]\n").arg(_connection_address_pub.c_str()),
                                 QMessageBox::Close);
        }
    }
    else{
        _connected = false;
        ui->lineEdit_address->setDisabled(false);
        ui->lineEdit_publisher->setDisabled(false);
        _timer->stop();

        connectionUpdate(false);
    }
}
