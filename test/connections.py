#!/usr/bin/env python
#
# Copyright (c) 2014-2015 brc developers (see AUTHORS)
#
# This file is part of brc.
#
# brc is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License with
# additional permissions to the one published by the Free Software
# Foundation, either version 3 of the License, or (at your option)
# any later version. For more information see LICENSE.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import zmq
import struct
c = zmq.Context()
s = c.socket(zmq.SUB)
s.connect("tcp://localhost:9112")
s.setsockopt(zmq.SUBSCRIBE, "")
while True:
    print "Connections:", struct.unpack("<Q", s.recv())[0]

