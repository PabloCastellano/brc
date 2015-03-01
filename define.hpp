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
#ifndef BRC_DEFINE_HPP
#define BRC_DEFINE_HPP

//#define CRYPTO_ENABLED
#define ONLY_LOCALHOST_CONNECTIONS

const size_t push_transaction_port = 9109;
const size_t publish_connections_count_port = 9112;
const size_t publish_summary_port = 9110;

// How many connections broadcaster should try to maintain.
const size_t target_connections = 40;

#define LOG_BRC "broadcaster"

#endif

