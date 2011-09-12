/* Copyright (C) 2011 Jan Kundrát <kundratj@fzu.cz>
*
* This file is part of the Deska, a tool for central administration of a grid site
* http://projects.flaska.net/projects/show/deska
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or the version 3 of the License.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
* */

#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <cstdlib>
#include "Connection_p.h"
#include "ProcessIO.h"
#include "UnixFdIO.h"
#include "deska/cli/CliConfig.h"

namespace Deska {
namespace Db {

#define DESKA_VIA_FD_R "DESKA_VIA_FD_R"
#define DESKA_VIA_FD_W "DESKA_VIA_FD_W"

Connection_p::Connection_p(): io(0)
{
    char *envUseFd = ::getenv(DESKA_VIA_FD_R);
    if (envUseFd) {
        std::string fdRead = ::getenv(DESKA_VIA_FD_R);
        char *tmp = ::getenv(DESKA_VIA_FD_W);
        if (!tmp) {
            throw std::runtime_error("Deska::Db::Connection_p: environment: missing " DESKA_VIA_FD_W);
        }
        std::string fdWrite(tmp);

        int rfd = boost::lexical_cast<int>(fdRead);
        int wfd = boost::lexical_cast<int>(fdWrite);
        if (rfd < 0) {
            throw std::runtime_error("Deska::Db::Connection_p: environment: illegal " DESKA_VIA_FD_R);
        }
        if (wfd < 0) {
            throw std::runtime_error("Deska::Db::Connection_p: environment: illegal " DESKA_VIA_FD_W);
        }

        io = new UnixFdIO(rfd, wfd);
    } else {
        // FIXME: don't hardcode these
        std::vector<std::string> args;
        //std::string test = Cli::CliConfig::getInstance()->getVar<std::string>("DBConnection.Server");

        args.push_back(std::string(CMAKE_CURRENT_SOURCE_DIR) + "/src/deska/server/app/deska_server.py");

        // FIXME: switch to boost::program_options, see redmine #179
        char *deska_user = ::getenv("DESKA_USER");
        if (deska_user) {
            args.push_back("-U");
            args.push_back(deska_user);
        }
        char *deska_db = ::getenv("DESKA_DB");
        if (deska_db) {
            args.push_back("-d");
            args.push_back(deska_db);
        }

        if (char *deska_cfggen_backend = ::getenv("DESKA_CFGGEN_BACKEND")) {
            args.push_back("--cfggen-backend");
            args.push_back(deska_cfggen_backend);
        }
        if (char *deska_cfggen_git_repo = ::getenv("DESKA_CFGGEN_GIT_REPO")) {
            args.push_back("--cfggen-git-repository");
            args.push_back(deska_cfggen_git_repo);
        }
        if (char *deska_cfggen_git_wc = ::getenv("DESKA_CFGGEN_GIT_WC")) {
            args.push_back("--cfggen-git-workdir");
            args.push_back(deska_cfggen_git_wc);
        }
        if (char *deska_cfggen_git_scripts = ::getenv("DESKA_CFGGEN_SCRIPTS")) {
            args.push_back("--cfggen-script-path");
            args.push_back(deska_cfggen_git_scripts);
        }

        io = new ProcessIO(args);
    }
    willRead.connect(boost::phoenix::bind(&IOSocket::readStream, *io));
    willWrite.connect(boost::phoenix::bind(&IOSocket::writeStream, *io));
    wantJustReadData.connect(boost::phoenix::bind(&IOSocket::recentlyReadData, *io));
}

Connection_p::~Connection_p()
{
    delete io;
}

}
}
