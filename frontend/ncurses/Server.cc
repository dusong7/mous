#include "Server.h"
#include "Config.h"
#include "Session.h"
#include "ServerContext.h"

#include <unistd.h>
#include <sys/select.h>

#include <string>
#include <algorithm>
#include <iostream>
using namespace std;

Server::Server()
{
    m_Data = new ServerContext;
    pipe(m_PipeFd);

    log.open("mous.log", ios::out);
}

Server::~Server()
{
    delete m_Data;
    close(m_PipeFd[0]);
    close(m_PipeFd[1]);
    m_Socket.Close();

    log.close();
}

int Server::Exec()
{
    if (!m_Data->Init())
        return 1;

    const Config* config = GlobalConfig::Instance();
    log << config << endl;
    if (config == NULL)
        return 2;

    log << config->pluginDir << endl;
    log << config->serverIp << endl;
    log << config->serverPort << endl;

    SocketOpt opt;
    opt.reuseAddr = true;
    opt.reusePort = true;
    opt.keepAlive = true;
    m_Socket.SetOption(opt);

    if (!m_Socket.Bind(config->serverIp, config->serverPort))
        return 1;
    if (!m_Socket.Listen())
        return 1;

    int maxfd = std::max(m_Socket.Fd(), m_PipeFd[0]) + 1;
    fd_set rset;
    FD_ZERO(&rset);

    TcpSocket clientSocket;
    for (bool stopService = false; !stopService; ) {
        FD_SET(m_Socket.Fd(), &rset);
        FD_SET(m_PipeFd[0], &rset);
        if (select(maxfd, &rset, NULL, NULL, NULL) <= 0)
            break;

        // open session
        if (FD_ISSET(m_Socket.Fd(), &rset)) {
            if (!m_Socket.Accept(clientSocket))
                break;
            OpenSession(clientSocket);
        }

        // close session / stop service
        if (FD_ISSET(m_PipeFd[0], &rset)) {
            char cmd;
            read(m_PipeFd[0], &cmd, 1);
            switch (cmd) {
                case 'q':
                {
                    Session::ptr_t ptr = 0;
                    read(m_PipeFd[0], &ptr, sizeof(ptr));
                    CloseSession(reinterpret_cast<Session*>(ptr));
                }
                    break;

                case 'Q':
                {
                    StopService();
                    stopService = true;
                }
                    break;

                default:
                    break;
            }
        }
    }

    m_Data->Cleanup();

    return 0;
}

void Server::StopService()
{
    m_Socket.Shutdown();

    SessionSetIter iter = m_SessionSet.begin();
    SessionSetIter end = m_SessionSet.end();
    for (; iter != end; ++iter) {
        (*iter)->Stop();
        delete *iter;
    }
    m_SessionSet.clear();

    cout << "StopService()" << endl;
}

void Server::OpenSession(TcpSocket& clientSocket)
{
    Session* session = new Session(m_Data);
    m_SessionSet.insert(session);
    session->Run(clientSocket, m_PipeFd[1]);

    cout << "OpenSession()" << endl;
}

void Server::CloseSession(Session* session)
{
    SessionSetIter iter = m_SessionSet.find(session);
    if (iter != m_SessionSet.end()) {
        m_SessionSet.erase(iter);
        session->Stop();
        delete session;
    }

    cout << "CloseSession()" << endl;
}