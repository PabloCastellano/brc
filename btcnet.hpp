/*
 * Copyright (c) 2014-2015 brc developers (see AUTHORS)
 *
 * This file is part of brc.
 *
 * brc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef BRC_BTCNET_HPP
#define BRC_BTCNET_HPP

#include <functional>
#include <bitcoin/bitcoin.hpp>
#include "util.hpp"
#include "define.hpp"

class broadcaster
{
public:
    typedef std::function<void (const std::error_code&)> start_handler;

    broadcaster();

    void start(size_t threads,
        size_t broadcast_hosts, start_handler handle_start);
    void stop();

    bool broadcast(const bc::data_chunk& raw_tx);

    size_t total_connections() const;

private:
    std::ofstream outfile_, errfile_;
    bc::threadpool pool_;
    bc::network::hosts hosts_;
    bc::network::handshake handshake_;
    bc::network::network network_;
    bc::network::protocol broadcast_p2p_;
};

#endif

