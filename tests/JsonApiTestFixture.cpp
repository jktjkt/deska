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

#include <boost/test/test_tools.hpp>
#include <boost/bind.hpp>

#include "JsonApiTestFixture.h"
#include "deska/db/JsonApi.h"

JsonApiTestFixture::JsonApiTestFixture()
{
    j = new Deska::Db::JsonApiParser();
    j->writeString.connect(boost::bind(&JsonApiTestFixture::slotWrite, this, _1));
    j->readString.connect(boost::bind(&JsonApiTestFixture::slotRead, this));
}

JsonApiTestFixture::~JsonApiTestFixture()
{
    delete j;
}

std::string JsonApiTestFixture::slotRead()
{
    std::string res = jsonDbOutput;
    jsonDbOutput.clear();
    return res;
}

void JsonApiTestFixture::slotWrite(const std::string &jsonDataToWrite)
{
    BOOST_CHECK_EQUAL(jsonDataToWrite, jsonDbInput);
    jsonDbInput.clear();
}
